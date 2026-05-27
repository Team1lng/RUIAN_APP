#include "layout_define.h"
#include "ak_common_graphics.h"
#include "../api/queue/queue.h"
#include "ak_thread.h"
#include "user_data.h"
#include "layout_watch_dog.h"

#define OS_EVENT_NUM_MAX 32
#define Urmet 0

bool test_flag = false;

typedef enum
{
	OS_EVENT_TYPE_NONE,
	OS_EVENT_TYPE_RECORD,
	/*
	* arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
	* arg2:arg1= 1,arg2参数无意义
	*	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
	*	   arg1=3,arg2=1:内存正常，2，内存已满
	*/
	OS_EVENT_TYPE_SD, 

	/*arg1:重复的ID号*/
	OS_EVENT_TYPE_DEVICE_REPEAT,

	OS_EVENT_TYPE_INTERPHONE,

	/* 铃声播放回调 arg1:函数入口，arg2:1:起始,arg2:2结束*/
	OS_EVENT_TYPE_RING,

	/*arg1 :arg1 = DEVICE_OUTDOOR_1 or DEVICE_OUTDOOR_2*/
	OS_EVENT_TYPE_OUTDOOR_CALL,
	OS_EVENT_TYPE_KEY_CALL,

	/*
	*	arg1(高8位,低8位预留):事件类型:1:其他设备要通话。arg2:高8位表示室内机，低8位表示门口机 (此事件上传到主线程，一般处理原则，退出监控页面，
		但是此设备正与其他户外机通话状态下则不做任何处理)
	*
	*					2:表示此命令正忙，需要等待,低8位:正忙的详细信息。arg2:高8位代表室内机，低8位 代表是户外机
	*	
	*/
	
    OS_EVENT_TYPE_TUYA,
	OS_EVENT_TYPE_INDOOR_CMD,
	
	OS_EVENT_TYPE_MOTION,

	OS_EVENT_TYPE_OUTDOOR_STATUS_CHANGE,

	OS_EVENT_TYPE_BELL_PRESS,
	OS_EVENT_TYPE_STANDBY,
}event_type;


typedef struct
{
	event_type type;
	unsigned long arg1;
	unsigned long arg2;
	
}event_msg;


typedef struct
{
	void* prev;
	void* next;

	event_msg msg;
	
}lv_event_info;



static event_pro_callback bell_press_event_callback = NULL;
static event_pro_callback standby_event_callback = NULL;
static event_pro_callback snap_event_callback = NULL;
static event_pro_callback record_video_event_callback = NULL;
static event_pro_callback sd_event_callback = NULL;

static void device_id_repeat_callback_default(unsigned long arg1,unsigned long arg2);
static event_pro_callback device_id_repeat_callback = device_id_repeat_callback_default;

static event_pro_callback interphone_call_callback = NULL;

static event_pro_callback monitor_call_callback = NULL;

static event_pro_callback outdoor_status_change = NULL;

static event_pro_callback monitor_motion_callback = NULL;

static event_pro_callback key_call_callback = NULL;



static event_pro_callback indoor_cmd_callback = NULL;
static event_pro_callback network_event_callback = NULL;
static event_pro_callback tuya_event_callback = NULL;



static const layout* cur_layout = NULL;
static lv_task_t* lv_os_event_ptask = NULL;

static queue_s lv_os_event_queue_head;
static queue_s lv_os_event_queue_free;

static ak_mutex_t lv_os_event_queue_head_mutex;
static ak_mutex_t lv_os_event_queue_free_mutex;

static lv_event_info lv_os_event_msg_buffer[OS_EVENT_NUM_MAX];

static lv_event_info* lv_os_event_queue_node_new(void)
{
	lv_event_info* node = NULL;
	ak_thread_mutex_lock(&lv_os_event_queue_free_mutex);
	if(queue_empty(&lv_os_event_queue_free) == 0)
	{
		node = (lv_event_info*)queue_delete_next(&lv_os_event_queue_free);
	}
	ak_thread_mutex_unlock(&lv_os_event_queue_free_mutex);
	return node;
}




static void lv_os_event_queue_node_del(lv_event_info* node)
{
	if(node != NULL)
	{
		node->msg.type = OS_EVENT_TYPE_NONE;
		node->msg.arg1 = node->msg.arg2 = 0;
		ak_thread_mutex_lock(&lv_os_event_queue_free_mutex);
		queue_insert((queue_s*)node, &lv_os_event_queue_free);
		ak_thread_mutex_unlock(&lv_os_event_queue_free_mutex);
	}
}




