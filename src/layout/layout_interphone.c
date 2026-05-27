#include "layout_define.h"
void interphone_call_event_extern_func(unsigned long arg1,unsigned long arg2);
static void interphone_call_event_inside_func(unsigned long arg1,unsigned long arg2);

//static void interphone_out_page_create(void);
static void interphone_call_talk_page_create(void);
static void interphone_out_page_create1(void);
static void interphone_ring_play_finsih_callback(void);
extern void outdoor_motion_event_register(event_pro_callback handle);
extern void motion_call_extern_func(unsigned long arg1, unsigned long arg2);


static lv_obj_t* callin_slider_label = NULL;
static lv_obj_t* callin_slider_img = NULL;

static lv_obj_t *slide_cont = NULL;

extern bool talking;


//当前本设备的状态
interphone_status_enum interphone_status = INTERPHONE_STATUS_IDLE;

//呼叫本机的对象设备
static network_device interphone_call_mastar_device;


/*
*通话标志位，0x00:空闲   
*			 0x01:选中，即将呼叫
*		     0x02:呼叫中，
*            0x03:正在通话
*/

static char interphone_device_btn_status_flag[5] = {0x00,0x00,0x00};



static void interphone_head_label_create(void)
{
	lv_obj_t* label = lv_label_create(lv_scr_act(), NULL);
	
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	


	// char *src[language_total] = {"Intercom","内线通话","Домофон","Interphone","Interfono","Sprechanlage","الاتصال الداخلي","Interkom","שיחה פנימית","Intercomunicador","Interfono"};
	
	lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_INTERCOM_ID));
	lv_label_set_long_mode(label,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(label,1024,60);
	lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(label,LV_LAYOUT_CENTER);
}


/*
static void interphone_cancel_btn_up(lv_obj_t* obj)
{
	goto_layout(pLAYOUT(home));
}
*/

extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

static void intercom_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *children = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(children, state);
}

static void intercom_cancel_btn_down(lv_obj_t *obj)
{
	intercom_btn_state_set(obj, LV_STATE_PRESSED);
}
static void intercom_cancel_btn_up(lv_obj_t *obj)
{
	intercom_btn_state_set(obj, LV_STATE_DEFAULT);

	goto_layout(pLAYOUT(home));
}


//取消按钮
static void interphone_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(intercom_cancel_btn_down, intercom_cancel_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
}


//获取设备id
static network_device interphone_index_device_get(int index)
{
	if(index ==1)
	{
		return user_data_get()->other.network_device == DEVICE_INDOOR_ID1? DEVICE_INDOOR_ID2:DEVICE_INDOOR_ID1;
	}
	if(index == 2)
	{
		return ((user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2))?DEVICE_INDOOR_ID3:DEVICE_INDOOR_ID2;
	}
	if(index == 3)
	{
		return ((user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID3))?DEVICE_INDOOR_ID4:DEVICE_INDOOR_ID3;
	}
	if(index == 4)
	{
		return (user_data_get()->other.network_device == DEVICE_INDOOR_ID5)?DEVICE_INDOOR_ID4:(user_data_get()->other.network_device == DEVICE_INDOOR_ID6)?DEVICE_INDOOR_ID4:DEVICE_INDOOR_ID5;
	}
	if(index == 5)
	{
		return (user_data_get()->other.network_device != DEVICE_INDOOR_ID6)?DEVICE_INDOOR_ID6:DEVICE_INDOOR_ID5;
	}
	return DEVICE_UNKONW;
}


static void interphone_device_id_btn_display(int id,lv_obj_t* obj)
{
	id -= 1;
	if(interphone_device_btn_status_flag[id] == 0)//空闲 灰色
	{
		lv_obj_set_style_local_bg_color(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x26,0x26,0x26));//空闲中的颜色
	}
	else if((interphone_device_btn_status_flag[id] == 1)||(interphone_device_btn_status_flag[id] == 2))//呼出呼入 绿色
	{
		lv_obj_set_style_local_bg_color(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xB8,0x8D,0x56));//选中和呼叫中的颜色
	}
	else if(interphone_device_btn_status_flag[id] == 3)//挂断 红色
	{
		lv_obj_set_style_local_bg_color(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x21,0xAF,0x8D));//通话中的颜色
	}
}


