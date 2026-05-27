#include "audio_play.h"
#include "leo_audio_play.h"
#include "stdbool.h"
#include "../../layout/resource/rom.h"
#include "../../layout/user_data.h"
#include "../../lvgl/src/lv_core/lv_obj.h"
#include "../../src/layout/layout_define.h"

extern void audio_play(audio_info*);

#define RING_PATH  "/app/data/rings/"//"/mnt/layout/resource/rings/"

extern bool is_audio_talk_open(void);



bool talking = false;



//触摸音效

void touch_sound_play(void)
{
	if(talking == true)
		return ;
	
	audio_info info = 
	{
		.data_type = 0,
		.data.bin.offset = ROM_RES_KEY_TOUCH_9_PCM,//,
		.data.bin.size = ROM_RES_KEY_TOUCH_9_PCM_SIZE,//,
		.ch = AUDIO_CHANNEL_MONO,
		.rate = AK_AUDIO_SAMPLE_RATE_16000,
		.type = AK_AUDIO_TYPE_PCM,//,
		.volume = 10,
		.start = NULL,
		.end = NULL
	};
	audio_play(&info);
}



//门铃声音
bool door_ring_play(int ring_index,int volume,audio_play_callback start,audio_play_callback end)
{
	if(ring_index > 6)
	{
		return false;
	}
	
	if(talking == true)
		return false;


	static char* file_info[7] = 
	{
		RING_PATH"1.mp3",
		RING_PATH"2.mp3",
		RING_PATH"3.mp3",
		RING_PATH"4.mp3",
		RING_PATH"5.mp3",
		RING_PATH"6.mp3",
		LOCAL_RING7_FILE_PATH
	};
	
	audio_info info = 
	{
		.data_type = 1,
		.ch = AUDIO_CHANNEL_MONO,
		.rate = AK_AUDIO_SAMPLE_RATE_16000,
		
		.type = AK_AUDIO_TYPE_MP3,//,
	};

	strcpy(info.data.file_path,file_info[ring_index]);
	info.volume = volume;
	info.start = start;
	info.end = end;
	audio_play(&info);
	return true;
}

/**
 * 播放自定义门铃声
 * 
 * @param file_path 音频文件路径
 * @param volume 播放音量
 * @param start 播放开始回调函数
 * @param end 播放结束回调函数
 * 
 * @return 成功返回true，通话中返回false
 * 
 * 功能说明：
 * 1. 检查当前是否在通话状态，若在通话中则拒绝播放并返回false
 * 2. 初始化音频参数结构体，配置为单声道16000Hz采样率的MP3格式
 * 3. 设置音频文件路径和音量
 * 4. 注册播放开始和结束的回调函数
 * 5. 调用底层音频播放接口开始播放
 * 6. 播放请求成功发出后立即返回true，不等待播放完成
 */
bool custom_door_ring_play(char * file_path,int volume,audio_play_callback start,audio_play_callback end)
{
    // 通话状态检测：若正在通话则不播放铃声
    if(talking == true)
        return false;
    
    // 初始化音频播放参数
    audio_info info = 
    {
        .data_type = 1,                     // 数据类型标识（1表示文件路径）
        .ch = AUDIO_CHANNEL_MONO,           // 单声道播放
        .rate = AK_AUDIO_SAMPLE_RATE_16000, // 采样率16000Hz
        .type = AK_AUDIO_TYPE_MP3,          // 音频格式为MP3
    };
    
    // 设置音频文件路径（注意：未检查缓冲区溢出风险）
    strcpy(info.data.file_path,file_path);
    
    // 设置播放音量
    info.volume = volume;
    
    // 注册播放状态回调函数
    info.start = start;                     // 播放开始时调用的回调函数
    info.end = end;                         // 播放结束时调用的回调函数
    
    // 调用底层音频播放接口
    audio_play(&info);
    
    // 返回播放请求结果（仅表示请求成功，不代表播放已完成）
    return true;
}

//电话铃声
bool interphone_ring_play(int volume,audio_play_callback start,audio_play_callback end)
{
	//extern bool is_mode_status_on(void);
	//if(is_mode_status_on()){
	if(user_data_get()->system_mute){
		return false;
	}

	audio_info info = 
	{
		.data_type = 1,
		.ch = AUDIO_CHANNEL_MONO,
		.rate = AK_AUDIO_SAMPLE_RATE_16000,
		
		.type = AK_AUDIO_TYPE_MP3,//,
	};
	static char* file = RING_PATH"10.mp3";
	strcpy(info.data.file_path,file);
	info.volume = volume;
	info.start = start;
	info.end = end;
	audio_play(&info);
	return true;
}

//开门铃声
bool open_door_ring_play(int volume)
{
	if(talking == true)
		return false;
	
	audio_info info = 
	{
		.data_type = 1,
		.ch = AUDIO_CHANNEL_MONO,
		.rate = AK_AUDIO_SAMPLE_RATE_16000,
		
		.type = AK_AUDIO_TYPE_MP3,//,
	};
	static char* file = RING_PATH"7.mp3";
	strcpy(info.data.file_path,file);
	info.volume = volume;
	info.start = NULL;
	info.end = NULL;
	audio_play(&info);
	return true;
}

//音量设置

bool audio_volume_set(int vol)
{
	return audio_output_volume_set(vol);
}




