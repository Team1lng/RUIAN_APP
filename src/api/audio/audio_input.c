#include "audio_input.h"
#include "ak_ai.h"
#include "string.h"
#include "ak_thread.h"
#include <fcntl.h>
#include <unistd.h>
bool video_record_data_push(char type,unsigned char* data,int len,bool is_video);
bool network_audio_send_package_push(char type,const char* data ,int len);


static bool audio_input_task_run = false;
static bool audio_input_thread_fun = false;
static bool audio_input_capture = false;

static void setup_default_audio_argument(void *audio_args, char args_type)
{

#if (AUDIO_USUAL == 0)
/*==========================通用版本===============================*/
	struct ak_audio_nr_attr default_ai_nr_attr = {-50, 0, 1};//{-20, 0, 1};//{-30, 0, 1};
	struct ak_audio_agc_attr default_ai_agc_attr = {16384/*24576*/, 2, 0, /*80*/1, 0, 1};
	struct ak_audio_aec_attr default_ai_aec_attr = {0, /*1024*/1024, 512,  0, 512, 1};
	struct ak_audio_aslc_attr default_ai_aslc_attr = {12000, 0, 0};//{16384, 0, 0};//{32768, -30, 1};

/*-----------------------波兰动态lgoo十寸室内机-------------------*/
#else
struct ak_audio_nr_attr default_ai_nr_attr ={-25, 0, 1};
struct ak_audio_agc_attr default_ai_agc_attr = {16384, 2, 0, 1, 0, 1};
struct ak_audio_aec_attr default_ai_aec_attr = {0, 1024, 512, 0, 512, 1, 0};
struct ak_audio_aslc_attr default_ai_aslc_attr = {9830, 1, 0};
#endif
/*-----------------------波兰动态lgoo十寸室内机-------------------*/


	struct ak_audio_nr_attr default_ao_nr_attr = {0};
	struct ak_audio_aslc_attr default_ao_aslc_attr = {0};
	switch (args_type) {
	   case 1:
		   *(struct ak_audio_nr_attr *) audio_args = default_ai_nr_attr;
		   break;
	   case 2:
		   *(struct ak_audio_agc_attr *) audio_args = default_ai_agc_attr;
		   break;
	   case 3:
		   *(struct ak_audio_aec_attr *) audio_args = default_ai_aec_attr;
		   break;
	   case 4:
		   *(struct ak_audio_aslc_attr *) audio_args = default_ai_aslc_attr;
		   break;
	   case 5:
		   *(struct ak_audio_nr_attr *) audio_args = default_ao_nr_attr;
		   break;
	   case 6:
		   *(struct ak_audio_aslc_attr *) audio_args = default_ao_aslc_attr;
		   break;
	   default:
		   break;
	}

    return;
}

static int audio_input_device_open(void)
{
	//extern void audio_output_close(void);
	//audio_output_close();

	int ai_handle_id = -1;
	struct ak_audio_in_param ai_param;
    ai_param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    ai_param.pcm_data_attr.channel_num = AUDIO_CHANNEL_MONO;
    ai_param.pcm_data_attr.sample_rate = AK_AUDIO_SAMPLE_RATE_16000;
    ai_param.dev_id = DEV_ADC;
	if(ak_ai_open(&ai_param,&ai_handle_id))
	{
		return -1;
	}



	struct ak_audio_nr_attr nr_attr;
	setup_default_audio_argument(&nr_attr, 1);
	ak_ai_set_nr_attr(ai_handle_id, &nr_attr);
	ak_ai_enable_nr(ai_handle_id, AUDIO_FUNC_ENABLE);

	struct ak_audio_agc_attr agc_attr;
	setup_default_audio_argument(&agc_attr, 2);
	ak_ai_set_agc_attr(ai_handle_id, &agc_attr);
	ak_ai_enable_agc(ai_handle_id, AUDIO_FUNC_ENABLE);
	
	struct ak_audio_aec_attr aec_attr;
	setup_default_audio_argument(&aec_attr, 3);
	ak_ai_set_aec_attr(ai_handle_id,&aec_attr);
	ak_ai_enable_aec(ai_handle_id, AUDIO_FUNC_ENABLE);

	struct ak_audio_aslc_attr aslc_attr;
	setup_default_audio_argument(&aslc_attr, 4);
	ak_ai_set_aslc_attr(ai_handle_id,&aslc_attr);
	
#if (AUDIO_USUAL == 0)
	/*==============================通用版本=================================*/
	
	struct ak_audio_eq_attr user_ai_eq_attr={
		0,
		1,
		{ 2400, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
		{-7168,0,0,0,0,0,0,0,0,0},
		{819,717,717,717,717,717,717,717,717,717},
		{TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1},
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		};
/*==============================通用版本=================================*/

#else
/*-----------------------波兰动态lgoo十寸室内机-------------------*/
	struct ak_audio_eq_attr user_ai_eq_attr={
	1024,
	2,
	{5000, 1500, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
	{-8192, -10240, 0, 0, 0, 0, 0, 0, 0, 0},
	{716, 1024, 717, 717, 717, 717, 717, 717, 717, 717},
	{TYPE_HSF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	{ 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}
	};
/*-----------------------波兰动态lgoo十寸室内机-------------------*/
#endif
	ak_ai_set_eq_attr(ai_handle_id,&user_ai_eq_attr);

	
	ak_ai_set_source(ai_handle_id, AI_SOURCE_MIC);
	ak_ai_set_gain(ai_handle_id,3);

 	ak_ai_set_volume(ai_handle_id, 1);// enable_agc



	
	return ai_handle_id;
}	
static void audio_input_device_close(int hand_id)
{
	if(hand_id != -1)
	{
		ak_ai_close(hand_id);
	}
}
static void* audio_input_task(void* arg)
{
	printf("===============================   audio mic input start ======================= \n");
	audio_input_thread_fun = true;

	int hand_id = audio_input_device_open();
	ak_ai_start_capture(hand_id);
	struct frame pcm_frame = {0};
	// unsigned int puot=0;
	while(audio_input_task_run == true)	
	{
		memset(&pcm_frame,0,sizeof(struct frame));
		if(ak_ai_get_frame(hand_id, &pcm_frame,0) == 0)
		{
			if(audio_input_capture == true)
			{
				network_audio_send_package_push(0, (const char*)pcm_frame.data,  pcm_frame.len);
			}
			ak_ai_release_frame(hand_id, &pcm_frame);
		}
		ak_sleep_ms(1);
	}
	ak_ai_stop_capture(hand_id);
	audio_input_device_close(hand_id);
	audio_input_thread_fun = false;
	printf("===============================   audio mic input stop ======================= \n");
	ak_thread_exit();
	return NULL;
}



static bool audio_input_wait_thread_quit(void)
{
	int timeout = 300;
	while(timeout--)
	{
		if(audio_input_thread_fun == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

bool audio_input_open(void)
{
	if(audio_input_task_run == true)
	{
		return false;
	}

	if(audio_input_wait_thread_quit() == false)
	{
		return false;
	}

	audio_input_task_run = true;
	audio_input_capture = false;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, audio_input_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}


bool audio_input_start(void)
{
	if(audio_input_capture == true)
	{
		return false;
	}
	audio_input_capture = true;
	return true;
}

bool audio_input_stop(void)
{
	if(audio_input_capture == false)
	{
		return false;
	}
	audio_input_capture = true;
	return true;
}
bool audio_input_close(void)
{
	if(audio_input_task_run == false)
	{
		return false;
	}
	audio_input_task_run = false;
	return true;
}

