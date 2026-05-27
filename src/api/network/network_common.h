#ifndef _NETWORK_COMMON_H_
#define _NETWORK_COMMON_H_
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include "../../lvgl/lvgl.h"



#define NETWORK_NAME "br0"

struct net_block_desc
{
    uint32_t version;
    uint32_t offset_to_priv;
    struct tpacket_hdr_v1 h1;
};

struct net_ring_buffer
{
    struct iovec *rd;
    uint8_t *map;
    struct tpacket_req3 req;
};

typedef enum
{
	/* 室内机与门口机通话 */
	NETWORK_EVENT_OUTDOOR_TALK,


}network_event;




#define network_device_get_ip(x,y)  x > DEVICE_UNIT_OUTDOOR_2?((unsigned int)(0xffff << 16)|x):((y << 16)|x)


typedef enum
{
	DEVICE_UNKONW,
	DEVICE_INDOOR_ID1 = 1,
	DEVICE_INDOOR_ID2,
	DEVICE_INDOOR_ID3,
	DEVICE_INDOOR_ID4,
	DEVICE_INDOOR_ID5,
	DEVICE_INDOOR_ID6,
	DEVICE_UNIT_OUTDOOR_1,
	DEVICE_UNIT_OUTDOOR_2,
	DEVICE_OUTDOOR_0,
	DEVICE_OUTDOOR_1,
	DEVICE_OUTDOOR_2,
	DEVICE_OUTDOOR_3,
	DEVICE_OUTDOOR_4,
	DEVICE_OUTDOOR_5,
	DEVICE_OUTDOOR_6,
	DEVICE_OUTDOOR_7,
	DEVICE_OUTDOOR_8,
	DEVICE_OUTDOOR_9,
	DEVICE_OUTDOOR_10,
	DEVICE_OUTDOOR_11,
	DEVICE_OUTDOOR_12,
	DEVICE_OUTDOOR_13,
	DEVICE_OUTDOOR_14,
	DEVICE_OUTDOOR_15,
	DEVICE_CCTV_1,
	DEVICE_CCTV_2,
	DEVICE_CCTV_3,
	DEVICE_CCTV_4,
	DEVICE_CCTV_5,
	DEVICE_CCTV_6,
	DEVICE_CCTV_7,
	DEVICE_CCTV_8,
	DEVICE_PUBLIC_OUTDOOR = 0XFC,
	DEVICE_GROUP_OUTDOOR = 0xFD,
	DEVICE_GROUP = 0XFE,
	DEVICE_ALL = 0XFF,
	DEVICE_TOTAL
}network_device;

typedef enum
{
	NEIGHBOR_DEVICE_UNKONW,
	NEIGHBOR_DEVICE_1 = 1,
	NEIGHBOR_DEVICE_2,
	NEIGHBOR_DEVICE_3,
	NEIGHBOR_DEVICE_4,
	NEIGHBOR_DEVICE_5,
	NEIGHBOR_DEVICE_6,
	NEIGHBOR_DEVICE_7,
	NEIGHBOR_DEVICE_8,
	NEIGHBOR_DEVICE_9,
	NEIGHBOR_DEVICE_10,
	NEIGHBOR_DEVICE_11,
	NEIGHBOR_DEVICE_12,
}network_neighbor_device;


#define network_cmd_data_init(data)\
	network_cmd_data data; \
	data.device = DEVICE_ALL;\
	data.family_id = user_data_get()->other.family_id; \
	data.cmd = 0; \
	data.arg1 =0; \
	data.arg2 = 0;\


#define COMMON_CMD_LEN 18

/*
*	arg1: 1:查询ID状态
*
*		  2:收到ID状态   
*
*			   arg2:最高位1表示id，冲突，低4位表示冲突的ID号
*					最高位0:表示该id存在，但不冲突
*/



#define NET_COMMON_CMD_TALKEND      0x51

#define NET_COMMON_CMD_SOUND        0x52


#define NET_COMMON_CMD_LIGHT        0x53



#define NET_COMMON_CMD_ID_REPEAT     0x55

#define NET_COMMON_CMD_OUTDOOR_CALL  0X56

/*
*
* arg1:当前想通话的通道
*
*/
#define NET_COMMON_CMD_OUTDOOR_TALK  0X57


/*
*	arg1 = 1:呼叫
*   arg1 = 2:有设备应答呼叫
*	arg1 = 3:有设备接受通话
*   arg1 = 4:设备挂断正在通话
*	arg1 = 5:设备正忙
*/
#define NET_COMMON_CMD_INTERCOM_CALL  0X58
#define NET_COMMON_CMD_STREAM_STATUS  0X59


/*
* arg1= 1:代表设备正在与门口机通话 arg2 = 当前通话的通道 
*/
#define NET_COMON_CMD_DEVICE_BUSY   0X60

#define NET_COMON_CMD_UPGRADE_OUTDOOR   0X61

#define NET_COMON_CMD_MOTION   0X62

#define NET_COMON_CMD_MESSACG   0X63

#define NET_COMON_CMD_TIME_LOCK   0X64

#define NET_COMMON_CMD_UNLOCK        0x54