//创建选择设备的按钮
static lv_obj_t* interphone_call_btn_create(int x,int y ,int w,int h ,btn_data* data,int id)
{
	lv_obj_t* parent = lv_cont_create(lv_scr_act(),NULL);//创建一个容器
	lv_obj_set_style_local_bg_opa(parent,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_pos(parent,x,y);
	lv_obj_set_size(parent,w,h);
	lv_obj_set_click(parent, false);//可以点击

	lv_obj_t* cont_obj1 = lv_cont_create(parent,NULL);
	lv_obj_set_pos(cont_obj1,27,30);
	lv_obj_set_size(cont_obj1,100,100);
	lv_obj_set_click(cont_obj1, false);
	lv_obj_set_style_local_bg_color(cont_obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0xFF,0xFF));
	lv_obj_set_style_local_radius(cont_obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_opa(cont_obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_30);


	lv_obj_t* cont_obj2 = lv_cont_create(cont_obj1,NULL);
	lv_obj_set_pos(cont_obj2,30,13);
	lv_obj_set_size(cont_obj2,40,40);
	lv_obj_set_click(cont_obj2, false);
	lv_obj_set_style_local_bg_color(cont_obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0xFF,0xFF));
	lv_obj_set_style_local_radius(cont_obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_opa(cont_obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);

	lv_obj_t* img = lv_img_create(cont_obj1,NULL);
	lv_obj_set_pos(img,13,50);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_INTERPHONE_HEAD_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_transform_zoom(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 200);
    lv_obj_set_style_local_transform_zoom(img, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 200);
	

	


	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_obj_align(label, parent, LV_ALIGN_IN_BOTTOM_MID, 0, -40);
	lv_obj_set_style_local_text_font(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,FONT_SIZE(31));
	network_device device = interphone_index_device_get(id);
	char buffer[12] = {0};
	sprintf(buffer,"ID%d",device);
	lv_label_set_text(label, buffer);//显示ID

	if(data != NULL)
	{
		parent->user_data = data;
		btn_touch_event_listen(parent);
	}
	
	
	interphone_device_id_btn_display(id,parent);//根据当前不同的状态显示

	return parent;
}

static int intercom_timeout_val = 30;
static lv_task_t *intercom_timer_ptask = NULL;

static void intercom_timer_task(struct _lv_task_t *task_t)
{
    static unsigned long sec = 0;
    struct ak_timeval timeval;
    ak_get_ostime(&timeval);

    if (timeval.sec != sec)
    {
        sec = timeval.sec;
        if (is_audio_talk_open() == false)
        {
        }
        // monitor_default_camera_busy_display();

        if (intercom_timeout_val == 0)
        {

            goto_layout(pLAYOUT(home));
        }
        else
        {
            intercom_timeout_val--;
        }
    }
}

static void interphone_call_out_btn_display(void);

static void interphone_device_btn_up(lv_obj_t* obj)
{
	int id = lv_obj_get_id(obj);
	interphone_device_btn_status_flag[id -1] = interphone_device_btn_status_flag[id -1]?0x00:0x01;
	interphone_device_id_btn_display(1,obj);//ID号码显示
	
	//点击直接切换到呼叫状态
	if(interphone_status == INTERPHONE_STATUS_IDLE)//设备处于空闲状态 呼叫
	{
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		data.arg1 = 1;
		data.arg2 = user_data_get()->other.network_device;
		if(interphone_device_btn_status_flag[id -1] == 0x01)
		{

			data.device = interphone_index_device_get(id);
			printf("==========================device is %d\n",data.device);
			network_send_cmd_data(&data);
			interphone_status = INTERPHONE_STATUS_PUBLISH;
			//页面跳转到呼叫页面

			/*开始倒计时30s 无人接听退出*/

			if (intercom_timer_ptask != NULL)
            	lv_task_del(intercom_timer_ptask);

			
       	 	intercom_timer_ptask = lv_task_create(intercom_timer_task, 500, LV_TASK_PRIO_LOW, NULL);
       		lv_task_ready(intercom_timer_ptask);

        	intercom_timer_task(intercom_timer_ptask);
			
			//interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val),  NULL, interphone_ring_play_finsih_callback);
			interphone_out_page_create1();
		}

	}
}



//挂断
static lv_task_t* interphone_publish_timer_task = NULL;
static void interphone_publish_timer_task_func(lv_task_t* task_t)
{
	if(interphone_status == INTERPHONE_STATUS_PUBLISH)
	{
		
		interphone_status = INTERPHONE_STATUS_IDLE;
	}

	lv_task_del(task_t);
	interphone_publish_timer_task = NULL;
}


static void interphone_call_out_btn_display(void)
{
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), 0);//获取到的是呼出通话按钮
	if(obj == NULL)
	{
		return ;
	}

	if((interphone_status == INTERPHONE_STATUS_IDLE))//空闲 或者等待通话
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_ON_PNG);//图片显示为接通
		lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&info);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_PRESSED,&info);

		//有选中的 
		if((interphone_device_btn_status_flag[0])||(interphone_device_btn_status_flag[1])||interphone_device_btn_status_flag[2])
		{
			//设置颜色
			lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x32,0xD7,0x4B));
			//点击使能
			lv_obj_set_click(obj, true);
		}
		else
		{
			lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x39,0x39,0x39));
			lv_obj_set_click(obj, false);
		}
	}
	else
	{
		//呼入或者通话中 直接变成挂断 显示图标和颜色	
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_OFF_PNG);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&info);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_PRESSED,&info);
		lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0x00,0x00));
		lv_obj_set_click(obj, true);
	}
}





