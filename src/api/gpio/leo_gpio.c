#include "leo_gpio.h"
#include "gpio_api.h"
#include "../../layout/layout_define.h"
#include <pthread.h>

/*============================cyy:自定义引脚==========================================*/
#define CUSTOM_EN_GPIO  28

void custom_open(bool enable){
	bool flag=false;
	flag=gpio_set(CUSTOM_EN_GPIO, enable == true?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
	if(flag){
		printf("=====================pin28 set succeed  !!!=====================\n");
	}
}

/*============================cyy:背光引脚==========================================*/
#define BL_EN_GPIO  82

void backlight_open(bool enable){
	gpio_set(BL_EN_GPIO, enable == true?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
}

/*============================cyy:引脚==========================================*/
#define AMP_ON_GPIO 2

void speak_enable_set(bool enable)
{
	gpio_set(AMP_ON_GPIO, enable == true?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
}

/*============================cyy:门引脚==========================================*/
#define DOOR_CHRIME 36
void door_chrime_en(bool enable)
{
	gpio_set(DOOR_CHRIME, enable == true?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
}


int check_speak_static(void)//高电平：功放开
{
	GPIO_LEVEL  level;
	gpio_read(AMP_ON_GPIO,&level);
	return (level == GPIO_LEVEL_HIGH?false:true);
}


/************************************************************
** 函数说明: 门钟按压检测
** 作者: xiaoxiao
** 日期: 2023-04-24 20:30:17
** 参数说明: 
** 注意事项: 
************************************************************/
#define BELL_DET 35
#define DOOR_CALL_DELAY 1000
static void *bell_det_task(void *arg)
{
	int gpio_group_pin[1] = {BELL_DET};
	GPIO_LEVEL gpio_group_level[1] = {GPIO_LEVEL_HIGH};
	for (int i = 0; i < 4; i++)
	{
		if (gpio_read(gpio_group_pin[i], &gpio_group_level[i]) == false)
		{
			printf("read gpio%d level failed \n", gpio_group_pin[i]);
		}
	}

	unsigned long long door1_call_ts = 0;

	unsigned long long door_call_ts;
	GPIO_LEVEL level;

	printf("***** gpio detection task create sccess ! *****\n");
	while (1)
	{
		/***** door1 检测 *****/
		door_call_ts = user_timestamp_get();
		if ((gpio_read(gpio_group_pin[0], &level) == true) && (level != gpio_group_level[0]))
		{
			gpio_group_level[0] = level;
			if ((level == GPIO_LEVEL_LOW) && (abs(door_call_ts - door1_call_ts) > DOOR_CALL_DELAY))
			{
				door1_call_ts = user_timestamp_get();
				bell_press_event_push(0);
			}
		}
		usleep(10 * 1000);
	}
	return NULL;
}


void bell_det_init(void)
{
	pthread_t thread_t;
	pthread_create(&thread_t, NULL, bell_det_task, NULL);
}