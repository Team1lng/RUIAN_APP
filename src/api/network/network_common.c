#include "network_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include<netinet/ether.h>
#include <unistd.h>          
#include <netdb.h>           
#include <sys/types.h>        
#include <sys/socket.h>       
//#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>       
#include <bits/ioctls.h>    
#include <net/if.h>           
#include <linux/if_ether.h>   
#include <linux/if_packet.h>  
#include <net/ethernet.h>
#include "ak_thread.h"
#include "ak_mem.h"
#include<net/if.h>
#include<net/if_arp.h>
#include<netinet/in.h>
//#include<netinet/ip.h>
#include<linux/if_ether.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>


#include "../../layout/layout_define.h"


#define NET_COMMON_CMD_START 0XAA
#define NET_COMMON_CMD_END	 0X55

#define ETH_P_CMD   0xFFFF //0x0800//
#define CMD_GLOBAL_IP_ADDR "228.255.255.1"//所有设备组播ip地址
#define CMD_GLOBAL_OUTDOOR_ADDR "229.255.255.1"
#define CMD_PUBLIC_OUTDOOR_ADDR "230.255.255.1"
static bool network_inited = false;
static int cmd_receive_fd = -1;
static int cmd_send_fd = -1;
static ak_mutex_t network_device_mutex;

char device_busy_disapear_times[DEVICE_TOTAL];
bool occupy_resource[DEVICE_TOTAL] = {false};
bool net_online_device[DEVICE_TOTAL] = {false};
bool net_online_global_device[DEVICE_TOTAL] = {false};//此数据结构只为了通过室内机升级门口机程序专用

static ak_mutex_t device_state_mutex ;

//版本信息
static char door1_version[] = "V0.0.0";
static char door2_version[] = "V0.0.0";

char *door1_version_get(void)
{
	return door1_version;
}

void door1_version_set(char *version)
{
    memcpy(door1_version, version, sizeof(door1_version));
}

char *door2_version_get(void)
{
	return door2_version;
}

void door2_version_set(char *version)
{
    memcpy(door2_version, version, sizeof(door2_version));
}

void network_inited_status_set(bool en)
{
	network_inited = en;
}
bool network_inited_status_get()
{
	return network_inited;
}

static network_device network_local_device = DEVICE_UNKONW;
void network_local_device_set(network_device device)
{
	network_local_device = device;
}

network_device network_local_device_get()
{
	return network_local_device;
}

static unsigned int oudoor_call_mask[DEVICE_TOTAL] = {0x00};

void outdoor_call_mask_set(network_device device,unsigned int mask)
{
	oudoor_call_mask[device] = mask;
}

unsigned int outdoor_call_mask_get(network_device device)
{
	return oudoor_call_mask[device] ;
}


static struct sockaddr_ll nework_local_sockaddr_ll;
struct sockaddr_ll* network_get_send_addres(void)
{
	return &nework_local_sockaddr_ll;
}
static bool network_cmd_socket_init(void)
{
#if 0
	//AF_INET  //PF_PACKET             //SOCK_DGRAM  SOCK_RAW
	if((cmd_receive_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CMD))) < 0)
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
    sll.sll_protocol = htons( ETH_P_CMD );
    if( bind( cmd_receive_fd, (struct sockaddr *) &sll, sizeof( sll ) ) == -1 )
    {
        perror( "bind" );
        return( 1 );
    }

    memset( &mr, 0, sizeof( mr ) );
    mr.mr_ifindex = req.ifr_ifindex;
    mr.mr_type    = PACKET_MR_PROMISC;
    if( setsockopt( cmd_receive_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,&mr, sizeof( mr ) ) == -1 )
    {
        perror( "setsockopt" );
        return( 1 );
    }



    int recv_buf_size = 512;
	if (setsockopt(cmd_receive_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(int)) < 0)
	{
		printf("setsockopt SO_RCVBUF\n");
		return false;
	}

	memset (&nework_local_sockaddr_ll, 0, sizeof (nework_local_sockaddr_ll));
	/*网卡eth0的index，非常重要，系统把数据往哪张网卡上发，就靠这个标识*/
	nework_local_sockaddr_ll.sll_ifindex   = req.ifr_ifindex;

#if 0
	/*标识包的类型为发出去的包*/
	nework_local_sockaddr_ll.sll_pkttype   = PACKET_OUTGOING;
	/*目标MAC地址长度为6*/
	nework_local_sockaddr_ll.sll_halen     = 6;    
	/*填写目标MAC地址*/
	nework_local_sockaddr_ll.sll_addr[0]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[1]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[2]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[3]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[4]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[5]   = 0xFF;
#endif
	return true;
#endif	
	
	if((cmd_receive_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}
	if((cmd_send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("create socket error raw_socket_send_aduio_fd\n");
		return false;
	}
	struct sockaddr_in local_addr;
	memset(&local_addr,0,sizeof(local_addr));
	
	local_addr.sin_family=AF_INET;
	local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	local_addr.sin_port=htons(6868);
	
	//设置端口复用                 允许在同一个本地地址和端口上启动套接字的多个实例
	int bOptval = true;
	int ret = setsockopt(cmd_send_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bOptval, sizeof(bOptval));
	if (ret != 0)
	{
		perror("SEND_CMD REUSE failed\n");
	}

	ret = setsockopt(cmd_receive_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bOptval, sizeof(bOptval));
	if (ret != 0)
	{
		perror("RECEIVE_CMD REUSE failed\n");
	}
	
	ret=bind(cmd_receive_fd,(struct sockaddr*)&local_addr,sizeof(local_addr));
	if(ret <0)
	{
		perror("cmd receice socked bind failed\n");
	}

	//加入当前户专属的组播
	struct ip_mreq mreq; // 多播地址结构体
	char group_ip[128];
	sprintf(group_ip,"228.%u.%u.1",user_data_get()->other.family_id >> 8,user_data_get()->other.family_id & 0xff );
	mreq.imr_multiaddr.s_addr=inet_addr(group_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	//允许套接字加入到一个特定的多播组中，从而能够接收发送到这个多播组的数据包。
	setsockopt(cmd_receive_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)); 

	//加入全局组播
	mreq.imr_multiaddr.s_addr=inet_addr(CMD_GLOBAL_IP_ADDR);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	setsockopt(cmd_receive_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));
	
	//设置数据是否发送到本地回环接口
	int loop = 0;
	if(setsockopt(cmd_send_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop))<0)
	{
			perror("IP_MULTICAST_LOOP");
			return false;
    }
	//指定哪张网卡发送
	struct ifreq ifr;
	memset(&ifr, 0x00, sizeof(ifr));
	strncpy(ifr.ifr_name, NETWORK_NAME, strlen(NETWORK_NAME));
	setsockopt(cmd_send_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));

	memset(&ifr, 0x00, sizeof(ifr));
	strncpy(ifr.ifr_name, NETWORK_NAME, strlen(NETWORK_NAME));
	setsockopt(cmd_receive_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
	return true;
}

