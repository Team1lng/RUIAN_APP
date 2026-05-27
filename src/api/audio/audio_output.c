#include "audio_output.h"
#include "../samplerate/samplerate.h"

static int audio_output_handle_id = -1;

static enum ak_audio_sample_rate auido_output_rate = AK_AUDIO_SAMPLE_RATE_64000;
static enum ak_audio_channel_type audio_ouput_channel = AUDIO_CHANNEL_RESERVED;

static ak_mutex_t audio_output_mutex;
bool audio_output_close(void);

#if 1
static void setup_default_ao_argument(void *audio_args, char args_type) {
	struct ak_audio_nr_attr default_ai_nr_attr = {0};//{-20, 0, 1};//{-30, 0, 1};
		struct ak_audio_agc_attr default_ai_agc_attr = {0};
		struct ak_audio_aec_attr default_ai_aec_attr = {0};
		struct ak_audio_aslc_attr default_ai_aslc_attr = {0};
	
		struct ak_audio_nr_attr default_ao_nr_attr = {-40, 0, 1};//{-20, 0, 1};//{-30, 0, 1};
		struct ak_audio_aslc_attr default_ao_aslc_attr = {9830, 1, 0};



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

#endif





static bool audio_output_mutex_init(void)
{
	static bool is_first_init = true;
	if(is_first_init == true)
	{
		is_first_init = false;
		ak_thread_mutex_init(&audio_output_mutex,NULL);
	}
	return true;
}

static bool audio_output_devices_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate)
{
	struct ak_audio_out_param param;
	param.dev_id = DEV_DAC;
	param.pcm_data_attr.channel_num = ch;
	param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	param.pcm_data_attr.sample_rate = rate;
	return ak_ao_open(&param, &audio_output_handle_id)?false:true;
}
static bool audio_output_restart_devices(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate)
{
	struct ak_audio_out_param param;
	param.dev_id = DEV_DAC;
	param.pcm_data_attr.channel_num = ch;
	param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	param.pcm_data_attr.sample_rate = rate;
	return ak_ao_reset_params(audio_output_handle_id, &param)?false:true;
}
static bool audio_output_devices_open_check(void)
{
	return (audio_output_handle_id == -1)?false:true;
}

static int audio_output_volume = 0;

bool audio_output_volume_set(int vol){
	if(audio_output_devices_open_check() == false)
	{
        audio_output_volume = -1;
		return false;
	}
	if(audio_output_volume != vol)
    {
        audio_output_volume = vol;
		ak_ao_set_volume(audio_output_handle_id, 0);
    }
	
	
	
	//if(audio_output_volume == 70)
	//{
		//ak_ao_set_volume(audio_output_handle_id, audio_output_volume-80);
	//}
	//ak_ao_set_speaker(audio_output_handle_id,1);
	//ak_ao_enable_eq(audio_output_handle_id,0);
	//ak_ao_enable_nr(audio_output_handle_id,0);
	//ak_ao_enable_hs(audio_output_handle_id,0);
	return true;
}

int audio_output_volume_get(void)
{
    return audio_output_volume;
}




