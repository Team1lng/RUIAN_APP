#include "audio_output.h"
#include "audio_play.h"
#include "queue.h"
#include "mad.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../common/leo_audio_play.h"
#include"ak_mem.h"

bool ring_play_event_push(unsigned long arg1,unsigned long arg2);

#define AUDIO_PLAY_QUEUE_MAX 8

#define AUDIO_READ_MAX 128*1024
typedef struct
{
  int fd;

  
	bool ao_open_init;

	int vol;

  unsigned char *start;
  
  unsigned long length;
}mp3_mad;


typedef struct
{
	void* prev;
	void* next;

	audio_info msg;
}audio_play_queue;
static audio_play_queue audio_play_buffer[AUDIO_PLAY_QUEUE_MAX];
static queue_s audio_play_queue_free;
static queue_s audio_play_queue_head;
static ak_mutex_t audio_play_queue_free_mutex;
static ak_mutex_t audio_play_queue_head_mutex;
	
static bool audio_play_status = false;
static bool audio_play_stop = false;
static audio_play_queue* audio_play_queue_node_new(void)
{
	audio_play_queue* node = NULL;
	ak_thread_mutex_lock(&audio_play_queue_free_mutex);
	if(queue_empty(&audio_play_queue_free) == 0)
	{
		node = (audio_play_queue*)queue_delete_next(&audio_play_queue_free);	
	}
	ak_thread_mutex_unlock(&audio_play_queue_free_mutex);
	return node;
}

static void audio_play_queue_node_del(audio_play_queue* node)
{
	if(node != NULL)
	{
		ak_thread_mutex_lock(&audio_play_queue_free_mutex);
		node->msg.end = NULL;
		node->msg.start = NULL;
		queue_insert((queue_s*)node, &audio_play_queue_free);
		ak_thread_mutex_unlock(&audio_play_queue_free_mutex);
	}
}


static audio_play_queue* audio_play_queue_empty(void)
{
	if(queue_empty(&audio_play_queue_head))
	{
		return NULL;
	}
	
	audio_play_queue * node = NULL;
	ak_thread_mutex_lock(&audio_play_queue_head_mutex);
	while(!queue_empty(&audio_play_queue_head))
	{
		if(node != NULL)
		{
			audio_play_queue_node_del(node);
		}
		node = (audio_play_queue *)queue_delete_next(&audio_play_queue_head);
	}
	ak_thread_mutex_unlock(&audio_play_queue_head_mutex);
	return node;
}


static void audio_play_finish_wait(void)
{

	unsigned int timeout = 0;
	while(audio_play_stop == false)
	{
		timeout++;
		if(timeout > 100)
		{
			timeout = 0;
			int total,free;
			if((audio_output_buffer_get(&total,&free) == false)||(free == 0))
			{
				break;
			}
		}
		ak_sleep_ms(1);
	}
}

static bool audio_play_pcm(const audio_info* info)
{
	if( info->start != NULL)
	{
		ring_play_event_push((unsigned long)info->start,1);
	}

	if(info->data_type == 0)
	{
		extern unsigned char* get_rom_bin_base(void);
		unsigned char* data = get_rom_bin_base() + info->data.bin.offset;
		int send_size = info->data.bin.size;
		int free_size = 0;
		int read_size = 0;

		unsigned char buf_temp[4096] = {0};
		while(send_size > 0)
		{
			free_size = send_size < 4096?send_size:4096;
			memcpy(buf_temp,&data[read_size],free_size);
			if(audio_output_write(buf_temp , free_size) == false)
			{
				return false;
			}
			read_size += free_size;
			send_size -= free_size;
		}
	}
	else
	{
		int fd = open(info->data.file_path,O_RDONLY);
		if(fd < 0)
		{
			return false;
		}
		unsigned char data[4096] = {0};
		int data_len = 0;
		while((data_len = read(fd,data,4096)) > 0)
		{
			
			audio_output_write(data , data_len);
		}
		close(fd);
	}
	audio_play_finish_wait();
	return true;
}







static enum mad_flow mp3_input(void *data, struct mad_stream *stream)
{
	mp3_mad *buffer = data;
	if(stream->next_frame != NULL)
	{
		buffer->length = &buffer->start[buffer->length] - stream->next_frame;
		memmove(buffer->start,stream->next_frame,buffer->length);
	}
	int	read_len = read(buffer->fd,&buffer->start[buffer->length],AUDIO_READ_MAX - buffer->length);
	if (read_len  <= 0)
	{
		return MAD_FLOW_STOP;
	}
	buffer->length += read_len;
	mad_stream_buffer(stream, buffer->start, buffer->length);
	return MAD_FLOW_CONTINUE;
}



