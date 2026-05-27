#ifndef AUDIO_OUTPUT_H 
#define AUDIO_OUTPUT_H
#include "ak_ao.h"
#include <stdbool.h>
#include <string.h>

#include "ak_thread.h"




bool audio_output_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate,int vol,int gain);

bool audio_output_write(unsigned char* data,int len);

bool audio_output_close(void);


bool audio_output_buffer_status_printf(void);


bool audio_output_buffer_get(int* total,int* remain);

bool audio_output_volume_set(int vol);

int audio_output_volume_get(void);

int audio_output_buffer_query(void);

#endif


