#include "layout_define.h"

extern bool audio_volume_set(int vol);
extern bool door_ring_play(int ring_index, int volume, void (*start)(void), void (*end)(void));
extern bool is_audio_play_ing(void);

int setting_page_which = -1;
void set_enter_setting_page_which(int which)
{
	setting_page_which = which;
}

int get_enter_setting_page_which()
{
	return setting_page_which;
}

static int device_confirm_status = -1;
void device_confirm_status_set(int status)
{
	device_confirm_status = status;
}

int device_confirm_status_get()
{
	return device_confirm_status;
}


static lv_obj_t *setting_page[5];

static struct tm setting_tm;
static bool setting_time_edit = false;

void setting_rtc_time_get(struct tm* date){
	/***********************************
	用安凯的api获取时间没有问题
	************************************/
	//ak_get_localdate(date);
	time_t seconds = time(NULL);
	localtime_r(&seconds,date);
	date->tm_year +=1900;
	date->tm_mon += 1;
	//date->tm_mday += 1;
}

void setting_rtc_time_set(struct tm* date){

	char date_str[64] = {0};
	date->tm_sec = 0; //强制秒为0
	/***********************************
	用安凯的api有时候无法保存时间,使用
	date保存时间 
	格式为:%04d-%02d-%02d %02d:%02d:%02d
	************************************/
	Debug("date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",date->tm_year,date->tm_mon,date->tm_mday,date->tm_hour,date->tm_min,date->tm_sec);
	sprintf(date_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",date->tm_year,date->tm_mon,date->tm_mday,date->tm_hour,date->tm_min,date->tm_sec);
	system(date_str);
	/***********************************
	将系统时间与RTC同步
	************************************/
	system("hwclock -w");
}

void setting_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *children = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(children, state);

}

void setting_btn_img_transform_set(lv_obj_t *obj)
{
	lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 256);
	lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 300);

	lv_obj_set_style_local_transition_prop_1(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_ZOOM);
	lv_obj_set_style_local_transition_prop_2(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_STYLE_TRANSFORM_ZOOM);

	lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 200);
	lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 200);

	static lv_anim_path_t path;
	path.cb = lv_anim_path_overshoot,
	path.user_data = NULL;
	lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &path);
	lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, &path);
}



static lv_obj_t *head_label = NULL;

//==================cyy设置界面的标签===================================
static void setting_head_label_set(uint32_t num)
{
	if (num == 0)
	{
		// char *src[language_total] = {"Set ring", "铃声设置", "Звенеть", "La cloche", "Tintineo", "Ring", "ضبط الرنين", "Nastavit zvonění", "תכנות צלצול","Definir Toque","Imposta Suoneria"};
		lv_label_set_text(head_label, str_get(LAYOUT_RING_LANG_SETRING_ID));
	}
	else if (num == 1)
	{
		// char *src[language_total] = {"Set room ID", "房号设置", "Комната нет", "Chambre Non", "Habitación no", "Raum Nr", "ضبط معرف الغرفة", "Nastavte ID místnosti", "תכנות מספר מסך","Definir ID da Zona","Imposta l'ID stanza"};
		lv_label_set_text(head_label, str_get(LAYOUT_SETTING_LANG_SETROOMID_ID));
	}
	else if (num == 2)
	{
		// char *src[language_total] = {"Set time", "时间设置", "Время", "Temps", "Tiempo", "Zeit", "ضبط الوقت", "Nastavte čas", "תכנות שעון","Definir Hora","Tempo impostato"};
		lv_label_set_text(head_label, str_get(LAYOUT_SETTING_LANG_SETTIME_ID));
	}
	else if (num == 3)
	{
		// char *src[language_total] = {"Screen saver", "屏保设置", "Экранная заставка", "Écran de veille", "Salvapantallas", "Screensaver", "شاشة التوقف", "Spořič obrazovky", "שומר מסך","Protector de Tela","Salvaschermo"};
		lv_label_set_text(head_label, str_get(LAYOUT_SETTING_LANG_SCREENSAVER_ID));
	}
	else if (num == 4)
	{
		// char *src[language_total] = {"Other", "其它设置", "Другой", "Autres", "Otros", "Andere", "اخرى", "Jiné", "אחר","Outros","Altro"};
		lv_label_set_text(head_label, str_get(LAYOUT_SETTING_LANG_OTHER_ID));
	}
	else if (num == 5)
	{
		// char *src[language_total] = {"Reset", "重置", "Перезагрузить", "Réinitialiser", "Reiniciar", "Zurücksetzen", "اعادة الضبط", "Resetovat", "ביצוע ריסט","Redefinir","Ripristina"};
		lv_label_set_text(head_label, str_get(LAYOUT_SETTING_LANG_RESET_ID));
	}
	lv_obj_align(head_label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
}



static void setting_head_label_create(void)
{
	int page_num = get_enter_setting_page_which();
	head_label = lv_label_create(lv_scr_act(), NULL);
	setting_head_label_set(page_num);
	lv_label_set_long_mode(head_label,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(head_label,1024,60);
	lv_obj_align(head_label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(head_label,LV_LAYOUT_CENTER);
}

lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);


	if (bg_color == true)
	{
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

		lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 45);
		lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 45);

	
	}
	else
	{
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
	}

	lv_obj_t *img = lv_img_create(btn, NULL);
	lv_img_set_src(img, img_src);

	setting_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);

	return btn;
}

static void setting_cancel_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void setting_cancel_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);

	goto_layout(pLAYOUT(home));
}



static void goto_wifi_layout(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		set_enter_setting_page_which(4);
		goto_layout(pLAYOUT(set_wifi));
	}
}



static void setting_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(setting_cancel_btn_down, setting_cancel_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	printf("%x",ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
}





static void setting_tabview_btn_up(lv_obj_t *obj)
{
	uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);//获得按下的按钮
	if (btn_id == LV_BTNMATRIX_BTN_NONE)
		return;

	if (lv_btnmatrix_get_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_DISABLED))
		return;

	lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);//清空按钮的所有属性
	lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);

	lv_obj_t *tabview = lv_obj_get_parent(obj);

	uint32_t id_prev = lv_tabview_get_tab_act(tabview);
	lv_tabview_set_tab_act(tabview, btn_id, LV_ANIM_OFF);
	setting_head_label_set(id_prev);
	uint32_t id_new = lv_tabview_get_tab_act(tabview);
	setting_head_label_set(id_new);

	lv_res_t res = LV_RES_OK;
	if (id_prev != id_new)
		res = lv_event_send(tabview, LV_EVENT_VALUE_CHANGED, &id_new);

#if LV_USE_GROUP
	if (lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_ENCODER)
	{
		lv_group_set_editing(lv_obj_get_group(tabview), false);
	}
#endif
	if (res != LV_RES_OK)
		return;
}
void tabview_cb(struct _lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_VALUE_CHANGED){
		uint16_t id =  lv_tabview_get_tab_act(obj);

		setting_head_label_set(id);
		if(id == 2 && user_data_get()->other.network_device == DEVICE_INDOOR_ID1)
		{
			setting_time_edit = true;

		}
			
	}
}


