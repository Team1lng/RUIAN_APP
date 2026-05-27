#ifndef _LEO_KEY_H_
#define _LEO_KEY_H_
#include <stdbool.h>
typedef enum
{
	key_UNKONW,
	key_return = 1,
	key_monitor,
	key_lock,
	key_talk,
	key_TOTAL
}key_device;

/***
** date 2022/05/05
** 初始化室内机按键检测IO设备
** 返回false 初始化失败
***/
bool det_key_pin_init(void);



/***
** date 2022/05/05
** 读取当前门口机call检测的数值
** 返回数值 对应的设备按下 
***/
key_device det_key_pin_call(void);

#endif