static void lv_os_event_handle(const event_msg* msg)
{
	switch(msg->type)
	{
		case OS_EVENT_TYPE_RECORD:
			if((msg->arg1 == 1 ) && (snap_event_callback != NULL))
			{
				snap_event_callback(msg->arg1,msg->arg2);
			}
			else if((msg->arg1 == 2) && (record_video_event_callback != NULL))
			{
				record_video_event_callback(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_SD:
			if(sd_event_callback != NULL)
			{
				sd_event_callback(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_DEVICE_REPEAT:
			if(device_id_repeat_callback != NULL)
			{	
				device_id_repeat_callback(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_INTERPHONE:
			if(interphone_call_callback != NULL)
			{
				interphone_call_callback(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_RING:
			if(msg->arg1 != 0)
			{
				typedef void(*func_callback)(void);
				func_callback func  = (func_callback)msg->arg1;
				func();
			}
		break;
		case OS_EVENT_TYPE_OUTDOOR_CALL:
			if(monitor_call_callback != NULL)
			{
				monitor_call_callback(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_KEY_CALL:
			if(key_call_callback != NULL)
			{
				key_call_callback(msg->arg1,msg->arg2);
			}
		break;

		case OS_EVENT_TYPE_INDOOR_CMD:
		if(indoor_cmd_callback != NULL)
		{
			indoor_cmd_callback(msg->arg1,msg->arg2);
		}
		break;
		case OS_EVENT_TYPE_TUYA:
		if(tuya_event_callback != NULL)
		{
			tuya_event_callback(msg->arg1,msg->arg2);
		}
		break;
		case OS_EVENT_TYPE_MOTION:
		if(monitor_motion_callback != NULL)
		{
			monitor_motion_callback(msg->arg1,msg->arg2);
		}
		break;
		case OS_EVENT_TYPE_OUTDOOR_STATUS_CHANGE:
			if(outdoor_status_change != NULL)
			{
				outdoor_status_change(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_BELL_PRESS:
			if(bell_press_event_callback != NULL)
			{
				bell_press_event_callback(msg->arg1,msg->arg2);
			}
		break;
		case OS_EVENT_TYPE_STANDBY:
					if(standby_event_callback != NULL)
					{
						standby_event_callback(msg->arg1,msg->arg2);
					}
		break;
		default:
			printf("unknow event type:%d \n",msg->type);
		break;
	}
}


static void lv_os_event_task(struct _lv_task_t* task_t)
{
	lv_event_info* node = NULL;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);	
	if(queue_empty(&lv_os_event_queue_head) == 0)//队列不为空
	{
		node = (lv_event_info*)queue_delete_next(&lv_os_event_queue_head);//从队列中读取出第一个节点
	}
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	if(node != NULL)
	{
		event_msg msg = node->msg;
		lv_os_event_queue_node_del(node);//将节点内容删除
		lv_os_event_handle(&msg);	
	}

	extern bool standby_timer_check();
	standby_timer_check();
}



static void lv_os_event_task_init(void)
{
	ak_thread_mutex_init(&lv_os_event_queue_free_mutex, NULL);
	ak_thread_mutex_init(&lv_os_event_queue_head_mutex, NULL);
	
	queue_initialize(&lv_os_event_queue_head);
	queue_initialize(&lv_os_event_queue_free);
	for(int i = 0 ; i < OS_EVENT_NUM_MAX ; i++)
	{
		queue_insert((queue_s *)&lv_os_event_msg_buffer[i], &lv_os_event_queue_free);
	}

	
	lv_os_event_ptask = lv_task_create(lv_os_event_task, 30, LV_TASK_PRIO_HIGHEST, NULL);
	lv_task_ready(lv_os_event_ptask);
}



static unsigned char *picture_data = NULL;

unsigned char * picture_data_get()
{
	return picture_data;
}


void background_open()
{

	static rom_bin_info bg_img = rom_bin_raw_get();
	if (picture_data == NULL)
	{

		picture_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, 1024 * 600 * 4);

	}

	rom_bin_raw_init(bg_img, picture_data, 1024, 600);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	fb_video_mode_enable(false);
	extern bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);

	char picture_name[64] = {0};
	bzero(picture_name, sizeof(picture_name));

	strcpy(picture_name, SYSTEM_BG_FILE_PATH);
	lv_jpg_decode_data(picture_name, &bg_img, 1024, 600);

	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_color(lv_disp_get_default(),lv_color_hex(0X000000));
	lv_disp_set_bg_image(lv_disp_get_default(), &bg_img);

}

void background_close()
{

	lv_disp_set_bg_image(lv_disp_get_default(),NULL);
	
}


/**
 * 初始化界面布局
 * 该函数负责依次初始化系统的各个界面组件，包括背景和主要功能模块
 * 调用顺序确保了界面层级的正确堆叠和功能模块的正常初始化
 * 
 * @note 此函数仅初始化布局，不处理事件逻辑
 * @note 函数调用顺序可能影响界面元素的显示层级
 */
static void layout_init(void)
{
    background_open(); /* 打开背景层，作为所有界面的底层 */
    layout_monitor_init(); /* 初始化监控模块布局，包括监控视图、控制按钮等元素 */ 
    layout_interphone_init();/* 初始化对讲机模块布局，配置音频控制面板和状态显示区域 */   
    layout_motion_init(); /* 初始化运动检测模块布局，设置检测区域和报警显示界面 */
}
    

char tuya_uid[64] = {0};
char tuya_key[64] = {0};

extern bool net_online_device[DEVICE_TOTAL];
extern char device_busy_disapear_times[DEVICE_TOTAL];
extern bool occupy_resource[DEVICE_TOTAL];

//======================================================//
static void account_heartbeat_send_task(struct _lv_task_t *task_t)
{

	//======================================================//
	/****当5秒内没收到占用门口机繁忙信号，就解除当前门口机繁忙****/
	for(int i = DEVICE_UNIT_OUTDOOR_1 ;i <= DEVICE_OUTDOOR_15; i++)
	{    
		/***防止接收了呼叫的室内机不正常退出****/
		unsigned int mask = outdoor_call_mask_get(i);
		// unsigned int family_id = outdoor_call_mask_get(i) >> 16;
		//获取设备id
		int index[5] = {0};
		index[0] = user_data_get()->other.network_device == DEVICE_INDOOR_ID1? DEVICE_INDOOR_ID2:DEVICE_INDOOR_ID1;
		index[1] = (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||
		(user_data_get()->other.network_device == DEVICE_INDOOR_ID2)?DEVICE_INDOOR_ID3:DEVICE_INDOOR_ID2;
		index[2] = (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2)
		||(user_data_get()->other.network_device == DEVICE_INDOOR_ID3)?DEVICE_INDOOR_ID4:DEVICE_INDOOR_ID3;
		index[3] = (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2)||
		(user_data_get()->other.network_device == DEVICE_INDOOR_ID3)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID4)?DEVICE_INDOOR_ID5:DEVICE_INDOOR_ID4;
		index[4] = (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2)||
		(user_data_get()->other.network_device == DEVICE_INDOOR_ID3)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID4)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID5)?DEVICE_INDOOR_ID6:DEVICE_INDOOR_ID5;

		for(int j=0 ;j<5;j++)
		{	
			if((mask & (0x01<<index[j])))
			{
				if(device_busy_disapear_times[i] ++ > 5)
				{
					outdoor_call_mask_set(i, 0);
					monitor_data_busy_enable(i,false);
					device_busy_disapear_times[i] = 0;
				}
				
			}
		}
	}

//======================================================//
/****当5秒内没收到主机发过来的tuya监控繁忙信号，就解除繁忙****/
	for(int i = DEVICE_UNIT_OUTDOOR_1 ;i <= DEVICE_OUTDOOR_15; i++)
	{
		if(monitor_data_busy_get(i) == true)
		{
			if(device_busy_disapear_times[i] ++ > 5)
			{
				monitor_data_busy_enable(i,false);
				device_busy_disapear_times[i] = 0;
			}
			
		}
	}

/***********告诉设备当前此设备在占用门口机资源**************/
		for(int i = DEVICE_UNIT_OUTDOOR_1 ;i <= DEVICE_OUTDOOR_15; i++)
		{
			if(occupy_resource[i] != false)
			{
				//printf("occupy_resource device is %d\n",i);
				network_cmd_data_init(data);
				data.cmd = NET_COMMON_CMD_DATA_BUSY;
				data.arg1 = i;
				data.arg2 = user_data_get()->other.family_id;
				if((i == DEVICE_UNIT_OUTDOOR_1) || (i == DEVICE_UNIT_OUTDOOR_2))
				{
					data.device = DEVICE_GROUP;
				}
				else
				{
					data.device = DEVICE_ALL;
				}
				network_send_cmd_data(&data);
			}
	
		}
}	


static void(*obj_touch_event_callback)(void) = NULL;
void layout_obj_touch_event_register(void(*handle)(void))//触摸按压回调
{
	obj_touch_event_callback = handle;
}

/************************************************************
** 函数说明: 待机事件
** 作者: xiaoxiao
** 日期: 2023-04-24 21:08:25
** 参数说明: 
** 注意事项: 
************************************************************/
void goto_standby_event(unsigned long arg1,unsigned long arg2)
{
	goto_layout(pLAYOUT(home));

}
static void layout_logo_watch_dog_push_task(lv_task_t *t)
{
	watch_dog_push();
}

/************************************************************
** 函数说明: 板极初始化
** 作者: 
** 日期: 2025-06-16 07:56:25
** 参数说明: 
** 注意事项: 
************************************************************/
void leo_api_init(void)
{
	test_flag = false;
	
	lv_os_event_task_init();//系统任务初始化
	
//	user_time_init();

	extern void media_file_list_init(void);
	media_file_list_init();//文件系统初始化

	extern void video_decode_init(void);
	video_decode_init();//视频解码初始化

	extern void video_raw_init(void);
	video_raw_init();//视频原始数据处理初始化

	extern void fb_video_mode_enable(bool);
	fb_video_mode_enable(false);

	extern void video_record_init(void);
	video_record_init();//背景的视频模式使能初始化 

	extern bool audio_play_init(void);
	audio_play_init();//音频播放初始化

	extern void video_play_init(void);
	video_play_init();//视频播放初始化

	extern bool network_init(network_device device);
	network_init(user_data_get()->other.network_device);//网络初始化


	fb_video_mode_enable(false);//关闭video播放模式

	layout_init();//进入页面前的初始化以及对室内通话和监控的初始化
	
	speak_enable_set(1);//功放使能

	//===================cyy==========================
	//custom_open(1);
	

	//
	layout_obj_touch_event_register(touch_sound_play);//触摸声音事件注册

	standby_event_event_register(goto_standby_event);//休眠事件

	bell_det_init();//门铃线程

	watch_dog_start();//看门狗

	lv_task_create(layout_logo_watch_dog_push_task, 1000, LV_TASK_PRIO_MID, NULL);

	extern void bell_press_default_event(unsigned long arg1,unsigned long arg2);
	bell_press_event_register(bell_press_default_event);//门铃按下事件
	
	lv_task_create(account_heartbeat_send_task, 1000, LV_TASK_PRIO_MID, NULL);


}


/************************************************************
** 函数说明: 门铃按压事件
** 作者: 
** 日期: 2025-06-16 07:56:25
** 参数说明: 
** 注意事项: 
************************************************************/
bool bell_press_event_push(char arg)//抓拍事件加入任务队列
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_BELL_PRESS;
	node->msg.arg1 = 1;
	node->msg.arg2 = arg;
	
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}


void bell_press_event_register(event_pro_callback handle)//拍照完成回调函数
{
	bell_press_event_callback = handle;
}


void standby_event_event_register(event_pro_callback handle)//待机回调函数
{
	standby_event_callback = handle;
}

bool record_jpeg_event_push(bool is_finish)//抓拍事件加入任务队列
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_RECORD;
	node->msg.arg1 = 1;
	node->msg.arg2 = is_finish?1:0;
	
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool stanby_event_push(bool is_finish)//待机事件加入任务队列
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_STANDBY;
	node->msg.arg1 = 1;
	node->msg.arg2 = is_finish?1:0;
	
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}


bool record_video_event_push(bool is_finish)//录像事件加入任务队列
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_RECORD;
	node->msg.arg1 = 2;
	node->msg.arg2 = is_finish?1:0;
	
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}


void record_jpeg_event_register(event_pro_callback handle)//拍照完成回调函数
{
	snap_event_callback = handle;
}

void record_video_event_register(event_pro_callback handle)//录像完成回调函数
{
	record_video_event_callback = handle;
}





bool sdcard_status_change_push(char arg1,char arg2)//sd卡状态改变加入任务队列
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_SD;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}

void sdcard_event_register(event_pro_callback handle)
{
	sd_event_callback = handle;
}


//处理设备ID重复
bool device_id_repeat_push(char arg1)
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_DEVICE_REPEAT;
	node->msg.arg1 = arg1;
	node->msg.arg2 = 0;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}


static lv_obj_t* device_id_repeat_msg_obj = NULL;
static void device_id_repeat_msg_btn_up(lv_obj_t* obj)
{
	lv_obj_del(obj);
	device_id_repeat_msg_obj = NULL;
}

static lv_obj_t* device_id_repeat_msg_create(char arg1)
{
	lv_obj_t* msg_box = lv_msgbox_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_color(msg_box,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
	lv_obj_set_style_local_bg_opa(msg_box,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);	
	static const char * btns[language_total][2] 	={{"Confirm",""},
													{"确认",""},
													{"Подтвердить",""},
													{"Confirmer",""},
													{"Confirmar",""},
													{"Bestätigen Sie",""},
													{"تاكيد",""},
													{"Potvrdit",""},
													{"אישור", ""},
													{"Confirmar",""},
													{"Conferma",""},
													{"Potwierdź",""},
													{"επιβεβαίωση",""},
													{"onaylayın",""}};
	char buffer[64] = {"0"};
	if(user_data_get()->user_language == language_english)
		sprintf(buffer,"\n\n Device ID%d repeat!\n\n",arg1);
	else if(user_data_get()->user_language == language_chinese)
		sprintf(buffer,"\n\n 设备ID%d冲突!\n\n",arg1);
	else if(user_data_get()->user_language == language_russian)
		sprintf(buffer,"\n\n Устройство ID%d повторить!\n\n",arg1);
	else if(user_data_get()->user_language == language_french)
		sprintf(buffer,"\n\n Appareil ID%d répéter!\n\n",arg1);
	else if(user_data_get()->user_language == language_spanish)
		sprintf(buffer,"\n\n Dispositivo ID%d repetir!\n\n",arg1);
	else if(user_data_get()->user_language == language_german)
		sprintf(buffer,"\n\n Gerät ID%d wiederholen!\n\n",arg1);
	else if(user_data_get()->user_language == language_arabic)
		sprintf(buffer,"\n\n !%dتكرار معرف الجهاز\n\n",arg1);
	else if(user_data_get()->user_language == language_czech)
		sprintf(buffer,"\n\n Opakujte ID%d zařízení!\n\n",arg1);
	else if(user_data_get()->user_language == language_hebrew)
		sprintf(buffer,"\n\n חזרה על מספר ההתקן ID%d!\n\n",arg1);
	else if(user_data_get()->user_language == language_portugal)
		sprintf(buffer,"\n\nRepita o Número do Dispositivo %d!\n\n",arg1);
	else if(user_data_get()->user_language == language_italy)
		sprintf(buffer,"\n\nRipetere ID %d dispositivo!\n\n",arg1);
	else if(user_data_get()->user_language == language_polish)
		sprintf(buffer,"\n\nAdres monitora %d powtórz!\n\n",arg1);
	else if(user_data_get()->user_language == language_greek)
		sprintf(buffer,"\n\nAdres monitora %d powtórz!\n\n",arg1);
	else if(user_data_get()->user_language == language_turkey)
		sprintf(buffer,"\n\nAygıt ID %d tekrarla!\n\n",arg1);
	else if(user_data_get()->user_language == language_nederlands);
		sprintf(buffer,"\n\nHerhaal apparaat ID %d!\n\n",arg1);
		
	lv_msgbox_set_text(msg_box, buffer);
	lv_msgbox_add_btns(msg_box, btns[user_data_get()->user_language]);
	lv_obj_set_size(msg_box, 400,300);

	static btn_data btn_data = btn_data_up_create(device_id_repeat_msg_btn_up);	
	msg_box->user_data = &btn_data;
	btn_touch_event_listen(msg_box);
	lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t* btnmatri_btn = lv_msgbox_get_btnmatrix(msg_box);
	lv_obj_set_style_local_bg_color(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_PRESSED,LV_COLOR_MAKE(0xFF,0,0));
	lv_obj_set_style_local_radius(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_DEFAULT,45);
	return msg_box;
}


static void device_id_repeat_callback_default(unsigned long arg1,unsigned long arg2)
{
	if(device_id_repeat_msg_obj == NULL)
	{
		device_id_repeat_msg_obj = device_id_repeat_msg_create((char)arg1);
	}
}


event_pro_callback device_id_repeat_register(event_pro_callback handle)
{
	event_pro_callback old =  device_id_repeat_callback;
	device_id_repeat_callback = handle;
	return old;
}

void interphone_call_event_register(event_pro_callback handle)
{
	interphone_call_callback = handle;
}

bool interphone_call_event_push(int arg1,int arg2)
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_INTERPHONE;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}


/************************************************************
* 作者: 
* @Description: 门口机可用状态改变事件发送
* @Author: xiaoxiao
* @Date: 2023-02-16 14:58:22
* @param: 
* @explain: 
************************************************************/
bool outdoor_status_change_event_push(unsigned long arg1,unsigned long arg2)
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_OUTDOOR_STATUS_CHANGE;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}


bool ring_play_event_push(unsigned long arg1,unsigned long arg2)
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_RING;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}