static void setting_tableview_create(void)
{
	lv_obj_t *obj = lv_tabview_create(lv_scr_act(), NULL);
	lv_tabview_set_btns_pos(obj, LV_TABVIEW_TAB_POS_RIGHT);
	lv_tabview_set_anim_time(obj, 0);
	lv_obj_set_pos(obj, 0, 100);
	lv_obj_set_size(obj, 1024, 500);
	lv_obj_set_style_local_bg_color(obj, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, lv_color_make(0x40, 0x40, 0x40));

	lv_obj_set_style_local_bg_color(obj, LV_TABVIEW_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0x40, 0x40, 0x40));

	lv_obj_set_style_local_bg_opa(obj, LV_TABVIEW_PART_TAB_BTN, LV_STATE_CHECKED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_TABVIEW_PART_TAB_BTN, LV_STATE_CHECKED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(obj, LV_TABVIEW_PART_TAB_BTN, LV_STATE_PRESSED, lv_color_make(0x40, 0x40, 0x40));
	


	lv_obj_set_style_local_bg_opa(obj, LV_TABVIEW_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

	// char *src1[language_total] = {"Set ring", "铃声设置", "Звенеть", "La cloche", "Tintineo", "Ring", "ضبط الرنين", "Nastavit zvonění", "תכנות צלצול","Definir Toque","Imposta Suoneria"};

	setting_page[0] = lv_tabview_add_tab(obj, str_get(LAYOUT_RING_LANG_SETRING_ID));

	lv_obj_set_style_local_bg_opa(setting_page[0], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(setting_page[0], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

	// char *src2[language_total] = {"Set room ID", "房号设置", "Комната нет", "Chambre Non", "Habitación no", "Raum Nr", "ضبط معرف الغرفة", "Nastavte ID místnosti", "תכנות מספר מסך","Definir ID da Zona","Imposta l'ID stanza"};
	setting_page[1] = lv_tabview_add_tab(obj, str_get(LAYOUT_SETTING_LANG_SETROOMID_ID));

	// lv_obj_set_size(setting_page[1], 724,500);
	lv_obj_set_style_local_bg_opa(setting_page[1], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(setting_page[1], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

	// char *src3[language_total] = {"Set time", "时间设置", "Время", "Temps", "Tiempo", "Zeit", "ضبط الوقت", "Nastavte čas", "תכנות שעון","Definir Hora","Tempo impostato"};
	setting_page[2] = lv_tabview_add_tab(obj, str_get(LAYOUT_SETTING_LANG_SETTIME_ID));
	// lv_obj_set_size(setting_page[2], 724,500);
	lv_obj_set_style_local_bg_opa(setting_page[2], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(setting_page[2], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

	// char *src4[language_total] =   {"Screen saver", "屏保设置", "Экранная заставка", "Écran de veille", "Salvapantallas", "Screensaver", "شاشة التوقف", "Spořič obrazovky", "שומר מסך","Protector de Tela","Salvaschermo"};

	setting_page[3] = lv_tabview_add_tab(obj, str_get(LAYOUT_SETTING_LANG_SCREENSAVER_ID));
	// lv_obj_set_size(setting_page[3], 724,500);
	lv_obj_set_style_local_bg_opa(setting_page[3], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(setting_page[3], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	// char *src5[language_total] = {"Other", "其它设置", "Другой", "Autres", "Otros", "Andere", "اخرى", "Jiné", "אחר","Outros","Altro"};

	setting_page[4] = lv_tabview_add_tab(obj, str_get(LAYOUT_SETTING_LANG_OTHER_ID));
	//lv_obj_set_size(setting_page[4], 724,500);
	lv_obj_set_style_local_bg_opa(setting_page[4], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(setting_page[4], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

	
	lv_tabview_set_tab_act(obj,get_enter_setting_page_which(),LV_ANIM_ON);
	

	lv_obj_t *child_btns = lv_obj_get_child_form_id(obj, 1);
	if (child_btns != NULL)
	{
		lv_obj_set_x(child_btns, 1024 - 300);
		lv_obj_set_width(child_btns, 300);

		lv_btnmatrix_set_align(child_btns, LV_LABEL_ALIGN_LEFT, 20, 0);

		static btn_data btn_data = btn_data_up_create(setting_tabview_btn_up);
		child_btns->user_data = &btn_data;
		btn_touch_event_listen(child_btns);
		lv_obj_set_style_local_border_side(child_btns, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);

		lv_obj_set_style_local_border_color(child_btns, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
		lv_obj_set_style_local_border_width(child_btns, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
	}
	
	lv_obj_set_event_cb(obj, tabview_cb);

}

static void setting_tab_img_line_create(void)
{
	lv_obj_t *img_ring = lv_img_create(lv_scr_act(), NULL);
	static rom_bin_info info_ring = rom_bin_info_get(ROM_RES_SETTING_RING_PNG);
	lv_img_set_src(img_ring, &info_ring);
	lv_obj_set_pos(img_ring, 955, 125);

	lv_obj_t *img_room = lv_img_create(lv_scr_act(), NULL);
	static rom_bin_info info_room = rom_bin_info_get(ROM_RES_SETTING_UNLOCK_PNG);
	lv_img_set_src(img_room, &info_room);
	lv_obj_set_pos(img_room, 955, 227);

	lv_obj_t *img_time = lv_img_create(lv_scr_act(), NULL);
	static rom_bin_info info_time = rom_bin_info_get(ROM_RES_SETTING_TIME_PNG);
	lv_img_set_src(img_time, &info_time);
	lv_obj_set_pos(img_time, 955, 332);

	lv_obj_t *img_saver = lv_img_create(lv_scr_act(), NULL);
	static rom_bin_info info_saver = rom_bin_info_get(ROM_RES_SETTING_STANDBY_PNG);
	lv_img_set_src(img_saver, &info_saver);
	lv_obj_set_pos(img_saver, 955, 432);

	lv_obj_t *img_reset = lv_img_create(lv_scr_act(), NULL);
	static rom_bin_info info_reset = rom_bin_info_get(ROM_RES_SETTING_OTHER_PNG);
	lv_img_set_src(img_reset, &info_reset);
	lv_obj_set_pos(img_reset, 955, 532);
}

static void setting_page_base_line_create(lv_obj_t *parent,int num)
{
	if(num < 1)
	{
		return ;
	}
	static lv_point_t line_point1[] = {{70, 298}, {612, 298}};
	/*Create a line and apply the new style*/
	lv_obj_t *line1 = lv_line_create(parent, NULL);

	lv_obj_set_style_local_line_color(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
	lv_obj_set_style_local_line_opa(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_line_width(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

	lv_line_set_points(line1, line_point1, 2);
	if(num < 2)
	{
		return ;
	}

	static lv_point_t line_point2[] = {{70, 397}, {612, 397}};
	/*Create a line and apply the new style*/
	lv_obj_t *line2 = lv_line_create(parent, line1);
	lv_line_set_points(line2, line_point2, 2);

}

static void setting_door_ring_label_display(int id)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[0], NULL), id);
	if (obj != NULL)
	{
		if(id  == 0){
			if (user_data_get()->audio.door1_ring > 6)
			{
				user_data_get()->audio.door1_ring = 0;
			}
			char buf[64];
			// char *str[language_total] = {"Ring","铃声","Звонок","Anneau","Anillo","Ring","رنين","Zvonění","צלצול ","Toque","Suoneria"};
			sprintf(buf,"%s %d",str_get(LAYOUT_SETTING_LANG_RING_ID),user_data_get()->audio.door1_ring+1);
			lv_label_set_text(obj, buf);
		}else if(id == 1){
			if (user_data_get()->audio.door2_ring > 6)
			{
				user_data_get()->audio.door2_ring = 0;
			}
			char buf[64];
			// char *str[language_total] = {"Ring","铃声","Звонок","Anneau","Anillo","Ring","رنين","Zvonění","צלצול ","Toque","Suoneria"};
			sprintf(buf,"%s %d",str_get(LAYOUT_SETTING_LANG_RING_ID),user_data_get()->audio.door1_ring+1);
			lv_label_set_text(obj, buf);
		}
	}
}

static void setting_door1_ring_btn_up(lv_obj_t *obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		// user_data_get()->audio.door1_ring = user_data_get()->audio.door1_ring == 5 ? 0 : (++(user_data_get()->audio.door1_ring));
		// setting_door_ring_label_display(0);
		// door_ring_play(user_data_get()->audio.door1_ring,get_sound_val(user_data_get()->audio.door_ring_val) , NULL, NULL);
		
		// user_data_save();
		set_enter_setting_page_which(0);
		goto_layout(pLAYOUT(ring));
	}
}

static lv_obj_t *setting_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, char *string, btn_data *pbtndata, lv_point_t point_a[])
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));

	lv_obj_set_style_local_border_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_border_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));

	lv_obj_set_style_local_border_width(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_width(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 1);
	lv_obj_set_style_local_border_side(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_side(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_BORDER_SIDE_BOTTOM);

	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_label_set_long_mode(label,LV_LABEL_LONG_CROP);
	lv_obj_set_size(label,500,30);
	lv_label_set_text(label, string);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	if (point_a != NULL)
	{
		point_a[0].x = x + 506;
		point_a[0].y = y + 30;

		point_a[1].x = point_a[0].x + 20;
		point_a[1].y = point_a[0].y + 20;

		point_a[2].x = point_a[0].x;
		point_a[2].y = point_a[1].y + 20;
		lv_obj_t *line_right = lv_line_create(parent, NULL);
		lv_obj_set_style_local_line_color(line_right, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
		// lv_obj_set_style_local_line_color(line_right, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x50, 0x50, 0x50));
		lv_obj_set_style_local_line_opa(line_right, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		lv_obj_set_style_local_line_width(line_right, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
		lv_line_set_points(line_right, point_a, 3);
	}
	btn->user_data = pbtndata;
	btn_touch_event_listen(btn);

	return btn;
}

static void setting_page_1_door1_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_anything_create(setting_door1_ring_btn_up);
	//static lv_point_t line_point_right1[3];
	// char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür","باب ","Dveře","דלת","Porta","Porta"};
	lv_obj_t *btn = setting_page_btn_create(parent, 70, 0, 542, 99, str_get(LAYOUT_HOME_LANG_DOOR_ID), &btn_data, NULL);

	lv_obj_t *door1_ring_info_label = lv_label_create(parent, NULL);

	lv_label_set_long_mode(door1_ring_info_label,LV_LABEL_LONG_CROP);
	lv_obj_set_size(door1_ring_info_label,100,30);
	lv_label_set_align(door1_ring_info_label,LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(door1_ring_info_label, btn, LV_ALIGN_IN_RIGHT_MID, -36, 0);

	lv_obj_set_id(door1_ring_info_label, 0);

	setting_door_ring_label_display(0);
}

// static void setting_door2_ring_btn_up(lv_obj_t *obj,lv_event_t event)
// {
// 	if(event == LV_EVENT_CLICKED){
// 		user_data_get()->audio.door2_ring = user_data_get()->audio.door2_ring == 5 ? 0 : (++(user_data_get()->audio.door2_ring));
// 		setting_door_ring_label_display(1);
		
// 		door_ring_play(user_data_get()->audio.door2_ring,get_sound_val(user_data_get()->audio.door_ring_val) , NULL, NULL);
	
// 		user_data_save();
// 	}
// }

// static void setting_page_1_door2_btn_create(lv_obj_t *parent)
// {
// 	static btn_data btn_data = btn_data_anything_create(setting_door2_ring_btn_up);
// 	//static lv_point_t line_point_right[3];
// 	char * src[language_total] = {"Door2", "大门2", "Дверь2", "Porte2", "Puerta2", "Tür2","باب 2","Dveře2"};
// 	lv_obj_t *btn = setting_page_btn_create(parent, 70, 101, 542, 99, src[user_data_get()->user_language], &btn_data, NULL);

// 	lv_obj_t *door_ring_info_label = lv_label_create(parent, NULL);
// 	lv_obj_set_id(door_ring_info_label, 1);

// 	setting_door_ring_label_display(1);
// 	lv_label_set_align(door_ring_info_label, LV_LABEL_ALIGN_RIGHT);
// 	lv_obj_align(door_ring_info_label, btn, LV_ALIGN_IN_RIGHT_MID, -36, 0);
// }

static void setting_ring_time_btn_up(lv_obj_t *obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		if(user_data_get()->ring_time < 60)
		{
			user_data_get()->ring_time += 10;
			
		}else{
			user_data_get()->ring_time = 10;

		}
		lv_label_set_text_fmt(lv_obj_get_child_form_id(lv_obj_get_child(setting_page[0],NULL), 10), "%ds",user_data_get()->ring_time);
			
		user_data_save();
	}
}


static void setting_page_1_door_ring_time_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_anything_create(setting_ring_time_btn_up);
	// char *src[language_total] = {"Ring time", "铃声时间", "Время звонка", "Temps de sonnerie", "Tiempo de timbre", "Klingelzeit", "مدة نغمة الرنين", "Doba zvonění", "זמן צלצול","Tempo de Toque","Tempo di squillo"};
	lv_obj_t *btn = setting_page_btn_create(parent, 70, 101, 542, 99, str_get(LAYOUT_SETTING_LANG_RINGTIME_ID), &btn_data, NULL);

	lv_obj_t *ring_time_label = lv_label_create(parent, NULL);
	lv_obj_set_id(ring_time_label, 10);


	lv_label_set_text_fmt(ring_time_label, "%ds",user_data_get()->ring_time);


	lv_obj_align(ring_time_label, btn, LV_ALIGN_IN_RIGHT_MID, -36, 0);

}




static void setting_ring_slider_event(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *obj_slider1 = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[0], NULL), 2);
		if (obj_slider1 == obj)
		{
			lv_obj_t *obj_label = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[0], NULL), 3);
			if (obj_label != NULL)
			{
				int a = lv_slider_get_value(obj);
				
				lv_label_set_text_fmt(obj_label, "%d",a);
				user_data_get()->audio.door_ring_val = a;
				user_data_save();

				//if (is_audio_play_ing() == true)
				//{
				//	audio_volume_set(user_data_get()->audio.door_ring_val);
				//}
				//else
				//{
				
				//audio_volume_set(get_sound_val(user_data_get()->audio.door_ring_val));
				door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(a), NULL, NULL);
				
				//}
			}
			return;
		}
		obj_slider1 = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[0], NULL), 4);
		if (obj_slider1 == obj)
		{
			lv_obj_t *obj_label = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[0], NULL), 5);
			if (obj_label != NULL)
			{
				int a = lv_slider_get_value(obj);
				
				lv_label_set_text_fmt(obj_label, "%d",a);
				user_data_get()->audio.intercom_ring_val = a;	
				user_data_save();
				interphone_ring_play(get_sound_val(user_data_get()->audio.intercom_ring_val),  NULL, NULL);

			}
			return;
		}
	}
}
static void setting_page_1_ring_vol_slider_create(lv_obj_t *parent)
{
	lv_obj_t *slider = lv_slider_create(parent, NULL);
	lv_obj_set_id(slider, 2);
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));

	lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_slider_set_range(slider, 0, 100);
	
	lv_obj_set_ext_click_area(slider, 8, 8, 20, 20);
	slider->event_cb = setting_ring_slider_event;
	

	lv_slider_set_value(slider, user_data_get()->audio.door_ring_val, LV_ANIM_OFF);

	lv_obj_set_pos(slider, 180, 249);
	lv_obj_set_size(slider, 356, 3);


	lv_obj_t *slider_label = lv_label_create(parent, NULL);
	lv_obj_align(slider_label, slider, LV_ALIGN_IN_RIGHT_MID, 50, 0);
	lv_obj_set_id(slider_label, 3);

	
	lv_label_set_text_fmt(slider_label, "%d",user_data_get()->audio.door_ring_val);

	lv_obj_t *label_main = lv_label_create(parent, NULL);
	// char *src[language_total] = {"Doorbell",
	// 							 "门铃音量",
	// 							 "Звонок",
	// 							 "Sonnette",
	// 							 "Timbre",
	// 							 "Türklingel",
	// 							 "جرس الباب",
	// 							 "Domovní \nzvonek",
	// 							 "פעמון",
	// 							 "Campainha",
	// 							 "Campanello"};
	lv_label_set_text(label_main, str_get(LAYOUT_SETTING_LANG_DOORBELL_ID));
	lv_obj_set_auto_realign(label_main, true);
	lv_obj_align(label_main, slider, LV_ALIGN_IN_LEFT_MID, -110, 0);
	
}