static void interphone_call_out_btn_up(lv_obj_t* obj)
{
	if(interphone_status == INTERPHONE_STATUS_IDLE)//设备处于空闲状态
	{
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		data.arg1 = 1;
		data.arg2 = user_data_get()->other.network_device;
		for(int i = 0 ; i < 5 ; i++)
		{
			if(interphone_device_btn_status_flag[i] == 0x01)
			{
				data.device = interphone_index_device_get(i+1);
				network_send_cmd_data(&data);
				interphone_status = INTERPHONE_STATUS_PUBLISH;
			}
		}
		if((interphone_status == INTERPHONE_STATUS_PUBLISH)&&(interphone_publish_timer_task == NULL))
		{
			interphone_publish_timer_task = lv_task_create(interphone_publish_timer_task_func, 500, LV_TASK_PRIO_MID, NULL);
			//挂断
		}
	}
	else if(interphone_status == INTERPHONE_STATUS_PUBLISH)
	{
		goto_layout(pLAYOUT(interphone));
	}
	else if(interphone_status == INTERPHONE_STATUS_OUT)
	{
		goto_layout(pLAYOUT(home));
	}
	else if(interphone_status == INTERPHONE_STATUS_TALK)
	{
		goto_layout(pLAYOUT(home));
	}
}




//呼叫按钮创建
static void interphone_call_out_btn_create(int y)
{
	lv_obj_t* imgbtn = lv_imgbtn_create(lv_scr_act(),NULL);
	lv_obj_set_id(imgbtn, 0);
	lv_obj_set_pos(imgbtn,466,y);
	lv_obj_set_size(imgbtn,100,100);
	lv_obj_set_style_local_bg_opa(imgbtn, LV_IMGBTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,360);

	interphone_call_out_btn_display();

	static btn_data btn_data = btn_data_up_create(interphone_call_out_btn_up);
	imgbtn->user_data = &btn_data;
	btn_touch_event_listen(imgbtn);
	
}
static lv_obj_t *call_in_slider_create(int x,int y,int w,int h);


static bool intercom_sound_flag = true;
static void intercom_sound_set(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) 
	{
		intercom_sound_flag	= intercom_sound_flag == true ? false: true;

		if (slide_cont != NULL) 
		{
			lv_obj_set_hidden(slide_cont, intercom_sound_flag);
		}
		
	}
}



//主动呼出的界面 数字系统不允许多个通话
static void interphone_out_page_create1(void)
{
	lv_obj_clean(lv_scr_act());//清空屏幕

	int id_buf = 0;
	
	for(int i = 0 ; i < 5 ; i++)
	{
		if((interphone_device_btn_status_flag[i] == 0x01)||(interphone_device_btn_status_flag[i] == 0x02))
		{	
			id_buf = i+1;
			break;
		}
	}
	interphone_call_btn_create(436,106,153,200,NULL,id_buf);
	
	interphone_call_out_btn_create(426);//挂断按钮

	/*
		接通前 左侧为铃声音量大小
		接通后 左侧为通话声音大小
	*/
	call_in_slider_create(90, 140, 18, 300);
	
	lv_obj_set_click(lv_scr_act(), true);
	static btn_data btn_data_t = btn_data_anything_create(intercom_sound_set);
	lv_scr_act()->user_data = &btn_data_t;
	btn_touch_event_listen(lv_scr_act());
}

static lv_obj_t *idle_slider_label = NULL;

//底部铃声大小音量调节滑块的回调函数
static void interphone_idle_slider_event(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED){
		lv_label_set_text_fmt(idle_slider_label, "%d", lv_slider_get_value(obj));
		lv_obj_align(idle_slider_label, obj, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	
		user_data_get()->audio.intercom_ring_val = lv_slider_get_value(obj);

		interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val), NULL, NULL);
		
		user_data_save();
	}
}


//创建底部铃声大小音量调节滑块
static lv_obj_t* interphone_idle_page_ring_slider_create(int x,int y ,int w,int h){
	lv_obj_t *slider = lv_slider_create(lv_scr_act(),NULL);
	lv_obj_set_ext_click_area(slider, 10, 10, 10, 10);
	lv_obj_set_pos(slider, x, y);
	lv_obj_set_size(slider, w, h);
	lv_slider_set_range(slider, 0, 100);
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
	
	
	lv_slider_set_value(slider, user_data_get()->audio.intercom_ring_val, LV_ANIM_OFF);

	static btn_data btn_data = btn_data_anything_create(interphone_idle_slider_event);
	slider->user_data = &btn_data;
	btn_touch_event_listen(slider);

	lv_obj_t *img = lv_img_create(lv_scr_act(),NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RING_PNG);
	lv_img_set_src(img, &info);
	lv_obj_align(img, slider, LV_ALIGN_OUT_LEFT_MID, -30, 0);

	idle_slider_label = lv_label_create(lv_scr_act(),NULL);
	lv_label_set_text_fmt(idle_slider_label, "%d", user_data_get()->audio.intercom_ring_val);
	lv_obj_align(idle_slider_label, slider, LV_ALIGN_OUT_RIGHT_MID, 30, 0);

	return slider;
}


