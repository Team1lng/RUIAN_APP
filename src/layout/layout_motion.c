#include "layout_define.h"
#include "../api/video/video_decode.h"

typedef enum
{
	LAYOUT_MOTION_HEAD_CONT_ID,

} layout_motion_obj_id;

extern void playback_sd_status_change_callback(unsigned long arg1, unsigned long arg2);

extern void outdoor_motion_event_register(event_pro_callback handle);
void motion_call_extern_func(unsigned long arg1, unsigned long arg2);
extern void tuya_event_register(event_pro_callback handle);
extern void check_tuya(void);

extern void monitor_call_motion_func(unsigned long arg1, unsigned long arg2);
extern void tuya_event_motion_proc(unsigned long arg1, unsigned long arg2);
extern void monitor_call_extern_func(unsigned long arg1, unsigned long arg2);

extern bool net_online_device[DEVICE_TOTAL];
extern void monitor_channel_label_create(void);

extern void tuya_event_extern_proc(unsigned long arg1, unsigned long arg2);

static void back(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		goto_layout(pLAYOUT(home));

	}
}

static void monitor_video_mode_open(void)
{
	fb_video_mode_enable(true); 					//打开
	monitor_open(); 

#if 1
		lv_area_t area[] = {
			{0, 0, 1024, 0 + 48},
			// {58, 33, 58 + 146, 33 + 59},
			// {820, 33, 820 + 146, 33 + 59},
			{
				60, 105, 60 + 65, 105 + 40
			},
			{
				68, 160, 68 + 40, 160 + 290
			},
			{
				914, 160, 914 + 40, 160 + 290
			},
			{
				900, 105, 900 + 70, 105 + 40
			},
			// {
			// 	467, 33, 467 + 90, 33 + 46
			// },
			{
				312, 150, 312 + 400, 150 + 300
			},
			{
				0,	 0,   0+1024,	 0+600
			}
			
		};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));
#endif
	lv_obj_set_click(lv_scr_act(), true);
	static btn_data btn_data = btn_data_anything_create(back);

	lv_scr_act()->user_data = &btn_data;
	btn_touch_event_listen(lv_scr_act());
	system_bg_fill_color(0x00, 0, 0, 1024, 600);	//填充颜色

}

static void monitor_video_mode_close(void)
{
	fb_video_mode_enable(false);
	usleep(100 * 1000);
	monitor_close();
}

static int motion_timeout_val = 10;

extern lv_task_t * monitor_timer_ptask;


