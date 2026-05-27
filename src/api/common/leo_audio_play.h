#ifndef _LEO_AUDIO_PLAY_H_
#define _LEO_AUDIO_PLAY_H_
#include "stdbool.h"

#include <sys/time.h>

void touch_sound_play(void);

bool door_ring_play(int ring_index,int volume,audio_play_callback start,audio_play_callback end);

bool custom_door_ring_play(char * file_path,int volume,audio_play_callback start,audio_play_callback end);

bool interphone_ring_play(int volume,audio_play_callback start,audio_play_callback end);

bool open_door_ring_play(int volume);
bool audio_volume_set(int vol);
bool pcm_16bit_volume_cover(unsigned char * src, int size, int volume);

#endif