void tuya_event_register(event_pro_callback handle){
	if(handle == NULL){
		Debug("=========tuya_event_register========>>>\n\n\n\n");
	}

	tuya_event_callback = handle;
	

}

void network_event_register(event_pro_callback handle){
	network_event_callback = handle;
}


void outdoor_call_event_register(event_pro_callback handle)
{
	monitor_call_callback = handle;
}

/************************************************************
* @Description: 门口机可用状态改变回调注册
* @Author: xiaoxiao
* @Date: 2023-02-16 14:35:10
* @param: 
* @explain: 
************************************************************/
void outdoor_status_change_event_register(event_pro_callback handle)
{
	outdoor_status_change = handle;
}


/**
 * 作者: 
 * 户外呼叫事件入队函数
 * 将户外呼叫事件添加到系统事件队列中
 * @param arg1 呼叫参数1 (字符类型)
 * @param arg2 呼叫参数2 (整型)
 * @return 成功返回true，失败返回false
 */
bool outdoor_call_event_push(char arg1, int arg2)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_OUTDOOR_CALL;
    node->msg.arg1 = arg1;
    node->msg.arg2 = arg2;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 作者: 
 * 注册户外运动事件回调函数
 * 设置用于处理户外运动检测事件的回调函数
 * @param handle 回调函数指针
 */