static void setting_page_1_ring_intercom_slider_create(lv_obj_t *parent)
{
	lv_obj_t *slider = lv_slider_create(parent, NULL);
	lv_obj_set_id(slider, 4);
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));

	lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_slider_set_range(slider, 0, 100);
	
	lv_obj_set_ext_click_area(slider, 8, 8, 20, 20);
	slider->event_cb = setting_ring_slider_event;


	lv_slider_set_value(slider, user_data_get()->audio.intercom_ring_val, LV_ANIM_OFF);

	lv_obj_set_pos(slider, 180, 349);
	lv_obj_set_size(slider, 356, 3);

	lv_obj_t *slider_label = lv_label_create(parent, NULL);
	lv_obj_align(slider_label, slider, LV_ALIGN_IN_RIGHT_MID, 50, 0);
	lv_obj_set_id(slider_label, 5);

	lv_label_set_text_fmt(slider_label, "%d",user_data_get()->audio.intercom_ring_val);


	lv_obj_t *label_main = lv_label_create(parent, NULL);
	// char *src[language_total] = {"Ringtone",
	// 							 "内呼音量",
	// 							 "Рингтон",
	// 							 "Ringtone",
	// 							 "Tintineo",
	// 							 "Klingelton",
	// 							 "نغمة الرنين",
	// 							 "Vyzváněcí tón",
	// 							 "סוג צלצול",
	// 							 "Tom de Toque",
	// 							 "Suoneria"};
	lv_label_set_text(label_main, str_get(LAYOUT_SETTING_LANG_RINGTONE_ID));
	lv_obj_set_auto_realign(label_main, true);
	lv_label_set_long_mode(label_main,LV_LABEL_LONG_CROP);
	lv_obj_set_size(label_main,100,30);
	lv_label_set_align(label_main, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label_main, slider, LV_ALIGN_IN_LEFT_MID, -110, 0);

}


static void setting_ring_page_create(void)
{
	setting_page_base_line_create(setting_page[0],2);//画线 那个页面 几条线

	setting_page_1_door1_btn_create(setting_page[0]);

	// /setting_page_1_door2_btn_create(setting_page[0]);

	setting_page_1_door_ring_time_create(setting_page[0]);

	setting_page_1_ring_vol_slider_create(setting_page[0]);

	setting_page_1_ring_intercom_slider_create(setting_page[0]);
	
}

static void floor_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(floor_setting));
}

static void setting_page_2_floor_setting_btn_create(lv_obj_t *parent)
{

	static btn_data btn_data = btn_data_up_create(floor_setting_btn_up);
	// char * src[language_total] = {"Device ID", "设备ID", "ID zařízení", "ID du périphérique", "ID del dispositivo", "Geräte-ID","معرف الجهاز","ID zařízení","זהות המכשיר","ID do dispositivo","Ripetere ID"};
	setting_page_btn_create(parent, 70, 0, 542, 99, str_get(LAYOUT_SETTING_LANG_DEVICEID_ID), &btn_data, NULL);

}

static void room_id_setting_btn_up(lv_obj_t *obj)
{
	enter_layout_passwd_ch(PASSWD_CH_SETTING_FLOOR);
	set_enter_setting_page_which(1);
	goto_layout(pLAYOUT(setting_password));
}

static void setting_page_2_room_id_setting_btn_create(lv_obj_t *parent)
{

	static btn_data btn_data = btn_data_up_create(room_id_setting_btn_up);
	// char *src[language_total] = {"Set room ID", "房号设置", "Комната нет", "Chambre Non", "Habitación no", "Raum Nr", "ضبط معرف الغرفة", "Nastavte ID místnosti", "תכנות מספר מסך","Definir ID da Zona","Imposta l'ID stanza"};
	setting_page_btn_create(parent, 70, 101, 542, 99, str_get(LAYOUT_SETTING_LANG_SETROOMID_ID), &btn_data, NULL);

}

static void floor_number_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(floor_number_select));
}
static void setting_page_2_floor_number_setting_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_up_create(floor_number_setting_btn_up);
	setting_page_btn_create(parent, 70, 202, 542, 99, str_get(LAYOUT_SET_FLOOR_NUMBER_ID), &btn_data, NULL);

}

static void setting_room_page_create(void)
{
	setting_page_2_floor_setting_btn_create(setting_page[1]);
	setting_page_2_room_id_setting_btn_create(setting_page[1]);//hlf:5.5
	setting_page_2_floor_number_setting_btn_create(setting_page[1]);
}


//第三个设置页面，也就是时间设置页面
static void setting_page_3_base_create(lv_obj_t* parent)
{
	static lv_point_t line_point1[] = {{91, 200}, {91+120, 200}};
	/*Create a line and apply the new style*/
	lv_obj_t *line1 = lv_line_create(parent, NULL);

	lv_obj_set_style_local_line_color(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xF8,0xCD,0xA5));
	lv_obj_set_style_local_line_opa(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_line_width(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_line_set_points(line1, line_point1, 2);


	static lv_point_t line_point2[] = {{231, 200}, {231+80, 200}};
	lv_obj_t *line2 = lv_line_create(parent, line1);
	lv_line_set_points(line2, line_point2, 2);

	static lv_point_t line_point3[] = {{331, 200}, {331+80, 200}};
	lv_obj_t *line3 = lv_line_create(parent, line1);
	lv_line_set_points(line3, line_point3, 2);

	static lv_point_t line_point4[] = {{446, 200}, {446+80, 200}};
	lv_obj_t *line4 = lv_line_create(parent, line1);
	lv_line_set_points(line4, line_point4, 2);

	static lv_point_t line_point5[] = {{553, 200}, {553+80, 200}};
	lv_obj_t *line5 = lv_line_create(parent, line1);
	lv_line_set_points(line5, line_point5, 2);



	static lv_point_t line_point6[] = {{91, 275}, {91+120, 275}};
	lv_obj_t *line6 = lv_line_create(parent, line1);
	lv_line_set_points(line6, line_point6, 2);

	static lv_point_t line_point7[] = {{231, 275}, {231+80, 275}};
	lv_obj_t *line7 = lv_line_create(parent, line1);
	lv_line_set_points(line7, line_point7, 2);

	static lv_point_t line_point8[] = {{331, 275}, {331+80, 275}};
	lv_obj_t *line8 = lv_line_create(parent, line1);
	lv_line_set_points(line8, line_point8, 2);

	static lv_point_t line_point9[] = {{446, 275}, {446+80, 275}};
	lv_obj_t *line9 = lv_line_create(parent, line1);
	lv_line_set_points(line9, line_point9, 2);

	static lv_point_t line_point10[] = {{553, 275}, {553+80, 275}};
	lv_obj_t *line10 = lv_line_create(parent, line1);
	lv_line_set_points(line10, line_point10, 2);


	static lv_point_t line_point11[] = {{213, 239}, {213+14, 239}};
	lv_obj_t *line11 = lv_line_create(parent, line1);
	lv_obj_set_style_local_line_width(line11, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
	lv_line_set_points(line11, line_point11, 2);

	static lv_point_t line_point12[] = {{314, 239}, {314+14, 239}};
	lv_obj_t *line12 = lv_line_create(parent, line11);
	lv_line_set_points(line12, line_point12, 2);


	lv_obj_t* obj1 = lv_obj_create(parent, NULL);
	lv_obj_set_style_local_bg_opa(obj1,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj1,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xF8,0xCD,0xA5));
	lv_obj_set_style_local_radius(obj1,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 45);
	lv_obj_set_pos(obj1,534,224);
	lv_obj_set_size(obj1,6,6);
	lv_obj_set_click(obj1, false);

	
	lv_obj_t* obj2 = lv_obj_create(parent, obj1);
	lv_obj_set_pos(obj2,534,248);
}

//通过roller选择时间
static void setting_page_3_roller_event(lv_obj_t* obj,lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t* parent = lv_obj_get_child(setting_page[2], NULL);
		if(parent == NULL)
		{
			return ;
		}
		if(setting_time_edit == false)
		{
			setting_time_edit = true;
		}
		
		lv_obj_t* obj_roller = lv_obj_get_child_form_id(parent, 0);
		if(obj_roller == obj )
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_tm.tm_year = 2021 + id;
			return ;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 1);
		if(obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_tm.tm_mon = 1 + id;
			return ;
		}

		obj_roller = lv_obj_get_child_form_id(parent,  2);
		if(obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_tm.tm_mday = 1 + id;
			return ;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 3);
		if(obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_tm.tm_hour = id;
			return ;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 4);
		if(obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_tm.tm_min = id;
			return ;
		}
	}

}

