#ifndef LEO_GPIO_H
#define LEO_GPIO_H
#include <stdbool.h>
#include "ak_thread.h"


enum audio_open{

    AUDIO_DOOR1 = 0X01,
    AUDIO_DOOR2,
    AUDIO_INTERCOM,
    AUDIO_CLOSE_ALL
};



void speak_enable_set(bool enable);

int check_speak_static(void);//高电平：功放开


void backlight_open(bool enable);

void door_chrime_en(bool enable);

void custom_open(bool enable);

#endif