void outdoor_motion_event_register(event_pro_callback handle)
{
    /* 存储回调函数指针，用于后续事件处理 */
    monitor_motion_callback = handle;
}

/**
 * 作者: 
 * 户外运动事件入队函数
 * 将户外运动检测事件添加到系统事件队列中
 * @param arg1 运动参数1 (字符类型)
 * @param arg2 运动参数2 (字符类型)
 * @return 成功返回true，失败返回false
 */
bool outdoor_motion_event_push(char arg1, char arg2)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_MOTION;
    node->msg.arg1 = arg1;
    node->msg.arg2 = arg2;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 注册按键呼叫事件回调函数
 * 设置用于处理按键呼叫事件的回调函数
 * @param handle 回调函数指针
 */
void key_call_event_register(event_pro_callback handle)
{
    /* 存储回调函数指针，用于后续事件处理 */
    key_call_callback = handle;
}

/**
 * 按键呼叫事件入队函数
 * 将按键呼叫事件添加到系统事件队列中
 * @param arg1 按键参数1 (字符类型)
 * @param arg2 按键参数2 (字符类型)
 * @return 成功返回true，失败返回false
 */
bool key_call_event_push(char arg1, char arg2)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_KEY_CALL;
    node->msg.arg1 = arg1;
    node->msg.arg2 = arg2;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 注册室内命令事件回调函数
 * 设置用于处理室内命令事件的回调函数
 * @param handle 回调函数指针
 */
