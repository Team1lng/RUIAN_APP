#include "network_common.h"
#include "../../layout/layout_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
// #include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include "ak_thread.h"
#include "ak_mem.h"
#include <net/if_arp.h>
#include <netinet/in.h>
// #include<netinet/ip.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "queue.h"
#include "leo_api.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "../audio/audio_input.h"

#define AUDIO_QUEUE_PACKAGE_MAX 64 // 5

#define AUDIO_PACKAGE_SIZE_MAX 1510 // 10*1024//1510

#define AUDIO_FRAME_MAX (2 * 1024)

static int network_audio_send_eth_id = 0;
static char network_audio_group_ip[128];
static const char audio_start_code[4] = {0x00, 0x00, 0x01, 0xfc};

struct sockaddr_ll *network_get_send_addres(void);
const char *nework_get_package_head(unsigned int type);
bool video_record_data_push(char type, unsigned char *data, int len, bool is_video);
bool audio_decode_queue_push(unsigned char *data, int len);

typedef struct
{
	char frame_type; // 0:pcm
	char *data;
	int len;
	unsigned long long pts;
	unsigned long long frame_index;
} network_audio_data;

typedef struct
{
	void *prev;
	void *next;
	network_audio_data package;

} network_audio_package;

static queue_s network_audio_send_queue_free;
static queue_s network_audio_send_queue_head;

static ak_mutex_t network_audio_send_head_mutex;
static ak_mutex_t network_audio_send_free_mutex;

static network_audio_package network_audio_package_buffer[AUDIO_QUEUE_PACKAGE_MAX];

static void network_audio_send_package_queue_init(void)
{
	static bool is_first = true; // 防止重复初始化
	if (is_first == false)
	{
		return;
	}
	is_first = false;

	ak_thread_mutex_init(&network_audio_send_head_mutex, NULL); // 线程互斥锁初始化
	ak_thread_mutex_init(&network_audio_send_free_mutex, NULL); // 线程互斥锁初始化

	queue_initialize(&network_audio_send_queue_head); // 队列初始化
	queue_initialize(&network_audio_send_queue_free); // 队列初始化

	memset(network_audio_package_buffer, 0, sizeof(network_audio_package) * AUDIO_QUEUE_PACKAGE_MAX);
	for (int i = 0; i < AUDIO_QUEUE_PACKAGE_MAX; i++)
	{
		queue_insert((queue_s *)&network_audio_package_buffer[i], &network_audio_send_queue_free); // 全空队列
	}
}

static network_audio_package *network_audio_send_package_queue_new(char type, const char *data, int size)
{
	network_audio_package *node = NULL;
	ak_thread_mutex_lock(&network_audio_send_free_mutex);
	if (queue_empty(&network_audio_send_queue_free) == 0)
	{
		node = (network_audio_package *)queue_delete_next(&network_audio_send_queue_free);
	}
	ak_thread_mutex_unlock(&network_audio_send_free_mutex);
	if (node == NULL)
	{
		return NULL;
	}

	if (node->package.data != NULL)
	{
		ak_mem_free(node->package.data);
	}
	node->package.data = (char *)ak_mem_alloc(MODULE_ID_APP, size);
	memcpy(node->package.data, data, size);
	static unsigned long long frame_index = 0;
	node->package.frame_index = frame_index++;
	node->package.len = size;
	node->package.pts = get_sys_ms();
	node->package.frame_type = 0;
	// printf("send index :%llu \n",node->package.frame_index);
	return node;
}

static void network_audio_send_package_queue_del(network_audio_package *node)
{
	if (node != NULL)
	{
		if (node->package.data != NULL)
		{
			ak_mem_free(node->package.data);
			node->package.data = NULL;
		}
		ak_thread_mutex_lock(&network_audio_send_free_mutex);
		queue_insert((queue_s *)node, &network_audio_send_queue_free);
		ak_thread_mutex_unlock(&network_audio_send_free_mutex);
	}
}