static lv_obj_t* setting_page_3_roller_create(lv_obj_t* parent,int x,int y ,int w,char* opt)
{
	lv_obj_t* roller = lv_roller_create(parent,NULL);
	lv_obj_set_style_local_text_line_space(roller,LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 50);//文本行间距
	lv_obj_set_style_local_text_color(roller,LV_ROLLER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x80,0x80,0x80));//未选中文本颜色，设置为灰色
	
	lv_obj_set_style_local_bg_opa(roller,LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_OPA_TRANSP);//选中区域背景透明度，设置为完全透明
	lv_obj_set_style_local_text_color(roller,LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, lv_color_make(0xF8,0xCD,0xA5));//选中文本颜色，设置为浅棕色

	if(x == 91)
		lv_roller_set_options(roller, opt, LV_ROLLER_MODE_NORMAL);
	else
		lv_roller_set_options(roller, opt, LV_ROLLER_MODE_INFINITE);
	
	lv_roller_set_visible_row_count(roller, 3);
	lv_obj_set_pos(roller,x,y);
	lv_obj_set_width(roller,w);
	lv_obj_set_ext_click_area(roller, 0, 0, 20, 20);
	static btn_data btn_data = btn_data_anything_create(setting_page_3_roller_event);
	
	roller->user_data = &btn_data;
	btn_touch_event_listen(roller);
	return roller;
}
static void setting_page_3_year_roller_create(lv_obj_t* parent)
{
	char opt[512] = {0};
	for(int  i = 2021 ; i < 2037 ; i++)
	{
		char buf[8] = {0};
		sprintf(buf,"%d%s",i, i== 2036?"":"\n");
		strcat(opt,buf);
	}
	lv_obj_t* obj =  setting_page_3_roller_create(parent,91,120,120,opt);
	lv_obj_set_id(obj, 0);

	int id = setting_tm.tm_year > 2021?(setting_tm.tm_year - 2021):0;
	lv_roller_set_selected(obj,id,LV_ANIM_OFF);
	
}

static void setting_page_3_month_roller_create(lv_obj_t* parent)
{
	char opt[64] = {0};
	for(int  i = 1 ; i < 13 ; i++)
	{
		char buf[8] = {0};
		sprintf(buf,"%02d%s",i, i== 12?"":"\n");
		strcat(opt,buf);
	}
	lv_obj_t* obj =  setting_page_3_roller_create(parent,231,120,80,opt);
	lv_obj_set_id(obj, 1);
	int id = setting_tm.tm_mon - 1;
	lv_roller_set_selected(obj,id,LV_ANIM_OFF);
	
}

static void setting_page_3_day_roller_create(lv_obj_t* parent)
{
	char opt[64] = {0};
	for(int  i = 1 ; i < 32 ; i++)
	{
		char buf[8] = {0};
		sprintf(buf,"%02d%s",i, i== 31?"":"\n");
		strcat(opt,buf);
	}
	lv_obj_t* obj =  setting_page_3_roller_create(parent,331,120,80,opt);
	lv_obj_set_id(obj, 2);

	int id = setting_tm.tm_mday- 1;
	lv_roller_set_selected(obj,id,LV_ANIM_OFF);
	
}

static void setting_page_3_hour_roller_create(lv_obj_t* parent)
{
	char opt[128] = {0};
	for(int  i = 0 ; i < 24 ; i++)
	{
		char buf[8] = {0};
		sprintf(buf,"%02d%s",i, i== 23?"":"\n");
		strcat(opt,buf);
	}
	lv_obj_t* obj =  setting_page_3_roller_create(parent,446,120,80,opt);
	lv_obj_set_id(obj, 3);
	
	int id = setting_tm.tm_hour;
	lv_roller_set_selected(obj,id,LV_ANIM_OFF);
	
}

static void setting_page_3_min_roller_create(lv_obj_t* parent)
{
	char opt[512] = {0};
	for(int  i = 0 ; i < 60 ; i++)
	{
		char buf[8] = {0};
		sprintf(buf,"%02d%s",i, i== 59?"":"\n");
		strcat(opt,buf);
	}
	lv_obj_t* obj =  setting_page_3_roller_create(parent,553,120,80,opt);
	lv_obj_set_id(obj, 4);

	int id = setting_tm.tm_min;
	lv_roller_set_selected(obj,id,LV_ANIM_OFF);
	
}


static void setting_time_page_create(void)
{	
	setting_page_3_base_create(setting_page[2]);
	
	setting_page_3_year_roller_create(setting_page[2]);
	setting_page_3_month_roller_create(setting_page[2]);
	setting_page_3_day_roller_create(setting_page[2]);
	setting_page_3_hour_roller_create(setting_page[2]);
	setting_page_3_min_roller_create(setting_page[2]);
}



static void screensaver_btnmatrix_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
		if (btn_id == LV_BTNMATRIX_BTN_NONE)
			return;
	
		if (lv_btnmatrix_get_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_DISABLED))
			return;
	
		lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);
		lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
	
		user_data_get()->other.screen_saver = btn_id;
		user_data_save();

	}
}
static void setting_page_4_btnmatrix_create(lv_obj_t* parent)
{
	lv_obj_t *obj = lv_btnmatrix_create(parent, NULL);

	lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	
	lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_BORDER_SIDE_BOTTOM);

	lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
	lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_make(0, 0, 0));
	
	lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 1);


	lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);
	static rom_bin_info info_off = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off);

	static rom_bin_info info_on = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on);

	// static const char *map[language_total][4] = {
	// 	{"Screen OFF", "\n", "Clock Display", ""},
	// 	{"关闭屏幕", "\n", "时钟屏保", ""},
	// 	{"Экран Вiкл", "\n", "Дисплей часов", ""},
	// 	{"Écran éteint", "\n", "Horloge Affichage", ""},
	// 	{"Pantalla apagada", "\n", "Pantalla de reloj", ""},
	// 	{"Bildschirm AUS", "\n", "Uhranzeige", ""},
	// 	{"قفل الشاسة", "\n", "عرض الساعة", ""},
	// 	{"Vypnout obrazovku", "\n", "Zobrazit hodiny", ""},
	// 	{"כיבוי מסך", "\n", "תצוגת שעון", ""},
	// 	{"Écran Desligado","\n","Exibir Relógio",""},
	// 	{"Schermo spento","\n","Orologio",""}
	// };
	lv_btnmatrix_set_map(obj, screenselct_str_get());
	lv_btnmatrix_set_align(obj, LV_LABEL_ALIGN_LEFT, 70, 0);
	lv_obj_set_x(obj, 70);
	lv_obj_set_width(obj, 542);
	lv_obj_set_height(obj, 205);

	static btn_data btn_data = btn_data_anything_create(screensaver_btnmatrix_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);

	lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);
	lv_btnmatrix_set_btn_ctrl(obj, user_data_get()->other.screen_saver, LV_BTNMATRIX_CTRL_CHECK_STATE);
}

static void setting_screensaver_page_create(void)
{
	setting_page_4_btnmatrix_create(setting_page[3]);
}



static lv_obj_t* reset_cont = NULL;
static int format_falg = 0;

static void setting_page_5_btn_up(lv_obj_t* obj)
{	
	lv_obj_t* child  = lv_obj_get_child_form_id( reset_cont, 0);//找到结点号为0的子对象 确认按钮
	if(child == obj)
	{	
		if(format_falg == 0)
			return ;
		else if(format_falg == 1){
			user_data_reset();
		}else if(format_falg == 2){
			if(is_sdcard_insert() == true){
				start_format_sd_card();
				ak_sleep_ms(1000);
				lv_obj_del(reset_cont->parent);
			}else
				return ;
		}
	}
	
	child  = lv_obj_get_child_form_id( reset_cont, 1);
	if(child == obj)
	{
		lv_obj_del(reset_cont->parent);
		setting_head_label_set(4);
	}
	
}


static void setting_page_5_confirm_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_pos(obj, 45, 275);
	lv_obj_set_size(obj, 228, 72);
	lv_obj_set_id(obj,0);
	
	// char *src[language_total] = {"Confirm", "确认", "Подтверждать", "Confirmer", "Confirmar", "Bestätigen Sie", "تاكيد", "Potvrdit", "אישור","Confirmar","Conferma"};
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CONFIRM_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(setting_page_5_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}



static void setting_page_5_cancel_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_pos(obj, 311, 275);
	lv_obj_set_size(obj, 228, 72);
	lv_obj_set_id(obj,1);
	// char *src[language_total] = {"Cancel", "取消", "Отмена", "Annuler", "Cancelar", "Absagen", "الغاء", "Zrušit", "ביטול","Cancelar","Annulla"};
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CANCEL_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(setting_page_5_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}


static void reset_btnm_up(lv_obj_t* obj,lv_event_t event){
	printf("\n00000%d0000000000000\n",event);
	if(event == LV_EVENT_VALUE_CHANGED){
		uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
		format_falg = ++btn_id;
	}
}