void indoor_cmd_event_register(event_pro_callback handle)
{
    /* 存储回调函数指针，用于后续事件处理 */
    indoor_cmd_callback = handle;
}

/**
 * 室内命令事件入队函数
 * 将室内命令事件添加到系统事件队列中
 * @param arg1 命令参数1 (无符号长整型)
 * @param arg2 命令参数2 (无符号长整型)
 * @return 成功返回true，失败返回false
 */
bool indoor_cmd_event_push(unsigned long arg1, unsigned long arg2)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_INDOOR_CMD;
    node->msg.arg1 = arg1;
    node->msg.arg2 = arg2;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head); // 加入队列
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 涂鸦相关的函数处理
 */

/**
 * 涂鸦监控切换事件
 * 触发涂鸦平台的监控通道切换事件
 * @param ch 目标监控通道号
 * @return 成功返回true，失败返回false
 */
bool tuya_monitor_swap_event(int ch)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_TUYA;
    node->msg.arg1 = TUYA_EVENT_MONITOR_SWAP;
    node->msg.arg2 = ch;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 涂鸦监控对讲事件
 * 触发涂鸦平台的监控对讲功能开关事件
 * @param state 对讲状态 (true=开启, false=关闭)
 * @return 成功返回true，失败返回false
 */
bool tuya_monitor_talk_event(bool state)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_TUYA;
    node->msg.arg1 = TUYA_EVENT_TALK;
    node->msg.arg2 = state;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 涂鸦监控开锁事件
 * 触发涂鸦平台的门锁开启功能事件
 * @param state 开锁状态 (true=开锁, false=无操作)
 * @return 成功返回true，失败返回false
 */
