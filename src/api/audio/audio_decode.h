#ifndef _AUDIO_DECODE_H_
#define _AUDIO_DECODE_H_
#include <stdbool.h>

bool audio_decode_open(bool falg);

bool audio_decode_start(bool falg);

bool audio_decode_stop(void);

bool audio_decode_close(void);

bool audio_decode_queue_push(unsigned char* data ,int len);

#endif

