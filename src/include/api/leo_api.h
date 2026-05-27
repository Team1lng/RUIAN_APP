#ifndef _LEO_API_H_
#define _LEO_API_H_
#include <stdbool.h>
#include "stdlib.h"
#include "sys/time.h"
#include "network_common.h"

static inline unsigned long long get_sys_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}
				

#define REC_MODE_MANUAL  0X01
#define REC_MODE_AUTO	 0X02
#define REC_MODE_MOTION  0X03
#define REC_MODE_ALARM   0X04


typedef struct 
{
	char file_path[128];
	bool has_audio;
    int width;
    int height;
	bool frame_type; //0:h264,1:mjpeg 2:h265
}record_info;


typedef enum
{
	MON_CH_NONE,
	MON_CH_UNIT_DOOR_1,
	MON_CH_UNIT_DOOR_2,
	MON_CH_DOOR_0,	
	MON_CH_DOOR_1,
	MON_CH_DOOR_2,
	MON_CH_DOOR_3,
	MON_CH_DOOR_4,
	MON_CH_DOOR_5,
	MON_CH_DOOR_6,
	MON_CH_DOOR_7,
	MON_CH_DOOR_8,
	MON_CH_DOOR_9,
	MON_CH_DOOR_10,
	MON_CH_DOOR_11,
	MON_CH_DOOR_12,
	MON_CH_DOOR_13,
	MON_CH_DOOR_14,
	MON_CH_DOOR_15,
	MON_CH_CCTV_1,
	MON_CH_CCTV_2,
	MON_CH_CCTV_3,
	MON_CH_CCTV_4,
	MON_CH_CCTV_5,
	MON_CH_CCTV_6,
	MON_CH_CCTV_7,
	MON_CH_CCTV_8,
	MON_CH_TOTAL
}MONITOR_CH;




void leo_api_init(void);

bool standby_timer_open(int timeout,void(*timeout_callback)(void));
bool standby_timer_close(void);







void monitor_channel_set(MONITOR_CH ch);
void monitor_open(void);
void monitor_close(void);
MONITOR_CH monitor_channel_get(void);

/***通过监控通道获取对应的门口机设备号***/
network_device get_outdoor_device_by_channel(MONITOR_CH CH);

/***通过门口机设备号获取对应的监控通道号***/
MONITOR_CH get_channel_by_outdoor_device(network_device device);



bool audio_talk_open(network_device,bool is_mobilephone);
bool audio_talk_close(void);
bool is_audio_talk_open(void);




bool record_pictrue_start(char mode,MONITOR_CH video_channel);


bool record_video_start(char mode,char audio_from,MONITOR_CH video_channel);

bool record_video_stop(char audio_flag);





bool media_thumb_device_open(int width,int height);

bool media_thumb_device_close(void);

bool media_thumb_load(int x,int y,int w,int h, const char* file_path);




bool video_play_open(const char* file);

bool video_play_stop(void);

bool video_play_pause(void);

char video_play_get_status(void); //0:stop 1:working 2:pause

bool video_play_duration_get(int* cur,int *total);

#endif