static bool network_cmd_socket_close(void)
{
	if((cmd_send_fd != -1)||(cmd_receive_fd != -1))
	{
		close(cmd_send_fd);
		cmd_send_fd = -1;
		close(cmd_receive_fd);
		cmd_receive_fd = -1;
	}
	return true;
}
static void network_device_init(network_device device)
{
	printf("=============>>local device:%d \n", device);
	// printf("=============>>local device:%d \n", device);
	// printf("=============>>local device:%d \n", device);
	network_local_device = device;
	if ((network_local_device == DEVICE_INDOOR_ID1) ||
		(network_local_device == DEVICE_INDOOR_ID2) ||
		(network_local_device == DEVICE_INDOOR_ID3) ||
		(network_local_device == DEVICE_INDOOR_ID4) ||
		(network_local_device == DEVICE_INDOOR_ID5) ||
		(network_local_device == DEVICE_INDOOR_ID6))
	{
		//system("1 >/proc/sys/net/ipv4/ip_forward");

		system("ip link del name "NETWORK_NAME" type bridge");

		system("ip link add name " NETWORK_NAME " type bridge");

		system("ip link set " NETWORK_NAME " up");

		system("ip link set dev eth0 master " NETWORK_NAME);

		system("ip link set dev eth1 master " NETWORK_NAME);

		system("ifconfig eth0 0.0.0.0 promisc");

		system("ifconfig eth1 0.0.0.0 promisc");

		system("ip addr flush dev br0");     
		//A类地址范围1.0.0.0 - 127.255.255.255
		char str_ip[64] = {0};
		sprintf(str_ip,"108.%d.%d.%d",user_data_get()->other.family_id >> 8 &0xff ,user_data_get()->other.family_id & 0xff,network_local_device);
	
		char buffer[128] = {0};
		sprintf(buffer, "ip addr add dev %s %s/8", NETWORK_NAME, str_ip);
		system(buffer);

		char cmd[256] = {0};
		sprintf(cmd, "ip route add 108.0.0.0/8 via 108.0.0.1 dev br0");
		system(cmd);
	
		
	}
	else if((network_local_device == DEVICE_UNIT_OUTDOOR_1) || (network_local_device == DEVICE_UNIT_OUTDOOR_2))
	{
		
		char str_ip[64] = {0};
		char buffer[128] = {0};
		sprintf(str_ip,"108.%d.%d.255",user_data_get()->other.family_id >> 8 & 0xff,user_data_get()->other.family_id & 0xff);
		sprintf(buffer, "ifconfig eth0 %s netmask 255.0.0.0", str_ip);
		system(buffer);

	}
	else
	{
		char buffer[128] = {0};
		char str_ip[64] = {0};
		sprintf(str_ip,"108.255.255.%d",network_local_device);

		sprintf(buffer, "ifconfig eth0 %s netmask 255.0.0.0", str_ip);

		system(buffer);

	}
	system("route add -net 224.0.0.0 netmask 224.0.0.0 " NETWORK_NAME);

	network_cmd_socket_init(); //套接字初始化

	memset(net_online_device, 0, sizeof(network_local_device));
	net_online_device[network_local_device] = true;
}






static bool net_common_code_check_valid(const char *buffer)
{
	/*先判断起始码和结束码是否一致*/
	if ((buffer[0] != NET_COMMON_CMD_START) || (buffer[COMMON_CMD_LEN - 1] != NET_COMMON_CMD_END))
	{
		return false;
	}

	/*判断校验和是否正确*/
	unsigned char num = (buffer[0] + buffer[1] + buffer[2] + buffer[3])&0xFF;
	if (num != buffer[COMMON_CMD_LEN - 2])
	{
		return false;
	}
	// /*判断设备是否正确*/
	// if(buffer[3] != NET_COMMON_CMD_ID_REPEAT)
	// {

		if ((buffer[2] != DEVICE_ALL))
		{

			unsigned int family_id = buffer[12] << 24 | buffer[13] << 16 | buffer[14] << 8 | buffer[15];
			if((family_id != user_data_get()->other.family_id))
			{
				return false;
			}
			else
			{
				if(buffer[2] != DEVICE_GROUP)
				{
					if(buffer[2] != network_local_device)
					{
						return false;
					}

				}
			}
		}
	// if ((buffer[2] != DEVICE_ALL) || (buffer[2] != DEVICE_PUBLIC_OUTDOOR) || (buffer[2] != DEVICE_GROUP_OUTDOOR) || (buffer[2] != DEVICE_GROUP) )
	// {
	// 	if(buffer[2] != network_local_device)
	// 	{
	// 		return false;
	// 	}
	// }
	// }

	// /*判断设备是否冲突*/
	// if (buffer[2] == buffer[1])
	// {
	// 	; // printf("device id repeat ID%d\n",buffer[2]);
	// }
	return true;
}