static inline signed int mp3_scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow mp3_output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;
	mp3_mad *buffer = data;
	if (audio_play_stop == true)
	{
		return MAD_FLOW_STOP;
	}
	pcm->channels = 1;
	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	if(buffer->ao_open_init == false)
	{
		audio_output_open(pcm->channels, pcm->samplerate ,buffer->vol,6);
		buffer->ao_open_init  = true;
	}
	#define MP3_OUTPUT_FRAME_SIZE 2304//4096//1180//
	unsigned char sample_buffer[MP3_OUTPUT_FRAME_SIZE] = {0};

	int sample_read_size = 0;

	while (nsamples--)
	{
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */
		if (audio_play_stop == true)
		{
			return MAD_FLOW_STOP;
		}

		sample = mp3_scale(*left_ch++);
		sample_buffer[sample_read_size++] = (sample >> 0) & 0xff;
		sample_buffer[sample_read_size++] = (sample >> 8) & 0xff;

		if (nchannels == 2)
		{
			sample = mp3_scale(*right_ch++);
			sample_buffer[sample_read_size++] = (sample >> 0) & 0xff;
			sample_buffer[sample_read_size++] = (sample >> 8) & 0xff;
		}

		/* output sample(s) in 16-bit signed little-endian PCM */
		if(audio_play_stop == true)
		{
			return MAD_FLOW_STOP;
		}
	}

	if (sample_read_size)
	{
		audio_output_write((unsigned char *)sample_buffer, sample_read_size);
	}

	if (audio_play_stop == true)
	{
		return MAD_FLOW_STOP;
	}
	return MAD_FLOW_CONTINUE;
}


static enum mad_flow mp3_error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
	return MAD_FLOW_CONTINUE;
}



static void audio_play_mp3(const audio_info* info)
{
	mp3_mad mp3_buffer;
	struct mad_decoder decodec;
	
	audio_volume_set(info->volume);
	mp3_buffer.ao_open_init = false;
	mp3_buffer.vol = info->volume;
	
	unsigned char *buffer = ak_mem_alloc(MODULE_ID_AO,AUDIO_READ_MAX);
	if(info->data_type == 0)
	{
		extern unsigned char* get_rom_bin_base(void);
		mp3_buffer.start = get_rom_bin_base() + info->data.bin.offset;
		mp3_buffer.length = info->data.bin.size;
		mp3_buffer.fd = -1;
	}
	else
	{
		mp3_buffer.fd  =open(info->data.file_path,O_RDONLY);
		if(mp3_buffer.fd < 0)
		{
			return ;
		}
		mp3_buffer.start = buffer;
		mp3_buffer.length = 0;		
	}
	
	mad_decoder_init(&decodec,&mp3_buffer,mp3_input,0,0,mp3_output,mp3_error,0);
	if( info->start != NULL)
	{
		 ring_play_event_push((unsigned long)info->start,1);
	}

	mad_decoder_run(&decodec,MAD_DECODER_MODE_SYNC);

	audio_play_finish_wait();

	mad_decoder_finish(&decodec);
	ak_mem_free(buffer);

	if(mp3_buffer.fd >= 0)
	{
		close(mp3_buffer.fd);
	}
}

static void audio_play_decode_start(const audio_info* info)
{
	audio_output_open(info->ch, info->rate,info->volume,5);
	
	if(info->type == AK_AUDIO_TYPE_PCM)
	{
		audio_play_pcm(info);
	}
	else
	{
		audio_play_mp3(info);
	}
}




static void* audio_play_task(void* arg)
{
	audio_play_queue* node = NULL;
	while(1)
	{
		if(node != NULL)
		{
			audio_play_queue_node_del(node);
		}
		node = audio_play_queue_empty();

		if( node != NULL)
		{
			audio_play_stop = false;
			
			audio_play_status = true;
			audio_play_decode_start(&node->msg);
			if(node->msg.end != NULL)
			{
				ring_play_event_push((unsigned long)node->msg.end,2);
			}
			
			audio_play_status = false;
		}
		else
		{
			ak_sleep_ms(10);
		}
	}

	ak_thread_exit();
	return NULL;
}

bool audio_play_init(void)
{
	queue_initialize(&audio_play_queue_free);
	queue_initialize(&audio_play_queue_head);
	ak_thread_mutex_init(&audio_play_queue_free_mutex,NULL);
	ak_thread_mutex_init(&audio_play_queue_head_mutex,NULL);
	for(int i = 0 ; i < AUDIO_PLAY_QUEUE_MAX ; i++)
	{
		queue_insert((queue_s*)&audio_play_buffer[i], &audio_play_queue_free);
	}

	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, audio_play_task ,  NULL , ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
	return true;
}



bool audio_play(audio_info* info)
{
	audio_play_stop = true;
	audio_play_queue* node = audio_play_queue_node_new();
	if(node == NULL)
	{
		printf("audio play queue full \n");
		return false;
	}

	memcpy(&(node->msg),info,sizeof(audio_info));
	ak_thread_mutex_lock(&audio_play_queue_head_mutex);
	queue_insert((queue_s *)node, &audio_play_queue_head);
	ak_thread_mutex_unlock(&audio_play_queue_head_mutex);
	return true;
}

bool is_audio_play_ing(void)
{
	return audio_play_status;
}

bool audio_play_stop_set(void)
{
	audio_play_queue* node;
	while((node = audio_play_queue_empty()))
	{
		audio_play_queue_node_del(node);
	}
	while(audio_play_status == true)
	{
		audio_play_stop = true;
		ak_sleep_ms(1);
	}
	return true;
}