typedef enum
{
	LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_1 = 0X01,
	LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_2,
	LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_3,
	LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_4,
	LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_5
}LAYOUT_INTERPHONE;

extern bool net_online_device[DEVICE_TOTAL];



//主动进入intercom的界面 设备空闲状态
static void interphone_idle_page_create1(void)
{
	lv_obj_clean(lv_scr_act());//清空屏幕
	
	interphone_head_label_create();//创建标题
	interphone_cancel_btn_create();//创建返回按钮

	//创建其他门号
	static btn_data btn_data = btn_data_up_create(interphone_device_btn_up);
	lv_obj_t * obj1 = interphone_call_btn_create(95,177,153,200,&btn_data,1);//创建设备按钮
	lv_obj_set_id(obj1,LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_1);
	network_device device = interphone_index_device_get(1);
	if(net_online_device[device] == true){
		lv_obj_set_style_local_bg_color(obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x21AF8D));
		lv_obj_set_click(obj1, true);
	}

	lv_obj_t * obj2 = interphone_call_btn_create(265,177,153,200,&btn_data,2);
	lv_obj_set_id(obj2,LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_2);
	device = interphone_index_device_get(2);
	if(net_online_device[device] == true){
		lv_obj_set_style_local_bg_color(obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x21AF8D));
		lv_obj_set_click(obj2, true);
	}

	lv_obj_t * obj3 = interphone_call_btn_create(438,177,153,200,&btn_data,3);
	lv_obj_set_id(obj3,LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_3);
	device = interphone_index_device_get(3);
	if(net_online_device[device] == true){
		lv_obj_set_style_local_bg_color(obj3,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x21AF8D));
		lv_obj_set_click(obj3, true);
	}
	lv_obj_t * obj4 = interphone_call_btn_create(611,177,153,200,&btn_data,4);
	lv_obj_set_id(obj4,LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_4);
	device = interphone_index_device_get(4);
	if(net_online_device[device] == true){
		lv_obj_set_style_local_bg_color(obj4,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x21AF8D));
		lv_obj_set_click(obj4, true);
	}

	lv_obj_t * obj5 = interphone_call_btn_create(784,177,153,200,&btn_data,5);
	lv_obj_set_id(obj5,LAYOUT_INTERPHONE_OBJ_ID_CALL_CONT_5);
	device = interphone_index_device_get(5);
	if(net_online_device[device] == true){
		lv_obj_set_style_local_bg_color(obj5,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x21AF8D));
		lv_obj_set_click(obj5, true);
	}
	//底部创建铃声音量调节的滑块
	interphone_idle_page_ring_slider_create(323,496,388,20);
	
}

static void interphone_call_in_handup_btn_up(lv_obj_t* obj)
{
	goto_layout(pLAYOUT(home));//直接退出回主界面
}

//挂断通话按钮创建
static void interphone_call_in_handup_btn_create(void)
{
	lv_obj_t* imgbtn = lv_imgbtn_create(lv_scr_act(),NULL);//图片按钮创建
	lv_obj_set_id(imgbtn, 0);
	lv_obj_set_pos(imgbtn,404,426);
	lv_obj_set_size(imgbtn,100,100);
	lv_obj_set_style_local_bg_opa(imgbtn, LV_IMGBTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,360);
	

	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_OFF_PNG);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_RELEASED,&info);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_PRESSED,&info);
	lv_obj_set_style_local_bg_color(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0x00,0x00));
	lv_obj_set_click(imgbtn, true);

	static btn_data btn_data = btn_data_up_create(interphone_call_in_handup_btn_up);
	imgbtn->user_data = &btn_data;
	btn_touch_event_listen(imgbtn);
}



//接听通话
static void interphone_call_in_talk_btn_up(lv_obj_t* obj)
{	if(obj != NULL)
		lv_obj_set_hidden(obj,true);//隐藏接听按钮

	lv_obj_t* obj_handup = lv_obj_get_child_form_id(lv_scr_act(),0);//获取到挂断按钮对象
	if(obj_handup != NULL)
	{
		lv_obj_set_x(obj_handup, 466);//设置挂断按钮的位置

		network_cmd_data_init(data);
		data.device = interphone_call_mastar_device;
		data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		data.arg1 = 3;//有设备接受通话
		data.arg2 = user_data_get()->other.network_device;
		network_send_cmd_data(&data);
		for(int i = 0 ; i < 5 ; i++)
		{
			if(interphone_call_mastar_device == interphone_index_device_get(i+1))//找到对应的呼入设备id
			{
				interphone_status = INTERPHONE_STATUS_TALK;//切换本机状态 为通话中
				interphone_device_btn_status_flag[i] = 0x03;//将呼入设备的标志位也改成通话中
			
				interphone_call_talk_page_create();//创建通话中的页面**************************************
				
				audio_play_stop_set();//停止声音播放

				audio_talk_open(interphone_call_mastar_device,false);//打开声音
				talking = true;
				audio_volume_set(get_sound_val(user_data_get()->audio.intercom_talk_val));
				//改变图标
				break;
			}
		}

	}
	
}

