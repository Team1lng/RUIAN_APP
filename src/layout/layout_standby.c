#include "layout_define.h"

extern bool wifi;
extern void outdoor_motion_event_register(event_pro_callback handle);
extern void motion_call_extern_func(unsigned long arg1, unsigned long arg2);

static lv_task_t* standby_clock_ptask = NULL;
static void standby_clock_update_task(lv_task_t* task)
{
	struct tm* pre_clock = NULL;
	if(task != NULL)
	{
		pre_clock  = (struct tm*)task->user_data;
	}

	time_t seconds = time(NULL);
	struct tm cur_clock;
	localtime_r(&seconds,&cur_clock);
	if((task == NULL)||(cur_clock.tm_sec != pre_clock->tm_sec))
	{
		if(pre_clock  != NULL)
		{
			*pre_clock = cur_clock;
		}

		lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), 0);
		if(parent != NULL)
		{
			lv_obj_t* hour_obj = lv_obj_get_child_form_id(parent, 0);
			if(hour_obj != NULL)
			{
				float hour;
				if (cur_clock.tm_hour > 12)
			        hour = (float)(cur_clock.tm_hour - 12);
			    else
			        hour = (float)cur_clock.tm_hour;

			    hour += cur_clock.tm_min / 60.0f;

			    int16_t angle = 360.0f / 12.0f * hour*10;
				lv_img_set_angle(hour_obj, (int16_t) angle);
			}
			
			lv_obj_t* min_obj = lv_obj_get_child_form_id(parent, 1);
			if(min_obj != NULL)
			{
				int16_t angle = 360.0f / 60.0f * cur_clock.tm_min*10;
				lv_img_set_angle(min_obj, (int16_t) angle);
			}

			lv_obj_t* sec_obj = lv_obj_get_child_form_id(parent, 2);
			if(sec_obj != NULL)
			{
				int16_t angle = 360.0f / 60.0f * cur_clock.tm_sec*10;
				lv_img_set_angle(sec_obj, (int16_t) angle);
			}
		}
	}
}
static void standby_analog_create(void)
{
	#ifdef _PLATFORM_800_1280
	lv_obj_t * bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(bg, 390, 366);
	lv_obj_set_auto_realign(bg, true);
    lv_obj_align(bg, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_opa(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_style_local_radius(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_color(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0x27,0x27,0x27));
	lv_obj_set_style_local_bg_grad_color(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0x15,0x15,0x15));
	lv_obj_set_style_local_bg_main_stop(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,0);
	lv_obj_set_style_local_bg_grad_stop(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,128);
	lv_obj_set_style_local_bg_grad_dir(bg,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_GRAD_DIR_VER);
	#endif

    lv_obj_t * bg_obj = lv_img_create(lv_scr_act(),NULL);
	lv_obj_set_id(bg_obj, 0);
	
	#ifdef _PLATFORM_800_1280
    rom_bin_info img_bg = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_BG_PNG);
	#else
	rom_bin_info img_bg = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_BG_2_PNG);
	#endif
    lv_img_set_src(bg_obj, &img_bg);  
	#ifdef _PLATFORM_800_1280
    lv_obj_set_size(bg_obj, 380, 350);
	#else
	lv_obj_set_size(bg_obj, 380, 380);
	#endif
    lv_obj_set_auto_realign(bg_obj, true);
    lv_obj_align(bg_obj, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_opa(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_style_local_radius(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_color(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0x27,0x27,0x27));
	lv_obj_set_style_local_bg_grad_color(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0x15,0x15,0x15));
	lv_obj_set_style_local_bg_main_stop(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,0);
	lv_obj_set_style_local_bg_grad_stop(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,128);
	lv_obj_set_style_local_bg_grad_dir(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_GRAD_DIR_VER);
	 
	

    lv_obj_t* hour_obj = lv_img_create(bg_obj,NULL);  // 时针
    lv_obj_set_id(hour_obj, 0);
	rom_bin_info img_hour = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_HOUR_PNG);
    lv_img_set_src( hour_obj, &img_hour);
    lv_obj_align(  hour_obj, bg_obj,LV_ALIGN_CENTER, 0, -23);
	lv_img_set_pivot(hour_obj,lv_obj_get_width(hour_obj)/2,lv_obj_get_height(hour_obj) - 26);
	
 // uint16_t h = Hour * 300 + Minute / 12 % 12 * 60;
 // lv_img_set_angle(hour_obj, h);
    lv_obj_t* min_obj = lv_img_create(bg_obj,NULL); 
	lv_obj_set_id(min_obj, 1);
	rom_bin_info img_min = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_MIN_PNG);
    lv_img_set_src( min_obj, &img_min);
    lv_obj_align( min_obj, bg_obj,LV_ALIGN_CENTER, 0, -36);
	lv_img_set_pivot(min_obj,lv_obj_get_width(min_obj)/2,lv_obj_get_height(min_obj) - 26);


    lv_obj_t* sec_obj  = lv_img_create(bg_obj,NULL);  //秒针
    lv_obj_set_id(sec_obj, 2);
	rom_bin_info img_sec = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_SEC_PNG);
    lv_img_set_src( sec_obj, &img_sec);
    lv_obj_align(  sec_obj, bg_obj,LV_ALIGN_CENTER, 0, -50);
	lv_img_set_pivot(sec_obj,lv_obj_get_width(sec_obj)/2,lv_obj_get_height(sec_obj) - 26);

	
	lv_obj_t* dot_obj = lv_img_create(bg_obj,NULL);
	rom_bin_info img_dot = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_DOT_PNG);
    lv_img_set_src(dot_obj, &img_dot); 
    lv_obj_align(dot_obj, bg_obj, LV_ALIGN_CENTER, 0, 0);

	static struct tm clock;
	time_t seconds = time(NULL);
	localtime_r(&seconds,&clock);
	standby_clock_update_task(NULL);
	
	if(standby_clock_ptask != NULL)
		lv_task_del(standby_clock_ptask);

    standby_clock_ptask = lv_task_create(standby_clock_update_task, 100, LV_TASK_PRIO_MID, &clock);  // 1秒任务
}


