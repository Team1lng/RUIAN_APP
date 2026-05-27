#include "network_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include "ak_thread.h"
#include "ak_mem.h"
#include <net/if_arp.h>
#include <netinet/in.h>
//#include<netinet/ip.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "queue.h"
#include "leo_api.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "../../layout/user_data.h"
#define UPGRADE_FILE_PATH "/mnt/tf/net_camera.up"
#define UPGRADE_PACKAGE_SIZE_MAX 1510 


static bool network_upgrade_sent_task_run = false;//任务

static ak_mutex_t network_upgrade_send_mutex;


static int file_size_get(FILE* fp){
    fseek(fp,0,SEEK_END);
    int file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    return file_size;
}

static void printf_progress(int percentage) {

    // char buff[128] = {0};
    // int str_len = 0;
    printf("%d%%", percentage);
    putc('\b', stdout);
    putc('\b', stdout);
    putc('\b', stdout);
    fflush(stdout);
}

//hlf:等待门口机升级
static void wait_door_upgrade(lv_task_t * task)
{
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), 77);
	lv_obj_t* bar = lv_obj_get_child_form_id(lv_obj_get_child(obj, NULL), 1087);

	lv_obj_del(bar);
	network_upgrade_sent_task_run = false;
}

extern bool cancle_falg;
static void *network_upgrade_send_package_task(void *arg){

	
	ak_thread_mutex_init(&network_upgrade_send_mutex, NULL);

	system("cp /mnt/tf/net_camera.up /tmp");
	FILE *fp = fopen("/tmp/net_camera.up", "rb");//只读打开文件
	if(fp == NULL) //打开失败
	{
		printf("=====>>>>> upgrade file open err <<<<<=====\n");
		return NULL;//返回 直接结束
	}

	
	int file_size = file_size_get(fp);//获取到文件总大小
	int idnex_num = 0;//发包的数量
	int sent_size = 0;//发送的文件长度
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), 77);
	lv_obj_t* bar = NULL;
	if(obj != NULL){
		
		bar = lv_bar_create(lv_obj_get_child(obj, NULL), NULL);
		lv_obj_set_id(bar,1087);
		lv_obj_set_size(bar, 354, 10);
		lv_obj_align(bar, bar->parent, LV_ALIGN_IN_TOP_MID, 0, 130);
		lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x393939));
		lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
		lv_bar_set_range(bar, 0, 100);
		lv_bar_set_start_value(bar, 0, LV_ANIM_OFF);

	}
	
	
	network_upgradecmd_data node ;//包
	memset(&node, 0, sizeof(network_upgradecmd_data));
	
	
	while(network_upgrade_sent_task_run == true){
		
		int ret = 0;
		//ak_thread_mutex_lock(&network_upgrade_send_mutex);	//上锁
		if((ret = fread(node.buf,sizeof(char), sizeof(node.buf),fp)) > 0)//读取到数据
		{
			//ak_thread_mutex_unlock(&network_upgrade_send_mutex);
			node.cmd = NET_COMON_CMD_UPGRADE_OUTDOOR;
			
			node.device = DEVICE_GROUP_OUTDOOR;
			node.arg1 = ++idnex_num;//发送的第几个包
			node.arg2 = ret;//数据长度
		
			sent_size += ret;//已经发送的数据长度
			printf("====>>>> read file num:%d  len:%d <<<<=====\n",idnex_num,ret);
			
			network_sendupgrade_cmd_data(&node);//封包和发送
			
			
			printf_progress(sent_size*100/file_size);
			if(sent_size*100/file_size <= 95)
				lv_bar_set_value(bar, sent_size*100/file_size, LV_ANIM_OFF);
			
			ak_sleep_ms(400);
			
		}
		else if(ret == 0){
			//ak_thread_mutex_lock(&network_upgrade_send_mutex);	//上锁ak_thread_mutex_lock(&network_upgrade_send_mutex);	//上锁
			/*arg1 == 1检查设备是否在线 arg1 == 2 arg2 == 1结束发送*/
			ak_sleep_ms(200);
			network_cmd_data_init(data);

			data.cmd			= NET_COMON_CMD_UPGRADE_OUTDOOR;
			data.arg1			= 2;
			data.arg2			= 1;
			data.device 		= DEVICE_ALL;
			network_send_cmd_data(&data);
			printf("read file end! file_len =:%d sent_len =:%d\n",file_size,sent_size);

			// lv_layout_task_create(wait_door_upgrade, 30000, LV_TASK_PRIO_MID, NULL);
			// lv_obj_del(bar);
			
			network_upgrade_sent_task_run = false;
		}

		//ak_thread_mutex_unlock(&network_upgrade_send_mutex);
		
		
	}
		
	fclose(fp);
	system("rm /tmp/net_camera.up");
	network_upgrade_sent_task_run = false;

	ak_thread_exit();
	return NULL;

}



static ak_pthread_t thread_upgrade_id;

bool network_upgrade_send_package_open(void){

	if(network_upgrade_sent_task_run == true){
		return false;
	}
	
	network_upgrade_sent_task_run = true;
	
	ak_thread_create(&thread_upgrade_id, network_upgrade_send_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_upgrade_id);

	return true;
}




//发送任务关闭
bool network_upgrade_sent_package_close(void){
	if(network_upgrade_sent_task_run == false){
		
		return false;
	}else{
		network_upgrade_sent_task_run = false;
		ak_thread_cancel(thread_upgrade_id);

	}
	return true;
}