static void motion_timer_task(struct _lv_task_t *task_t)
{
	lv_obj_t *timer_label = (lv_obj_t *)task_t->user_data;
	lv_label_set_text_fmt(timer_label, "%02d", motion_timeout_val);
	lv_obj_align(timer_label, timer_label->parent, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t *time_label = (lv_obj_t *)timer_label->user_data;
	time_t seconds = time(NULL);
	
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	lv_label_set_text_fmt(time_label, "%02d:%02d", tm.tm_hour, tm.tm_min);
	lv_obj_align(time_label, time_label->parent, LV_ALIGN_CENTER, 0, 0);
	

	static unsigned long sec = 0;

	static struct ak_timeval timeval;
	ak_get_ostime(&timeval);

	if (timeval.sec != sec) {
		sec = timeval.sec;
		
		if (motion_timeout_val == 0) 
		{
			goto_layout(pLAYOUT(home));
		}
		else 
		{
			motion_timeout_val--;
			lv_label_set_text_fmt(timer_label, "%02d", motion_timeout_val);
			lv_obj_align(timer_label, timer_label->parent, LV_ALIGN_CENTER, 0, 0);
		}
	}
}


static void motion_time_label_create(void)
{
	
	lv_obj_t * time = lv_cont_create(lv_scr_act(), NULL);

	lv_cont_set_layout(time, LV_LAYOUT_OFF);
	lv_obj_set_pos(time, 840, 0);
	lv_obj_set_size(time, 146, 48);
	lv_obj_set_style_local_bg_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
	lv_obj_set_style_local_bg_opa(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_radius(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 40);

	lv_obj_t * time_label = lv_label_create(time, NULL);

	lv_label_set_long_mode(time_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(time_label, time, LV_ALIGN_CENTER, 0, 0);

	
	lv_obj_t * timer = lv_cont_create(lv_scr_act(), time);

	lv_obj_set_pos(timer, 467, 0);
	lv_obj_set_size(timer, 90, 36);
	lv_obj_set_style_local_bg_opa(timer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_font(timer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));

	lv_obj_t * timer_label = lv_label_create(timer, NULL);
	timer_label->user_data = time_label;

	lv_label_set_align(timer_label, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(timer_label, LV_LABEL_LONG_EXPAND);
	lv_obj_set_style_local_text_color(timer_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));

	if (monitor_timer_ptask != NULL) {
		lv_task_del(monitor_timer_ptask);
	}

	monitor_timer_ptask = lv_task_create(motion_timer_task, 1000, LV_TASK_PRIO_HIGH, timer_label);
	// lv_task_ready(monitor_timer_ptask);

	motion_timer_task(monitor_timer_ptask);
	
}



static void motion_record(void)
{
	//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
	if(is_audio_talk_open() == false)
	{
					
		MONITOR_CH	monitor_ch = monitor_channel_get();
		network_device	device = get_outdoor_device_by_channel(monitor_ch);
		audio_talk_open(device, true);
	}
	extern bool sent_tuya_start(char mode,MONITOR_CH video_channel);
	sent_tuya_start(REC_MODE_MANUAL, monitor_channel_get());
	
	record_video_start(REC_MODE_MANUAL, 0x01, monitor_channel_get());

	network_cmd_data_init(data);
	MONITOR_CH ch = monitor_channel_get() ;
	network_device device = get_outdoor_device_by_channel(ch);
	data.device = device;
	data.cmd = NET_COMMON_CMD_KEY_FRAME;
	data.arg1 = 1;
	data.arg2 = 0;
	network_send_cmd_data(&data);

}
extern bool video_auto_record_falg ;
extern bool tuya_motion_record ;
static void layout_motion_tuya_push_task(lv_task_t *task)
{
	printf("----\nvideo_auto_record_falg=>%d\n-----------",video_auto_record_falg);
	if((video_auto_record_falg == true))
	{
		tuya_motion_record = true;
		//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
		motion_record();//自动拍照
		lv_task_del(task);
	}

}


static void motion_top_cont_create(void)
{
	if (monitor_enter_flag_get() != MON_ENTER_TUYA)
	{
		lv_obj_t *top_cont = lv_cont_create(lv_scr_act(), NULL);
		lv_cont_set_layout(top_cont, LV_LAYOUT_OFF);
		lv_obj_set_id(top_cont,LAYOUT_MOTION_HEAD_CONT_ID);
		lv_obj_set_pos(top_cont, 0, 0);
		lv_obj_set_size(top_cont, 1024, 48);
		lv_obj_set_style_local_bg_color(top_cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_MAKE(0x39, 0x39, 0x39));
		lv_obj_set_style_local_bg_opa(top_cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);

	}
}


static void LAYOUT_ENETER_FUNC(motion)
{
	//将页面恢复为有线网卡的路由，恢复设备和设备之间的通信
	system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");

#ifdef _PLATFORM_800_1280
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
#else
	//在刷出第一诊后，才在视频现实函数设置显示器和屏幕透明
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
#endif

	monitor_video_mode_open();
	sdcard_event_register(playback_sd_status_change_callback);

	
	motion_timeout_val = 10;
	network_cmd_data_init(data);
	MONITOR_CH ch = monitor_channel_get() ;
	network_device device = get_outdoor_device_by_channel(ch);	

	data.device = device;
	data.cmd = NET_COMON_CMD_MOTION;
	data.arg1 = 1;
	data.arg2 = 0;
	network_send_cmd_data(&data);

	outdoor_motion_event_register(NULL);

	standby_timer_close();
	motion_top_cont_create();
	motion_time_label_create();
	monitor_channel_label_create();
	//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
	lv_task_t * motion_record_task = lv_layout_task_create(layout_motion_tuya_push_task, 500, LV_TASK_PRIO_HIGH, NULL);
	lv_task_ready(motion_record_task);
	outdoor_call_event_register(monitor_call_motion_func);
	tuya_event_register(tuya_event_motion_proc);
	check_tuya();
	
}

static void LAYOUT_QUIT_FUNC(motion)
{
	//将页面恢复为用户选择的路由，恢复设备和设备之间的通信
	if(user_data_get()->cctv_connect_mode == 0){
		system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
	}else{
		system("route del -net 224.0.0.0 netmask 224.0.0.0 br0");
	}
	network_cmd_data_init(data);
	MONITOR_CH ch = monitor_channel_get() ;
	network_device device = get_outdoor_device_by_channel(ch);			
		
	data.cmd = NET_COMMON_CMD_SOUND;
	data.arg1 = 0| user_data_get()->user_language << 16;
	data.arg2 = 0;
	data.device = device;
	network_send_cmd_data(&data);
	

	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);

	monitor_video_mode_close();

	sdcard_event_register(NULL);

	record_video_stop(0x00);					//停止录像

	audio_talk_close(); 							//音频关闭	


	if (monitor_timer_ptask != NULL) //对话任务未退出
	{
		lv_task_del(monitor_timer_ptask);			//删除对话人物
		monitor_timer_ptask = NULL; 				//置空任务
	}//停止声音播放
	

	lv_obj_set_click(lv_scr_act(), false);
	
	lv_scr_act()->user_data = NULL;


	standby_timer_open(-1, NULL);
	
	outdoor_call_event_register(monitor_call_extern_func);
	tuya_event_register(tuya_event_extern_proc);
	printf("quit the layout motion\n");

}


//外部呼叫的回调函数
void motion_call_extern_func(unsigned long arg1, unsigned long arg2)
{
	network_device	device = (network_device)arg1;//确定呼叫的门口机设备
	MONITOR_CH ch = get_channel_by_outdoor_device(device);
	if ((device < DEVICE_UNIT_OUTDOOR_1) && (device > DEVICE_OUTDOOR_15)) //不是门口机 返回
	{
		return;
	}
	net_online_device[device] = true;
	monitor_channel_set(ch); 		//选择通道
	goto_layout(pLAYOUT(motion));

	
}



void layout_motion_init(void)
{
	outdoor_motion_event_register(motion_call_extern_func);
}




CREATE_LAYOUT(motion);