static bool net_common_door_version_check_valid(const char *buffer)
{
	/*先判断起始码和结束码是否一致*/
	if ((buffer[0] != NET_COMMON_CMD_START) || (buffer[27] != NET_COMMON_CMD_END))
	{
		return false;
	}

	/*判断校验和是否正确*/
	unsigned char num = (buffer[0] + buffer[1] + buffer[2] + buffer[3])&0xFF;
	if (num != buffer[26])
	{
		return false;
	}

		if ((buffer[2] != DEVICE_ALL))
		{

			unsigned int family_id = buffer[12] << 24 | buffer[13] << 16 | buffer[14] << 8 | buffer[15];
			if((family_id != user_data_get()->other.family_id))
			{
				return false;
			}
			else
			{
				if(buffer[2] != DEVICE_GROUP)
				{
					if(buffer[2] != network_local_device)
					{
						return false;
					}

				}
			}
		}
	return true;
}




typedef struct
{
	const char* str;
	unsigned char cmd;
	void(*proc)(network_device device,unsigned int family_id,unsigned int arg1,unsigned int arg2);

	bool log_open;
}net_common_event_info;

static int device_times[DEVICE_TOTAL] = {0};
static int global_device_times[DEVICE_TOTAL] = {0};
static void net_common_deivce_repeat_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	/*
	*	arg1: 1:查询ID状态
	*
	*		   
	*
	*	device:表示冲突的ID号
	*					
	*/
	if(arg1 == 1)
	{
		if((device == network_local_device)&&((user_data_get()->other.family_id) == arg2))//ID冲突
		{
			extern bool device_id_repeat_push(char device);
			device_id_repeat_push(device);
		}
		else
		{
			ak_thread_mutex_lock(&device_state_mutex);
			net_online_global_device[device]= true;
			global_device_times[device] = 0;
			if((device >= DEVICE_INDOOR_ID1) &&(device <=DEVICE_UNIT_OUTDOOR_2))
			{
				if(user_data_get()->other.family_id == arg2)
				{
					if((device == DEVICE_UNIT_OUTDOOR_1) || (device == DEVICE_UNIT_OUTDOOR_2))
					{
						if(net_online_device[device] == false)
						{
							MONITOR_CH ch = get_channel_by_outdoor_device(device);
							set_tuya_channel_state(ch, true);	//使能涂鸦通道的状态
						}
					}
					if(net_online_device[device] == false)
					{
						outdoor_status_change_event_push(0,0);
					}
					net_online_device[device] = true;
					device_times[device] = 0;
				}
				
			}
			else if((device >= DEVICE_OUTDOOR_0) && (device <= DEVICE_OUTDOOR_15))
			{
				if(net_online_device[device] == false)
				{
					MONITOR_CH ch = get_channel_by_outdoor_device(device);
					set_tuya_channel_state(ch, true);	//使能涂鸦通道的状态
				}
				if(net_online_device[device] == false)
				{
					outdoor_status_change_event_push(0,0);
				}
				net_online_device[device] = true;
				device_times[device] = 0;
			}
			ak_thread_mutex_unlock(&device_state_mutex);
		}
	

	}
}

static void net_common_outdoor_call_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	// goto_layout(pLAYOUT(close));
	printf("receivr outdoor_call_arg2 is %d\n",arg2);
	printf("device: %d, family_id: %d, arg1: %d\n",device, family_id, arg1);
	extern bool outdoor_call_event_push(char,int);
	outdoor_call_event_push(device,arg2);

}

static void net_common_outdoor_talk_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	/*
	* 室内机接收户外机通话前，需要知道是否已经有设备正在通话
	*
	* 如果没有，则发送到主线程（如果在监控页面，需要退出监控）
	*
	* 如果设备正在通话，则通知设备，设备正忙。
	*/
//	MONITOR_CH monitor_ch = monitor_channel_get();
	MONITOR_CH talk_ch = (MONITOR_CH)arg1;
	if((talk_ch < MON_CH_UNIT_DOOR_1)&&(talk_ch > MON_CH_DOOR_15 ))
	{
		printf("out door talk Parameter error %d \n",arg1);
		return ;
	}
	network_device outdoor_device =  get_outdoor_device_by_channel(talk_ch);

	extern bool indoor_cmd_event_push(unsigned long arg1,unsigned long arg2);
	unsigned long arg = (arg2 << 8)|(outdoor_device);
	indoor_cmd_event_push(1<<8,arg);
}

static void net_common_interphone_call_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	extern void interphone_call_event_push(unsigned int,unsigned int);
	interphone_call_event_push(arg1,arg2);
}

static void net_common_stream_status_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	MONITOR_CH ch = monitor_channel_get();
	if((ch >=MON_CH_UNIT_DOOR_1) && (ch <= MON_CH_DOOR_15))
	{
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_STREAM_STATUS;
		data.arg1 = 0;
		data.arg2 = 0;			
		data.device = get_outdoor_device_by_channel(ch);
		network_send_cmd_data(&data);
	}
	
}
static void net_common_device_busy_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	extern bool indoor_cmd_event_push(unsigned long arg1,unsigned long arg2);
	unsigned long arg2_1 = ((device&0xFF)<<8)|(arg2);
	indoor_cmd_event_push((2<<8)|arg1,arg2_1);
}

extern bool network_upgrade_sent_package_close(void);
extern bool network_upgrade_send_package_open(void);


static lv_task_t * common_upgrade_ptask = NULL;