bool tuya_monitor_unlock_event(bool state)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_TUYA;
    node->msg.arg1 = TUYA_EVENT_OPEN_DOOR;
    node->msg.arg2 = state;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}

/**
 * 涂鸦监控第二门锁事件
 * 触发涂鸦平台的第二个门锁开启功能事件
 * @param state 开锁状态 (true=开锁, false=无操作)
 * @return 成功返回true，失败返回false
 */
bool tuya_monitor_unlock2_event(bool state)
{
    /* 分配事件节点内存 */
    lv_event_info* node = lv_os_event_queue_node_new();
    if(node == NULL)
    {
        return false;
    }
    
    /* 设置事件类型和参数 */
    node->msg.type = OS_EVENT_TYPE_TUYA;
    node->msg.arg1 = TUYA_EVENT_OPEN_DOOR2;
    node->msg.arg2 = state;
    
    /* 线程安全地将事件插入队列 */
    ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
    queue_insert((queue_s*)node, &lv_os_event_queue_head);
    ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
    
    return true;
}


bool tuya_monitor_absent_mode_event(bool state)//cyy: 离家模式
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_ABSENT_MODE;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}

bool tuya_monitor_home_mode_event(bool state)//居家模式
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_HOME_MODE;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}

bool tuya_monitor_sleep_mode_event(bool state)//睡眠模式
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_SLEEP_MODE;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}

