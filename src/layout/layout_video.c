#include "layout_define.h"
#include "ak_common_graphics.h"
extern int *playback_pview_select_get(void);
extern media_type playback_pview_type_get(void);
extern const char *playback_pview_path_get(void);
extern int *playback_pview_total_get(void);
extern int *playback_pview_item_get(void);
extern void playback_sd_status_change_callback(unsigned long arg1, unsigned long arg2);

extern void layout_playback_quit_mask_set(unsigned int mask);
extern unsigned int layout_playback_quit_mask_get();

static lv_task_t *layout_video_task = NULL;

static void media_video_info_label_display(void);
static void media_video_progress_bar_display(int cur, int total);

static void media_video_lock_img_display(bool en);

//媒体视频展示
static bool media_video_display(void)
{
	gui_raw_clear();
	#ifndef _PLATFORM_800_1280
	system_bg_fill_color_2(0xff, 0, 0, 1024, 600); //填充颜色   hlf:system_bg_fill_color -> system_bg_fill_color_2
	#else
	system_bg_fill_color(0xff, 0, 0, 1024, 600);
	#endif

	int *select = playback_pview_select_get();							   //获得播放的视频结点
	media_info *info = media_info_get(playback_pview_type_get(), *select); //获取指定播放媒体文件信息
	char file_path[64] = {0};

	strcpy(file_path, playback_pview_path_get());
	strcat(file_path, info->file_name);

	// if(media_thumb_load(0,0,1024,600,file_path) == false)
	// {
	// 	media_file_delete(info->type, *select);
	// 	playback_thumb_parameter_init();
	// 	goto_layout(pLAYOUT(playback));
	// 	return false;
	// }

	media_file_new_clear(info->type, *select);

	media_video_info_label_display();

	//	media_video_lock_img_display(info->is_lock);
	media_video_lock_img_display(false);

	video_play_open(file_path);
	fb_video_mode_enable(true);

	// extern void fb_playback_video_mode_enable(bool en);
	// fb_playback_video_mode_enable(true);

	return true;
}

static void media_video_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *children = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(children, state);
}

static void media_video_btn_img_transform_set(lv_obj_t *obj)
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

static lv_obj_t *media_video_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color)
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

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img, img_src);

	media_video_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);

	return btn;
}

static void media_video_cancel_btn_down(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_PRESSED);
}

static void media_video_cancel_cancel_btn_up(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_DEFAULT);

	int *select_index = playback_pview_select_get();
	int *media_total = playback_pview_total_get();
	int *item_index = playback_pview_item_get();
	int count = (*media_total) - 1;
	(*item_index) = count - (count - (*select_index)) / 6 * 6;
	layout_playback_quit_mask_set(0x01);
	goto_layout(pLAYOUT(playback));
}
static void media_video_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(media_video_cancel_btn_down, media_video_cancel_cancel_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	media_video_btn_create(25, 25, 60, 60, &btn_data, &info, true);
}

static void media_video_delete_btn_down(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_PRESSED);
}

bool video_delete = false;
static void media_video_msgbox_btn_up(lv_obj_t *obj)
{
	unsigned int btn_id = lv_msgbox_get_active_btn(obj);
	if (btn_id == 1)
	{
		video_play_stop();
		int *select_index = playback_pview_select_get();
		const media_type type = playback_pview_type_get();
		media_file_delete(type, *select_index);
		int total = media_file_total_get(type, false);
		if (total < 1)
		{
			goto_layout(pLAYOUT(home));
		}
		else
		{
			int *ptotal = playback_pview_total_get();
			*ptotal = total;
			if ((*select_index) > (total - 1))
			{
				*select_index = total - 1;
			}
			media_info *info = media_info_get(type, *select_index);
			if (info->type == FILE_TYPE_SD_MIXED_VIDEO)
			{
				media_video_display();
				lv_obj_del(lv_obj_get_parent(obj));
				lv_obj_invalidate(lv_scr_act());
			}
			else
			{
				layout_playback_quit_mask_set(0x01);
				goto_layout(pLAYOUT(photo));
			}
		}
	}
	else if (btn_id == 0)
	{
		lv_obj_del(lv_obj_get_parent(obj));
	}
}