//接听按钮创建
static void interphone_call_in_talk_btn_create(void)
{
	lv_obj_t* imgbtn = lv_imgbtn_create(lv_scr_act(),NULL);//图片按钮创建
	lv_obj_set_id(imgbtn, 0);
	lv_obj_set_pos(imgbtn,522,426);
	lv_obj_set_size(imgbtn,100,100);
	lv_obj_set_style_local_bg_opa(imgbtn, LV_IMGBTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,360);
	

	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_ON_PNG);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_RELEASED,&info);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_PRESSED,&info);
	lv_obj_set_style_local_bg_color(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x32,0xD7,0x4B));
	lv_obj_set_click(imgbtn, true);

	static btn_data btn_data = btn_data_up_create(interphone_call_in_talk_btn_up);//通话
	imgbtn->user_data = &btn_data;
	btn_touch_event_listen(imgbtn);
}

static void interphone_callin_slider_event(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED){
		lv_label_set_text_fmt(callin_slider_label, "%d", lv_slider_get_value(obj));
		lv_obj_align(callin_slider_label, obj, LV_ALIGN_OUT_TOP_MID, 0, -15);

		if((interphone_status == INTERPHONE_STATUS_IN) ||(interphone_status == INTERPHONE_STATUS_OUT) || (interphone_status == INTERPHONE_STATUS_PUBLISH)){
			user_data_get()->audio.intercom_ring_val = lv_slider_get_value(obj);
			
			if(is_audio_play_ing() == false)
			{
				interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val), NULL, interphone_ring_play_finsih_callback);
			}
			else
			{
				audio_volume_set(lv_slider_get_value(obj));
			}
		
		}else if(interphone_status == INTERPHONE_STATUS_TALK){

			user_data_get()->audio.intercom_talk_val = lv_slider_get_value(obj);
			audio_output_volume_set(get_sound_val(user_data_get()->audio.intercom_talk_val));
		}
		user_data_save();
	}
}