bool tuya_monitor_enter_event(void)
{
	Debug("====tuya_monitor_enter_event=======>>>>tuya event");
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MONITOR_ENTER;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;
}
bool tuya_monitor_quit_event(void)
{
	lv_event_info* node = lv_os_event_queue_node_new();
	if(node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MONITOR_QUIT;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s*)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);	
	return true;

}

#if 0
bool goto_layout(const layout* layout)
{
	if((layout == NULL)||(layout->enter == NULL))
	{
		return false;
	}
	if((cur_layout != NULL)&&(cur_layout->quit != NULL))
	{
		cur_layout->quit();
	}
	lv_obj_clean(lv_scr_act());
	device_id_repeat_msg_obj = NULL;
	
	lv_anim_del_all();
	lv_area_t area = {0,0,LV_HOR_RES_MAX , LV_VER_RES_MAX};
	gui_draw_area_set(&area,1);

	cur_layout = layout;
	layout->enter();

	return true;
}
#endif


bool goto_layout(const layout* layout)
{
	if((layout == NULL)||(layout->enter == NULL))
	{
		return false;
	}
	lv_anim_del_all();
	lv_task_clean();
	if((cur_layout != NULL)&&(cur_layout->quit != NULL))
	{
		cur_layout->quit();
	}

	lv_obj_clean(lv_scr_act());
	device_id_repeat_msg_obj = NULL;
	
	lv_area_t area = {0,0,LV_HOR_RES_MAX , LV_VER_RES_MAX};
	gui_draw_area_set(&area,1);


	lv_port_disp_data_set(GP_OPT_BLIT_TRANSPARENT_COLORKEY,5,0X000008,0XFFFFFF,COLOR_KEEP);


	cur_layout = layout;
	layout->enter();
	return true;
}