static void common_upgradeover_task(struct _lv_task_t *task_t){
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 77);
		if(cont != NULL){
			
			lv_obj_t* bar = lv_obj_get_child_form_id(lv_obj_get_child(cont, NULL), 1087);
			lv_bar_set_value(bar, 100, LV_ANIM_OFF);
			// lv_obj_del(bar);
			
			lv_obj_t *obj = (lv_obj_t *)cont->user_data;
			
	
			if(lv_obj_get_child_form_id(obj->parent, 1) != NULL)
				lv_obj_del(lv_obj_get_child_form_id(obj->parent, 1));

			// char *src[language_total] = {"Outdoor Upgrade over!!!","室外机升级完成!!!","Наружное обновление завершено!!!","Mise à niveau extérieure sur!!!","Actualización al aire libre!!!","Outdoor-Upgrade vorbei!!!","!!!الانتهاء من ترقية القطعة الخارجية ","Aktualizace je dokončena","שדרוג יחידה חיצונית הסתיים","Actualização da Unidade Exterior Terminada","Upgrade all'aperto terminato"};
			
			lv_label_set_text(obj, str_get(LAYOUT_NETWORKCOMMON_LANG_OUTDOORUPGRADEOVER_ID));
			lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 100);
			ak_sleep_ms(1500);
			lv_obj_del(cont);
			standby_timer_reset();
			if (common_upgrade_ptask != NULL) {
				lv_task_del(common_upgrade_ptask);
				common_upgrade_ptask = NULL;
			}
		}

}


void common_upgrade_task_cb(struct _lv_task_t *task_t){
	if (common_upgrade_ptask != NULL) {
		lv_task_del(common_upgrade_ptask);
		common_upgrade_ptask = NULL;
	}

}



//arg1 == 1 升级完成 arg2 == 2 失败 重新发送
static void net_common_upgrade_reset_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int  arg2)
{
	if(arg1 == 2){
		printf("outdoor upgrade over!!!\n");
		if (common_upgrade_ptask != NULL) {
			lv_task_del(common_upgrade_ptask);
		}
		//hlf
		common_upgrade_ptask= lv_task_create(common_upgradeover_task, 30000, LV_TASK_PRIO_MID, NULL);
		
		
	}else if(arg1 == 3){//升级失败 重新发送
		printf("outdoor recive error sent again!!!\n");

		network_upgrade_sent_package_close();//先关闭
		lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 77);
		if(cont == NULL)
			return ;
		lv_obj_t *bar = lv_obj_get_child_form_id(cont, 1087);
		if(bar == NULL)
			return ;
		lv_obj_del(bar);
		
		network_cmd_data_init(data);
		data.cmd			= NET_COMON_CMD_UPGRADE_OUTDOOR;
		data.arg1			= 2;
		data.arg2			= 2;
		data.device 		= DEVICE_ALL;
		network_send_cmd_data(&data);
		
		ak_sleep_ms(200);
		
		network_upgrade_send_package_open();//重新发送
	}

}

static void net_common_motion_detect_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	if(arg1 == 0){
		//printf("recive motion cmd!!!\n");
		if(is_sdcard_insert() == false || user_data_get()->motion.motion == false)
		{
			return ;
		}
		extern bool outdoor_motion_event_push(char arg1,char arg2);
		outdoor_motion_event_push(device,0);
	}

}

static int times_falg = 0;
static int times_year = 0;
static int times_mon = 0;
static int times_day = 0;
static int times_hour = 0;
static int times_min = 0;
static int times_sec = 0;





static void net_common_time_lock_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2){

	if(arg1 < 0 || arg1 > 5)
		return ;
	times_falg++;
	if(arg1 == 0){
		
		times_year = arg2 + 2000;
		if(times_falg == 6)
		{
			standby_timer_close();
			char date_str[64] = {0};
			sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",times_year,times_mon,times_day,times_hour,times_min,times_sec);
			system(date_str);
			system("hwclock -w");
			times_falg = 0;
			standby_timer_open(-1,NULL);
		}
	}else if(arg1 == 1){
		
		times_mon = arg2;
		if(times_falg == 6){
			standby_timer_close();
			char date_str[64] = {0};
			sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",times_year,times_mon,times_day,times_hour,times_min,times_sec);
			system(date_str);
			system("hwclock -w");
			times_falg = 0;
			standby_timer_open(-1,NULL);
		}
	}else if(arg1 == 2){
		
		times_day = arg2;
		if(times_falg == 6){
			standby_timer_close();
			char date_str[64] = {0};
			sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",times_year,times_mon,times_day,times_hour,times_min,times_sec);
			system(date_str);
			system("hwclock -w");
			times_falg = 0;
			standby_timer_open(-1,NULL);
		}
	}else if(arg1 == 3){
	
		times_hour = arg2;
		if(times_falg == 6){
			standby_timer_close();
			char date_str[64] = {0};
			sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",times_year,times_mon,times_day,times_hour,times_min,times_sec);
			system(date_str);
			system("hwclock -w");
			times_falg = 0;
			standby_timer_open(-1,NULL);
		}
	}else if(arg1 == 4){
		
		times_min = arg2;
		if(times_falg == 6){
			standby_timer_close();
			char date_str[64] = {0};
			sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",times_year,times_mon,times_day,times_hour,times_min,times_sec);
			system(date_str);
			system("hwclock -w");
			times_falg = 0;
			standby_timer_open(-1,NULL);
		}
	}else if(arg1 == 5){
		times_sec = arg2;
		if(times_falg == 6){
			standby_timer_close();
			char date_str[64] = {0};
			sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",times_year,times_mon,times_day,times_hour,times_min,times_sec);
			system(date_str);
			system("hwclock -w");
			times_falg = 0;
			standby_timer_open(-1,NULL);
		}
	}
}

/************************************************************
* @Description: 门口机开锁情况上报涂鸦
* @Author: xiaoxiao
* @Date: 2022-12-14 11:52:49
* @param: 
* @explain: 
************************************************************/
static void net_common_lock_func(network_device device,unsigned family_id,unsigned int arg1,unsigned int arg2)
{
	if(((device >= DEVICE_UNIT_OUTDOOR_1) || (device <= DEVICE_OUTDOOR_15)) && (monitor_channel_get() == get_channel_by_outdoor_device(device)))
	{
		if(arg1 == 0)
		{
			if(arg2 == 0)
			{
				tuya_dp_148_response_accessory_lock(false);
			}
			else if(arg2 == 1)
			{
				tuya_dp_148_response_accessory_lock(true);
			}

		}
		else if(arg1 == 1)
		{
			if(arg2 == 0)
			{
				tuya_dp_239_response_accessory_lock(false);
			}
			else if(arg2 == 1)
			{
				tuya_dp_239_response_accessory_lock(true);
			}		
		}
	}
}

