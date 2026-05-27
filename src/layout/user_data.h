#ifndef _USER_DATA_H_
#define _USER_DATA_H_
#include "stdbool.h"
#include "../api/xls/tuya_uuid_and_key.h"

enum language{
    language_english = 0,	//0-英语
	language_chinese,		//1-中文
	language_russian,		//2-俄罗斯
    language_french,		//3-法语
    language_spanish,		//4-西班牙
    language_german,    	//5-德语   
    language_arabic,		//6-阿拉伯	
    language_czech,  		//7-捷克
	language_hebrew,  		//8-希伯来语
	language_portugal,  	//9-葡萄牙
	language_italy,			//10-意大利
	language_polish,  		//11-波兰
	language_greek,  		//12-希腊
	language_turkey,		//13-土耳其
	language_nederlands,    //14-荷兰
    language_total, 		//15-全部


};

enum cctv_brand{
	brand_MAZi = 0,
	brand_Hikivision ,
	brand_Dahua,
	brand_XM,
	brand_urmet,
	brand_Onvif,
	brand_other,
	brand_total,
};

typedef struct
{
	bool enable;
	char record_flag;	 	//0:photo,1:video;
	char record;			//0=使用自动拍照功  1=自动拍照 2=自动录像(未插入sd卡不能使用）
	bool motion;
}user_motion_info;

typedef struct
{
	bool key_sound;
	int door1_ring;
	int door2_ring;
	
	int outdoor_ring_val;
	int door_ring_val;
	int intercom_ring_val;

	int outdoor_talk_val;
	
	int door_talk_val;

	int intercom_talk_val;
	
}user_audio_info;


typedef struct
{
	bool auto_record;
	
	bool alarm_1_enable;
	bool alarm_1_trigger;

	bool alarm_2_enable;
	bool alarm_2_trigger;
	
}user_alarm_info;


typedef struct
{
	int network_device;

	char password[4];

	short int family_id;

	char screen_saver; 

	int mode;
}user_other_info;

typedef struct
{
	bool wifi_open_flag;//wifi打开标志�?0:关闭 1:打开
	bool wifi_connect_flag;


}user_wifi_info;


typedef struct
{
	char ip[20];
	char url[128];
	char user[20];
	char pswd[20];
	int stream; // [0] main stream; [1] sub stream
	int substream_hight; // [0] main stream; [1] sub stream
	int substream_low; // [0] main stream; [1] sub stream
	// bool  online;		/* [-1]-IPC  [0]-DAHUA  [1]-HIKVISION  [2]-XM */
	int cannel;  /*[1] cctv1   [2] cctv2 */
	
}user_onvif_info;


typedef struct
{
	char ch_name[32];
	int id; // [0] main stream; [1] sub stream
	
}user_tuya_info;


typedef struct
{
	char super_password[4];

	bool system_mute;
	
	enum language user_language;
		
	user_motion_info motion;

	char auto_record_mode; //0:off,1:photo,2:video;

	user_audio_info audio;

	user_alarm_info alarm;

	user_other_info other;
	
	user_wifi_info wifi;

	user_onvif_info onvif_dev[8];

	user_tuya_info tuya_ch_name[30];

	int onvif_dev_count;

	int cctv_brand_Index;
	int cctv_connect_mode; //0:line,1:wire
	int door1_delay;
	int door2_delay;
	int ring_time;

	char floor[2];

	bool nobody_message;
	
}user_data_info;


bool user_data_save(void);
bool user_data_init(void);
user_data_info* user_data_get(void);
void user_data_reset(void);


#endif