static void media_video_delete_msgbox_create(void)
{
	lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(obj, 1024, 600);
	lv_obj_t *msg_box = lv_msgbox_create(obj, NULL);
	lv_obj_set_id(msg_box, 200);
	lv_obj_set_style_local_bg_color(msg_box, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
	lv_obj_set_style_local_bg_opa(msg_box, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	// static const char * btns[language_total][3] ={{"Cancle", "Confirm", ""},{"取消","确认",""},{"Отмена","Подтверждать",""},{"Annuler","Confirmer",""},{"Cancelar","Confirmar",""},{"Absagen","Bestätigen Sie",""},{"الغاء","تاكيد",""},{"Zrušit","Potvrdit",""}, {"ביטול", "אישור",""},{"Cancelar","Confirmar"},{"Annulla","Conferma",""}};
	// char *src[language_total] = {"\n\nDelete ?\n\n","\n\n删除 ?\n\n","\n\nУдалить ?\n\n","\n\nSupprimez ?\n\n","\n\nEliminar ?\n\n","\n\nLöschen ?\n\n","\n\n? حذف\n\n","\n\nVymazat ?\n\n", "\n\nמחיקה ?\n\n","\n\nApagar?\n\n","\n\nElimina ?\n\n"};
	lv_msgbox_set_text(msg_box, str_get(LAYOUT_VIDEO_LANG_DELETE_ID));

	lv_msgbox_add_btns(msg_box, btns_str_get());

	lv_obj_set_size(msg_box, 470, 300);

	static btn_data btn_data = btn_data_up_create(media_video_msgbox_btn_up);
	msg_box->user_data = &btn_data;
	btn_touch_event_listen(msg_box);
	lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/

	lv_obj_t *btnmatri_btn = lv_msgbox_get_btnmatrix(msg_box);

	lv_obj_set_style_local_bg_color(btnmatri_btn, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_COLOR_MAKE(0xFF, 0, 0));
	lv_obj_set_style_local_radius(btnmatri_btn, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 45);
}

static void media_video_delete_btn_up(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_DEFAULT);

	lv_obj_t *msgbox = lv_obj_get_child_form_id(lv_scr_act(), 200);
	if (msgbox != NULL)
	{
		lv_obj_del(msgbox);
	}
	else
	{

		if (video_play_get_status() == 1)
		{
			video_play_pause();
		}

		media_video_delete_msgbox_create();
	}
}

static void media_video_delete_btn_create(void)
{
	static btn_data btn_data = btn_data_create(media_video_delete_btn_down, media_video_delete_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_DELETE_PNG);
	media_video_btn_create(917, 25, 60, 60, &btn_data, &info, true);
}

static void media_video_left_btn_down(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_PRESSED);
}

static void media_video_left_btn_up(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_DEFAULT);

	video_play_stop();

	int *select_index = playback_pview_select_get();
	int *media_total = playback_pview_total_get();
	(*select_index) += 1;
	if ((*select_index) == (*media_total))
	{
		(*select_index) = 0;
	}
	media_info *info = media_info_get(playback_pview_type_get(), *select_index);
	layout_playback_quit_mask_set(0x01);
	if (info->type == FILE_TYPE_SD_MIXED_VIDEO)
	{
		
		//hlf
		#ifdef _PLATFORM_800_1280
			goto_layout(pLAYOUT(video));
		#else
			media_video_info_label_display();
			media_video_display();
		#endif
	}
	else
	{
		goto_layout(pLAYOUT(photo));
	}
}

static void media_video_left_btn_create(void)
{
	int *total = playback_pview_total_get();
	if (*total < 2)
	{
		return;
	}

	static btn_data btn_data = btn_data_create(media_video_left_btn_down, media_video_left_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_LEFT_PNG);
	media_video_btn_create(0, 260, 80, 80, &btn_data, &info, false);
}