static void net_common_secondary_confirm_device_func(network_device device, unsigned family_id,unsigned int arg1, unsigned int arg2)
{
	int status = device_confirm_status_get();
	if(arg1 == 0)
	{
		if(status == 0)
		{
			device_confirm_status_set(1);
		}
	}
	else if(arg1 == 1)
	{
		if(status == 1)
		{
			device_confirm_status_set(2);
		}
	}

}

/************************************************************
** 函数说明: 繁忙查询(已废弃)
** 作者: xiaoxiao
** 日期: 2023-03-17 16:20:02
** 参数说明: 
** 注意事项: 
************************************************************/
static void net_common_data_busy_ack_func(network_device device, unsigned family_id,unsigned int arg1, unsigned int arg2)
{
	unsigned int mask = outdoor_call_mask_get(arg1);
	mask |= (0x01<<device);
	outdoor_call_mask_set(arg1, mask);
	monitor_data_busy_enable(arg1,true);
	
}
/************************************************************
* @Description: 数据繁忙释放信号处理
* @Author: xiaoxiao
* @Date: 2022-12-02 17:32:36
* @param: device:发送此信号的门口机ID  arg1：代表门口机序号
* @explain: 多个室内机同时抢占arg1门口机资源，同步记录当前抢占arg1门口机资源的掩码
************************************************************/
static void net_common_data_busy_release_func(network_device device,unsigned family_id, unsigned int arg1, unsigned int arg2)
{
	unsigned int mask = outdoor_call_mask_get(arg1);
	mask &= ~(0x01 << device);
	outdoor_call_mask_set(arg1,mask);

}

/************************************************************
* @Description: 释放数据信号处理
* @Author: xiaoxiao
* @Date: 2022-12-02 17:30:30
* @param: arg1 代表门口机序号
* @explain: 收到此信号把门口机占用标志清0
************************************************************/
static void net_common_data_release_func(network_device device, unsigned family_id,unsigned int arg1, unsigned int arg2)
{
	outdoor_call_mask_set(arg1,0x00);
	monitor_data_busy_enable(arg1,false);
}

/************************************************************
* @Description: 接收室内机发送的资源繁忙信号
* @Author: xiaoxiao
* @Date: 2022-12-02 08:58:47
* @explain: 此信号是全局信号，跨户号，二次确认机繁忙信号需要判断是否发送给自己的，公共大厅机资源则不需要
************************************************************/
static void net_common_data_busy_func(network_device device, unsigned family_id,unsigned int arg1, unsigned int arg2)
{
	network_device outdoor_device = (network_device)arg1;
	// if((outdoor_device == DEVICE_UNIT_OUTDOOR_1 ) || (outdoor_device == DEVICE_UNIT_OUTDOOR_2))
	// {
	// 	if(arg2 != user_data_get()->other.family_id)//如果抢占的资源时二次确认机，则只有同户的室内机才处理此信号
	// 	{
	// 		return ;
	// 	}
	// }
	// unsigned int mask = outdoor_call_mask_get(outdoor_device);
	// mask |= 0x01 << device;
	// outdoor_call_mask_set(outdoor_device,mask );
	device_busy_disapear_times[outdoor_device] = 0;
	outdoor_status_change_event_push(0,0);
	monitor_data_busy_enable(outdoor_device,true);
}


static void net_common_unlock_time_set_func(network_device device,unsigned int family_id , unsigned int arg1, unsigned int arg2)
{
	if(user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		return ;
	}
	if(arg1 == 0)
	{
		if(arg2 == 1)
		{
			network_cmd_data data;
			data.family_id 		= family_id;
			data.cmd			= NET_COMMON_CMD_UNLOCK_TIME;
			data.arg1			= 1;
			data.arg2			= user_data_get()->door1_delay;
			data.device 		= device;
			network_send_cmd_data(&data);
		}
		else if(arg2 == 2)
		{
			network_cmd_data data;
			data.family_id 		= family_id;
			data.cmd			= NET_COMMON_CMD_UNLOCK_TIME;
			data.arg1			= 2;
			data.arg2			= user_data_get()->door2_delay;
			data.device 		= device;
			network_send_cmd_data(&data);
		}
	
	}

}


static void net_common_mode_sync_func(network_device device,unsigned int family_id , unsigned int arg1, unsigned int arg2)
{
	if((arg1 > 0) || (arg1 < 2))
	{
		user_data_get()->other.mode = arg1;
		user_data_save();
		tuya_response_mode_sync();
	}

}
//门口机call机让室内机待机回调函数
static void net_common_indoor_standby_func(network_device device,unsigned int family_id , unsigned int arg1, unsigned int arg2)
{
	if((arg1 < DEVICE_UNIT_OUTDOOR_1 ) || (arg1 > DEVICE_OUTDOOR_15))
	{
		return;
	}
	if(get_channel_by_outdoor_device(arg1) == monitor_channel_get()){
	// usleep(1000*500);
	stanby_event_push(true);
	}
}

