#include "network_common.h"
#include <sys/ioctl.h>  
#include <sys/socket.h>  
#include <net/if.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include<netinet/ether.h>
#include<pthread.h>
#include<unistd.h>//_exit
#include "ak_thread.h"
  
#define MAX_LINE 256  
  
char get_local_eth_mac_address( char *eth,char *mac_address, char *ip_address) {  
    char line[MAX_LINE];  
    FILE* fp;  
    // char iface[IFNAMSIZ];  
    struct ifreq ifr;  
    char ip[32];  
    int found = 0;  
  
    // 打开ifconfig命令的输出文件  
    fp = popen("ifconfig", "r");  
    if (fp == NULL) {  
        perror("Failed to run ifconfig command");  
        exit(1);  
    }  
  
	int get_mac_flag = false;
    while (fgets(line, sizeof(line), fp) != NULL) {  
		if(get_mac_flag){
			sscanf(line, "%*s%s%*s%*s",ip);
			if (strncmp(ip, "addr", 4) == 0){
				strcpy(ip_address,ip+5);
				found = 1;
			}
			// printf("\n-------ip_address=>%s-------\n",ip_address);
			break;
		}
        if (strncmp(line, eth, 5) == 0) { // 检查是否为wlan0接口  
            sscanf(line, "%*s%*s%*s%*s%s",ifr.ifr_hwaddr.sa_data); // 读取MAC地址  
			if(ifr.ifr_hwaddr.sa_data){
				strcpy(mac_address,ifr.ifr_hwaddr.sa_data);
			}
			// printf("\n-------mac_address=>%s-------\n",mac_address);
            get_mac_flag = true;
        }
		usleep(1000*10);
		
    }  
  
    pclose(fp); // 关闭ifconfig命令的输出文件  
  
    if (!found) { // 如果未找到wlan0接口或MAC地址，返回NULL或错误消息（根据需要修改）  
        printf("\nFailed to find %s interface or MAC address.\n",eth);  
        return false; // 或返回错误代码等其他处理方式（根据需要修改）  
    } else { // 如果找到了wlan0接口和MAC地址，返回MAC地址字符串  
        printf(" \nlocal MAC address get successful, MAC address=>%s,IP address=>%s\n",mac_address,ip_address);  
        return true; // 返回复制的MAC地址字符串（注意释放内存）  
    }  
}






void my_sendto(int sockfd, char *out, unsigned char *msg, int msg_len);
void *recv_msg(void *arg)
{
	int sockfd = (int)arg;
	while(1)
	{
		printf("\n----reciving\n");
		unsigned char buf[1500]="";
		recvfrom(sockfd, buf,sizeof(buf), 0,NULL,NULL);
		printf("\n----recive_buf=>%s\n",buf);
		
		if(ntohs(*(unsigned short *)(buf+12)) == 0x0806)//arp报文
		{
			if(ntohs(*(unsigned short *)(buf+14+6)) == 2)//应答
			{
				//获取mac buf中的
				unsigned char tmp_ip[4]={192,168,0,110};
				if(memcmp(tmp_ip,buf+14+14, 4) == 0)
				{
					char mac[18]="";
					sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",
					buf[6+0],buf[6+1],buf[6+2],buf[6+3],buf[6+4],buf[6+5]);
					char ip[16]="";
					sprintf(ip,"%d.%d.%d.%d",\
					buf[28+0],buf[28+1],buf[28+2],buf[28+3]);
					printf("IP:%s--->MAC:%s\n",ip,mac);
					break;
                    
				}
				
			}
			
			
		}
		sleep(1);
	}
		return NULL;
	
}
int get_mac_adress_begin( char *soure_mac,char *soure_ip,char *goal_ip)
{
	//1、创建原始套接字
	int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sockfd < 0)
	{
		perror("socket");
		return 0;
	}
	