#define NET_COMMON_CMD_LOCK_STATE        0x66
#define NET_COMMON_CMD_LOCK        		0x65
#define NET_COMMON_CMD_UNLOCK_TIME 0x6f

#define NET_COMMON_CMD_DEVICE_CONFIRM  0x67
#define NET_COMMON_CMD_DEVICE_CONFIRM_ANSWER 0x68

#define NET_COMMON_CMD_DATA_BUSY 0x69
#define NET_COMMON_CMD_DATA_BUSY_ACK 0x6a

#define NET_COMMON_CMD_DATA_BUSY_RELEASE 0x6b

#define NET_COMMON_CMD_DATA_RELEASE 0x6c

#define NET_COMMON_CMD_TUYA_MONITOR 0x6d

#define NET_COMMON_CMD_BUSY_QUERY 0x6e


#define NET_COMMON_CMD_MODE_SYNC 0x71

#define NET_COMMON_CMD_KEY_FRAME 0x72

#define NET_COMMON_CMD_DEVICE_STANDBY 0x74

#define NET_COMMON_CMD_SET_LANGUAGE 0x75

#define NET_COMMON_CMD_FLOOR       0x76 //hlf

#define NET_COMMON_CMD_DOOR_VERSION       0x77 //hlf:门口机版本

// #define NET_COMMON_CMD_RESET_OUTDOOR_NETWORK       0x78



typedef struct
{
	network_device device;
	unsigned int family_id;
	char cmd;
	unsigned int arg1;
	unsigned int arg2;
}network_cmd_data;

typedef struct
{
	network_device device;
	unsigned int family_id;
	char cmd;
	int arg1;
	int arg2;
	char buf[1024];
}network_upgradecmd_data;

typedef struct
{
	network_device device;
	unsigned int family_id;
	char cmd;
	int arg1;
	int arg2;
	char buf[10];
}network_door_version_data;


#define netwrok_cmd_create(a,b,c,d) { \
	.device = a,\
	.cmd = b,\
	.arg1 = c,\
	.arg2 = d\
};



#define MONITOR_ENTER_NONE	 0X00
#define MONITOR_ENTER_MANUAL 0x01
#define MONITOR_ENTER_CALL	 0X02
#define MONITOR_ENTER_TUYA   0X03



bool monitor_data_busy_get(network_device device);


void monitor_data_busy_enable(network_device device, bool en);


/************************************************************
* @Description: 网络初始化
* @Author: xiaoxiao
* @Date: 2022-12-12 17:13:44
* @param: 
* @explain: 
************************************************************/
bool network_init(network_device device);

/************************************************************
* @Description: 网络重置
* @Author: xiaoxiao
* @Date: 2022-12-12 17:14:09
* @param: 
* @explain: 
************************************************************/
bool network_init_restart(network_device device);

/************************************************************
* @Description: 设置网络初始话状态
* @Author: xiaoxiao
* @Date: 2022-12-12 17:13:32
* @param: 
* @explain: 
************************************************************/
void network_inited_status_set(bool en);

/************************************************************
* @Description: 获取网络初始化状态
* @Author: xiaoxiao
* @Date: 2022-12-12 17:13:15
* @param: 
* @explain: 
************************************************************/
bool network_inited_status_get();
/************************************************************
* @Description: 设置当前设备的种类
* @Author: xiaoxiao
* @Date: 2022-12-12 17:14:36
* @param: 
* @explain: 
************************************************************/
void network_local_device_set(network_device device);
/************************************************************
* @Description: 获取当前设备的种类
* @Author: xiaoxiao
* @Date: 2022-12-12 17:15:42
* @param: 
* @explain: 
************************************************************/
network_device network_local_device_get();

/************************************************************
* @Description: 普通命令发送函数
* @Author: xiaoxiao
* @Date: 2022-12-12 17:17:26
* @param: 
* @explain: 
************************************************************/
bool network_send_cmd_data(network_cmd_data* package);

/************************************************************
* @Description: 升级命令发送函数
* @Author: xiaoxiao
* @Date: 2022-12-12 17:18:03
* @param: 
* @explain: 
************************************************************/
bool network_sendupgrade_cmd_data(network_upgradecmd_data* package);




bool network_video_send_package_open(unsigned long id);
bool network_video_send_package_push(char type,const char* data ,int len);
bool network_video_send_package_close(void);

bool network_video_receive_package_open(char * ip);
bool network_video_receive_package_close(void);

bool network_audio_send_package_open(char * eth_ip);
bool network_audio_send_package_close(void);
bool network_audio_send_package_push(char type,const char* data ,int len);



bool network_audio_receive_package_open(char * eth_ip);
bool network_audio_receive_package_close(void);

int network_common_socket_eth_p_get(char type,unsigned int slave_id,char * eth_p_id);


void outdoor_call_mask_set(network_device device,unsigned int mask);

unsigned int outdoor_call_mask_get(unsigned int index);




void get_mac_address_by_ip(char *goal_ip,char *eth);




char *door1_version_get(void);
void door1_version_set(char *version);
char *door2_version_get(void);
void door2_version_set(char *version);



#endif