static lv_obj_t *call_in_slider_create(int x,int y,int w,int h){
	
	slide_cont = NULL;

	slide_cont = lv_cont_create(lv_scr_act(),NULL);
	lv_obj_set_pos(slide_cont, 0, 0);
	lv_obj_set_size(slide_cont, 200, 600);
	lv_obj_set_style_local_bg_opa(slide_cont, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 0);

	
	lv_obj_t *callin_slider = lv_slider_create(slide_cont,NULL);
	lv_obj_set_id(callin_slider, 10);
	
	lv_obj_set_ext_click_area(callin_slider, 10, 10, 10, 10);
	lv_obj_set_pos(callin_slider, x, y);
	lv_obj_set_size(callin_slider, w, h);
	lv_slider_set_range(callin_slider, 0, 100);
	lv_obj_set_style_local_bg_color(callin_slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(callin_slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(callin_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
	
	lv_slider_set_value(callin_slider, user_data_get()->audio.intercom_ring_val, LV_ANIM_OFF);

	static btn_data btn_data = btn_data_anything_create(interphone_callin_slider_event);
	callin_slider->user_data = &btn_data;
	btn_touch_event_listen(callin_slider);

	callin_slider_img = lv_img_create(slide_cont,NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RING_PNG);
	lv_img_set_src(callin_slider_img, &info);
	lv_obj_align(callin_slider_img, callin_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);

	callin_slider_label = lv_label_create(slide_cont,NULL);
	lv_label_set_text_fmt(callin_slider_label, "%d", user_data_get()->audio.intercom_ring_val);
	lv_obj_align(callin_slider_label, callin_slider, LV_ALIGN_OUT_TOP_MID, 0, -15);
	

	lv_obj_set_hidden(slide_cont, true);
	return callin_slider;
}


//被呼叫的页面创建
static void interphone_call_in_page_create(void)
{
	lv_obj_clean(lv_scr_act());//清空屏幕
	
	
	for(int i = 1 ; i < 6 ; i++)
	{
		if(interphone_call_mastar_device == interphone_index_device_get(i))//找到对应设备的id的结点位置
		{
			interphone_device_btn_status_flag[i-1] = 0x02;//设置标志位
			interphone_call_btn_create(435,106,153,200,NULL,i);//创建按钮 只创建呼叫的那一张页面
			
			break;
		}
	}
	interphone_call_in_handup_btn_create();//挂断按钮创建
	interphone_call_in_talk_btn_create();//通话按钮创建
	/*
		左侧调整音量键 
		接通前是铃声音量 
		接通后是通话音量
	*/
	call_in_slider_create(90, 140, 18, 300);
	
	lv_obj_set_click(lv_scr_act(), true);
	static btn_data btn_data_t = btn_data_anything_create(intercom_sound_set);
	lv_scr_act()->user_data = &btn_data_t;
	btn_touch_event_listen(lv_scr_act());
	
}


//通话页面的滑块的回调函数
static lv_obj_t *talking_label = NULL;
static void talking_slider_event(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED){	
		 if(interphone_status == INTERPHONE_STATUS_TALK){
		 	lv_label_set_text_fmt(talking_label, "%d", lv_slider_get_value(obj));
			lv_obj_align(talking_label, obj, LV_ALIGN_OUT_TOP_MID, 0, -15);
			user_data_get()->audio.intercom_talk_val = lv_slider_get_value(obj);
			//audio_volume_set(get_sound_val(user_data_get()->audio.intercom_talk_val));
			audio_output_volume_set(get_sound_val(user_data_get()->audio.intercom_talk_val));
			user_data_save();
		}else
			return ;
	}
}
//通话页面的滑块创建
static lv_obj_t* talking_page_talk_slider_create(int x,int y ,int w,int h){
	slide_cont = NULL;

	slide_cont = lv_cont_create(lv_scr_act(),NULL);
	lv_obj_set_pos(slide_cont, 0, 0);
	lv_obj_set_size(slide_cont, 200, 600);
	lv_obj_set_style_local_bg_opa(slide_cont, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 0);


	lv_obj_t *slider = lv_slider_create(slide_cont,NULL);
	lv_obj_set_ext_click_area(slider, 10, 10, 10, 10);
	lv_obj_set_pos(slider, x, y);
	lv_obj_set_size(slider, w, h);
	lv_slider_set_range(slider, 0, 100);
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));

	lv_slider_set_value(slider, user_data_get()->audio.intercom_talk_val, LV_ANIM_OFF);

	static btn_data btn_data = btn_data_anything_create(talking_slider_event);
	slider->user_data = &btn_data;
	btn_touch_event_listen(slider);

	lv_obj_t *img = lv_img_create(slide_cont,NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_PNG);
	lv_img_set_src(img, &info);
	lv_obj_align(img, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);

	talking_label = lv_label_create(slide_cont,NULL);
	lv_label_set_text_fmt(talking_label, "%d", user_data_get()->audio.intercom_talk_val);
	lv_obj_align(talking_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);
	
	lv_obj_set_hidden(slide_cont, true);
	return slider;

	
}


//通话中页面创建
static void interphone_call_talk_page_create(void)
{
	lv_obj_clean(lv_scr_act());//清空屏幕上所有东西
	intercom_sound_flag = true;
	

	if (intercom_timer_ptask != NULL){
		lv_task_del(intercom_timer_ptask);
		intercom_timer_ptask =NULL;
	}
	
	lv_obj_t * cont = NULL;
	for(int i = 1 ; i < 6 ; i++)
	{
		if(interphone_device_btn_status_flag[i-1] == 0x03)
		{
			cont =interphone_call_btn_create(435,106,153,200,NULL,i);//创建通话中的设备按钮
			break;
		}
	}
	lv_obj_t * child_cont = lv_obj_get_child_back(cont, NULL);
	
	lv_obj_clean(child_cont);
	lv_obj_t* img = lv_img_create(child_cont,NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_INTERCOM_TALKING_PNG);
	lv_img_set_src(img, &info);
	lv_obj_align(img, child_cont, LV_ALIGN_CENTER, 0, 0);

	
	interphone_call_out_btn_create(426);//呼叫按钮创建
	/*
		左侧调整通话音量
	*/
	talking_page_talk_slider_create(90, 140, 18, 300);
	
	lv_obj_set_click(lv_scr_act(), true);
	static btn_data btn_data_t = btn_data_anything_create(intercom_sound_set);
	lv_scr_act()->user_data = &btn_data_t;
	btn_touch_event_listen(lv_scr_act());

	
}
static bool out_falg = false;

void layout_interphone_init(void)
{
	interphone_call_event_register(interphone_call_event_extern_func);//外部事件
}

//进入页面之后的函数
static void LAYOUT_ENETER_FUNC(interphone)
{
	//将页面恢复为有线网卡的路由，恢复设备和设备之间的通信
	system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);
	intercom_timeout_val = 30;
	intercom_sound_flag = true;
	
	out_falg = false;

	memset(interphone_device_btn_status_flag,0,sizeof(interphone_device_btn_status_flag));
	if(interphone_status == INTERPHONE_STATUS_IDLE)//状态是空闲
	{
		interphone_idle_page_create1();//空闲
		
	}
	else if(interphone_status == INTERPHONE_STATUS_IN)//状态是被呼叫
	{
		
		interphone_call_in_page_create();
		
	}
	interphone_call_event_register(interphone_call_event_inside_func);//进入之后的事件
	
}