static void other_reset_btn_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED)
	{

		format_falg = 0;
		setting_head_label_set(5);
		//背景的阴影
		lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
		lv_obj_set_size(cont_bg,1024,600);
		lv_obj_set_pos(cont_bg,0,0);
		lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
		
		//按钮的容器
		reset_cont = lv_cont_create(cont_bg, NULL);
		lv_obj_set_size(reset_cont,584,362);
		lv_obj_set_pos(reset_cont,220,120);
		lv_obj_set_style_local_bg_color(reset_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x212121));
		lv_obj_set_style_local_bg_opa(reset_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
		lv_obj_set_style_local_border_color(reset_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(reset_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
		lv_obj_set_style_local_border_color(reset_cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(reset_cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
		lv_obj_set_style_local_border_color(reset_cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(reset_cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);


		lv_obj_t *obj = lv_btnmatrix_create(reset_cont, NULL);
		lv_obj_set_id(obj,66);
		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));

		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);

		static rom_bin_info info_off1 = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off1);
		

		static rom_bin_info info_on1 = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on1);

		// static const char *map1[language_total][4] = {
		// 	{"Format device", "\n", "Format SD card", ""},
		// 	{"设备格式化", "\n", "SD卡格式化", ""},
		// 	{"Устройство формата", "\n", "Формат SD-карты", ""},
		// 	{"Formater l'appareil", "\n", "Formater la carte SD", ""},
		// 	{"Formatear dispositivo", "\n", "Formatear tarjeta SD", ""},
		// 	{"Gerät formatieren", "\n", "SD-Karte formatieren", ""},
		// 	{"تهيئة التجهار", "\n", "تهيئة بطاقة الذاكرة", ""},
		// 	{"Smazat zařízení", "\n", "Naformátujte SD kartu", ""},
		// 	{"אתחול מסך", "\n", "אתחול כרטיס SD", ""},
		// 	{"Formatar Dispositivo","\n","Formatar Cartão SD", ""},
		// 	{"Reset dispositivo","\n","Formatta la scheda SD", ""},
			
		// };
		lv_btnmatrix_set_map(obj, formatselct_str_get());
		lv_btnmatrix_set_align(obj, LV_LABEL_ALIGN_LEFT, 130, 0);
		lv_obj_set_x(obj, 50);
		lv_obj_set_y(obj, 20);
		lv_obj_set_size(obj, 384,220);
		

		static btn_data btn_data = btn_data_anything_create(reset_btnm_up);
		obj->user_data = &btn_data;
		btn_touch_event_listen(obj);
		

		lv_btnmatrix_set_one_check(obj, true);
		lv_btnmatrix_set_btn_ctrl(obj, 0, LV_BTNMATRIX_CTRL_CHECKABLE);

		lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
		lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_BORDER_SIDE_BOTTOM);
		
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_hex(0x000000));
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x000000));
		
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 1);
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED,1);

	
		lv_obj_set_style_local_text_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x666666));

		if(is_sdcard_insert() == false){
			lv_btnmatrix_set_btn_ctrl(obj, 1, LV_BTNMATRIX_CTRL_DISABLED);
		}else{
			lv_btnmatrix_set_btn_ctrl(obj, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
		}

		
		setting_page_5_confirm_btn_create(reset_cont);
		setting_page_5_cancel_btn_create(reset_cont);
		

	}

}





static void setting_page_5_reset_btn_create(lv_obj_t *parent)
{

	static btn_data btn_data = btn_data_anything_create(other_reset_btn_up);
	static lv_point_t line_point_right1[6];
	// char *src[language_total] = {"Formatting", "格式化", "Формат", "Formatage", "Formateo", "Formatierung", "جار التهيئة", "Formátování", "מאתחל","A Formatar","Formattazione"};
	setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?801:601, 542, 99, str_get(LAYOUT_SETTING_LANG_FORMAT_ID), &btn_data, line_point_right1);
}

static void record_btn_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
			lv_obj_t *label = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL), 1);
		if(label != NULL){
			if(user_data_get()->motion.record == 0){
				
				user_data_get()->motion.record = 1;
				// char *src[language_total] = {"Photo", "拍照", "Фото", "Photo", "Foto", "Foto", "صورة", "Fotografie", "תמונה","Fotos","Foto"};
				lv_label_set_text(label, str_get(LAYOUT_MONITOR_LANG_PHOTO_ID));
			
			}else if(user_data_get()->motion.record == 1){
				if(is_sdcard_insert() == true){
					user_data_get()->motion.record = 2; 
					// char *src[language_total] = {"Video", "录像", "Видео", "Vidéo", "Video", "Video", "فديو ", "Video", "וידאו","Vídeos","Registrazione"};
					lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_VIDEO_ID));
				
				}else if(is_sdcard_insert() == false){
					user_data_get()->motion.record = 0;
					// char *src[language_total] = {"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
					lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_OFF_ID));
				}
			}else if(user_data_get()->motion.record == 2){
			
				user_data_get()->motion.record = 0;
				// char *src[language_total] = {"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
				lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_OFF_ID));

			}
			lv_obj_align(label, obj, LV_ALIGN_IN_RIGHT_MID, -18, 0);
			user_data_save();
		}
	}
}

static void setting_page_5_record_btn_create(lv_obj_t* parent){

	static btn_data btn_data = btn_data_anything_create(record_btn_up);
	// char *src[language_total] = {"Record", "自动拍照", "Запись", "Record", "Registro", "Datensatz", "تسجيل", "Záznam", "הקלטה","Gravar","Registrazione"};

	lv_obj_t *btn = setting_page_btn_create(parent, 70, 201, 542, 99, str_get(LAYOUT_SETTING_LANG_RECORD_ID), &btn_data, NULL);

	lv_obj_t *auto_flag = lv_label_create(parent, NULL);
	lv_obj_set_id(auto_flag, 1);
	lv_obj_set_style_local_text_font(auto_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));	
	lv_obj_set_style_local_text_color(auto_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	
	if(user_data_get()->motion.record == 0){
		
		// char *src1[language_total] = {"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
		lv_label_set_text(auto_flag, str_get(LAYOUT_SETTING_LANG_OFF_ID));
	}else if(user_data_get()->motion.record == 1){
		// char *src1[language_total] = {"Photo", "拍照", "Фото", "Photo", "Foto", "Foto", "صورة", "Fotografie", "תמונה","Fotos","Foto"};
		lv_label_set_text(auto_flag, str_get(LAYOUT_MONITOR_LANG_PHOTO_ID));
	
		
	}else if(user_data_get()->motion.record == 2){
		if(is_sdcard_insert() == true){
			// char *src1[language_total] = {"Video", "录像", "Видео", "Vidéo", "Video", "Video", "فديو ", "Video", "וידאו","Vídeos","Registrazione"};
			lv_label_set_text(auto_flag, str_get(LAYOUT_SETTING_LANG_VIDEO_ID));

		}else {
			
			user_data_get()->motion.record = 0;
			// char *src1[language_total] = {"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
			lv_label_set_text(auto_flag, str_get(LAYOUT_SETTING_LANG_OFF_ID));
		}
	}
	lv_obj_align(auto_flag, btn, LV_ALIGN_IN_RIGHT_MID, -18, 0);
	user_data_save();
}

static void motion_btn_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		if(is_sdcard_insert() == false)
		{
			if(user_data_get()->motion.motion == false)
			{
				return ;
			}
		}
		else
		{		
			if(user_data_get()->motion.motion == false)
			{
				user_data_get()->motion.motion = true;
				//给门口机发指令开始移动侦测
				// char *src1[language_total] = {"ON", "开", "Oткрыто", "Ouvert", "Abierto", "Öffnen", "تشغيل", "Zap.", "פועל","Ligado","Ligado"};
				lv_label_set_text(lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL), 100), str_get(LAYOUT_SETTING_LANG_ON_ID));
			}
			else
			{
				user_data_get()->motion.motion = false;

				// char *src1[language_total] = {"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
				lv_label_set_text(lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL), 100), str_get(LAYOUT_SETTING_LANG_OFF_ID));
			}
			
			user_data_save();
		
		}
	}
}


static void setting_page_5_move_btn_create(lv_obj_t* parent)
{
	static btn_data btn_data = btn_data_anything_create(motion_btn_up);
	// char *src[language_total] = {"Motion Detection", "移动侦测", "Обнаружение движения", "Détection de mouvement", "Detección de movimiento", "Bewegungserkennung", "كشف الحركة", "Detekce pohybu", "זיהוי תנועה","Detecção de Movimento","Detecção de Movimento"};
	
	lv_obj_t *btn = setting_page_btn_create(parent, 70, 301, 542, 99, str_get(LAYOUT_SETTING_LANG_MOTIONDETECTION_ID), &btn_data, NULL);

	lv_obj_t *motion_flag = lv_label_create(parent, NULL);
	lv_obj_set_id(motion_flag, 100);
	lv_obj_set_style_local_text_font(motion_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));	
	lv_obj_set_style_local_text_color(motion_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

	if(user_data_get()->motion.motion == false || is_sdcard_insert() == false)
	{
		// char *src1[language_total] ={"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
		lv_label_set_text(motion_flag, str_get(LAYOUT_SETTING_LANG_OFF_ID));
	}
	else
	{
		// char *src1[language_total] = {"ON", "开", "Oткрыто", "Ouvert", "Abierto", "Öffnen", "تشغيل", "Zap.", "פועל","Ligado","Ligado"};
		lv_label_set_text(motion_flag, str_get(LAYOUT_SETTING_LANG_ON_ID));
	}
	lv_obj_align(motion_flag, btn, LV_ALIGN_IN_RIGHT_MID, -18, 0);
}


static void set_door_delay_time1(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED)
	{
		if(user_data_get()->door1_delay < 25)
		{
			user_data_get()->door1_delay++;
		}
		else 
		{
			user_data_get()->door1_delay = 1;
		}
			lv_obj_t*  a = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL),44);
		if(a)
			lv_label_set_text_fmt(a,"%ds",user_data_get()->door1_delay);
		
		lv_obj_align(a , obj, LV_ALIGN_IN_RIGHT_MID, -18, 0);
		user_data_save();

		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_UNLOCK_TIME;
		data.arg1 = 1;
		data.arg2 = user_data_get()->door1_delay;
		data.device = DEVICE_GROUP;
		network_send_cmd_data(&data);

		data.device = DEVICE_OUTDOOR_0;
		network_send_cmd_data(&data);
	}
}

static void set_door_delay_time2(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED)
	{
		if(user_data_get()->door2_delay < 25)
		{
			user_data_get()->door2_delay++;
		}
		else 
		{
			user_data_get()->door2_delay = 1;
		}
		lv_obj_t*  a = lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL),55);
		if(a)
			lv_label_set_text_fmt(a,"%ds",user_data_get()->door2_delay);

		lv_obj_align(a , obj, LV_ALIGN_IN_RIGHT_MID, -18, 0);
	
		user_data_save();

		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_UNLOCK_TIME;
		data.arg1 = 2;
		data.arg2 = user_data_get()->door2_delay;
		data.device = DEVICE_GROUP;
		network_send_cmd_data(&data);

		data.device = DEVICE_OUTDOOR_0;
		network_send_cmd_data(&data);
	}
}


