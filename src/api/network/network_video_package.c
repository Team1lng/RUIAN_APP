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
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include "ak_thread.h"
#include "ak_mem.h"
#include <net/if.h>
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
#include "ak_common.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/in.h>
#include "../../layout/user_data.h"
#include "tuya_sdk.h"
#define VIDEO_QUEUE_PACKAGE_MAX 64

#define VIDEO_PACKAGE_SIZE_MAX (32 * 1024) // 1500 //10*1024//1510

#define VIDEO_FRAME_MAX (218 * 1024)

#define VIDEO_IP_ADDRES "255.255.255.255" //"192.168.37.1"//

static int network_video_send_eth_id = 0;

static const char video_start_code[4] = {0x00, 0x00, 0x01, 0xfc};

// 第一帧是否为I帧
unsigned long receive_video_index = 0;
unsigned long prev_video_index = 0;
static bool is_first_frame_i_frame = false; // 第一帧是否为I帧

extern struct sockaddr_ll *network_get_send_addres(void);
extern const char *nework_get_package_head(unsigned int type);
extern bool video_decode_push(char, unsigned char *data, int len);
extern bool video_record_data_push(char, unsigned char *data, int len, bool is_video);

typedef struct
{
	char frame_type; // 0:h264,1mjpeg
	char *data;
	int len;
	unsigned long long pts;
	unsigned long long frame_index;
} network_video_data;

typedef struct
{
	void *prev;
	void *next;

	network_video_data package;
} network_video_package;
static queue_s network_video_send_queue_free;
static queue_s network_video_send_queue_head;

static ak_mutex_t network_video_send_head_mutex;
static ak_mutex_t network_video_send_free_mutex;

static network_video_package network_video_package_buffer[VIDEO_QUEUE_PACKAGE_MAX];
static void network_video_send_package_queue_init(void)
{
	static bool is_first = true;
	if (is_first == false)
	{
		return;
	}
	is_first = false;

	ak_thread_mutex_init(&network_video_send_head_mutex, NULL);
	ak_thread_mutex_init(&network_video_send_free_mutex, NULL);

	queue_initialize(&network_video_send_queue_head);
	queue_initialize(&network_video_send_queue_free);
	memset(network_video_package_buffer, 0, sizeof(network_video_package) * VIDEO_QUEUE_PACKAGE_MAX);
	for (int i = 0; i < VIDEO_QUEUE_PACKAGE_MAX; i++)
	{
		queue_insert((queue_s *)&network_video_package_buffer[i], &network_video_send_queue_free);
	}
}

static network_video_package *network_video_send_package_queue_new(char type, const char *data, int size)
{
	network_video_package *node = NULL;
	ak_thread_mutex_lock(&network_video_send_free_mutex);
	if (queue_empty(&network_video_send_queue_free) == 0)
	{
		node = (network_video_package *)queue_delete_next(&network_video_send_queue_free);
	}
	ak_thread_mutex_unlock(&network_video_send_free_mutex);
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
	node->package.frame_type = type;
	node->package.len = size;
	node->package.pts = get_sys_ms();
	// printf("send index :%llu \n",node->package.frame_index);
	return node;
}
static void network_video_send_package_queue_del(network_video_package *node)
{
	if (node != NULL)
	{
		if (node->package.data != NULL)
		{
			ak_mem_free(node->package.data);
			node->package.data = NULL;
		}

		ak_thread_mutex_lock(&network_video_send_free_mutex);
		queue_insert((queue_s *)node, &network_video_send_queue_free);
		ak_thread_mutex_unlock(&network_video_send_free_mutex);
	}
}

static void network_video_send_release_all(void)
{
	ak_thread_mutex_lock(&network_video_send_head_mutex);
	while (!queue_empty(&network_video_send_queue_head))
	{
		network_video_package *node = (network_video_package *)queue_delete_next(&network_video_send_queue_head);
		network_video_send_package_queue_del(node);
	}
	ak_thread_mutex_unlock(&network_video_send_head_mutex);
}

