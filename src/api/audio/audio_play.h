#ifndef _AUDIO_PLAY_H_
#define _AUDIO_PLAY_H_
#include "audio_output.h"

#include "../../lvgl/src/lv_core/lv_obj.h"
typedef void (*audio_play_callback)(void);

typedef enum
{
	/*切换监控*/
	TUYA_EVENT_MONITOR_SWAP,

	/*开锁*/
	TUYA_EVENT_OPEN_DOOR,

    /*Absent mode*/
    TUYA_EVENT_ABSENT_MODE,

	/*Absent mode*/
    TUYA_EVENT_HOME_MODE,

	/*Absent mode*/
    TUYA_EVENT_SLEEP_MODE,
	
	/*通话*/
	TUYA_EVENT_TALK,

	/*进入监控*/
	TUYA_EVENT_MONITOR_ENTER,

	/*退出监控*/
	TUYA_EVENT_MONITOR_QUIT,
	
	TUYA_EVENT_OPEN_DOOR2
}tuya_event;



typedef struct
{
	char data_type; //0 rom_bin_info ,1:file path
	union
	{
		rom_bin_info bin;
		char file_path[128];
	}data;
	enum ak_audio_channel_type ch;
	enum ak_audio_sample_rate rate;
	enum ak_audio_type type;

	int volume;

	audio_play_callback start;
	audio_play_callback end;
}audio_info;


bool audio_play_init(void);


bool is_audio_play_ing(void);


bool audio_play_stop_set(void);

#endif