//接受网络指令，执行相应的回调
static net_common_event_info net_common_event[]  =
{
	{"device id repeat",			NET_COMMON_CMD_ID_REPEAT,			net_common_deivce_repeat_func,		false},
	{"out door call", 				NET_COMMON_CMD_OUTDOOR_CALL,		net_common_outdoor_call_func,		false},
	{"indoor and outdoor talk", 	NET_COMMON_CMD_OUTDOOR_TALK,		net_common_outdoor_talk_func,		false},
	{"interphone call ", 			NET_COMMON_CMD_INTERCOM_CALL,		net_common_interphone_call_func,	false},
	{"stream status empty",			NET_COMMON_CMD_STREAM_STATUS,		net_common_stream_status_func,		false},
	{"device busy",					NET_COMON_CMD_DEVICE_BUSY,			net_common_device_busy_func,		true},
	{"outdoor upgrade reset",		NET_COMON_CMD_UPGRADE_OUTDOOR,		net_common_upgrade_reset_func,		false},
	{"motion detect",				NET_COMON_CMD_MOTION,				net_common_motion_detect_func,		false},
	{"time lock",               	NET_COMON_CMD_TIME_LOCK,       	 	net_common_time_lock_func,          false},
	{"lock close",            		NET_COMMON_CMD_LOCK_STATE,      	net_common_lock_func,          		false},
	{"secondary_confirm_device ",  	NET_COMMON_CMD_DEVICE_CONFIRM, 		net_common_secondary_confirm_device_func, false},
	{"data busy ack ",  			NET_COMMON_CMD_DATA_BUSY_ACK, 		net_common_data_busy_ack_func, false},
	{"data busy release ",  		NET_COMMON_CMD_DATA_BUSY_RELEASE, 	net_common_data_busy_release_func, false},
	{"data release ",  				NET_COMMON_CMD_DATA_RELEASE, 		net_common_data_release_func, false},
	{"data_busy ", 	 				NET_COMMON_CMD_DATA_BUSY, 			net_common_data_busy_func, false},
	{"unlock time set", 			NET_COMMON_CMD_UNLOCK_TIME,			net_common_unlock_time_set_func,false},
	{"mode_sync", 					NET_COMMON_CMD_MODE_SYNC,			net_common_mode_sync_func,false},
	{"indoor standy", NET_COMMON_CMD_DEVICE_STANDBY,net_common_indoor_standby_func,false},
	
};
static bool net_common_event_process(network_device device,unsigned int family_id ,unsigned char cmd ,unsigned int arg1,unsigned int arg2)
{
	int size = sizeof(net_common_event)/sizeof(net_common_event_info);
	for(int i = 0; i < size ; i++)
	{
		if(net_common_event[i].cmd == cmd)
		{
			if(net_common_event[i].log_open == true)
			{
				//printf("receive cmd %s \n",net_common_event[i].str);
			}
			net_common_event[i].proc(device,family_id,arg1,arg2);

			// printf("receive cmd %s ---device is %d ------arg1 is %d ---arg2 is %d\n",net_common_event[i].str,device,arg1,arg2);

			break;
		}
	}
	return true;
}



static void* network_cmd_receive_task(void* arg)
{
	fd_set readfds; 													
	struct timeval timeout;
	int timeout_count = 0;
	char buffer[512] = {0};
	usleep(2*1000*1000);//开机还没进主界面，门口机call机的话，有概率前面几帧不正常
	while(cmd_receive_fd != -1)
	{
		FD_ZERO(&readfds);										
		FD_SET(cmd_receive_fd, &readfds);									

		timeout.tv_sec = 0;		
		timeout.tv_usec = 5000;
		int ret_select = select(cmd_receive_fd+1, &readfds, NULL, NULL, &timeout);
		timeout_count++;
		if (ret_select > 0)
		{

			if (FD_ISSET(cmd_receive_fd, &readfds))									
			{

                //int read_len = recv(cmd_receive_fd, buffer, sizeof(buffer), 0);
				
				int read_len = recvfrom(cmd_receive_fd, buffer, sizeof(buffer), 0, NULL, NULL);

				if((read_len == 18)&&(net_common_code_check_valid(&buffer[0])))
				{

						unsigned int arg1 = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
						unsigned int arg2 = buffer[8] << 24 | buffer[9] << 16 | buffer[10] << 8 | buffer[11];
						unsigned int family_id = buffer[12] << 24 | buffer[13] << 16 | buffer[14] << 8 | buffer[15];
					//timeout_count = 0;
					net_common_event_process((network_device)buffer[1],family_id,buffer[3],arg1 ,arg2);
				}
				else if((read_len == 28)&&(net_common_door_version_check_valid(&buffer[0])))
				{
					char door_version[10] = {0};
					memcpy(&door_version[0], &buffer[16], 10);
					// printf("read_len == 28, door_version:%s\n", door_version);

					if(buffer[7] == 1)
					{
						// printf("===========door1_version_set\n");
						door1_version_set(door_version);
					}
					else if(buffer[7] == 0)
					{
						// printf("===========door2_version_set\n");
						door2_version_set(door_version);
					}
				}
				
			}
		}
// 		else
// 		{
// 			printf("===============no cmd\n");
// 			network_cmd_data_init(data);
// 			if(network_local_device_get() == DEVICE_INDOOR_ID1){
// 			data.device = DEVICE_GROUP;
// 			data.cmd = NET_COMMON_CMD_SET_LANGUAGE;
// 			data.arg1 = user_data_get()->user_language;
// 			data.arg2 = user_data_get()->other.family_id;
// 			network_send_cmd_data(&data);

// 			data.device = DEVICE_PUBLIC_OUTDOOR;
// 			network_send_cmd_data(&data);

// }
		// }

		if(timeout_count > 200)
		{
			timeout_count = 0;
			/*每各一秒查询ID状态*/
			network_cmd_data_init(data);
			data.device = DEVICE_GROUP;
			data.cmd = NET_COMMON_CMD_ID_REPEAT;
			data.arg1 = 1;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);

if(network_local_device_get() == DEVICE_INDOOR_ID1){
			data.device = DEVICE_GROUP;
			data.cmd = NET_COMMON_CMD_SET_LANGUAGE;
			data.arg1 = user_data_get()->user_language;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);

			data.device = DEVICE_PUBLIC_OUTDOOR;
			network_send_cmd_data(&data);

}


			
			ak_thread_mutex_lock(&device_state_mutex);
			for(int i = 1;i<24;i++){
				
				if(i == network_local_device)
					continue ;
				
				++(device_times[i]);
				++(global_device_times[i]);
				if(device_times[i] >= 5)
				{
					if((i >= DEVICE_UNIT_OUTDOOR_1) && (i <= DEVICE_OUTDOOR_15))
					{
						if(net_online_device[i] == true)
						{
							MONITOR_CH ch = get_channel_by_outdoor_device(i);
							set_tuya_channel_state(ch, false);	//使能涂鸦通道的状态
						}
					}
					if(net_online_device[i] == true)
					{
						outdoor_status_change_event_push(0,0);
					}
					net_online_device[i] = false;					
					device_times[i] = 0;
				}
				if(global_device_times[i] >= 2)
				{					
					net_online_global_device[i] = false;
					global_device_times[i] = 0;
				}
			}

			

			ak_thread_mutex_unlock(&device_state_mutex);
			
		//	system("sync");

			//system("echo 3 > /proc/sys/vm/drop_caches");
			//system("free");
		}
        ak_sleep_ms(1);
	}

	ak_thread_exit();
	return NULL;
}