static void media_video_right_btn_down(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_PRESSED);
}

static void media_video_right_btn_up(lv_obj_t *obj)
{
	media_video_btn_state_set(obj, LV_STATE_DEFAULT);

	video_play_stop();
	int *select_index = playback_pview_select_get();
	int *media_total = playback_pview_total_get();
	(*select_index) -= 1;
	if ((*select_index) < 0)
	{
		(*select_index) = (*media_total) - 1;
	}
	media_info *info = media_info_get(playback_pview_type_get(), *select_index);
	layout_playback_quit_mask_set(0x01);
	if (info->type == FILE_TYPE_SD_MIXED_VIDEO)
	{
		//hlf
		#ifdef _PLATFORM_800_1280
			goto_layout(pLAYOUT(video));
		#else
			media_video_info_label_display();
			media_video_display();
		#endif
	}
	else
	{
		goto_layout(pLAYOUT(photo));
	}
}

static void media_video_right_btn_create(void)
{
	int *total = playback_pview_total_get();
	if (*total < 2)
	{
		return;
	}

	static btn_data btn_data = btn_data_create(media_video_right_btn_down, media_video_right_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_RIGHT_PNG);
	media_video_btn_create(944, 260, 80, 80, &btn_data, &info, false);
}

static void media_video_info_label_display(void)
{
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), 0);
	if (parent != NULL)
	{
		int *select = playback_pview_select_get();
		media_info *info = media_info_get(playback_pview_type_get(), *select);

		lv_obj_t *label_channel = lv_obj_get_child_form_id(parent, 0);
		// char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür","باب","Dveře","שער","Porta","Porta"};
		if (label_channel != NULL)
		{
			char str_num[1] = {0};
			if (info->ch <= 2)
			{
				sprintf(str_num, "%d", info->ch);
			}
			else
			{
				sprintf(str_num, "%d", info->ch - 2);
			}
			if (info->ch > 16)
			{
				sprintf(str_num, "%d", info->ch - 18);
				char *door_str = (char *)malloc(strlen(str_num) + 8);
				strcpy(door_str, "CCTV");
				strcat(door_str, str_num);
				lv_label_set_text(label_channel, door_str);
				free(door_str);
			}
			else
			{
				char *door_str = (char *)malloc(strlen(str_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
				strcpy(door_str, str_get(LAYOUT_HOME_LANG_DOOR_ID));
				strcat(door_str, str_num);
				lv_label_set_text(label_channel, door_str);
				free(door_str);
			}
		}
		// char *door_str = (char *) malloc(strlen(str_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
		// strcpy(door_str, str_get(LAYOUT_HOME_LANG_DOOR_ID));
		// strcat(door_str, str_num);
		// if(label_channel != NULL)
		// {
		// 	lv_label_set_text(label_channel, door_str);
		// }
		// free(door_str);
		lv_obj_t *label_time = lv_obj_get_child_form_id(parent, 1);
		if (label_time != NULL)
		{
			char str[128] = {"0"};
			strncpy(&str[0], &info->file_name[0], 4);
			str[4] = '-';
			strncat(&str[5], &info->file_name[4], 2);
			str[7] = '-';
			strncat(&str[8], &info->file_name[6], 2);
			str[10] = ' ';
			str[11] = ' ';

			strncat(&str[12], &info->file_name[9], 2);
			str[14] = ':';
			strncat(&str[15], &info->file_name[11], 2);
			str[17] = ':';
			strncat(&str[18], &info->file_name[13], 2);
			lv_label_set_text(label_time, str);
		}

		lv_obj_align(label_channel, parent, LV_ALIGN_IN_TOP_LEFT, 0, 0);
		lv_obj_align(label_time, parent, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	}
}

//创建视频标题
static void media_video_info_label_create(void)
{
	lv_obj_t *obj = lv_cont_create(lv_scr_act(), NULL); //包含通道和时间的容器
	lv_obj_set_id(obj, 0);
	lv_obj_set_pos(obj, 38, 500);
	lv_obj_set_size(obj, 400, 70);

	lv_obj_t *label_channel = lv_label_create(obj, NULL);
	lv_obj_set_id(label_channel, 0);
	lv_obj_t *label_time = lv_label_create(obj, NULL);
	lv_obj_set_id(label_time, 1);
	//	media_video_info_label_display();
}

static void media_video_lock_img_display(bool en)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), 2);
	if (obj != NULL)
	{
		lv_obj_set_hidden(obj, en ? false : true);
	}
}

