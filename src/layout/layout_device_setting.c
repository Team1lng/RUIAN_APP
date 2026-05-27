
#include "layout_define.h"
#include "layout_setting_common.h"
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

#define LAYOUT_DEVICE_SETING_BG_ADJ_ID 0X01

static void reset_network_init_check_task(lv_task_t * task)
{
	if(network_inited_status_get() == true)
	{
		lv_task_del(task);
		lv_obj_del(lv_obj_get_parent((lv_obj_t *)(task->user_data)));
	}
}

static void room_id_btnmatrix_up(lv_obj_t *obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
		if (btn_id == LV_BTNMATRIX_BTN_NONE)
			return;
	
		if (lv_btnmatrix_get_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_DISABLED))
			return;
	
		lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);
		lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
	
		user_data_get()->other.network_device = btn_id == 0?DEVICE_INDOOR_ID1:
												btn_id == 1?DEVICE_INDOOR_ID2:
												btn_id == 2?DEVICE_INDOOR_ID3:
												btn_id == 3?DEVICE_INDOOR_ID4:
												btn_id == 4?DEVICE_INDOOR_ID5:DEVICE_INDOOR_ID6;
		user_data_save();
		network_local_device_set(user_data_get()->other.network_device);

		if(network_inited_status_get() == true)
		{
			lv_obj_t * obj = network_init_reset_func();
			network_inited_status_set(false);
			lv_layout_task_create(reset_network_init_check_task, 2000, LV_TASK_PRIO_MID, obj);
			
		}
	}
}
static void setting_page_2_btnmatrix_create(lv_obj_t *parent)
{
	lv_obj_t *obj = lv_btnmatrix_create(parent, NULL);
	
	lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));

	lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);

	lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_BORDER_SIDE_BOTTOM);

	lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
	lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_make(0, 0, 0));
	
	lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 1);


	
	static rom_bin_info info_off = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off);

	static rom_bin_info info_on = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on);


	// static const char *map[language_total][12] = {{"Device ID1",  	"\n", "Device ID2",  "\n", "Device ID3",  "\n", "Device ID4","\n", "Device ID5","\n", "Device ID6",  ""},
	// 											{ "设备号 1",  	  	"\n", "设备号 2",     "\n", "设备号 3",     "\n", "设备号 4",  "\n", "设备号 5","\n", "设备号 6",   ""},
	// 											{ "прылады: 1",      "\n", "прылады: 2", "\n", "прылады: 3",   "\n", "прылады: 4", "\n", "прылады: 5", "\n", "прылады: 6",""},
	// 											{ "Matériel 1",   "\n", "Matériel 2",   "\n", "Matériel 3",   "\n", "Matériel 4", "\n", "Matériel 5",   "\n", "Matériel 6",  ""},
	// 											{ "Equipo 1", "\n", "Equipo 2", "\n", "Equipo 3", "\n", "Equipo 4", "\n", "Equipo 5", "\n", "Equipo 6",""},
 	// 											{ "Ausrüstung 1",	  "\n", "Ausrüstung 2",     "\n", "Ausrüstung 3",     "\n", "Ausrüstung 4", "\n", "Ausrüstung 5", "\n", "Ausrüstung 6", ""},
 	// 											{ "1 معدات", "\n", "2 معدات",  "\n",  "3 معدات","\n", "4 معدات",  "\n",  "5 معدات","\n", "6 معدات",  "" },
 	// 											{ "ID1 zařízení",   "\n", "ID2 zařízení",  "\n", "ID3 zařízení",  "\n", "ID4 zařízení",  "\n", "ID5 zařízení",  "\n", "ID6 zařízení", ""},
	// 											{ "ID1 מכשיר",   "\n", "ID2  מכשיר",  "\n", "ID3  מכשיר",  "\n", "ID4  מכשיר",  "\n", "ID5  מכשיר",  "\n", "ID6  מכשיר", ""},
	// 											{"ID1 do Dispositivo1","\n","ID2 do Dispositivo","\n","ID3 do Dispositivo","\n","ID4 do Dispositivo","\n","ID5 do Dispositivo","\n","ID6 do Dispositivo",""},
	// 											{"Ripetere ID1","\n","Ripetere ID2","\n","Ripetere ID3","\n","Ripetere ID4","\n","Ripetere ID5","\n","Ripetere ID6",""}
	// 											};
												
 	lv_btnmatrix_set_map(obj, deviceidselect_str_get());
	lv_btnmatrix_set_align(obj, LV_LABEL_ALIGN_LEFT, 70, 0);
	lv_obj_set_x(obj, 70);
	lv_obj_set_y(obj, 100);
	lv_obj_set_width(obj, 892);
	lv_obj_set_height(obj, 500);

	static btn_data btn_data = btn_data_anything_create(room_id_btnmatrix_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);

	lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);

	int device_floor = user_data_get()->other.network_device;
	int btn_id =  device_floor == DEVICE_INDOOR_ID1?0:device_floor == DEVICE_INDOOR_ID2?1:device_floor == DEVICE_INDOOR_ID3?2:\
	device_floor == DEVICE_INDOOR_ID4? 3:device_floor == DEVICE_INDOOR_ID5? 4 :5;
	lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
	
}


//返回设置页面按钮
static void floor_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void floor_back_btn_up(lv_obj_t *obj)
{
    set_enter_setting_page_which(1);
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(setting));
	
}

static lv_obj_t* floor_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(floor_back_btn_down, floor_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	printf("%x",ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}
static void setting_page_bg_create()
{
    lv_obj_t * cont = lv_cont_create(lv_scr_act(),NULL);
    lv_obj_set_pos(cont, 0, 100);
	lv_obj_set_size(cont, 1024, 500);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
}
static void layout_floor_id_head_label_create(void)
{
	lv_obj_t *head_label = lv_label_create(lv_scr_act(), NULL);

	lv_label_set_long_mode(head_label,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(head_label,1024,60);
	lv_obj_align(head_label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(head_label,LV_LAYOUT_CENTER);

	// char * src[language_total] = {"Device ID", "设备ID", "ID zařízení", "ID du périphérique", "ID del dispositivo", "Geräte-ID","معرف الجهاز","ID zařízení","זהות המכשיר","ID do dispositivo","Ripetere ID"};
	lv_label_set_text(head_label, str_get(LAYOUT_SETTING_LANG_DEVICEID_ID));
}
void LAYOUT_ENETER_FUNC(floor_setting)
{
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    system_bg_enable_set(false);//系统背景使能设置
    setting_page_bg_create();
	layout_floor_id_head_label_create();
    setting_page_2_btnmatrix_create(lv_scr_act());
    floor_back_btn_create();

}

void LAYOUT_QUIT_FUNC(floor_setting)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);

}
CREATE_LAYOUT(floor_setting);