#include "audio_input.h"
#include "ak_ai.h"
#include "string.h"
#include "ak_thread.h"
#include "queue.h"
#include "ak_mem.h"
#include "audio_output.h"
#include "leo_api.h"
#include "../../layout/user_data.h"
#include "../../layout/layout_define.h"

#define AUDIO_DECODE_QUEUE_MAX 8

bool audio_output_buffer_get(int* total,int* remain);


typedef struct
{
	unsigned char* data;
	int len;
}audio_decode_info;
typedef struct
{
	void* prev;
	void* next;

	audio_decode_info msg;
}audio_decode_queue;

static audio_decode_queue audio_decode_buffer[AUDIO_DECODE_QUEUE_MAX];
static queue_s audio_decode_queue_free;
static queue_s audio_decode_queue_head;
static ak_mutex_t audio_decode_queue_free_mutex;
static ak_mutex_t audio_decode_queue_head_mutex;
	
static audio_decode_queue* audio_decode_queue_node_new(unsigned char* data ,int len)
{
	audio_decode_queue* node = NULL;
	ak_thread_mutex_lock(&audio_decode_queue_free_mutex);
	if(queue_empty(&audio_decode_queue_free) == 0)
	{
		node = (audio_decode_queue*)queue_delete_next(&audio_decode_queue_free);	
		if(node->msg.data != NULL)
		{
			ak_mem_free(node->msg.data);
		}
		node->msg.data = ak_mem_alloc(MODULE_ID_AI, len);
		node->msg.len = len;
		memcpy(node->msg.data,data,len);
	}
	ak_thread_mutex_unlock(&audio_decode_queue_free_mutex);
	return node;
}

static void audio_decode_queue_node_del(audio_decode_queue* node)
{
	if(node != NULL)
	{
		if(node->msg.data != NULL)
		{	
			ak_mem_free(node->msg.data);
			node->msg.data = NULL;
		}
		ak_thread_mutex_lock(&audio_decode_queue_free_mutex);
		queue_insert((queue_s*)node, &audio_decode_queue_free);
		ak_thread_mutex_unlock(&audio_decode_queue_free_mutex);
	}
}

static bool audio_decode_task_run = false;
static bool audio_decode_thread_run = false;
static bool audio_decode_ready = false;


static void audio_decode_device_open(bool falg)
{
	//audio_output_open(AUDIO_CHANNEL_MONO,AK_AUDIO_SAMPLE_RATE_16000,user_data_get()->audio.door_talk_val,5);
	if(falg == true)
		audio_output_open(AUDIO_CHANNEL_MONO,AK_AUDIO_SAMPLE_RATE_16000, get_sound_val(user_data_get()->audio.door_talk_val) , 4);
	else 
		audio_output_open(AUDIO_CHANNEL_MONO,AK_AUDIO_SAMPLE_RATE_16000, get_sound_val(user_data_get()->audio.intercom_talk_val) , 4);
}

static void* audio_decode_task(void* arg)
{
	audio_decode_thread_run = true;
	
	bool audio_skip_frame = false;
	unsigned long long	check_audio_buffer_timer = get_sys_ms();
	
	while(audio_decode_task_run == true)	
	{
		audio_decode_queue *node = NULL;
		ak_thread_mutex_lock(&audio_decode_queue_head_mutex);
		if(queue_empty(&audio_decode_queue_head) == 0)
		{
			node = (audio_decode_queue *)queue_delete_next(&audio_decode_queue_head);
		}
		ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
		
		if(node != NULL)
		{
			if((audio_decode_ready == true)&&(audio_skip_frame == false))
			{
				if(audio_output_write(node->msg.data, node->msg.len) == false)
				{
					audio_decode_device_open((bool)arg);
				}
			}
			audio_decode_queue_node_del(node);
		}
		else 
		{
			unsigned long long ms = get_sys_ms();
			if((audio_skip_frame == true)||((ms - check_audio_buffer_timer)> 3000))
			{
				check_audio_buffer_timer = ms;

				int total, remain;
				if((audio_output_buffer_get(&total,&remain) == true)&&(remain > 10000))
				{
					audio_skip_frame = true;
				}
				else
				{
					audio_skip_frame = false;
				}
			}
        	ak_sleep_ms(1);//hlf:10.1声音断续
		}
		// ak_sleep_ms(1);
	}
	unsigned char mute_audio_frame[1024];
	for(int i =0; i< 10; i++)
	{
		memset(mute_audio_frame,0,sizeof(mute_audio_frame));
		if(audio_output_write(mute_audio_frame, sizeof(mute_audio_frame)) == false)
		{
			printf(" mute audio_output_write failed\n");
		}
		ak_sleep_ms(1);
	}
	ak_thread_mutex_lock(&audio_decode_queue_head_mutex);
	audio_decode_ready = false;
	while(queue_empty(&audio_decode_queue_head) == 0)
	{
		audio_decode_queue* node = (audio_decode_queue *)queue_delete_next(&audio_decode_queue_head);
		audio_decode_queue_node_del(node);
	}
	ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
	audio_decode_thread_run = false;
	ak_thread_exit();
	return NULL;
}



static bool audio_decode_wait_thread_quit(void)
{
	int timeout = 300;
	while(timeout--)
	{
		if(audio_decode_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}



static void audio_decode_queue_init(void)
{
	static bool is_first = true;
	if(is_first == false)
	{
		return ;
	}

	is_first = false;
	
	queue_initialize(&audio_decode_queue_free);
	queue_initialize(&audio_decode_queue_head);
	ak_thread_mutex_init(&audio_decode_queue_free_mutex,NULL);
	ak_thread_mutex_init(&audio_decode_queue_head_mutex,NULL);
	for(int i = 0 ; i < AUDIO_DECODE_QUEUE_MAX ; i++)
	{
		queue_insert((queue_s*)&audio_decode_buffer[i], &audio_decode_queue_free);
	}
}

bool audio_decode_open(bool falg)
{
	audio_decode_queue_init();
	
	if(audio_decode_task_run == true)
	{
		return false;
	}

	if(audio_decode_wait_thread_quit() == false)
	{
		return false;
	}
	audio_decode_task_run = true;
	audio_decode_ready = false;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, audio_decode_task, (void*)falg, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

bool audio_decode_queue_push(unsigned char* data ,int len)
{
	ak_thread_mutex_lock(&audio_decode_queue_head_mutex);
	if(audio_decode_ready == false)
	{	
		ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
		return false;
	}

	audio_decode_queue* node = audio_decode_queue_node_new(data,len);
	if(node != NULL)
	{
		queue_insert((queue_s *)node, &audio_decode_queue_head);
	}
	else
	{
		printf("node is null\n");
		ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
		return false;

	}
	ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
	
	return true;
}

bool audio_decode_start(bool falg)
{
	if(/*(audio_decode_thread_run == false)||*/(audio_decode_ready == true))
	{
		return false;
	}
	audio_decode_ready = true;
	audio_decode_device_open(falg);
	return true;
}

bool audio_decode_stop(void)
{
	if(audio_decode_ready == false)
	{
		return false;
	}
	audio_decode_ready = true;
	return true;
}
bool audio_decode_close(void)
{
	if(audio_decode_task_run == false)
	{
		return false;
	}
	printf("===========audio_decode_task_run is false\n");
	audio_decode_task_run = false;
	return true;
}