//视频上锁的图片创建
static void media_video_lock_img_create(void)
{
	lv_obj_t *obj = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 2);
	lv_obj_set_pos(obj, 499, 30);
	lv_obj_set_size(obj, 27, 32);

	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_LOCK_PNG);
	lv_img_set_src(obj, &info);
	if (obj != NULL)
		lv_obj_set_hidden(obj, true);
}

//按钮显示
static void media_video_play_btn_display(void)
{
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), 1);
	if (parent != NULL)
	{
		lv_obj_t *obj = lv_obj_get_child_form_id(parent, 0);
		if (video_play_get_status() == 1) //已经播放 暂停
		{

			lv_obj_set_hidden(parent, true);

			rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAUSE_PNG);
			lv_img_set_src(obj, &info);
			lv_obj_align(obj, parent, LV_ALIGN_CENTER, 0, 0);
		}
		else
		{ //是暂停 转为播放
			lv_obj_set_hidden(parent, false);
			rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PLAY_PNG);
			lv_img_set_src(obj, &info);
			lv_obj_align(obj, parent, LV_ALIGN_CENTER, 10, 0);
		}
	}
}

//播放按钮的回调
static void media_video_play_btn_up(lv_obj_t *obj)
{

	if (video_play_get_status() == 0)
	{

		int *select = playback_pview_select_get();
		media_info *info = media_info_get(playback_pview_type_get(), *select);
		char file_path[64] = {0};

		strcpy(file_path, playback_pview_path_get());
		strcat(file_path, info->file_name);

		video_play_open(file_path);
		fb_video_mode_enable(true);
		// extern void fb_playback_video_mode_enable(bool en);
		// fb_playback_video_mode_enable(true);
	}
	else
	{
		video_play_pause();
	}
}

//创建播放暂停按钮
static void media_video_play_btn_create(void)
{
	lv_obj_t *obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 1);
	lv_obj_set_pos(obj, 437, 225);
	lv_obj_set_size(obj, 150, 150);

	lv_obj_set_style_local_bg_color(obj, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0, 0));
	lv_obj_set_style_local_radius(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 90);
	lv_obj_set_style_local_bg_opa(obj, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);

	lv_obj_t *img = lv_img_create(obj, NULL);
	lv_obj_set_id(img, 0);
	media_video_play_btn_display();

	static btn_data btn_data = btn_data_up_create(media_video_play_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);

	lv_obj_set_click(lv_scr_act(), true);
	lv_scr_act()->user_data = &btn_data;
	btn_touch_event_listen(lv_scr_act());
}

//播放任务
static void layout_video_timer_task(lv_task_t *task_t)
{
	char *play_state = (char *)task_t->user_data;
	char cur_state = video_play_get_status();

	if ((*play_state) != cur_state)
	{
		*play_state = cur_state;
		media_video_play_btn_display();

		int cur, total;
		video_play_duration_get(&cur, &total);
		media_video_progress_bar_display(cur, total);

		if (cur_state == 1)
			standby_timer_close();
		else
			standby_timer_open(-1, NULL);
	}

	if (cur_state == 1)
	{
		int cur, total;
		video_play_duration_get(&cur, &total);
		media_video_progress_bar_display(cur, total);
	}
	else if (cur_state == 0)
	{
		lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), 10086);
		lv_label_set_text(obj1, "");
	}
}