static bool device_oneself_trigger() //防止第一次上电，网卡工作异常，发送一个空数据包给自己刺激正常工作
{
	if((cmd_send_fd == -1) || cmd_receive_fd == -1)
	{
				
		network_cmd_data_init(data);
		for(int i = 0; i < 10 ; i++)
		{
			data.device = network_local_device;
			data.cmd = 0XFF;
			data.arg1 = 3; //升级失败
			data.arg2 = 1;
			network_send_cmd_data(&data);
		}
		return true;
	}
	return false;
}


//网络初始化
bool network_init(network_device device)
{
	network_inited = false;
	ak_thread_mutex_init(&network_device_mutex,NULL);
	ak_thread_mutex_init(&device_state_mutex,NULL);
	network_device_init(device);
	device_oneself_trigger();
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_cmd_receive_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	network_inited = true;
	return true;
}


//网络重初始化
bool network_init_restart(network_device device)
{
	network_cmd_socket_close();
	network_init(device);
	return true;
}

/************************************************************
* @Description: 没有用到包头来做校验
* @Author: xiaoxiao
* @Date: 2022-12-08 11:22:08
* @param: 
* @explain: 
************************************************************/
const char* nework_get_package_head(int type)
{
	static char* mac_local = NULL;
	if(mac_local == NULL)
	{
	    struct ifreq req;
		int fd = socket(PF_INET,SOCK_DGRAM,0);
		strcpy(req.ifr_name,NETWORK_NAME);
		ioctl(fd,SIOCGIFHWADDR,&req);
		close(fd);

		mac_local = ak_mem_alloc(MODULE_ID_APP, 60);
		memset(mac_local,0,60);
	#if 1
		//设置目的网卡地址
		mac_local[0] = 0x01;
	    mac_local[1] = 0x01;
	    mac_local[2] = 0x01;
	    mac_local[3] = 0x01;
	    mac_local[4] = 0x01;
	    mac_local[5] = 0x01; 
	#endif
		memcpy(&mac_local[6], req.ifr_hwaddr.sa_data, 6);
		//mac_local[12] = 0x88;
		
	}
	mac_local[12] =type/256;
	mac_local[13] =type%256;
	return mac_local;
}

/************************************************************
* @Description: 
* @Author: xiaoxiao
* @Date: 2022-11-12 08:34:28
* @LastEditTime: 2022-11-12
* @explain: 命令封装
************************************************************/
static void network_cmd_code_get(network_cmd_data*src ,char*dst)
{
	network_cmd_data* device = (network_cmd_data*)src;
	
	dst[0] = NET_COMMON_CMD_START;
	dst[1] = network_local_device;
	dst[2] = device->device;
	dst[3] = device->cmd;
	dst[4] = (device->arg1 >> 24) & 0xFF;
	dst[5] = (device->arg1 >> 16) & 0xFF;
	dst[6] = (device->arg1 >> 8) & 0xFF;
	dst[7] = device->arg1 & 0xFF;

	dst[8] = (device->arg2 >> 24) & 0xFF;
	dst[9] = (device->arg2 >> 16) & 0xFF;
	dst[10] = (device->arg2 >> 8) & 0xFF;
	dst[11] = device->arg2 & 0xFF;

	dst[12] = (device->family_id >> 24) & 0xFF;
	dst[13] = (device->family_id >> 16) & 0xFF;
	dst[14] = (device->family_id >> 8) & 0xFF;
	dst[15] = device->family_id & 0xFF;
	
	dst[16] = (dst[0] + dst[1] + dst[2] + dst[3])&0xFF;
	dst[17] = NET_COMMON_CMD_END;
}


//发送命令
bool network_send_cmd_data(network_cmd_data* data)
{
	char serv_ip[128];
	if(data->device == DEVICE_ALL)//发送到所有设备的组播地址
	{
		sprintf(serv_ip,CMD_GLOBAL_IP_ADDR);
	}
	else if(data->device == DEVICE_GROUP)//发送cmd到门口机的组播地址
	{
		sprintf(serv_ip,"228.%u.%u.1",user_data_get()->other.family_id >> 8 & 0xff,user_data_get()->other.family_id & 0xff );
	}
	else if(data->device == DEVICE_GROUP_OUTDOOR)//查找户外机组播地址
	{
		sprintf(serv_ip,CMD_GLOBAL_OUTDOOR_ADDR);
	}
	else if(data->device == DEVICE_PUBLIC_OUTDOOR)
	{
		sprintf(serv_ip,CMD_PUBLIC_OUTDOOR_ADDR);
	}
	//点播异常，采用组播方式
	else if((data->device >= DEVICE_INDOOR_ID1) && (data->device <= DEVICE_UNIT_OUTDOOR_2))//室内机分机组播地址
	{
		////hlf:user_data_get()->other.family_id -> data->family_id
		sprintf(serv_ip,"228.%u.%u.1",data->family_id >> 8 & 0xff,data->family_id & 0xff );

	}else if((data->device >= DEVICE_OUTDOOR_0) && (data->device <= DEVICE_OUTDOOR_15))//大厅机组播地址
	{
		sprintf(serv_ip,CMD_PUBLIC_OUTDOOR_ADDR);
	}
	
	// printf("\n\n--------udp目的设备-------------------%d---------------------------\n\n",data->device);
	// printf("\n\n--------udp目的cmd-------------------%x---------------------------\n\n",data->cmd);
	// printf("\n\n--------udp目的地址-------------------%s---------------------------\n\n",serv_ip);

	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
	serv_addr.sin_port = htons(6868);
	int addr_len = sizeof(struct sockaddr_in);
	char package[18];
	//memcpy(package,nework_get_package_head(ETH_P_CMD),60);
	network_cmd_code_get(data,&package[0]);//完成封包

	ak_thread_mutex_lock(&network_device_mutex);
	if(sendto(cmd_send_fd, package,sizeof(package), 0,(struct sockaddr_in *)&serv_addr ,addr_len) < 0)
	{
		perror("sendto fail ");
	}
	ak_thread_mutex_unlock(&network_device_mutex);
	ak_sleep_ms(1);
	return true;
}