//退出
static void LAYOUT_QUIT_FUNC(interphone)
{	
		//将页面恢复为用户选择的路由，恢复设备和设备之间的通信
	if(user_data_get()->cctv_connect_mode == 0){
		system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
	}else{
		system("route del -net 224.0.0.0 netmask 224.0.0.0 br0");
	}
	if(interphone_publish_timer_task != NULL)//清空任务
	{
		lv_task_del(interphone_publish_timer_task);
		interphone_publish_timer_task =NULL;
	}
	if (intercom_timer_ptask != NULL){
		lv_task_del(intercom_timer_ptask);
		intercom_timer_ptask =NULL;
	}
           
	talking = false;
	audio_play_stop_set();//停止设置
	audio_talk_close();//关闭通话

	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);

	lv_obj_set_click(lv_scr_act(), false);


	lv_scr_act()->user_data = NULL;
	


	if(out_falg == false)
	{
		network_cmd_data_init(data);
		data.device = DEVICE_GROUP;
		data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		data.arg1 = 4;
		data.arg2 = user_data_get()->other.network_device;//挂断电话的数据
		network_send_cmd_data(&data);//发送命令
		standby_timer_open(-1, NULL);

	}

	out_falg = false;
	
	memset(interphone_device_btn_status_flag,0,sizeof(interphone_device_btn_status_flag));//清空数据
	interphone_status = INTERPHONE_STATUS_IDLE;//设置状态为空闲

	interphone_call_event_register(interphone_call_event_extern_func);//外部事件
	
}



//铃声响完之后的回调函数
static void interphone_ring_play_finsih_callback(void)
{
	if((interphone_status == INTERPHONE_STATUS_OUT)||(interphone_status == INTERPHONE_STATUS_IN)||(interphone_status == INTERPHONE_STATUS_PUBLISH))//如果是呼叫或者是被呼叫状态
	{
		interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val),NULL,interphone_ring_play_finsih_callback);//播放铃声
	}
}

/*
*	arg1 = 1:呼叫
*   arg1 = 2:有设备应答呼叫
*	arg1 = 3:有设备接受通话
*   arg1 = 4:设备挂断正在通话
*	arg1 = 5:设备正忙
*   arg2 为对方设备号
*/

//在页面外被呼叫
void interphone_call_event_extern_func(unsigned long arg1,unsigned long arg2)
{
	if(arg1 == 1)//呼叫
	{
		if(interphone_status == INTERPHONE_STATUS_IDLE)//如果内线通话的状态是空闲
		{
			interphone_call_mastar_device = (network_device)(arg2);;
			network_cmd_data_init(data);
			data.device = interphone_call_mastar_device;
			data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			data.arg1 = 2;
			data.arg2 = user_data_get()->other.network_device;
			network_send_cmd_data(&data);
		
			interphone_status = INTERPHONE_STATUS_IN;//重新设置设备的状态
	
			goto_layout(pLAYOUT(interphone));//跳转到页面
		
			interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val),NULL,interphone_ring_play_finsih_callback);//铃声响起
		}
	}
}


