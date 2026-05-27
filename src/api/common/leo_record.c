#include "leo_api.h"
#include "file_api.h"
#include "stdio.h"
#include "video_decode.h"
#include "../../include/tuya/tuya_sdk.h"


/***********************************

检查记录文件参数是否正确

************************************/
static bool check_record_video_parameter(char mode,char audio_from,char video_channel)
{
	if(is_video_recording() == true){

		printf("Video recording is working \n\r");
		return false;
	}

	return true;
}



/***********************************

打开记录音频通道。
return 
	true:记录声音，false:不记录声音

***********************************
static bool record_audio_channel_open(char audio_from,MONITOR_CH video_channel)
{
    if(is_audio_talk_open() == true)
    {
        return true;
    }
	return false;
}
*/

/*
 *  0:h264,1:mjpeg
 */
static char record_video_info(MONITOR_CH ch,int* width,int* height)
{
	if((ch >= MON_CH_UNIT_DOOR_1)&& (ch <= MON_CH_DOOR_15))
	{
        *width = 1280;
        *height = 720;
		return 0;
	}
    else if((ch >= MON_CH_CCTV_1)&&(ch <= MON_CH_CCTV_2))
    {
        // *width = 640;
        // *height = 360;
		*width = 1920;
        *height = 1080;
        // extern void live555SendSPSandPPS(void);
        // live555SendSPSandPPS();
        return 0;
    }
	return 1;
}

/**********************************************
mode: 记录模式 手动,自动，移动侦测。
audio_frome: 记录音频来源。
	0x00:不记录音频。
	0x01:记录户外机
	0x02:记录室内机
	0x03:记录室内机和户外机
**********************************************/
bool record_video_start(char mode,char audio_from,MONITOR_CH video_channel){
	//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
	if(check_record_video_parameter(mode,audio_from,video_channel) == false){
		
		return false;
	}

	
	char file_path[64] = {0};
	if(create_one_media_file(FILE_TYPE_SD_MIXED_VIDEO, video_channel, mode, file_path) == false){
		
		printf("failed to create media file \n\r");
		return false;
	}
	bool is_has_audio = audio_from == 0x00 ?false : true; 

	//bool is_has_audio = record_audio_channel_open(audio_from,video_channel);
    int width = 0,height = 0;
	char video_type = record_video_info(video_channel,&width,&height);
	
	if(video_record_start(file_path, is_has_audio ,width,height,video_type) == false)
	{	
		printf("open mux video fail \n\r");
		return false;
	}
	return true;
}




bool check_record_video_stop_parameter(char audio_flag){
	
	if(is_video_recording() == false)
	{
		printf("Video recording is not working \n\r");
		return false;
	}

	return true;
}

/***********************************

关闭视频记录
	audio_flag: 关闭记录音频。
		0x01: 关闭所有音频
		0x02: 只是关闭进入mic的通道
************************************/
bool record_video_stop(char audio_flag){

	if(check_record_video_stop_parameter(audio_flag) == false){
		
		return false;
	}
	
	video_record_stop();

	return true;
}



static bool check_record_picture_parameter(void)
{
	if(is_jpg_record_ing() == true)
	{
		return false;
	}


	return true;
}

static bool record_pictrue_file_path_get(char mode,MONITOR_CH video_channel,char* file_path){

	if(create_one_media_file(is_sdcard_insert()? FILE_TYPE_SD_MIXED_PHOTO:FILE_TYPE_FLASH_PHOTO, video_channel, mode, file_path) == false)
	{
		printf("Error getting file path \n\r");
		return false;
	}
	return true;
}



bool record_pictrue_start(char mode,MONITOR_CH video_channel)
{	

	if(check_record_picture_parameter() == false)//正在拍照 
	{
		return false;
	}

	char file_path[64] = {0};
	
	if(record_pictrue_file_path_get(mode,video_channel, file_path) == false)
	{
		
		printf("failed to create media file \n\r");
		return false;
	}

	return jpg_record(file_path);
}



#define TUYA_PATH "/tmp/tuya.jpg"


bool sent_tuya_start(char mode,MONITOR_CH video_channel){
	if(is_online_tuya_cloud() == false)
		return false;

	if(check_record_picture_parameter() == false)//正在拍照 
	{
		return false;
	}

	if(access(SD_PHOTO_PATH, F_OK) != 0)
		system("touch "TUYA_PATH);
	return sent_tuya_record(TUYA_PATH);
}