static void network_upgradecmd_code_get(network_upgradecmd_data*src ,char*dst)
{
	network_upgradecmd_data* device = (network_upgradecmd_data*)src;
	
	dst[0] = NET_COMMON_CMD_START;
	dst[1] = network_local_device;
	dst[2] = device->device;
	dst[3] = device->cmd;
	
	
	dst[4] = (device->arg1 >> 24) & 0xFF;
	dst[5] = (device->arg1 >> 16) & 0xFF;
	dst[6] = (device->arg1 >> 8) & 0xFF;
	dst[7] = device->arg1 & 0xFF;

	dst[8] = (device->arg2 >> 24) & 0xFF;
	dst[9] = (device->arg2 >> 16) & 0xFF;
	dst[10] = (device->arg2 >> 8) & 0xFF;
	dst[11] = device->arg2 & 0xFF;

	dst[12] = (device->family_id >> 24) & 0xFF;
	dst[13] = (device->family_id >> 16) & 0xFF;
	dst[14] = (device->family_id >> 8) & 0xFF;
	dst[15] = device->family_id & 0xFF;
	memcpy(&dst[16],device->buf,1024);
	
	dst[1040] = NET_COMMON_CMD_END;
}


bool network_sendupgrade_cmd_data(network_upgradecmd_data* data)
{
	char package[1041] = {0};
	network_upgradecmd_code_get(data,&package[0]);//完成封包
	char serv_ip[128];
	if(data->device == DEVICE_ALL)
	{
		printf("cmd all\n");
		sprintf(serv_ip,CMD_GLOBAL_IP_ADDR);
	}
	else if(data->device == DEVICE_GROUP)
	{
		sprintf(serv_ip,"228.%u.%u.1",user_data_get()->other.family_id >> 8 & 0xff,user_data_get()->other.family_id & 0xff );
	}
	else if(data->device == DEVICE_GROUP_OUTDOOR)
	{
		sprintf(serv_ip,CMD_GLOBAL_OUTDOOR_ADDR);
	}
	else if(data->device == DEVICE_PUBLIC_OUTDOOR)
	{
		sprintf(serv_ip,CMD_PUBLIC_OUTDOOR_ADDR);
	}
	else
	{
		unsigned int slave_id = network_device_get_ip(data->device,data->family_id);

		network_common_socket_eth_p_get(2,slave_id,serv_ip);
	}
	printf("device is %d\n",data->device);
	printf("serv_ip is %s\n",serv_ip);
	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
	serv_addr.sin_port = htons(6868);
	int addr_len = sizeof(struct sockaddr_in);
	ak_thread_mutex_lock(&network_device_mutex);
	if(sendto(cmd_send_fd, package,sizeof(package), 0, (struct sockaddr_in *)&serv_addr ,addr_len) < 0)
	{
		perror("sendto fail ");
	}
	ak_thread_mutex_unlock(&network_device_mutex);
	ak_sleep_ms(1);
	return true;
}

int network_common_socket_eth_p_cmd_get(unsigned int slave_id,char *video_eth_ip)
{
	sprintf(video_eth_ip,"108.%u.%u.%u",slave_id >> 24,(slave_id >> 16) & 0xff,(slave_id & 0xffff));
	return 0;
}

//获取音频的结点
int network_common_socket_eth_p_audio_get(unsigned int slave_id, char * audio_eth_ip)
{

	if((slave_id & 0xffff) > (network_device_get_ip(DEVICE_INDOOR_ID6,user_data_get()->other.family_id) & 0xffff))
	{

		sprintf(audio_eth_ip,"226.%u.%u.%u",slave_id >> 24,slave_id >> 16 & 0xff,(slave_id & 0xffff));
	}
	else
	{
		sprintf(audio_eth_ip,"108.%u.%u.%u",slave_id >> 24,slave_id >> 16 & 0xff,(slave_id & 0xffff));
	}
	
	return 0;
}


int network_common_socket_eth_p_video_get(unsigned int slave_id,char *video_eth_ip)
{
	sprintf(video_eth_ip,"227.%u.%u.%u",slave_id >> 24,(slave_id >> 16) & 0xff,(slave_id & 0xffff));
	return 0;
}



int network_common_socket_eth_p_upgrade_get(unsigned int slave_id)
{
#define SOCKET_ETH_P_BASE_UPGRADE 0X36000000
	return SOCKET_ETH_P_BASE_UPGRADE|slave_id;
}


//获取到套接字类别 和对方设备ip
int network_common_socket_eth_p_get(char type,unsigned int slave_id,char *eth_ip)
{
	/*
	*type 1:代表音频，0:代表视频, 2:代表升级包
	*/
	if(type == 1)
	{
		network_common_socket_eth_p_audio_get(slave_id,eth_ip);
	}
	else if(type == 0)
	{
		network_common_socket_eth_p_video_get(slave_id,eth_ip);
	}
	else if(type == 2)
	{
		network_common_socket_eth_p_cmd_get(slave_id,eth_ip);
	}
	return 0;
}