const layout* get_cur_layout(void){
	return cur_layout;
}


static void btn_event_handler(lv_obj_t * obj, lv_event_t event)
{
	if(obj == NULL)
	{
		return ;
	}
	btn_data* btn_ev = (btn_data*)obj->user_data;

	if(btn_ev == NULL)
	{
		return ;
	}

	if(event == LV_EVENT_PRESSED)
	{
		if(btn_ev->down != NULL)
		{
			btn_ev->down(obj);
		}

		extern void touch_sound_play(void);
		touch_sound_play();
	//	int audio_buffer = audio_output_buffer_query();
	//	printf("audio_buffer is %d\n",audio_buffer);
		
		if(obj_touch_event_callback != NULL)
		{
			obj_touch_event_callback();
		}


		extern bool standby_timer_reset(void);
		standby_timer_reset();
	}
	else if((event == LV_EVENT_CLICKED)&&(btn_ev->up != NULL))
	{
		btn_ev->up(obj);
	}
	else if(btn_ev->anything_func != NULL)
	{
		btn_ev->anything_func(obj,event);
	}
}


void btn_touch_event_listen(lv_obj_t* obj)
{
	 lv_obj_set_event_cb(obj, btn_event_handler);
}



int get_sound_val(int val){
/*
	if(val >= 0 && val <5)
		return 0;
	else if(val >= 5 && val < 15)
		return 5;
	else if(val >= 15 && val < 25)
		return 15;
	else if(val >= 25 && val < 35)
		return 25;
	else if(val >= 35 && val < 45)
		return 35;
	else if(val >= 45 && val < 55)
		return 45;
	else if(val >= 55 && val < 65)
		return 55;
	else if(val >= 65 && val < 75)
		return 65;
	else if(val >= 75 && val < 85)
		return 75;
	else if(val >= 85 && val <= 95)
		return 85;
	else if(val >= 95 && val <= 100)
		return 90;
		*/
		return val;
}


void rtc_time_sync(void){

	/***********************************
	将系统时间与RTC同步
	************************************/
	unsigned int time_t;
	int time_tone;
	struct tm tm;
	if(tuya_ipc_get_service_time(&time_t,&time_tone) == 0)
	{
		tuya_ipc_get_local_time(time_t,&tm);
/*
		struct ak_date date;
		date.year = tm.tm_year;
		date.month = tm.tm_mon - 1;
		date.day = tm.tm_mday -1;
		date.hour = tm.tm_hour;
		date.minute = tm.tm_min;
		date.second = tm.tm_sec;
		*/
		setting_rtc_time_set(&tm);
		system("hwclock -s");
	}
	else
	{
		setting_rtc_time_get(&tm);
	}

	printf("send time lock \n");
	network_cmd_data_init(data);
	data.device = DEVICE_GROUP;
	data.cmd = NET_COMON_CMD_TIME_LOCK;
	for(int i = 0; i < 6; i++){
		data.arg1 = i;
		if(i == 0)
		{
			data.arg2 = tm.tm_year - 2000;
			printf("arg1 = %d arg2 = %d \n",data.arg1,data.arg2);
			network_send_cmd_data(&data);
		}
		else if(i == 1)
		{
			data.arg2 = tm.tm_mon;
			printf("arg1 = %d arg2 = %d \n",data.arg1,data.arg2);
			network_send_cmd_data(&data);
		}
		else if(i == 2)
		{
			data.arg2 = tm.tm_mday;
			printf("arg1 = %d arg2 = %d \n",data.arg1,data.arg2);
			network_send_cmd_data(&data);
		}
		else if(i == 3) 
		{
			data.arg2 = tm.tm_hour;
			printf("arg1 = %d arg2 = %d \n",data.arg1,data.arg2);
			network_send_cmd_data(&data);
		}
		else if(i == 4)
		{
			data.arg2 = tm.tm_min;
			printf("arg1 = %d arg2 = %d \n",data.arg1,data.arg2);
			network_send_cmd_data(&data);
		}
		else if(i == 5)
		{
			data.arg2 = tm.tm_sec;
			printf("arg1 = %d arg2 = %d \n",data.arg1,data.arg2);
			network_send_cmd_data(&data);
		}
	}
	
}