//在页面内被呼叫
static void interphone_call_event_inside_func(unsigned long arg1,unsigned long arg2)
{

	network_device device = (network_device)(arg2);//对方的设备号
	printf("device is %d\n",device);
	
	if(arg1 == 1)//被呼叫
	{
		if(interphone_status == INTERPHONE_STATUS_IDLE)//本机状态如果是空闲
		{
			interphone_call_mastar_device = (network_device)(arg2);;//获取呼叫的设备
			
			network_cmd_data_init(data);
			data.device = interphone_call_mastar_device;
			data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			data.arg1 = 2;
			data.arg2 = user_data_get()->other.network_device;
			network_send_cmd_data(&data);
		
			interphone_status = INTERPHONE_STATUS_IN;
			interphone_call_in_page_create();//被呼叫界面创建

			interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val),NULL,interphone_ring_play_finsih_callback);
		}else {//设备正忙 发送命令
		//	interphone_call_mastar_device = (network_device)(arg2 & 0x0f);//call的			
			network_cmd_data_init(data);
			data.device = (network_device)(arg2 & 0x0f);//call的	;
			data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			data.arg1 = 5;
			data.arg2 = user_data_get()->other.network_device;
			network_send_cmd_data(&data);
		}
		
	}
	
	else if(arg1 == 2)//有设备应答呼叫
	{
		if(interphone_status != INTERPHONE_STATUS_PUBLISH)//如果设备的状态不是等待应答 直接退出
		{
			return ;
		}
		
		for(int i = 0 ; i < 5; i++)
		{
			//找到设备并且他的状态为选中即将呼叫
			if((device == interphone_index_device_get(i+1))&&(interphone_device_btn_status_flag[i] == 0x01))
			{
				interphone_device_btn_status_flag[i] = 0x02;//设备切换标志位呼叫中
				interphone_status = INTERPHONE_STATUS_OUT;//切换状态为呼出
			}
		}
		
		if(interphone_status == INTERPHONE_STATUS_OUT)//如果当前设备的状态为呼出
		{
			interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val),NULL,interphone_ring_play_finsih_callback);
			interphone_out_page_create1();//创建呼叫的页面
			standby_timer_close();
			
		}
	}
	else if(arg1 == 3)//有设备接受通话
	{
		if(interphone_status == INTERPHONE_STATUS_OUT)
		{
			for(int i = 0 ; i < 5; i++)
			{
				if((device == interphone_index_device_get(i+1))&&(interphone_device_btn_status_flag[i] == 0x02))//找到id号并且在呼叫中
				{
					interphone_device_btn_status_flag[i] = 0x03;//状态切换为通话中
					interphone_status = INTERPHONE_STATUS_TALK;//切换为通话中
				}
				else if((interphone_device_btn_status_flag[i] == 0x02)||(interphone_device_btn_status_flag[i] == 0x01))//没有找到id号
				{
					network_cmd_data_init(data);
					data.device = interphone_index_device_get(i+1);
					data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
					data.arg1 = 4;
					data.arg2 = user_data_get()->other.network_device;
					network_send_cmd_data(&data);
					interphone_device_btn_status_flag[i] = 0x00;//标志位为空闲
				}
			}

			if(interphone_status == INTERPHONE_STATUS_TALK)//设备的状态切换为正在通话
			{		

				interphone_call_talk_page_create();//创建通话的页面
				
				audio_play_stop_set();//暂停的设置
				
				audio_talk_open(device,false);//打开通话
				talking = true;
				audio_volume_set(get_sound_val(user_data_get()->audio.intercom_talk_val));
				standby_timer_close();
 			}
 		}
	}
	else if(arg1 == 4)//设备挂断正在通话
	{
		if(interphone_status == INTERPHONE_STATUS_IN)//设备的状态是被呼叫
		{
			if(device == interphone_call_mastar_device)//呼入设备一致
			{
				goto_layout(pLAYOUT(home));//跳转
			}
		}
		else if(interphone_status == INTERPHONE_STATUS_OUT || interphone_status == INTERPHONE_STATUS_PUBLISH)//设备的状态是呼出
		{
			//int out_device_num = 0;
			for(int i = 0 ; i < 5; i++)
			{
				if(device == interphone_index_device_get(i+1) && interphone_device_btn_status_flag[i] == 0x02)//找到标志位是呼叫的点
				{
					//out_device_num++;
					goto_layout(pLAYOUT(home));
				}
			}
			//if(out_device_num < 2)//只找到一个或者没有找到 直接回桌面
			//{
			//	goto_layout(pLAYOUT(home));
			//}
		}
		else if(interphone_status == INTERPHONE_STATUS_TALK)//正在通话
		{
		
			for(int i = 0 ; i < 5; i++)
			{
				if((device == interphone_index_device_get(i+1))&&(interphone_device_btn_status_flag[i] == 0x03))//找到正在通话的点
				{
					goto_layout(pLAYOUT(home));//直接跳转
				}
			}	
		}
	}
	else if(arg1 == 5)//对方设备正忙
	{
		out_falg = true;
		goto_layout(pLAYOUT(interphone));//返回interphone页面
		lv_obj_t *msg = lv_msgbox_create(lv_scr_act(), NULL);//创建消息框

		lv_obj_set_size(msg, 600,340);
		lv_msgbox_set_anim_time(msg, 0);
		lv_obj_align(msg, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

		lv_obj_set_style_local_bg_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x1A1A1A));
		lv_obj_set_style_local_bg_opa(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

		lv_obj_set_style_local_border_width(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, 2);
		lv_obj_set_style_local_border_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_text_font(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(31));	
		lv_obj_set_style_local_text_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xffffff));	

		// char *str[language_total] = {"Device is busy! Please wait!!!",
		// 								"设备正忙! 请稍等!!!",
		// 								"Устройство занято! пожалуйста ждите!!!",
		// 								"Le périphérique est occupé! S'il vous plaît, attendez!!!",
		// 								"El dispositivo está ocupado! Espere por favor!!!",

		// 								"Gerät ist beschäftigt! Warten Sie mal!!!",
		// 								"!!!الجهاز مشغول !يرجى الانتظار",
		// 								"Zařízení je obsazené!  Prosím, čekejte!!!",
		// 								"ההתקן תפוס נא להמתין!!!",
		// 								"O Dispositivo Está Ocupado! Por Favor, Aguarde",
		// 								"Dispositivo occupato! attendere"};
		
		lv_msgbox_set_text(msg, str_get(LAYOUT_INTERPHONE_LANG_DEVICEISBUSY_ID));
		lv_msgbox_start_auto_close(msg,1500);
		
	}
}



CREATE_LAYOUT(interphone)