#if 0
    /*mac报文头部 14B*/
		0xff,0xff,0xff,0xff,0xff,0xff,/*目的mac地址*/
		0x00,0x0c,0x29,0x79,0xf9,0x7f,/*源mac地址 根据自己修改 ubuntu_mac*/
		0x08,0x06,/*帧类型*/
		
		/*ARP报文头部 28B*/
		0x00,0x01,/*硬件类型*/
		0x08,0x00,/*协议类型*/
		6,/*硬件地址长度*/
		4,/*协议地址长度*/
		0x00,0x01,/*1 ARP请求*/
		0x00,0x0c,0x29,0x79,0xf9,0x7f,/*源mac地址 根据自己修改 ubuntu_mac*/
		192,168,0,111,/*源IP 根据自己修改 ubuntu_ip*/
		0x00,0x00,0x00,0x00,0x00,0x00,/*目的mac地址*/
		192,168,0,110/*目的IP*/
#endif
	//msg存放arp请求报文
    // unsigned 
	char msg[256];
    sprintf(msg,
    "0xff,0xff,0xff,0xff,0xff,0xff,%s,0x08,0x06,0x00,0x01,0x08,0x00,6,4,0x00,0x01,%s,%s,0x00,0x00,0x00,0x00,0x00,0x00,%s",soure_mac,soure_mac,soure_ip,goal_ip);
	printf("\n---------msg=>%s\n",msg);
	// 创建线程接受arp应答
	pthread_t tid;
	pthread_create(&tid,NULL, recv_msg, (void *)sockfd);
	
	//发送arp请求帧数据
	// my_sendto(sockfd, "wlan0",msg, 42);
	
	//等待线程结束
	pthread_join(tid, NULL);
	
	close(sockfd);
	return 0;
}

static void * get_arg_mac_task(char *ip){
	// 获取开始时间戳  
	long long int start_time = 0, end_time = 0;  
    clock_t start = clock(); // 使用clock()函数获取当前时间戳（以秒为单位）  
    start_time = start;  
    FILE* fp;  
    //FILE* ping_fp;  
	char line[MAX_LINE]; 
	// char ping_line[MAX_LINE]; 
	char mac_address[32];   










	fp = popen("arp -a", "r");  
    if (fp == NULL) {  
        perror("Failed to run ifconfig command");  
        exit(1);  
    }  
  
    while (fgets(line, sizeof(line), fp) != NULL) {  
		sscanf(line, "%*s%*s%*s%s%*s%*s%*s",mac_address);
		printf("\n----------nmac_address=>%s\n",mac_address);

		usleep(10);
	}
  
    pclose(fp); // 关闭ifconfig命令的输出文件 
	// 获取结束时间戳并计算时间差  
    clock_t end = clock(); // 再次获取当前时间戳  
    end_time = end;  
	 double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC; // 计算经过的时间（以秒为单位）
	// 打印时间戳和时间差  
    printf("Start time: %lld\n", start_time);  
    printf("End time: %lld\n", end_time);  
    printf("Elapsed time: %f seconds\n", elapsed_time);  

	ak_thread_exit();
	return NULL;
}






// void my_sendto(int sockfd, char *out, unsigned char *msg, int msg_len)
// {
// 	//通过ioctl得到网络接口
// 	struct ifreq ethreq;
// 	strncpy(ethreq.ifr_name, out, IFNAMSIZ);
// 	if(-1 == ioctl(sockfd, SIOCGIFINDEX, &ethreq))
// 	{
// 		perror("ioctl");
// 		close(sockfd);
// 		_exit(-1);
// 	}

// 	//帧数据 出去的本地接口
// 	struct sockaddr_ll sll;
// 	bzero(&sll,sizeof(sll));
// 	sll.sll_ifindex = ethreq.ifr_ifindex;
// 	//2、发送组好报文的帧数据
// 	if(sendto(sockfd, msg, msg_len, 0, (struct sockaddr *)&sll, sizeof(sll)) != true){
// 		perror("\n--------send false\n");
// 	}
// }


void get_mac_address_by_ip(char *goal_ip,char *eth){
    char source_mac[128];
    char source_ip[128];
	if(get_local_eth_mac_address(eth,source_mac,source_ip) == true){
	ak_pthread_t mac_task_id;
    ak_thread_create(&mac_task_id, get_arg_mac_task, goal_ip, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	}
}