static void setting_page_5_door_delay_btn_create(lv_obj_t* parent)
{
	static btn_data btn_data1 = btn_data_anything_create(set_door_delay_time1);
	static btn_data btn_data2 = btn_data_anything_create(set_door_delay_time2);

	// char *src1[language_total] = {"Door unlock delay", "大门开锁延时", "Задержка разблокировки двери", "Délai de déverrouillage de la porte", "Retardo de desbloqueo de puerta", "Tür Entriegelungsverzögerung", "تأخير فتح الباب ", "Zpoždění odemknutí dveří", "השהיית פתיחת דלת","Atraso na Abertura da Porta","Atraso na Abertura da Porta"};
	// char *src2[language_total] = {"Garage unlock delay", "车库门开锁延时", "Задержка разблокировки гараж", "Délai de déverrouillage de la garage", "Retardo de desbloqueo de garaje", "Garage Entriegelungsverzögerung", "تأخير فتح باب المرآب", "Zpoždění odemknutí garáže", "השהיית פתיחת דלת 2","Atraso na Abertura da Garagem","Atraso na Abertura da Garagem"};
	lv_obj_t *btn1 = setting_page_btn_create(parent, 70, 401, 542, 99, str_get(LAYOUT_SETTING_LANG_DOORUNLOCKDELAY_ID), &btn_data1, NULL);
	lv_obj_t *btn2 = setting_page_btn_create(parent, 70, 501, 542, 99, str_get(LAYOUT_SETTING_LANG_GARAGEUNLOCKDELAY_ID), &btn_data2, NULL);


	lv_obj_t *time_flag1 = lv_label_create(parent, NULL);
	lv_obj_t *time_flag2 = lv_label_create(parent, NULL);

	lv_obj_set_style_local_text_font(time_flag1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));	
	lv_obj_set_style_local_text_color(time_flag1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	
	lv_obj_set_style_local_text_font(time_flag2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));	
	lv_obj_set_style_local_text_color(time_flag2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	
	lv_obj_set_id(time_flag1, 44);
	lv_obj_set_id(time_flag2, 55);
	
	lv_label_set_text_fmt(time_flag1, "%ds", user_data_get()->door1_delay);
	lv_label_set_text_fmt(time_flag2, "%ds", user_data_get()->door2_delay);
	lv_obj_align(time_flag1, btn1, LV_ALIGN_IN_RIGHT_MID, -18, 0);
	lv_obj_align(time_flag2, btn2, LV_ALIGN_IN_RIGHT_MID, -18, 0);

	

}




extern bool network_upgrade_sent_package_close(void);
extern bool network_upgrade_send_package_open(void);

extern bool net_online_global_device[DEVICE_TOTAL];//此变量只方便升级门口机用


static void cancel_btn_up(lv_obj_t* obj)
{	
	standby_timer_reset();
	printf("outdoor upgrade cancel!!!\n");
	network_upgrade_sent_package_close();

	network_cmd_data_init(data);
	data.cmd			= NET_COMON_CMD_UPGRADE_OUTDOOR;
	data.arg1			= 2;
	data.arg2			= 2;
	data.device 		= DEVICE_ALL;
	network_send_cmd_data(&data);
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 77);
	if(cont != NULL)
		lv_obj_del(cont);
}

bool outdoot_online_check(void)
{
	for(int i = 0 ;i<17; i ++)
	{
		if(net_online_global_device[DEVICE_UNIT_OUTDOOR_1 + i])
		{
			return true;
		}
	}
	return false;
}

static void upgrade_btn_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		if(1){
			standby_timer_close();
			lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
			lv_obj_set_id(cont_bg,77);
			lv_obj_set_size(cont_bg,1024,600);
			lv_obj_set_pos(cont_bg,0,0);
			lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
			lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
			
			lv_obj_t *cont = lv_cont_create(cont_bg, NULL);
			lv_obj_set_size(cont,750,360);
			lv_obj_set_pos(cont,137,120);
			lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
			lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
			lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
			lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
			lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
			lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
			lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
			lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);
			
			
			lv_obj_t* obj = lv_label_create(cont,NULL);
			// char * src[language_total] = {"Please wiat!!! Upgrading outdoor!!!!", 
			// 							  "请稍等!!! 门口机升级中!!!",
			// 							  "Пожалуйста ждите!!! Обновление наружного!!!", 
			// 							  "Veuillez patienter!!! Mise à niveau en plein air!!!",
			// 							  "Espere por favor!!! Actualización al aire libre!!!", 
			// 							  "Bitte warten!!! Outdoor aufrüsten!!!",
			// 							  "يرجى الانتظار جار الترقية!!!",
			// 	"Prosím počkej!!! Aktualizuji zařízení!!!",
			// 	"נא להמתין משדרג יחידה חיצונית",
			// 	"A Actualizar Unidade Exterior!!! Por favor, Aguarde",
			// 	"Attendere!!! Aggiornamento esterno",
			// };
			lv_label_set_text(obj, str_get(LAYOUT_SETTING_LANG_PLEASEWAIT_ID));
			lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
			lv_obj_set_style_local_value_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
			lv_obj_align(obj, cont, LV_ALIGN_IN_TOP_MID, 0, 100);	
			cont_bg->user_data = obj;
			
			
			lv_obj_t* cancle_btn = lv_btn_create(obj->parent, NULL);
			lv_obj_set_id(cancle_btn,1);
			lv_obj_set_style_local_bg_opa(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
			lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
			lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
			lv_obj_set_size(cancle_btn, 220, 70);
			lv_obj_align(cancle_btn, cancle_btn->parent, LV_ALIGN_IN_BOTTOM_MID, 0,-30);
			
			
			lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
			lv_obj_t* label = lv_label_create(cancle_btn,NULL);
			
			// char *src1[language_total] = {"Cancel", "取消", "Отмена", "Annuler", "Cancelar", "Absagen", "الغاء", "Zrušit", "ביטול","Cancelar","Annulla"};
			lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CANCEL_ID));
			
			lv_obj_align(label, cancle_btn, LV_ALIGN_CENTER, 0,0);
			
			
			static btn_data btn_data = btn_data_up_create(cancel_btn_up);
			cancle_btn->user_data = &btn_data;
			btn_touch_event_listen(cancle_btn);
			
			network_upgrade_send_package_open();
		}else {
			lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(),NULL);
			
			lv_obj_set_style_local_bg_color(msgbox, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0X1A1A1A));
			lv_obj_set_style_local_bg_opa(msgbox, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);
			lv_obj_set_style_local_border_width(msgbox, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, 2);
			lv_obj_set_style_local_border_color(msgbox, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0XEFCC8C));
			
			lv_obj_set_size(msgbox,530,300);
			lv_obj_align(msgbox, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

			// char *src2[language_total] = {"Outdoor unit offline!",
			// 							   "门口机不在线!",
			// 							   "Наружный блок отключен!",
			// 							   "Unité extérieure hors ligne!",
			// 							   "Unidad exterior fuera de línea!",
			// 							   "Außengerät offline!",
			// 							   "!الوحدة الخارجية غير متصلة",
			// 							  "Venkovní jednotka offline!",
			// 							  "יחידה חיצונית מנותקת",
			// 							  "Unidade Exterior Desligada",
			// 							  "Unità esterna offline"};
			
			lv_msgbox_set_text(msgbox,str_get(LAYOUT_SETTING_LANG_OUTDOOROFFLINE_ID));
			lv_msgbox_set_anim_time(msgbox, 0);
			lv_msgbox_start_auto_close(msgbox, 1600);

		}
	

	}
}

static void setting_page_5_upgrade_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_anything_create(upgrade_btn_up);

	static lv_point_t line_point_right1[4];
	// char *src[language_total] = {"Upgrade", "升级", "Обновлять", "Mise à niveau", "Mejora", "Aufrüstung", "ترقية", "Aktualizace", "שדרוג","Actualizar","Aggiornamento"};
	lv_obj_t *btn = setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?601:401, 542, 99, str_get(LAYOUT_SETTING_LANG_UPGRADE_ID), &btn_data, line_point_right1);
	if (access("/mnt/tf/net_camera.up", F_OK) != 0)
	{
		lv_obj_set_click(btn, false);
	}
}

/*
数据判断wifi是否打开 设置sw开关的状态  
wifi打开并且连接 则显示连接的wifi账号
wifi打开无连接 显示ON
wifi关闭 则显示OFF
*/
linked_info link_info = {0};
extern bool wpa_cli_wlan_status(bool *continue_flag);

static void setting_page_5_wifi_btn_create(lv_obj_t *parent){
	static btn_data btn_data = btn_data_anything_create(goto_wifi_layout);//回调函数

	
	lv_obj_t *btn = setting_page_btn_create(parent, 70, 0, 542, 99, "Wifi", &btn_data, NULL);

	lv_obj_t *wifi_flag = lv_label_create(parent, NULL);//创建wifi打开标志


	if(user_data_get()->wifi.wifi_open_flag){
		if(link_info.completed){
			lv_label_set_text(wifi_flag, link_info.wlan_ssid);
		}
		else
		{
			// char *src1[language_total] = {"ON", "开", "Oткрыто", "Ouvert", "Abierto", "Öffnen", "تشغيل", "Zap.", "פועל","Ligado","Ligado"};
			lv_label_set_text(wifi_flag, str_get(LAYOUT_SETTING_LANG_ON_ID));
		}
	}
	else
	{
		// char *src1[language_total] = {"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
		lv_label_set_text(wifi_flag, str_get(LAYOUT_SETTING_LANG_OFF_ID));
	}
		
		
		

/******************************************************************/		
	lv_obj_set_style_local_text_font(wifi_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));	
	lv_obj_set_style_local_text_color(wifi_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));	
	// lv_obj_set_style_local_text_color(wifi_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x50, 0x50, 0x50));	
	lv_obj_align(wifi_flag, btn, LV_ALIGN_IN_RIGHT_MID, -18, 0);
}


static void goto_language_layout(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		set_enter_setting_page_which(4);
		goto_layout(pLAYOUT(set_language));
	}
}

