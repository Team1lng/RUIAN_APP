#include "leo_key.h"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>


#include <errno.h>
#include <getopt.h>


#define DET_KEY_KO_PATH "/usr/modules/ak_saradc.ko"
#define DET_KEY_DEV "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
static int det_key_dev_fd = -1;

static key_device det_key_level = 0;



/***
** return:数值高于680 低于800
** monitor:数值小于100
** lock:数值小于1400 大于1300
** talk:数值高于1800小于1900
***/
#define DET_KEY_RETURN_ACTION_HIHG_LEVEL 	850
#define DET_KEY_RETURN_ACTION_LOW_LEVEL 	650

#define DET_KEY_MONITOR_ACTION_LEVEL 		200

#define DET_KEY_LOCK_ACTION_HIHG_LEVEL 		1450
#define DET_KEY_LOCK_ACTION_LOW_LEVEL 		1250

#define DET_KEY_TALK_ACTION_HIHG_LEVEL 		1950
#define DET_KEY_TALK_ACTION_LOW_LEVEL 		1750


static int det_key_pin_read(void)
{
    char temp[64] = {0};
    lseek(det_key_dev_fd,0,SEEK_SET);
    if(read(det_key_dev_fd,temp,sizeof(temp)) < 0)
    {
        printf("read det call value failed \n");
        return -1;
    }

    /***** 将字符串转换为10进制的数值 *****/
    int value = 0;
    sscanf(temp,"%d",&value);

    if(value <= DET_KEY_RETURN_ACTION_HIHG_LEVEL && value >= DET_KEY_RETURN_ACTION_LOW_LEVEL)
    {
		return 1;
	}
	else if(value <= DET_KEY_MONITOR_ACTION_LEVEL)
	{
		return 2;
	}
	else if(value <= DET_KEY_LOCK_ACTION_HIHG_LEVEL && value >= DET_KEY_LOCK_ACTION_LOW_LEVEL)
	{
		return 3;
	}
	else if(value <= DET_KEY_TALK_ACTION_HIHG_LEVEL && value >= DET_KEY_TALK_ACTION_LOW_LEVEL)
	{
		return 4;
	}
	else 
	{
		return 0;
	}	
		
}



//初始化
bool det_key_pin_init(void)
{
    /***** 安装AVIN 的驱动文件 *****/
    system("insmod "DET_KEY_KO_PATH);
    /***** 打开设备结点 ******/
    det_key_dev_fd = open(DET_KEY_DEV,O_RDONLY);
    if(det_key_dev_fd < 0)
    {
        printf("open %s failed \n",DET_KEY_DEV);
        return false;
    }
    det_key_level = det_key_pin_read();
    
    return true;
}


bool det_key_pin_uninit(void)
{
    if(det_key_dev_fd < 0)
    {
        return true;
    }

    close(det_key_dev_fd);
    det_key_dev_fd = -1;
    system("remmod "DET_KEY_KO_PATH);
    return true;
}


key_device det_key_pin_call(void)
{
    key_device key = det_key_pin_read();

    if(key != det_key_level)
    {
        det_key_level = key;
        if(det_key_level == 0)
        {
			return false;
        }
    }
	return det_key_level;
}