static void network_audio_send_release_all(void)
{
	ak_thread_mutex_lock(&network_audio_send_head_mutex);
	while (!queue_empty(&network_audio_send_queue_head))
	{
		network_audio_package *node = (network_audio_package *)queue_delete_next(&network_audio_send_queue_head);
		network_audio_send_package_queue_del(node);
	}
	ak_thread_mutex_unlock(&network_audio_send_head_mutex);
}

static bool network_audio_send_thread_run = false;
static bool networK_audio_send_task_run = false;
static bool network_audio_send_ready = false;
static int audio_package_send_fd = -1;
static bool network_audio_send_socket_open(void)
{
	if (audio_package_send_fd != -1)
	{
		return false;
	}

	printf("==========>>> audio send socket %s <<<==========\n", network_audio_group_ip);
	if ((audio_package_send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

	struct in_addr localInterface;
	localInterface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(audio_package_send_fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
		perror("Setting local interface error");
		exit(1);
	}
	else
	{
		printf("Setting the local interface...OK\n");
	}

	bool bOptval = true;
	int ret = setsockopt(audio_package_send_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&bOptval, sizeof(bOptval));
	if (ret != 0)
	{
		perror("AUDIO_SEND_SOCKET REUSE failed\n");
	}

	int loop = 0;
	// 设置数据是否发送到本地回环接口
	if (setsockopt(audio_package_send_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
	{
		perror("IP_MULTICAST_LOOP");
		return -1;
	}

	struct ifreq ifr;
	memset(&ifr, 0x00, sizeof(ifr));
	strncpy(ifr.ifr_name, NETWORK_NAME, strlen(NETWORK_NAME));
	setsockopt(audio_package_send_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
	return true;
}

static bool network_audio_send_socket_close(void)
{
	if (audio_package_send_fd == -1)
	{
		return false;
	}

	close(audio_package_send_fd);
	audio_package_send_fd = -1;

	return true;
}

static void network_send_package_audio(network_audio_package *node)
{
	network_audio_data *package = &(node->package);
	int send_size = 0, remain_size = package->len;
	char *buffer = (char *)ak_mem_alloc(MODULE_ID_APP, AUDIO_PACKAGE_SIZE_MAX);
	memset(buffer, 0, AUDIO_PACKAGE_SIZE_MAX);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(network_audio_group_ip);
	serv_addr.sin_port = htons(1234);
	int addr_len = sizeof(struct sockaddr_in);
	// int file_fd = open("/mnt/write.h264",O_CREAT|O_WRONLY);
	while (remain_size > 0)
	{
		memcpy(&buffer[0], nework_get_package_head(network_audio_send_eth_id), 60);
		if (send_size == 0)
		{
			memcpy(&buffer[60], audio_start_code, 4);
			buffer[64] = (remain_size >> 24) & 0xFF;
			buffer[65] = (remain_size >> 16) & 0xFF;
			buffer[66] = (remain_size >> 8) & 0xFF;
			buffer[67] = remain_size & 0xFF;

			buffer[68] = (package->pts >> 24) & 0xFF;
			buffer[69] = (package->pts >> 16) & 0xFF;
			buffer[70] = (package->pts >> 8) & 0xFF;
			buffer[71] = package->pts & 0xFF;

			buffer[72] = (package->frame_index >> 24) & 0xFF;
			buffer[73] = (package->frame_index >> 16) & 0xFF;
			buffer[74] = (package->frame_index >> 8) & 0xFF;
			buffer[75] = package->frame_index & 0xFF;
			buffer[76] = package->frame_type;

			if (remain_size > (AUDIO_PACKAGE_SIZE_MAX - 77))
			{

				memcpy(&buffer[77], &package->data[send_size], AUDIO_PACKAGE_SIZE_MAX - 77);
				// write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
				printf("heheheheheh\n");
				if (sendto(audio_package_send_fd, buffer, AUDIO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)&serv_addr, addr_len) < 0)
				{
					perror(" send to fail \n");
				}
				remain_size -= (AUDIO_PACKAGE_SIZE_MAX - 77);
				send_size += (AUDIO_PACKAGE_SIZE_MAX - 77);
			}
			else
			{
				memcpy(&buffer[77], &package->data[send_size], remain_size);
				// write(file_fd,buffer,remain_size + 31);
				if (sendto(audio_package_send_fd, buffer, remain_size + 77, 0, (struct sockaddr_in *)&serv_addr, addr_len) < 0)
				{
					perror(" send to fail \n");
				}
				//	printf("send audio size:%d \n",remain_size);
				break;
			}
		}

		// ak_sleep_ms(1);
		if (remain_size > (AUDIO_PACKAGE_SIZE_MAX - 60))
		{
			memcpy(&buffer[60], &package->data[send_size], AUDIO_PACKAGE_SIZE_MAX - 60);
			//	write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
			if (sendto(audio_package_send_fd, buffer, AUDIO_PACKAGE_SIZE_MAX, 0, (struct sockaddr_in *)&serv_addr, addr_len) < 0)
			{
				perror(" send to fail \n");
			}
			remain_size -= (AUDIO_PACKAGE_SIZE_MAX - 60);
			send_size += (AUDIO_PACKAGE_SIZE_MAX - 60);
		}
		else
		{
			memcpy(&buffer[60], &package->data[send_size], remain_size);
			// write(file_fd,buffer,remain_size + 14);
			if (sendto(audio_package_send_fd, buffer, remain_size + 60, 0, (struct sockaddr_in *)&serv_addr, addr_len) < 0)
			{
				perror(" send to fail \n");
			}
			break;
		}
	}
	ak_mem_free(buffer);
	// close(file_fd);
	// printf("write finish \n");
	// while(1);
}

static void *network_audio_send_package_task(void *arg)
{
	network_audio_send_thread_run = true;

	network_audio_send_socket_open();

	network_audio_send_ready = true;
	while (networK_audio_send_task_run == true)
	{
		network_audio_package *node = NULL;
		ak_thread_mutex_lock(&network_audio_send_head_mutex);
		if (queue_empty(&network_audio_send_queue_head) == 0)
		{
			node = (network_audio_package *)queue_delete_next(&network_audio_send_queue_head);
		}
		ak_thread_mutex_unlock(&network_audio_send_head_mutex);

		if (node != NULL)
		{
			network_send_package_audio(node);
			network_audio_send_package_queue_del(node);
		}
		// ak_sleep_ms(1);
	}

	ak_thread_mutex_lock(&network_audio_send_head_mutex);
	network_audio_send_ready = false;
	ak_thread_mutex_unlock(&network_audio_send_head_mutex);

	audio_input_close();

	network_audio_send_socket_close();
	network_audio_send_release_all();

	network_audio_send_thread_run = false;

	printf("===========<<< network audio send finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool network_audio_send_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_audio_send_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

// 发包打开
bool network_audio_send_package_open(char *eth_ip)
{
	network_audio_send_package_queue_init(); // 初始化 互斥锁和队列

	if (networK_audio_send_task_run == true) // 发送任务已经开始
	{
		return false;
	}

	if (network_audio_send_wait_thread_quit() == false) // 等待线程退出 线程未退出 false
	{
		return false;
	}
	strcpy(network_audio_group_ip, eth_ip);
	networK_audio_send_task_run = true; // 设置任务运行的标志位
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_audio_send_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);

	return true;
}

// 发包关闭
bool network_audio_send_package_close(void)
{
	if (networK_audio_send_task_run == false)
	{
		return false;
	}
	networK_audio_send_task_run = false;
	return true;
}

// 发送的包加入队列
bool network_audio_send_package_push(char type, const char *data, int len)
{
	ak_thread_mutex_lock(&network_audio_send_head_mutex);
	if (network_audio_send_ready == false)
	{
		ak_thread_mutex_unlock(&network_audio_send_head_mutex);
		return false;
	}

	network_audio_package *node = network_audio_send_package_queue_new(type, data, len); // 创建新包
	if (node == NULL)
	{
		printf("network_audio_send_package_push full\n\r");
		ak_thread_mutex_unlock(&network_audio_send_head_mutex);
		return false;
	}

	queue_insert((queue_s *)node, &network_audio_send_queue_head); // 插入队列
	ak_thread_mutex_unlock(&network_audio_send_head_mutex);
	return true;
}

static bool network_audio_receive_thread_run = false;
static bool networK_audio_receive_task_run = false;
static int audio_package_receive_fd = -1;

struct ip_mreq mreq; // 多播地址结构体

static bool network_audio_receive_socket_open(void)
{
	printf("+++++++++network_audio_group_ip is %s\n", network_audio_group_ip);
	if (audio_package_receive_fd != -1)
	{
		return false;
	}
	audio_package_receive_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (audio_package_receive_fd < 0)
	{
		perror("socket failed!");
		return false;
	}

	struct sockaddr_in local_addr;
	memset(&local_addr, 0, sizeof(local_addr));

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(1234);

	bool bOptval = true;
	int ret = setsockopt(audio_package_receive_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&bOptval, sizeof(bOptval));
	if (ret != 0)
	{
		perror("AUDIO_RECEIVE_SOCKET REUSE failed\n");
	}

	ret = bind(audio_package_receive_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0)
	{
		perror("bind failed !");
		return false;
	}

	mreq.imr_multiaddr.s_addr = inet_addr(network_audio_group_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	ret = setsockopt(audio_package_receive_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if (ret < 0)
	{
		perror("setsockopt failed !");
		return false;
	}
	else
	{
		printf("setsockopt success\n");
	}
	return true;
}

static bool network_audio_receive_socket_close(void)
{
	if (audio_package_receive_fd == -1)
	{
		return false;
	}
	// 离开多播组
	int ret = setsockopt(audio_package_receive_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
	if (ret < 0)
	{
		perror("IP_DROP_MEMBERSHIP");
	}

	close(audio_package_receive_fd);
	audio_package_receive_fd = -1;

	return true;
}

unsigned long long os_get_ms(void)
{
	struct ak_timeval tv;
	ak_get_ostime(&tv);
	return tv.usec / 1000 + tv.sec * 1000;
}

static void *network_audio_receive_package_task(void *arg)
{
	fd_set readfds;
	struct timeval timeout;

	network_audio_receive_thread_run = true;

	network_audio_receive_socket_open();

	char *receive_frame_buffer = ak_mem_alloc(MODULE_ID_APP, AUDIO_FRAME_MAX);
	// unsigned long long receive_frame_pts = 0;
	// unsigned long long receive_video_index = 0;
	unsigned int receive_frame_size = 0;
	unsigned int receive_frame_count = 0;
	bool receive_frame_start = false;

	char *buf_ptr = NULL;
	char buffer[AUDIO_PACKAGE_SIZE_MAX] = {0};
	// char nouse = 10;

	while (networK_audio_receive_task_run == true)
	{
		FD_ZERO(&readfds);
		/*将所要检测端socket句柄加入到集合中*/
		FD_SET(audio_package_receive_fd, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;
		/*设置select等待的最大时间 检测集合read中的句柄是否有可读信息*/
		int ret_select = select(audio_package_receive_fd + 1, &readfds, NULL, NULL, &timeout);
		if (ret_select > 0)
		{
			/*如果这个被监视端句柄真的变为可读了*/
			if (FD_ISSET(audio_package_receive_fd, &readfds))
			{

				int ret = recvfrom(audio_package_receive_fd, buffer, sizeof(buffer), 0, NULL, NULL);
				if (ret > 60)
				{
					// write(file_fd,buffer,ret);
					buf_ptr = &buffer[60];
					ret -= 60;
					while (ret > 0)
					{
						if (memcmp(buf_ptr, audio_start_code, 4) == 0)
						{
							receive_frame_count = 0;

							receive_frame_size = (buf_ptr[4] << 24) | (buf_ptr[5] << 16) | (buf_ptr[6] << 8) | buf_ptr[7];
							// receive_frame_pts =  (buf_ptr[8] << 24) | (buf_ptr[9] << 16) | (buf_ptr[10] << 8) | buf_ptr[11];
							// receive_video_index =  (buf_ptr[12] << 24) | (buf_ptr[13] << 16) | (buf_ptr[14] << 8) | buf_ptr[15];
							// printf("===========================>>> receive : %d \n\r",receive_frame_size);
							// frame_type = buf_ptr[16];
							//	receive_frame_key_frame  =buf_ptr[16]?true:false;
							buf_ptr += 17;
							ret -= 17;
							if (ret < 0)
							{
								receive_frame_start = false;
							}
							else
							{
								receive_frame_start = true;
							}
						}

						if (receive_frame_start == true)
						{
							if ((receive_frame_count + ret) <= receive_frame_size)
							{
								memcpy(&receive_frame_buffer[receive_frame_count], buf_ptr, ret);
								receive_frame_count += ret;
								ret = 0;

								if (receive_frame_count == receive_frame_size) // && nouse == 0)
								{
									receive_frame_start = false;
									extern MON_ENTER_FLG monitor_enter_flag_get(void);
									if ((tuya_ipc_get_client_online_num() > 0) && (monitor_enter_flag_get() == MON_ENTER_TUYA))
									{
// printf("1===========================>>> receive : %d \n\r",receive_frame_size);
										#if defined(TUYA_AUDIO_SAMPLE) && (TUYA_AUDIO_SAMPLE == 16000)
											tuya_ipc_ring_buffer_append_data(9 /*E_CHANNEL_AUDIO*/, (unsigned char *)receive_frame_buffer, receive_frame_size, 3 /*E_AUDIO_FRAME*/, os_get_ms());
										#else
											int input_size = receive_frame_size;
											int output_size = input_size / 2; // 8K数据量是16K的一半

											short *pcm_16k_buf = (short *)receive_frame_buffer;
											short *pcm_8k_buf = (short *)malloc(output_size);
											// short *pcm_8k_buf = (short *)receive_frame_buffer;//采样率16k强转为8k

											if (pcm_8k_buf)
											{
												int sample_num_16k = input_size / 2; // 每个采样占2字节
												int sample_num_8k = sample_num_16k / 2;
												// int sample_num = receive_frame_size / 4;

												for (int i = 0; i < sample_num_8k; i++)
												{
													pcm_8k_buf[i] = (pcm_16k_buf[2 * i] + pcm_16k_buf[2 * i + 1]) / 2;
												}

												tuya_ipc_ring_buffer_append_data(9 /*E_CHANNEL_AUDIO*/, (unsigned char *)pcm_8k_buf, output_size, 3 /*E_AUDIO_FRAME*/, os_get_ms());
												free(pcm_8k_buf);
											}
										#endif
									}
									else
									{
										audio_decode_queue_push((unsigned char *)receive_frame_buffer, receive_frame_size);
										// printf("2===========================>>> receive : %d \n\r",receive_frame_size);
										video_record_data_push(0, (unsigned char *)receive_frame_buffer, receive_frame_size, false);
									}

									// char buf[1024] = {0};
									// unsigned int len = 0;
									// tuya_g711_encode(1,receive_frame_buffer,receive_frame_size,buf,&len);

									ak_sleep_ms(1);
								}
								// else if(nouse > 0){
								//		nouse--;
								// }
							}
							else
							{
								printf("audio unknow data:ret = %d count :%d\n", ret, receive_frame_count);
								ret = 0;
							}
						}
						else
						{
							printf("unknow data:ret = %d\n", ret);
							ret = -1;
						}
					}
				}
			}
		}
		// ak_sleep_ms(1);
	}

	if (receive_frame_buffer != NULL)
	{
		ak_mem_free(receive_frame_buffer);
	}
	network_audio_receive_socket_close();
	network_audio_receive_thread_run = false;

	printf("===========<<< network audio receive finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool network_audio_receive_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_audio_receive_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

bool network_audio_receive_package_open(char *eth_ip)
{
	if (networK_audio_receive_task_run == true)
	{
		return false;
	}

	if (network_audio_receive_wait_thread_quit() == false)
	{
		return false;
	}
	strcpy(network_audio_group_ip, eth_ip);
	networK_audio_receive_task_run = true;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_audio_receive_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);

	return true;
}

bool network_audio_receive_package_close(void)
{
	if (networK_audio_receive_task_run == false)
	{
		return false;
	}
	networK_audio_receive_task_run = false;
	return true;
}