static void setting_page_5_language_btn_create(lv_obj_t *parent){
	static btn_data btn_data = btn_data_anything_create(goto_language_layout);//回调函数

	// char *str[language_total] = {"Language", "语言", "Язык", "Langues", "Idioma", "Sprache", "اللغة", "Jazyk", "שפה","Idioma","Lingua"};
	lv_obj_t *btn = setting_page_btn_create(parent, 70, 101, 542, 99, str_get(LAYOUT_SETTING_LANG_LANGUAGE_ID), &btn_data, NULL);
	
	lv_obj_t *language_flag = lv_label_create(parent, NULL);//创建wifi打开标志

	switch(user_data_get()->user_language){
		case 0:
			lv_label_set_text(language_flag,"English");
			break;
		case 1:
			lv_label_set_text(language_flag,"中文");
			break;
		case 2:
			lv_label_set_text(language_flag,"Pусский");
			break;
		case 3:
			lv_label_set_text(language_flag,"Français");
			break;
		case 4:
			lv_label_set_text(language_flag,"Español");
			break;
		case 5:
			lv_label_set_text(language_flag,"Deutsch");
			break;
		case 6:
			lv_label_set_text(language_flag,"عربي");
			break;
		case 7:
		lv_label_set_text(language_flag, "Čeština");
			break;
		case 8:
			lv_label_set_text(language_flag, "עברית");
			break;
		case 9:
			lv_label_set_text(language_flag, "Português");
			break;
		case 10:
			lv_label_set_text(language_flag, "Italiano");
			break;
		case 11:
			lv_label_set_text(language_flag, "Polish");
			break;
		case 12:
			lv_label_set_text(language_flag, "GREEK");
			break;
		case 13:
			lv_label_set_text(language_flag, "Türkçe");
			break;
		case 14:
			lv_label_set_text(language_flag, "Nederlands");
			break;
		default:
			break;
	}
		

/******************************************************************/		
	lv_obj_set_style_local_text_font(language_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));//设置字体大小	
	lv_obj_set_style_local_text_color(language_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));	
	
	lv_obj_align(language_flag, btn, LV_ALIGN_IN_RIGHT_MID, -15, 0);//设置对齐方式
		lv_label_set_long_mode(language_flag,LV_LABEL_LONG_CROP);//设置文本过长处理模式
	lv_obj_set_size(language_flag,100,30);//设置标签尺寸
	// lv_label_set_align(language_flag, LV_LABEL_ALIGN_RIGHT);

}

static void goto_system_layout(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		set_enter_setting_page_which(4);
		goto_layout(pLAYOUT(set_system));
	}
}



static void setting_page_5_system_info_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_anything_create(goto_system_layout);

	static lv_point_t line_point_right1[5];
	// char *src[language_total] = {"System", "系统", "Система", "Système", "Sistema", "System", "النظام", "Systém", "מערכת","Sistema","Sistema"};
	setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?701:501, 542, 99, str_get(LAYOUT_SETTING_LANG_SYSTEM_ID), &btn_data, line_point_right1);
}

static void security_passwd_btn_up(lv_obj_t* obj)
{
	enter_layout_passwd_ch(PASSWD_CH_MODIFY_PASSWD);
	set_enter_setting_page_which(4);
	goto_layout(pLAYOUT(setting_password));
}

static void setting_page_5_security_passwd_btn_create(lv_obj_t* parent){

	static btn_data btn_data = btn_data_up_create(security_passwd_btn_up);
	static lv_point_t line_point_right1[6];
	// char *src[language_total] = {"Security password","安全密码","Пароль бяспекі","Mot de passe sécurisé","Contraseña de Seguridad","Sicherheitspasswort","كلمة سر آمنة","Bezpečnostní heslo","סיסמא ביטחון","Senha de segurança","Password di sicurezza"};
	setting_page_btn_create(parent, 70,(user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?901:701, 542, 99,str_get(LAYOUT_SETTING_LANG_SECURITYPASSWORD_ID), &btn_data, line_point_right1);
	
}


static lv_task_t * matching_second_confirm_disp_task = NULL;
static void second_confirm_cancel_btn_up(lv_obj_t* obj)
{	
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 80);
	if(cont != NULL)
	{
		lv_obj_del(cont);
	}
	//通知门口机，取消匹配进程
	device_confirm_status_set(-1);
	network_cmd_data_init(data);
	data.device = DEVICE_GROUP_OUTDOOR;
	data.cmd = NET_COMMON_CMD_DEVICE_CONFIRM;
	data.arg1 = 0;
	data.arg2 = 0;
	network_send_cmd_data(&data);
	lv_task_del(matching_second_confirm_disp_task);

}
static void second_confirm_deviece_confirm_display(lv_task_t * task)
{
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 80);
	if(cont != NULL)
	{
		lv_obj_del(cont);
	}
	lv_task_del(task);
}
static void second_confirm_deviece_tips_display(lv_task_t * task)
{
	lv_obj_t * obj = task->user_data ;
    if(obj == NULL)
    {
        return ;
    }


			// char * src1[language_total] = {"Please wait a moment,looking for the unit doorway machine",
			// 							  "请稍等，正在寻找单元门口机",
			// 							  "Подождите минутку, ищете портал", 
			// 							  "Attendez, je cherche un portier",
			// 							  "Por favor, espere un momento, buscando la puerta de la unidad", 
			// 							  "Bitte warten Sie einen Moment\nWir suchen das Gerät Türöffnungsmaschine",
			// 							  "لحظة من فضلك ، أنا أبحث عن مدخل الوحدة ",
			// 							  "Počkejte chvíli. Hledáme stroj pro dveře jednotky",
			// 							  "אנא חכה רגע. אנחנו מחפשים את מכונת הדלת היחידה",
			// 							  "Por favor, Aguarde um Momento. A procurar Unidade Exterior",
			// 							  "Per favore aspetta un momento, sto cercando la porta dell'unità"};

// 			char * src2[language_total] = {"The unit doorway device has been found \n is waiting for device confirmation\n\
// 			Please press and hold the call button of the device for more than 5s \n\
// 			to pair the device", 
// 										  "已找到单元门口机设备,正在等待设备确认\n\
// 										  请长按设备的呼叫按键5秒以上进行设备配对",
// 										  "Найдено устройство дверного прожектора\n\
// 										  ждущее подтверждения устройства \n\
// 										  Нажмите кнопку вызова устройства\n\
// 										  чтобы настроить устройство не менее 5 секунд", 
// 										  "Un périphérique de porte d'Unit é a été trouvé et attend la confirmation du périphérique  \n \
// 										  please appuyez longtemps sur le bouton d'appel du périphérique pendant plus de 5S pour\n\
// 										  l'appariement du périphérique",
// 										  "Localice el dispositivo de la puerta de la unidad, esperando la confirmación del dispositivo \n\
// 										  Por favor, presione el botón de llamada del dispositivo durante más de 5 segundos \n\
// 										  para emparejar el dispositivo", 
// 										  "Das Gerät wurde gefunden, wartet auf Gerätebestätigung \n\
// 										  Bitte halten Sie die Anruftaste des Geräts länger als 5s gedrückt, um das Gerät zu koppeln ",
// 										  "تم العثور على الجهاز ، في انتظار تأكيد الجهاز\n\
// اضغط باستمرار على زر الاتصال من الجهاز لمدة 5 ثوان أو أكثر إلى مباراة الجهاز",
// 										  "Dveřní jednotka nalezena, čeká na potvrzení zařízení\n\
// 										  Pro spárování zařízení podržte tlačítko volání více než 5s",
// "מכשיר הדלת של היחידה נמצא, מחכה לאישור מכשיר \n\
// אנא לחץ על כפתור השיחה של המכשיר במשך יותר מ-5 שניות כדי לזוג את המכשיר",
// "A Unidade Exterior foi Encontrada e \n\
// Aguarda a Confirmação da Unidade Interior \n\
// Pressione Segurando o Botão de Falar por mais de 5 segundos para Emparelhar",
// "È stato trovato un dispositivo della porta dell'unità ed è in attesa di conferma dal dispositivo\n\
// si prega di premere a lungo il pulsante di chiamata sul dispositivo per più di 5 secondi\n\
// accoppiamento del dispositivo"};
			// char * src3[language_total] = {"Pairing completed, exiting!", 
			// 							  "配对完成,正在退出!",
			// 							  "спаривание завершено, выход!", 
			// 							  "Appariement terminé, sortie!",
			// 							  "Emparejamiento completo, saliendo!", 
			// 							  "Kopplung abgeschlossen, beendet!",
			// 							  "الانتهاء من الاقتران ، الخروج!",
			// 								"Párování dokončeno, ukončeno!",
			// 								"זוג מושלם, יוצא",
			// 								"Emparelhamento Concluído, A Sair!",
			// 								"Accoppiamento completato! Esci."};
	int status = device_confirm_status_get();										
	if(status == 0)
	{
		network_cmd_data_init(data);
		data.device = DEVICE_GROUP_OUTDOOR;
		data.cmd = NET_COMMON_CMD_DEVICE_CONFIRM;
		data.arg1 = 1;
		data.arg2  = user_data_get()->other.family_id;
		network_send_cmd_data(&data);
		lv_label_set_text(obj, str_get(LAYOUT_SETTING_LANG_LOOKINGFORUNITOUTDOOR_ID));
		lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
	}
	else if(status == 1)
	{
		lv_label_set_text(obj, str_get(LAYOUT_SETTING_LANG_UNITOUTDOORHASFOUND_ID));
		lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
	}
	else if(status == 2)
	{
		lv_task_del(matching_second_confirm_disp_task);
		matching_second_confirm_disp_task = NULL;
		lv_label_set_text(obj, str_get(LAYOUT_SETTING_LANG_PAIRCOMPLETE_ID));
		lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
		lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
		matching_second_confirm_disp_task = lv_layout_task_create(second_confirm_deviece_confirm_display, 1000, LV_TASK_PRIO_MID, NULL);	
		device_confirm_status_set(-1);

		//hlf:多户连接交换机时，其中一户发送的查找户外机命令被其他户接受，导致其他户不能call机。所以查找成功后会发送一次取消匹配进程
		device_confirm_status = -1;
		network_cmd_data_init(data);
		data.device = DEVICE_GROUP_OUTDOOR;
		data.cmd = NET_COMMON_CMD_DEVICE_CONFIRM;
		data.arg1 = 0;
		data.arg2 = 0;
		network_send_cmd_data(&data);

	} 
}
static void second_confirm_btn_create_btn_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
		{
			device_confirm_status_set(0);
			//通知门口机，开始匹配进程
			network_cmd_data_init(data);
			data.device = DEVICE_GROUP_OUTDOOR;
			data.cmd = NET_COMMON_CMD_DEVICE_CONFIRM;
			data.arg1 = 1;
			data.arg2  = user_data_get()->other.family_id;
			network_send_cmd_data(&data);
			lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
			lv_obj_set_id(cont_bg,80);
			lv_obj_set_size(cont_bg,1024,600);
			lv_obj_set_pos(cont_bg,0,0);
			lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
			lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
			
			lv_obj_t *cont = lv_cont_create(cont_bg, NULL);
			lv_obj_set_size(cont,750,360);
			lv_obj_set_pos(cont,137,120);
			lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
			lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
			lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
			lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
			lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
			lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
			lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
			lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);
			
			
			lv_obj_t* obj = lv_label_create(cont,NULL);
			lv_obj_set_id(obj,80);
			// char * src[language_total] = {"Please wait a moment,looking for the unit doorway machine",
			// 							  "请稍等，正在寻找单元门口机",
			// 							  "Подождите минутку, ищете портал", 
			// 							  "Attendez, je cherche un portier",
			// 							  "Por favor, espere un momento, buscando la puerta de la unidad", 
			// 							  "Bitte warten Sie einen Moment. Wir suchen das Gerät Türöffnungsmaschine",
			// 							  "لحظة من فضلك ، أنا أبحث عن مدخل الوحدة ",
			// 							  "Počkejte probíhá vyhledávání dveřní jednotky.",
			// 							  "בבקשה חכה רגע, מחפש את מכונת הדלת היחידה",
			// 							  "Por favor, Aguarde um Momento. A procurar Unidade Exterior",
			// 							   "Per favore aspetta un momento, sto cercando la porta dell'unità"};

			lv_label_set_text(obj, str_get(LAYOUT_SETTING_LANG_LOOKINGFORUNITOUTDOOR_ID));
			lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
			lv_obj_set_style_local_value_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
			lv_obj_align(obj, cont, LV_ALIGN_IN_TOP_MID, 0, 100);
			matching_second_confirm_disp_task = lv_layout_task_create(second_confirm_deviece_tips_display, 2000, LV_TASK_PRIO_MID, obj);	
			cont_bg->user_data = obj;
			
			
			lv_obj_t* cancle_btn = lv_btn_create(obj->parent, NULL);
			lv_obj_set_id(cancle_btn,1);
			lv_obj_set_style_local_bg_opa(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
			lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
			lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
			lv_obj_set_size(cancle_btn, 220, 70);
			lv_obj_align(cancle_btn, cancle_btn->parent, LV_ALIGN_IN_BOTTOM_MID, 0,-30);
			
			
			lv_obj_align(obj, obj->parent, LV_ALIGN_IN_TOP_MID, 0, 50);
			lv_obj_t* label = lv_label_create(cancle_btn,NULL);
			
			// char *src1[language_total] = {"Cancel", "取消", "Отмена", "Annuler", "Cancelar", "Absagen", "الغاء", "Zrušit", "ביטול","Cancelar","Annulla"};
			lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CANCEL_ID));
			lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
			lv_obj_align(label, cancle_btn, LV_ALIGN_CENTER, 0,0);
			
			
			static btn_data btn_data = btn_data_up_create(second_confirm_cancel_btn_up);
			cancle_btn->user_data = &btn_data;
			btn_touch_event_listen(cancle_btn);
			
		
		}
	

	}
}