//进度条显示
static void media_video_progress_bar_display(int cur, int total)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), 3);
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), 10086);

	if (obj != NULL)
	{
		lv_bar_set_range(obj, 0, total / 10);
		lv_bar_set_anim_time(obj, 30);
		lv_bar_set_value(obj, cur / 10, LV_ANIM_OFF);

		if (obj1 != NULL)
		{
			int a = total - cur;
			lv_label_set_text_fmt(obj1, "%d", a == 0 ? 0 : (a + 1000) / 1000);
			lv_obj_align(obj1, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -50, -30);
		}
	}
}

//进度条创建
static void media_video_progress_bar_create(void)
{
	lv_obj_t *obj = lv_bar_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 3);

	lv_obj_set_pos(obj, 32, 580);
	lv_obj_set_size(obj, 960, 8);
	lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(0xC4, 0xC4, 0xC4));
	lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0x43, 0x72, 0xEB));

	lv_obj_t *time = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_id(time, 10086);

	lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_label_set_text(time, "");
	lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -50, -30);
}

static void video_sd_status_change_callback(unsigned long arg1, unsigned long arg2);


static void LAYOUT_ENETER_FUNC(video)
{
#ifdef _PLATFORM_800_1280
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
#else
	//在刷出第一诊后，才在视频现实函数设置显示器和屏幕透明
	// lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);//hlf:视频页有黑块
	// lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);//hlf:回放页ui会叠加到此页,控件创建完成需要刷新
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
#endif

	sdcard_event_register(video_sd_status_change_callback);
	lv_area_t area[] =
		{
			{25, 25, 25 + 60, 25 + 60},
			{917, 25, 917 + 60, 25 + 60},
			{0, 260, 0 + 80, 260 + 80},
			{944, 260, 944 + 80, 260 + 80},
			{38, 500, 38 + 400, 500 + 70},
			{260, 140, 260 + 470 + 30, 140 + 290 + 30},
			{499, 30, 499 + 27, 30 + 32},
			{32, 580, 32 + 960, 580 + 18},
			{940, 535, 940 + 50, 535 + 40}};

	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	media_video_info_label_create(); //标题

	media_video_lock_img_create(); //上锁

	if (media_video_display() == false)
	{
		return;
	}
	//视频显示

	media_video_cancel_btn_create(); //取消按钮

	media_video_delete_btn_create(); //删除按钮

	media_video_left_btn_create(); //左翻

	media_video_right_btn_create(); //右翻

	media_video_play_btn_create(); //播放

	media_video_progress_bar_create(); //进度条

	static char play_state = 0;
	play_state = 0;
	layout_video_task = lv_task_create(layout_video_timer_task, 10, LV_TASK_PRIO_MID, &play_state); //播放任务

	layout_obj_touch_event_register(NULL);

	//hlf
	#ifndef _PLATFORM_800_1280
		// system_bg_fill_color_2(0x00, 0, 0, 1024, 600);
		screen_force_refresh();
	#endif

}

static void LAYOUT_QUIT_FUNC(video)
{

	fb_video_mode_enable(false);
	video_play_stop();
	usleep(100 * 1000);

	if (!layout_playback_quit_mask_get())
	{
		ak_sleep_ms(100);
		media_thumb_device_close(); //关闭
	}
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);

	sdcard_event_register(NULL);

	// extern void fb_playback_video_mode_enable(bool en);
	// fb_playback_video_mode_enable(false);

	lv_obj_set_click(lv_scr_act(), false);
	lv_scr_act()->user_data = NULL;

	if (layout_video_task != NULL)
	{
		lv_task_del(layout_video_task);
		layout_video_task = NULL;
	}
	standby_timer_open(-1, NULL);
	layout_obj_touch_event_register(touch_sound_play);

}

CREATE_LAYOUT(video);

/*
 * arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
 * arg2:arg1= 1,arg2参数无意义
 *	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
 *	   arg1=3,arg2=1:内存正常，2，内存已满
 */
static void video_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
	if (arg1 == 1)
	{
		goto_layout(pLAYOUT(playback));
	}
}