bool audio_output_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate,int vol,int gain)
{
	audio_output_mutex_init();
	ak_thread_mutex_lock(&audio_output_mutex);
	if((audio_output_devices_open_check() == false)||(auido_output_rate != rate)||(audio_ouput_channel != ch))
	{
		if(audio_output_devices_open_check() == true)
		{
			audio_output_restart_devices(ch,rate);
			//audio_output_close();
		}
		else
		{
			audio_output_devices_open(ch,rate);
		}
		auido_output_rate = rate;
		audio_ouput_channel = ch;
        audio_output_volume = -1;
	}
	
	


	
	struct ak_audio_nr_attr nr_attr = {0};
	setup_default_ao_argument(&nr_attr, 5);
	ak_ao_set_nr_attr(audio_output_handle_id,&nr_attr);
	
	struct ak_audio_aslc_attr aslc_attr;
	setup_default_ao_argument(&aslc_attr, 6);
	ak_ao_set_aslc_attr(audio_output_handle_id,&aslc_attr);

	
	struct ak_audio_eq_attr user_ao_eq_attr={
		 0,
	
		1,
	
		{ 700, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
	
		{0,0,0,0,0,0,0,0,0,0},
	
		{716,717,717,717,717,717,717,717,717,717},
	
		{TYPE_HPF,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1},
	
		0,
	
		0,
	
		0,
	
		0,
	
		0,
	
		0,
	
		1,
	
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	
		};
	ak_ao_set_eq_attr(audio_output_handle_id,&user_ao_eq_attr);
			

	
#if (AUDIO_USUAL == 0)
	ak_ao_set_gain(audio_output_handle_id,5);// gain);通用版
#else
	ak_ao_set_gain(audio_output_handle_id,3);// gain)波兰动态lgoo十寸室内机
#endif

	audio_output_volume_set(vol);//(vol);

		
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}




#define PCM_SIZE_MAX			4*1024 

/*
*	设置16BIT的PCM音量，转换一次的最大size不能超过PCM_SIZE_MAX
*
*	src:pcm源数据
*	
*	size:pcm数据的长度,比如80将音量设置为之前的80%
*
*	设置音量-百分比
*/
static bool pcm_16bit_volume_cover(unsigned char * src, int size, int volume)
{
	int i	= 0;
	unsigned char dst[PCM_SIZE_MAX] = {0};

	if (size > PCM_SIZE_MAX)
	{
		return false;
	}

	if (volume == 100)
	{
		return true;
	}

	if (volume == 0)
	{
		memset(src, 0, size);
		return true;
	}

	memset(dst, 0, PCM_SIZE_MAX);


	float bar = volume * 1.0 / 100;

	for (i = 0; i < size; i += 2)
	{
		short src_data = (src[i + 1] << 8) | (src[i] &0xFF);

		src_data = src_data * bar;

		if (src_data > 32767)
		{
			src_data = 32767;
		}
		else if (src_data < -32768)
		{
			src_data = -32768;
		}

		dst[i]	= src_data & 0xFF;
		dst[i + 1] = (src_data >> 8) & 0xFF;

	}

	memcpy(src, dst, size);

	return true;
}

bool audio_output_write(unsigned char* data,int len)
{
	ak_thread_mutex_lock(&audio_output_mutex);

	if(audio_output_devices_open_check() == false)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}
	
	int play_len = 0;
	pcm_16bit_volume_cover(data, len, audio_output_volume);
	if(ak_ao_send_frame(audio_output_handle_id, data, len, &play_len))
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}

	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}

bool audio_output_close(void)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if(audio_output_handle_id != -1)
	{
		ak_ao_close(audio_output_handle_id);
		audio_output_handle_id = -1;
	}
	
	auido_output_rate = AK_AUDIO_SAMPLE_RATE_64000;
	audio_ouput_channel = AUDIO_CHANNEL_RESERVED;
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}


bool audio_output_buffer_status_printf(void)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if(audio_output_handle_id == -1)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}
	
	struct ak_dev_buf_status status;
	ak_ao_get_buf_status(audio_output_handle_id, &status);
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}


bool audio_output_buffer_get(int* total,int* remain)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if(audio_output_handle_id == -1)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}
	
	struct ak_dev_buf_status status;
	ak_ao_get_buf_status(audio_output_handle_id, &status);
	ak_thread_mutex_unlock(&audio_output_mutex);
	*total = status.buf_total_len;
	*remain = status.buf_remain_len;
	return true;
}




/*
*	查询声卡中剩余的音频数据
*/

int audio_output_buffer_query(void)
{
	struct ak_dev_buf_status buf_status;
	ak_thread_mutex_lock(&audio_output_mutex);
	if(audio_output_handle_id == -1)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return -1;
	}
	ak_ao_get_buf_status(audio_output_handle_id, &buf_status);
	ak_thread_mutex_unlock(&audio_output_mutex);
	return  buf_status.buf_remain_len;
	
}