static bool network_video_send_thread_run = false;
static bool networK_video_send_task_run = false;
static bool network_video_send_ready = false;
static int video_package_send_fd = -1;
static bool network_video_send_socket_open(void)
{
	if (video_package_send_fd != -1)
	{
		return false;
	}

	return true;
}

static bool network_video_send_socket_close(void)
{
	if (video_package_send_fd == -1)
	{
		return false;
	}

	close(video_package_send_fd);
	video_package_send_fd = -1;

	return true;
}

static void network_send_package_video(network_video_package *node)
{
	network_video_data *package = &(node->package);
	int send_size = 0, remain_size = package->len;
	char *buffer = (char *)ak_mem_alloc(MODULE_ID_APP, VIDEO_PACKAGE_SIZE_MAX);
	memset(buffer, 0, VIDEO_PACKAGE_SIZE_MAX);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(VIDEO_IP_ADDRES);
	serv_addr.sin_port = htons(network_video_send_eth_id); // network_video_send_eth_id

	while (remain_size > 0)
	{
		// memcpy(&buffer[0],nework_get_package_head(network_video_send_eth_id),60);
		if (send_size == 0)
		{
			memcpy(&buffer[0], video_start_code, 4);
			buffer[4] = (remain_size >> 24) & 0xFF;
			buffer[5] = (remain_size >> 16) & 0xFF;
			buffer[6] = (remain_size >> 8) & 0xFF;
			buffer[7] = remain_size & 0xFF;

			buffer[8] = (package->pts >> 24) & 0xFF;
			buffer[9] = (package->pts >> 16) & 0xFF;
			buffer[10] = (package->pts >> 8) & 0xFF;
			buffer[11] = package->pts & 0xFF;

			buffer[12] = (package->frame_index >> 24) & 0xFF;
			buffer[13] = (package->frame_index >> 16) & 0xFF;
			buffer[14] = (package->frame_index >> 8) & 0xFF;
			buffer[15] = package->frame_index & 0xFF;
			buffer[16] = package->frame_type;

			if (remain_size > (VIDEO_PACKAGE_SIZE_MAX - 17))
			{
				memcpy(&buffer[17], &package->data[send_size], VIDEO_PACKAGE_SIZE_MAX - 17);
				// write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
				if (sendto(video_package_send_fd, buffer, VIDEO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
				{
					perror(" send to fail \n");
				}
				remain_size -= (VIDEO_PACKAGE_SIZE_MAX - 17);
				send_size += (VIDEO_PACKAGE_SIZE_MAX - 17);
			}
			else
			{
				memcpy(&buffer[17], &package->data[send_size], remain_size);
				// write(file_fd,buffer,remain_size + 31);
				if (sendto(video_package_send_fd, buffer, remain_size + 17, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
				{
					perror(" send to fail \n");
				}
				break;
			}
		}

		// ak_sleep_ms(1);
		if (remain_size > VIDEO_PACKAGE_SIZE_MAX)
		{
			memcpy(&buffer[0], &package->data[send_size], VIDEO_PACKAGE_SIZE_MAX - 0);
			//	write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
			if (sendto(video_package_send_fd, buffer, VIDEO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			remain_size -= (VIDEO_PACKAGE_SIZE_MAX);
			send_size += (VIDEO_PACKAGE_SIZE_MAX);
		}
		else
		{
			memcpy(&buffer[0], &package->data[send_size], remain_size);
			// write(file_fd,buffer,remain_size + 14);
			if (sendto(video_package_send_fd, buffer, remain_size + 0, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			break;
		}
	}
	ak_mem_free(buffer);
}

static void *network_video_send_package_task(void *arg)
{
	network_video_send_thread_run = true;

	network_video_send_socket_open();

	network_video_send_ready = true;
	while (networK_video_send_task_run == true)
	{
		network_video_package *node = NULL;
		ak_thread_mutex_lock(&network_video_send_head_mutex);
		if (queue_empty(&network_video_send_queue_head) == 0)
		{
			node = (network_video_package *)queue_delete_next(&network_video_send_queue_head);
		}
		ak_thread_mutex_unlock(&network_video_send_head_mutex);
		if (node != NULL)
		{
			network_send_package_video(node);
			network_video_send_package_queue_del(node);
		}
		ak_sleep_ms(1);
	}

	ak_thread_mutex_lock(&network_video_send_head_mutex);
	network_video_send_ready = false;
	ak_thread_mutex_unlock(&network_video_send_head_mutex);

	network_video_send_socket_close();
	network_video_send_release_all();
	network_video_send_thread_run = false;

	printf("===========<<< network video send finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool network_video_send_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{

		if (network_video_send_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

bool network_video_send_package_open(unsigned long id)
{
	network_video_send_package_queue_init();

	if (networK_video_send_task_run == true)
	{
		return false;
	}

	if (network_video_send_wait_thread_quit() == false)
	{
		return false;
	}

	network_video_send_eth_id = id;
	networK_video_send_task_run = true;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_video_send_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);

	return true;
}

bool network_video_send_package_close(void)
{
	if (networK_video_send_task_run == false)
	{
		return false;
	}
	networK_video_send_task_run = false;
	return true;
}

bool network_video_send_package_push(char type, const char *data, int len)
{
	ak_thread_mutex_lock(&network_video_send_head_mutex);
	if (network_video_send_ready == false)
	{
		ak_thread_mutex_unlock(&network_video_send_head_mutex);
		return false;
	}

	network_video_package *node = network_video_send_package_queue_new(type, data, len);
	if (node == NULL)
	{
		printf("network_video_send_package_push full\n\r");
		ak_thread_mutex_unlock(&network_video_send_head_mutex);
		return false;
	}
	queue_insert((queue_s *)node, &network_video_send_queue_head);
	ak_thread_mutex_unlock(&network_video_send_head_mutex);
	return true;
}

static bool network_video_receive_thread_run = false;
static bool networK_video_receive_task_run = false;
static int video_package_receive_fd = -1;
static char network_video_receive_group_ip[128];
static struct ip_mreq mreq; // 多播地址结构体

void network_video_receive_group_ip_set(char *ip)
{
	*network_video_receive_group_ip = *ip;
}

static bool network_video_receive_socket_open(void)
{
	if (video_package_receive_fd != -1)
	{
		return false;
	}
	printf("==========>>> video receive socket %s <<<==========\n", network_video_receive_group_ip);
#if 0 // PF_INET  htons(ETH_P_ALL )
	if((video_package_receive_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

    struct ifreq req;
    int fd = socket(PF_INET,SOCK_DGRAM,0);
    strcpy(req.ifr_name,NETWORK_NAME);
    ioctl(fd,SIOCGIFINDEX,&req);
    close(fd);


    struct sockaddr_ll sll;
    struct packet_mreq mr;
    memset( &sll, 0, sizeof( sll ) );
    sll.sll_family   = PF_PACKET;
    sll.sll_ifindex  = req.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_IP);//htons( ETH_P_ALL/*network_video_receive_eth_id*/  );
    if( bind( video_package_receive_fd, (struct sockaddr *) &sll, sizeof( sll ) ) == -1 )
    {
        perror( "bind" );
        return( 1 );
    }

    memset( &mr, 0, sizeof( mr ) );
    mr.mr_ifindex = req.ifr_ifindex;
    mr.mr_type    = PACKET_MR_PROMISC;
    if( setsockopt( video_package_receive_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,&mr, sizeof( mr ) ) == -1 )
    {
        perror( "setsockopt" );
        return( 1 );
    }

    int on=1;
    if(setsockopt(video_package_send_fd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on)) < 0)
    {
        printf("setsockopt");
    }
#else
	video_package_receive_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (video_package_receive_fd < 0)
	{
		perror("socket failed!");
		return false;
	}

	struct sockaddr_in local_addr;
	memset(&local_addr, 0, sizeof(local_addr));

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(4321);

	int bOptval = true;
	int ret = setsockopt(video_package_receive_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&bOptval, sizeof(bOptval));
	if (ret != 0)
	{
		perror("REUSE failed\n");
	}

	mreq.imr_multiaddr.s_addr = inet_addr(network_video_receive_group_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	ret = setsockopt(video_package_receive_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if (ret < 0)
	{
		perror("setsockopt failed !");
		return false;
	}
	else
	{
		printf("setsockopt success\n");
	}
	struct ifreq interface;
	memset(&interface, 0x00, sizeof(interface));
	sprintf(interface.ifr_name, "%s", NETWORK_NAME);
	if (setsockopt(video_package_receive_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
	{
		printf("eth1-client:SO_BINDTODEVICEfailed");
		return false;
	}
	ret = bind(video_package_receive_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0)
	{
		perror("bind failed !");
		return false;
	}
#endif
	printf("create video receive socket success \n");
	return true;
}

static bool network_video_receive_socket_close(void)
{
	if (video_package_receive_fd == -1)
	{
		return false;
	}
	// // 离开多播组
	// int ret = setsockopt(video_package_receive_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
	// if(ret < 0){
	// 		perror("IP_DROP_MEMBERSHIP");
	// 		return -1;
	// }
	close(video_package_receive_fd);
	video_package_receive_fd = -1;

	return true;
}

/**
 * 解析H.264码流（Annex-B格式），打印所有NALU单元的内容和长度
 * @param data  指向H.264码流数据的指针
 * @param len   码流数据的总长度（字节数）
 */
// void print_nalu_units(const uint8_t *data, uint32_t len) {
//     if (data == NULL || len == 0) {
//         printf("无效的输入数据\n");
//         return;
//     }

//     uint32_t pos = 0;  // 当前解析位置
//     uint32_t nalu_count = 0;  // NALU单元计数器

//     printf("开始解析H.264码流，总长度: %u字节\n", len);
//     printf("========================================\n");

//     while (pos < len) {
//         // 1. 查找起始码（0x000001或0x00000001）
//         uint32_t start_code_len = 0;
//         uint32_t start_pos = pos;

//         // 检查4字节起始码（0x00000001）
//         if (pos + 3 < len && 
//             data[pos] == 0x00 && 
//             data[pos+1] == 0x00 && 
//             data[pos+2] == 0x00 && 
//             data[pos+3] == 0x01) {
//             start_code_len = 4;
//         }
//         // 检查3字节起始码（0x000001）
//         else if (pos + 2 < len && 
//                  data[pos] == 0x00 && 
//                  data[pos+1] == 0x00 && 
//                  data[pos+2] == 0x01) {
//             start_code_len = 3;
//         }

//         // 如果没找到起始码，移动到下一个字节继续查找
//         if (start_code_len == 0) {
//             pos++;
//             continue;
//         }

//         // 2. 记录当前NALU的起始位置（跳过起始码）
//         nalu_count++;
//         uint32_t nalu_start = pos + start_code_len;
//         pos = nalu_start;

//         // 3. 查找下一个起始码，确定当前NALU的结束位置
//         uint32_t nalu_end = nalu_start;
//         while (nalu_end < len) {
//             // 提前判断是否有足够的字节检查起始码
//             if (nalu_end + 3 < len) {
//                 // 检查4字节起始码
//                 if (data[nalu_end] == 0x00 && 
//                     data[nalu_end+1] == 0x00 && 
//                     data[nalu_end+2] == 0x00 && 
//                     data[nalu_end+3] == 0x01) {
//                     break;
//                 }
//                 // 检查3字节起始码
//                 else if (data[nalu_end] == 0x00 && 
//                          data[nalu_end+1] == 0x00 && 
//                          data[nalu_end+2] == 0x01) {
//                     break;
//                 }
//             }
//             nalu_end++;
//         }

//         // 4. 计算NALU长度
//         uint32_t nalu_len = nalu_end - nalu_start;

//         // 5. 打印NALU信息
//         printf("NALU #%u:\n", nalu_count);
//         printf("  起始码长度: %u字节\n", start_code_len);
//         printf("  NALU长度: %u字节\n", nalu_len);
//         printf("  NALU头: 0x%02X ", data[nalu_start]);
        
//         // 解析NALU头类型
//         uint8_t nalu_header = data[nalu_start];
//         uint8_t nalu_type = nalu_header & 0x1F;  // 取低5位
//         printf("(类型: %u) ", nalu_type);
        
//         // 简单打印常见类型说明
//         switch (nalu_type) {
//             case 7:  printf("(SPS - 序列参数集)\n"); break;
//             case 8:  printf("(PPS - 图像参数集)\n"); break;
//             case 5:  printf("(I帧 - 关键帧)\n"); break;
//             case 1:  printf("(P帧 - 预测帧)\n"); break;
//             case 6:  printf("(SEI - 补充增强信息)\n"); break;
//             default: printf("(其他类型)\n"); break;
//         }

//         // 6. 打印NALU前16字节内容（避免过长）
//         printf("  内容前16字节: ");
//         uint32_t print_len = (nalu_len > 16) ? 16 : nalu_len;
//         for (uint32_t i = 0; i < print_len; i++) {
//             printf("%02X ", data[nalu_start + i]);
//         }
//         if (nalu_len > 16) {
//             printf("... (省略剩余 %u 字节)", nalu_len - 16);
//         }
//         printf("\n========================================\n");

//         // 移动到下一个NALU的起始码位置
//         pos = nalu_end;
//     }

//     printf("解析完成，共找到 %u 个NALU单元\n", nalu_count);
// }

extern unsigned long long os_get_ms(void);

//==================cyy:视频接受任务==========================================
static void *network_video_receive_package_task(void *arg)
{
	// 打印函数和行号
	// printf("========1=======%s=============%d\n",__func__,__LINE__);

	fd_set readfds; // 文件描述符集合，用于select监听

	struct timeval timeout; // select超时时间结构体

	network_video_receive_thread_run = true; // 标记视频接收线程已启动

	network_video_receive_socket_open(); // 打开视频接收套接字

	// 分配接收帧缓冲区，用于存储完整视频帧
	char *receive_frame_buffer = ak_mem_alloc(MODULE_ID_VDEC, VIDEO_FRAME_MAX);

	// unsigned long long receive_frame_pts = 0;// 视频帧时间戳

	// unsigned long long receive_video_index = 0;// 视频帧序号

	unsigned int receive_frame_size = 0; // 接收帧数据大小

	unsigned int receive_frame_count = 0; // 已接收数据字节数

	bool receive_frame_start = false; // 帧接收状态标记

	char frame_type = 0; // 视频帧类型（H.264或MJPEG）

	char *buf_ptr = NULL; // 缓冲区指针，用于数据解析

	char *buffer = ak_mem_alloc(MODULE_ID_VDEC, VIDEO_PACKAGE_SIZE_MAX); // 分配网络接收缓冲区

	// char receive_frame_key_frame = 0;// 关键帧标记

	static int key_frame_count = 0; // 关键帧请求计数（用于涂鸦云加速出图）

	// 接收任务主循环
	while (networK_video_receive_task_run == true)
	{
		// 打印函数和行号（调试用，已注释）
		// printf("=========2======%s=============%d\n",__func__,__LINE__);

		FD_ZERO(&readfds); // 清空文件描述符集合

		FD_SET(video_package_receive_fd, &readfds); // 将接收套接字添加到监听集合

		// 设置select超时时间为5毫秒
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;

		// 使用select监听套接字数据
		int ret_select = select(video_package_receive_fd + 1, &readfds, NULL, NULL, &timeout);
		if (ret_select > 0)
		{
			// 检测到套接字有数据可读
			if (FD_ISSET(video_package_receive_fd, &readfds))
			{
				// 从套接字接收网络数据
				// int ret = recv(video_package_receive_fd, buffer, sizeof(buffer), 0);
				int ret = recvfrom(video_package_receive_fd, buffer, VIDEO_PACKAGE_SIZE_MAX, 0, NULL, NULL);
				if (ret > 0)
				{

					buf_ptr = &buffer[0]; // 设置缓冲区指针到数据起始位置
					// ret -= 60; // 跳过链路层头部

					// 循环处理接收到的数据
					while (ret > 0)
					{
						// 检测视频帧起始码
						if (memcmp(buf_ptr, video_start_code, 4) == 0) // cyy:比较buf_ptr指向的 4 字节数据与video_start_code是否完全相同
						{
							// 重置已接收数据计数
							receive_frame_count = 0;

							// 解析视频帧大小
							receive_frame_size = (buf_ptr[4] << 24) | (buf_ptr[5] << 16) | (buf_ptr[6] << 8) | buf_ptr[7];

							// 解析时间戳
							// receive_frame_pts =  (buf_ptr[8] << 24) | (buf_ptr[9] << 16) | (buf_ptr[10] << 8) | buf_ptr[11];

							// 解析帧序号
							// receive_video_index =  (buf_ptr[12] << 24) | (buf_ptr[13] << 16) | (buf_ptr[14] << 8) | buf_ptr[15];

							// 打印帧大小
							// printf("===========================>>> receive : %d \n\r",receive_frame_size);

							// 解析帧类型
							frame_type = buf_ptr[16];

							// 解析关键帧标记
							// receive_frame_key_frame  =buf_ptr[16]?true:false;

							// 移动指针跳过帧头部
							buf_ptr += 17;

							// 减少剩余数据长度
							ret -= 17;

							// 检查数据有效性
							if ((ret <= 0) || (receive_frame_size > VIDEO_FRAME_MAX))
							{
								// 打印错误信息
								printf("hlf ret:%d, receive_frame_size:%d\n", ret, receive_frame_size);

								// 标记帧接收未开始
								receive_frame_start = false;
							}
							else
							{
								// 标记开始接收帧数据
								receive_frame_start = true;
							}
						}

						// 处理正在接收的视频帧
						if (receive_frame_start == true)
						{
							// 检查是否能完整接收当前分片数据
							if ((receive_frame_count + ret) <= receive_frame_size)
							{
								// 复制数据到帧缓冲区
								memcpy(&receive_frame_buffer[receive_frame_count], buf_ptr, ret);

								// 更新已接收数据计数
								receive_frame_count += ret;

								// 清零剩余数据长度
								ret = 0;

								// 检查是否接收完一整帧

								if (receive_frame_count == receive_frame_size)
								{
									// 标记帧接收完成

									receive_frame_start = false;

									// 检测H.264是否为关键帧
									extern int h264_is_keyframe(const unsigned char *buffer, int len);

									// 获取涂鸦云连接状态
									extern int is_tuya_cloud_connected_num(void);
									// 获取监控进入标志
									extern MON_ENTER_FLG monitor_enter_flag_get(void);
									// 涂鸦云相关逻辑：请求关键帧加速出图

									if ((is_tuya_cloud_connected_num() > 0) && (monitor_enter_flag_get() == MON_ENTER_TUYA))
									{
										// 检测H.264是否为关键帧（I帧）

										//print_nalu_units(receive_frame_buffer,receive_frame_size);

										int is_keyframe = h264_is_keyframe((const unsigned char *)(receive_frame_buffer + 4),
																		   receive_frame_size - 4);
										if (is_keyframe)
										{
											if(is_first_frame_i_frame == false)
												tuya_ipc_ring_buffer_video_release_data();
											is_first_frame_i_frame = true;
											// 收到I帧，重置请求计数
											key_frame_count = 0;
											//printf("收到0x%x帧，重置key_frame_count为0\n",is_keyframe);
										}

										// 仅在第一帧是I帧时才进行上传处理,一旦出现丢包或异常，需等待下一个I帧
										if (is_first_frame_i_frame)
										{
											//printf("===============================> [%s]__LINE__,%d\n", __func__, __LINE__);
											// 将视频数据添加到涂鸦IPC环形缓冲区
											tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN,
																			 (unsigned char *)receive_frame_buffer,
																			 receive_frame_size,
																			 is_keyframe ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME,
																			 // h264_is_keyframe((const unsigned char *)(receive_frame_buffer + 4), receive_frame_size - 4) ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME,
																			 os_get_ms());

											tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_SUB,
																			 (unsigned char *)receive_frame_buffer,
																			 receive_frame_size,
																			 is_keyframe ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME,
																			 // h264_is_keyframe((const unsigned char *)(receive_frame_buffer + 4), receive_frame_size - 4) ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME,
																			 os_get_ms());
										}
										else
										{
											// 第一帧不是I帧时跳过上传（直到收到I帧）
											// 可在此处增加超时重连或强制请求I帧的逻辑
											if (key_frame_count < 50)
											{
												// 主动请求I帧
												network_cmd_data_init(data);
												MONITOR_CH ch = monitor_channel_get();
												network_device device = get_outdoor_device_by_channel(ch);
												data.device = device;
												data.cmd = NET_COMMON_CMD_KEY_FRAME;
												data.arg1 = 1;
												data.arg2 = 0;
												network_send_cmd_data(&data);
												key_frame_count++;
											}
										}
									}
									else
									{
										// 推送给视频解码器
										video_decode_push(frame_type, (unsigned char *)receive_frame_buffer, receive_frame_size);
										////printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
										// 推送给视频录制模块（跳过帧头部4字节）
										video_record_data_push(frame_type, (unsigned char *)receive_frame_buffer + 4, receive_frame_size - 4, true);
									}
									// 打印帧序号（调试用，已注释）
									// printf("===========================>>> receive : %llu \n\r",receive_video_index);
									// 线程休眠1毫秒（调试用，已注释）
									// ak_sleep_ms(1);
								}
							}
							else
							{
								is_first_frame_i_frame = false;
								// 打印数据长度错误
								printf(" 数据长度错误 ret = %d count :%d\n", ret + receive_frame_count, receive_frame_size);
								printf("video unknow data:ret = %d count :%d\n", ret, receive_frame_count);
								// 清零剩余数据长度
								ret = 0;
							}
						}
						else
						{
							is_first_frame_i_frame = false;
							// 打印未知数据错误
							printf("unknow data:ret = %d\n", ret);
							// 清零剩余数据长度
							ret = -1;
						}
					}
				}
			}
		}
		// 线程休眠1毫秒，避免CPU占用过高
		ak_sleep_ms(1);
	}

	is_first_frame_i_frame = false;
	// 重置关键帧请求计数
	key_frame_count = 0;

	// 释放网络接收缓冲区
	ak_mem_free(buffer);
	// 释放帧数据缓冲区
	ak_mem_free(receive_frame_buffer);

	// 关闭视频接收套接字
	network_video_receive_socket_close();
	// 标记视频接收线程已停止
	network_video_receive_thread_run = false;

	// 打印线程结束信息
	printf("===========<<< network video receive finish >>>===========\n");
	// 退出线程
	ak_thread_exit();
	return NULL;
}

static bool network_video_receive_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_video_receive_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

static lv_task_t *connect_task = NULL;

static void connect_task_event()
{
	MONITOR_CH ch = monitor_channel_get(); // 获取当前通道
	network_device cur_device = get_outdoor_device_by_channel(ch);
	network_cmd_data_init(data);
	data.cmd = NET_COMMON_CMD_SOUND;
	data.arg1 = 3 | ((user_data_get()->user_language) << 16);
	data.arg2 = 1;
	data.device = cur_device;
	data.cmd = NET_COMMON_CMD_SOUND;
	network_send_cmd_data(&data);
}

bool network_video_receive_package_open(char *ip)
{
	if (networK_video_receive_task_run == true)
	{
		return false;
	}

	if (network_video_receive_wait_thread_quit() == false)
	{
		return false;
	}

	// 开辟一条线程，往门口机发接通指令，已确定摄像头开的时候亮灯
	connect_task = lv_task_create(connect_task_event, 1000, LV_TASK_PRIO_MID, &clock);
	strcpy(network_video_receive_group_ip, ip);

	networK_video_receive_task_run = true;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_video_receive_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

bool network_video_receive_package_close(void)
{

	if (connect_task != NULL)
	{
		lv_task_del(connect_task);
		connect_task = NULL;
	}
	if (networK_video_receive_task_run == false)
	{
		return false;
	}
	networK_video_receive_task_run = false;
	return true;
}
