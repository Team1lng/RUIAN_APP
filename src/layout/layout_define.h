#ifndef LAYOUT_DEFINE_H
#define LAYOUT_DEFINE_H
#include <stdlib.h>  
#include <time.h>  
#include <stdio.h> 
#include <sys/statfs.h>
#include <sys/vfs.h>

#include "ak_mem.h"

#include "../lvgl/lvgl.h"
//#include "resource/icon_font/output/font_icon_def.h"
#include "resource/rom.h"

#include "user_data.h"
#include "../lv_drivers/lv_port_disp.h"

#include "../../api/freetype/lv_freetype.h"
#include "../api/network/network_common.h"

#include "file_api.h"
#include "leo_api.h"
#include "../../api/gpio/leo_gpio.h"
#include "../../api/gpio/leo_key.h"

#include "user_data.h"
#include "layout_interphone.h"
#include "layout_monitor.h"
#include "layout_setting.h"
#include "layout_passwd.h"
#include "layout_setting_cctv_brand.h"
#include "../api/audio/audio_play.h"
#include "../api/common/leo_audio_play.h"
#include "../api/wlan/wlan.h"
#include "../api/common/leo_audio_play.h"
#include "../api/xls/tuya_uuid_and_key.h"
#include "../api/onvif/include/sat_ipcamera.h"


#include "tuya_sdk.h"

#include "tuya_ipc_p2p.h"
#include "tuya_ipc_api.h"
#include "layout_common.h"
#include "../api/common/os_standby.h"
#include "user_time.h"
#define SYSTEM_BG_FILE_PATH "/app/data/background/frame.jpg"
#define BACKGROUND_FILE_PATH "/app/data/background/"
#define  LOCAL_RING7_FILE_PATH "/app/data/rings/customize.mp3"
#define  LOCAL_RING_FILE_PATH "/app/data/rings"


#define POLISH 0 //0代表静态logo，1表示动态logo


typedef void(*event_pro_callback)(unsigned long arg1,unsigned long arg2);

#define Debug (printf("\n\033[0;32;40m[***%s***]:%u\033[0m \t", __PRETTY_FUNCTION__, __LINE__),printf)

typedef struct 
{
	void(*enter)(void);
	void(*quit)(void);
}layout;

#define CREATE_LAYOUT(x)  layout layout_##x = {\
									.enter = layout_##x##_enter, \
									.quit = layout_##x##_quit };

#define pLAYOUT(x) &layout_##x

#define DEFINE_LAYOUT(x) extern layout layout_##x;

#define LAYOUT_ENETER_FUNC(x)  layout_##x##_enter(void)

#define LAYOUT_QUIT_FUNC(x)   layout_##x##_quit(void)




bool goto_layout(const layout* layout);
extern bool test_flag;


const layout* get_cur_layout(void);



typedef struct 
{	
	void*user_data;
	void(*down)(lv_obj_t* obj);
	void(*up)(lv_obj_t* obj);
	void (*anything_func)(lv_obj_t * obj, lv_event_t event);
}btn_data;


#define btn_data_create(down_ex,up_ex,user_data_ex)  {.down = down_ex,\
											.up = up_ex,\
											.user_data = user_data_ex, \
											.anything_func = NULL\
											};

#define btn_data_up_create(x)  {.user_data = NULL,\
								.down = NULL,\
								.up = x,\
								.anything_func = NULL\
								};

#define btn_data_up_data_create(x,user_data_ex)  {.user_data = user_data_ex, \
								.down = NULL,\
								.up = x,\
								.anything_func = NULL\
								};
#define btn_data_anything_create(x)  {.user_data = NULL,\
								.down = NULL,\
								.up = NULL,\
								.anything_func = x\
								};

void btn_touch_event_listen(lv_obj_t* obj);




unsigned char * picture_data_get();

void background_open();
void background_close();


int get_sound_val(int val);

void record_jpeg_event_register(event_pro_callback handle);

void record_video_event_register(event_pro_callback handle);

void sdcard_event_register(event_pro_callback handle);

event_pro_callback device_id_repeat_register(event_pro_callback handle);

void interphone_call_event_register(event_pro_callback handle);

void outdoor_call_event_register(event_pro_callback handle);

void key_call_event_register(event_pro_callback handle);

void indoor_cmd_event_register(event_pro_callback handle);

void layout_obj_touch_event_register(void(*handle)(void));

bool bell_press_event_push(char arg);//门钟按压事件

void standby_event_event_register(event_pro_callback handle);//待机回调函数

void bell_press_event_register(event_pro_callback handle);//门钟按压回调注册

void bell_det_init(void);//门钟按压检测初始化

/************************************************************
* @Description: 门口机可用状态改变回调注册
* @Author: xiaoxiao
* @Date: 2023-02-16 14:35:10
* @param: 
* @explain: 
************************************************************/
void outdoor_status_change_event_register(event_pro_callback handle);

/************************************************************
* @Description: 门口机可用状态改变事件发送
* @Author: xiaoxiao
* @Date: 2023-02-16 14:58:22
* @param: 
* @explain: 
************************************************************/
bool outdoor_status_change_event_push(unsigned long arg1,unsigned long arg2);


bool stanby_event_push(bool is_finish);//待机事件加入任务队列


void tuya_event_register(event_pro_callback handle);
bool tuya_monitor_swap_event(int ch);
bool tuya_monitor_talk_event(bool state);
bool tuya_monitor_unlock_event(bool state);
bool tuya_monitor_unlock2_event(bool state);

bool tuya_monitor_absent_mode_event(bool state);
bool tuya_monitor_home_mode_event(bool state);
bool tuya_monitor_sleep_mode_event(bool state);

bool tuya_monitor_enter_event(void);
bool tuya_monitor_quit_event(void);
void rtc_time_sync(void);



DEFINE_LAYOUT(home);
DEFINE_LAYOUT(monitor);
DEFINE_LAYOUT(playback);
DEFINE_LAYOUT(video);
DEFINE_LAYOUT(photo);
DEFINE_LAYOUT(setting);
DEFINE_LAYOUT(standby);
DEFINE_LAYOUT(interphone);
DEFINE_LAYOUT(cctv);
DEFINE_LAYOUT(set_wifi);
DEFINE_LAYOUT(add_wifi);
DEFINE_LAYOUT(connect_wifi);
DEFINE_LAYOUT(set_cctv);
DEFINE_LAYOUT(set_language);
DEFINE_LAYOUT(set_system);
DEFINE_LAYOUT(tuya_register);
DEFINE_LAYOUT(motion);
DEFINE_LAYOUT(setting_password);
DEFINE_LAYOUT(floor_setting);
DEFINE_LAYOUT(room_select);
DEFINE_LAYOUT(close);
DEFINE_LAYOUT(background);
DEFINE_LAYOUT(ring);
DEFINE_LAYOUT(customize_ring);
DEFINE_LAYOUT(product_introduction);
DEFINE_LAYOUT(add_cctv);
DEFINE_LAYOUT(cctv_information);
DEFINE_LAYOUT(write_cctv);
DEFINE_LAYOUT(cctv_brand);
DEFINE_LAYOUT(logo);

DEFINE_LAYOUT(floor_number_select);


//#define LEO_FUNC_TEST
#endif