static void setting_page_5_second_confirm_btn_create(lv_obj_t* parent){
	static btn_data btn_data = btn_data_anything_create(second_confirm_btn_create_btn_up);
	
	static lv_point_t line_point_right1[4];
	// char *src[language_total] = {"Find outdoor unit","查找户外机","машина для поиска на открытом воздухе","Trouver des unités extérieures","Encontrar una unidad exterior","Außeneinheit suchen","ابحث عن وحدة في الهواء الطلق","Vahledání dveřní jednotky","מצא יחידה בחוץ","Localizar Unidade Exterior","Trova unità esterna"};
	setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?1001:801, 542, 99, str_get(LAYOUT_SETTING_LANG_FINDOUTDOORUNIT_ID), &btn_data, line_point_right1);
}


static void background_change_btn_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED)
	{
		set_enter_setting_page_which(4);
		goto_layout(pLAYOUT(background));
	}

}
static void setting_page_5_background_change_btn_create(lv_obj_t* parent)
{
	static btn_data btn_data = btn_data_anything_create(background_change_btn_up);
	static lv_point_t line_point_right1[4];
	// char *src[language_total] = {"Wallpaper Settings","壁纸设置","Настройка обоев","Paramètres du papier peint","Configuración del Fondo de pantalla","Hintergrundbildereinstellungen","خلفيات","Nastavení tapety","הגדרות נייר הארנק","Configurações Imagem de Fundo","Impostazioni sfondo"};
	setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?1101:901, 542, 99, str_get(LAYOUT_SETTING_LANG_WALLPAPERSETTING_ID), &btn_data, line_point_right1);

}

static void cctv_btn_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED)
	{
		set_enter_setting_page_which(4);
		goto_layout(pLAYOUT(set_cctv));
	}

}
static void setting_page_5_cctv_btn_create(lv_obj_t* parent)
{
	static btn_data btn_data = btn_data_anything_create(cctv_btn_up);
	static lv_point_t line_point_right1[4];
	setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?1201:1001, 542, 99, "CCTV", &btn_data, line_point_right1);

}

static void nobody_message_btn_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED)
	{
		if (user_data_get()->nobody_message == false)
		{
			user_data_get()->nobody_message = true;
			lv_label_set_text(lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL), 110), str_get(LAYOUT_SETTING_LANG_ON_ID));
		}
		else
		{
			user_data_get()->nobody_message = false;
			lv_label_set_text(lv_obj_get_child_form_id(lv_obj_get_child(setting_page[4], NULL), 110), str_get(LAYOUT_SETTING_LANG_OFF_ID));
		}

		user_data_save();
	}
}

//无人在家语音播报开关
static void setting_page_5_nobody_at_home_btn_create(lv_obj_t* parent)
{
	static btn_data btn_data = btn_data_anything_create(nobody_message_btn_up);
	static lv_point_t line_point_right1[4];
	lv_obj_t *btn = setting_page_btn_create(parent, 70, (user_data_get()->other.network_device == DEVICE_INDOOR_ID1)?1301:1101, 542, 99, str_get(LAYOUT_NOBODY_AT_HOME_ID), &btn_data, NULL);

	lv_obj_t *motion_flag = lv_label_create(parent, NULL);
	lv_obj_set_id(motion_flag, 110);
	lv_obj_set_style_local_text_font(motion_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));	
	lv_obj_set_style_local_text_color(motion_flag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

	if(user_data_get()->nobody_message == false)
	{
		// char *src1[language_total] ={"OFF", "关", "близко", "Fermer", "Cerrar", "Nah dran", " ايقاف", "Vyp.", "סגור","Desligado","Desligado"};
		lv_label_set_text(motion_flag, str_get(LAYOUT_SETTING_LANG_OFF_ID));
	}
	else
	{
		// char *src1[language_total] = {"ON", "开", "Oткрыто", "Ouvert", "Abierto", "Öffnen", "تشغيل", "Zap.", "פועל","Ligado","Ligado"};
		lv_label_set_text(motion_flag, str_get(LAYOUT_SETTING_LANG_ON_ID));
	}
	lv_obj_align(motion_flag, btn, LV_ALIGN_IN_RIGHT_MID, -18, 0);
}

static void setting_other_page_create(void)
{

	lv_cont_set_fit4(lv_obj_get_child(setting_page[4], NULL) , LV_FIT_NONE, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_TIGHT);
	
	setting_page_5_wifi_btn_create(setting_page[4]);
	
	setting_page_5_language_btn_create(setting_page[4]);

	setting_page_5_record_btn_create(setting_page[4]);

	setting_page_5_move_btn_create(setting_page[4]);

	if(user_data_get()->other.network_device == DEVICE_INDOOR_ID1)
	{
		setting_page_5_door_delay_btn_create(setting_page[4]);
	}
	
	setting_page_5_upgrade_btn_create(setting_page[4]);

	setting_page_5_system_info_btn_create(setting_page[4]);
	
	setting_page_5_reset_btn_create(setting_page[4]);

	setting_page_5_security_passwd_btn_create(setting_page[4]);

	setting_page_5_second_confirm_btn_create(setting_page[4]);

	setting_page_5_background_change_btn_create(setting_page[4]);

	setting_page_5_cctv_btn_create(setting_page[4]);

	setting_page_5_nobody_at_home_btn_create(setting_page[4]);
}

static lv_task_t * set_scan_wifi_ptask = NULL;


static void scan_wifi_task(struct _lv_task_t *task_t){

	wpa_cli_scan_wifi(&user_data_get()->wifi.wifi_open_flag);
	wpa_cli_wlan_status(&user_data_get()->wifi.wifi_open_flag);
	
}


static void scan_wifi(void){
	if(user_data_get()->wifi.wifi_open_flag == false)
		return ;
	
	if (set_scan_wifi_ptask != NULL) {
			lv_task_del(set_scan_wifi_ptask);
	}
	
	set_scan_wifi_ptask = lv_task_create(scan_wifi_task, 2000, LV_TASK_PRIO_MID, NULL);
	lv_task_ready(set_scan_wifi_ptask);
	
	scan_wifi_task(set_scan_wifi_ptask);

}




static void LAYOUT_ENETER_FUNC(setting)
{
	setting_time_edit = false;
	setting_rtc_time_get(&setting_tm);//获取时间

	//rtc_time_sync();
	
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	setting_cancel_btn_create();//取消按钮


	setting_head_label_create();//头


	setting_tableview_create(); //scr_act_id :0  选项页


	setting_tab_img_line_create();//图片



	setting_ring_page_create();//铃声界面
	
	setting_room_page_create();


	/*进入界面比较耗时，先将其他加载后再 加载此项 */
	setting_time_page_create();

	setting_screensaver_page_create();

	setting_other_page_create();


	//开线程 创建搜索wifi的任务
	scan_wifi();
	
}
static void LAYOUT_QUIT_FUNC(setting)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	if(device_confirm_status !=-1)
	{
		//通知门口机，取消匹配进程
		device_confirm_status = -1;
		network_cmd_data_init(data);
		data.device = DEVICE_GROUP_OUTDOOR;
		data.cmd = NET_COMMON_CMD_DEVICE_CONFIRM;
		data.arg1 = 0;
		data.arg2 = 0;
		network_send_cmd_data(&data);
	}
	if(setting_time_edit == true)
	{
		setting_time_edit = false;
		//setting_rtc_time_set(&setting_tm);
		standby_timer_close();

		setting_rtc_time_set(&setting_tm);
		time_t seconds = time(NULL);
		struct tm tm = {0};
		localtime_r(&seconds, &tm);

		system("hwclock -s");
		rtc_time_sync();
		standby_timer_open(-1,NULL);
	}

	if (set_scan_wifi_ptask != NULL) //计时任务未退出
	{
		lv_task_del(set_scan_wifi_ptask);			//退出计时任务
		set_scan_wifi_ptask	= NULL; 				//置空指针
	}
}

CREATE_LAYOUT(setting);