static void standby_click_up(lv_obj_t* obj)
{
	goto_layout(pLAYOUT(home));
}

// static void standby_clear_sound_buffer_event(lv_task_t* t)
// {
// 	int total;
// 	int remain;
// 	audio_output_buffer_get(&total.&remain);
// 	if(remain >50*1024){
// 		// ak_ao_clear_frame_buffer(audio_output_handle_id_);
// 	}
// }

static void LAYOUT_ENETER_FUNC(standby)
{
	standby_timer_close();
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);

	lv_obj_t* obj = lv_scr_act();
	static btn_data btn_data = btn_data_up_create(standby_click_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
	lv_obj_set_click(obj, true);

	if(user_data_get()->other.screen_saver == 1)
	{
		standby_analog_create();
	}else
		backlight_open(0);

	outdoor_motion_event_register(motion_call_extern_func);
	// lv_layout_task_create(standby_clear_sound_buffer_event,10*1000,LV_TASK_PRIO_MID,NULL);
	//hlf:回放页跳转待机概率有缩略图残留
	// #ifdef _PLATFORM_800_1280
	// 	screen_force_refresh();
	// #endif
	
}

static void LAYOUT_QUIT_FUNC(standby)
{	

	if(standby_clock_ptask != NULL)
	{
		lv_task_del(standby_clock_ptask);
		standby_clock_ptask = NULL;
	}
	standby_timer_open(-1, NULL);
	
	
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	
	lv_scr_act()->user_data = NULL;
	lv_obj_set_click(lv_scr_act(), false);

	backlight_open(1);
	outdoor_motion_event_register(NULL);
	if(user_data_get()->other.network_device == DEVICE_INDOOR_ID1)
	{
		rtc_time_sync();
	}

}

CREATE_LAYOUT(standby);

