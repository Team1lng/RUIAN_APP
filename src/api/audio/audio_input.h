#ifndef _AUDIO_INPUT_H_
#define _AUDIO_INPUT_H_
#include <stdbool.h>
#define AUDIO_USUAL 0 // 0为通用版本，1为波兰十寸室内机版本


bool audio_input_open(void);

bool audio_input_start(void);

bool audio_input_stop(void);

bool audio_input_close(void);

#endif

