#ifndef TUYA_SDK_H
#define TUYA_SDK_H

#include <stdbool.h>
#include  "tuya_cloud_types.h"

#define IPC_APP_PID   "zpcpoemup3qwmz2l"//"srk8f6yojlxql0ko"//"ksmgbrtsbl99h1ju"

#define KOCOM_DP_DOORBELL             (136)
#define KOCOM_DP_LOCK                 (148)
#define KOCOM_DP_LOCK2                (239)
#define KOCOM_DP_PICTURE              (154)
#define KOCOM_DP_ALARM_MSG            (185)
#define KOCOM_DP_SWITCH_CHANNEL       (231)
#define KOCOM_DP_ABSENT_MODE          (232)
#define KOCOM_DP_HOME_MODE            (236)
#define KOCOM_DP_SLEEP_MODE           (237)
#define KOCOM_DP_DEVICE_ACTIVE        (233)
#define KOCOM_DP_ABNORMAL_UNLOCK      (234)
#define KOCOM_DP_ACCESS_LOCK_SUPPORT     (238)

void tuya_ipc_ring_buffer_video_release_data(void);//释放涂鸦队列音视频数据

bool wifi_work_restart(void);//重新连接涂鸦服务器

bool tuya_wifi_sdk_init(const char *pid,bool wifi_enable);//初始化涂鸦库
bool is_online_tuya_cloud(void);


void tuya_current_channel_set(int channel);//设置APP预览通道

void tuya_set_current_language(int language);//设置上传APP的语言

int tuya_get_current_language(void);//

int is_tuya_cloud_connected_num(void);//当前APP连接室内机的数目

int tuya_switch_channel_upload_results(int channel);//设置通道返回的结果

int tuya_channel_valid_report(void);//上传有效通道到APP

int tuya_dp_148_response_accessory_lock(BOOL_T state);//设置开锁状态

int tuya_dp_239_response_accessory_lock(BOOL_T state);//设置锁2状态

int tuya_dp_232_response_absent_mode(BOOL_T state);//离家模式

int tuya_dp_236_response_home_mode(BOOL_T state);

int tuya_dp_237_sleep_home_mode(BOOL_T state);

void all_device_mode_sync(void);//同步所有设备模式

void tuya_response_mode_sync(void);//同步涂鸦模式

int tuya_dp_233_response_device_active(void);//

int tuya_dp_234_response_abnormal_unlock(void);

int tuya_dp_uploads_security_msg(char id , char *data,int size);

void update_tuya_lock_state(BOOL_T state);

void set_tuya_channel_state(int channel, bool state);

typedef struct
{
    int temp;
    int condition;
    int humidity;
    int pressure;
    int pm10;
    int pm25;
    int aqi;
    int thigh;
    int tlow;
}tuya_api_weather;

tuya_api_weather tuya_weather_get();

#endif

