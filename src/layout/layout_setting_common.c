
#include "layout_define.h"
#include "layout_setting_common.h"

int setting_password_edit_index_get(lv_obj_t *parent)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}
	int edit_index = *((int *)label->user_data);
	return edit_index;
}


bool setting_password_input_reset(lv_obj_t *parent, int edit_max)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}
	(*((int *)label->user_data)) = 0;

	for (int i = 0; i < edit_max; i++)
	{
		lv_obj_t *obj = lv_obj_get_child_form_id(parent, 10 + i);
		if (obj != NULL)
		{
			lv_obj_set_hidden(obj, true);
		}
		obj = lv_obj_get_child_form_id(parent, 20 + i);
		if (obj != NULL)
		{
			lv_obj_set_hidden(obj, true);
		}

		obj = lv_obj_get_child_form_id(parent, 30 + i);
		if (obj != NULL)
		{
			lv_obj_set_state(obj, LV_STATE_DEFAULT);
		}
	}
	return true;
}

bool setting_password_input_label_create(lv_obj_t *parent, int x, int y, int row)
{
	static int edit_index = 0;
	edit_index = 0;
	/***** 行号减1 *****/
	if (row < 1)
	{
		return false;
	}

	for (int i = 0; i < row; i++)
	{
		lv_obj_t *label = lv_label_create(parent == NULL ? lv_scr_act() : parent, NULL);
		lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
		lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
		lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_FONT_SIZE_31);
		lv_label_set_text(label, "");
		lv_obj_set_size(label, 60, 60);
		label->user_data = &edit_index;

		lv_obj_set_pos(label, x, y + 14 + i * 170);
		lv_obj_set_id(label, 10 + i * 4);

		label = lv_label_create(parent == NULL ? lv_scr_act() : parent, label);
		lv_obj_set_x(label, x + (60 + 24) * 1);
		lv_obj_set_id(label, 11 + i * 4);

		label = lv_label_create(parent == NULL ? lv_scr_act() : parent, label);
		lv_obj_set_x(label, x + (60 + 24) * 2);
		lv_obj_set_id(label, 12 + i * 4);

		label = lv_label_create(parent == NULL ? lv_scr_act() : parent, label);
		lv_obj_set_x(label, x + (60 + 24) * 3);
		lv_obj_set_id(label, 13 + i * 4);

		lv_obj_t *rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
		rot->user_data = &edit_index;
		lv_obj_set_size(rot, 16, 16);
		lv_obj_set_hidden(rot, true);

		lv_obj_set_style_local_bg_opa(rot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(rot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_radius(rot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 60);

		lv_obj_set_pos(rot, x + 22, y + 29 + i * 170);
		lv_obj_set_id(rot, 20 + i * 4);

		rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, rot);
		lv_obj_set_x(rot, x + 22 + (60 + 24) * 1);
		lv_obj_set_id(rot, 21 + i * 4);

		rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, rot);
		lv_obj_set_x(rot, x + 22 + (60 + 24) * 2);
		lv_obj_set_id(rot, 22 + i * 4);

		rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, rot);
		lv_obj_set_x(rot, x + 22 + (60 + 24) * 3);
		lv_obj_set_id(rot, 23 + i * 4);

		lv_obj_t *line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
		line->user_data = &edit_index;
		lv_obj_set_size(line, 60, 4);
		lv_obj_set_style_local_bg_opa(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0x0096ff));  
		lv_obj_set_style_local_radius(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 60);
		lv_obj_set_id(line, 30 + i * 4);
		lv_obj_set_pos(line, x, y + 74 + i * 170);

		line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, line);
		lv_obj_set_x(line, x + (60 + 24) * 1);
		lv_obj_set_id(line, 31 + i * 4);

		line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, line);
		lv_obj_set_x(line, x + (60 + 24) * 2);
		lv_obj_set_id(line, 32 + i * 4);

		line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, line);
		lv_obj_set_x(line, x + (60 + 24) * 3);
		lv_obj_set_id(line, 33 + i * 4);
	}
	return true;
}

/*

*/
lv_obj_t *setting_passowrd_num_keyboard_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *click_data)
{
	lv_obj_t *keyboard = lv_keyboard_create(parent == NULL ? lv_scr_act() : parent, NULL);
	if (keyboard == NULL)
	{
		printf("create keyboard failed \n");
		return NULL;
	}
	lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_PASSWD);
	lv_obj_set_pos(keyboard, x, y);
	lv_obj_set_size(keyboard, w, h);

	lv_obj_set_style_local_text_font(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE(36));

	lv_obj_set_style_local_border_opa(keyboard, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(keyboard, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, LV_OPA_10);
	lv_obj_set_style_local_bg_color(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_color(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0x0096ff));

	lv_obj_set_style_local_radius(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, 60);

	lv_obj_set_style_local_pad_inner(keyboard, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 15);

	static rom_bin_info img = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_btnmatrix_set_pattern_image(keyboard, 11, &img);

    keyboard->user_data = click_data;
	btn_touch_event_listen(keyboard);

	return keyboard;
}




/***
** 日期: 2022-05-09 11:22
** 作者: leo.liu
** 函数作用：获取输入字符串
** 返回参数说明：
***/
bool setting_password_get_string(lv_obj_t *parent, char *buffer)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}
	int edit_index = *((int *)label->user_data);
	if (edit_index == 0)
	{
		return false;
	}

	for (int i = 0; i < edit_index; i++)
	{
		label = lv_obj_get_child_form_id(parent, 10 + i);
		if (label == NULL)
		{
			printf("find obj failed %d\n", i);
		}
		strcat(buffer, lv_label_get_text(label));
	}
	return true;
}

static void setting_password_line_set_cheked(lv_obj_t *parent, int cur_id, int max_edit)
{
	for (int i = 0; i < max_edit; i++)
	{
		int id = 30 + i;
		lv_obj_t *obj = lv_obj_get_child_form_id(parent, id);
		lv_obj_set_state(obj, id == cur_id ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
	}
}

/***
** 日期: 2022-05-09 10:57
** 作者: leo.liu
** 函数作用：删除一个字符
** 返回参数说明：
***/
bool setting_password_del_string(lv_obj_t *parent, int max_edit)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}

	int edit_index = *((int *)label->user_data);
	if (edit_index == 0)
	{
		return false;
	}
	edit_index -= 1;
	printf("edit_index is %d\n",edit_index);
	int id = 10 + edit_index;
	lv_obj_t *obj = lv_obj_get_child_form_id(parent, id);
	if (obj == NULL)
	{
		printf("dind id:%d failed \n", id);
		return false;
	}
	lv_obj_set_hidden(obj, true);

	id = 20 + edit_index;
	obj = lv_obj_get_child_form_id(parent, id);
	if (obj == NULL)
	{
		return false;
	}
	lv_obj_set_hidden(obj, true);

	setting_password_line_set_cheked(parent, 30 + edit_index - 1, max_edit);

	(*((int *)label->user_data))--;
	return true;
}

static void setting_password_input_string_task(lv_task_t *task_t)
{
	lv_obj_t *edit_label = (lv_obj_t *)(task_t->user_data);
	if (edit_label == NULL)
	{
		return;
	}

	int edit_index = *((int *)edit_label->user_data);
	int cur_index = (edit_label->obj_id) % 10;
	if (cur_index >= edit_index)
	{
		return;
	}

	lv_obj_set_hidden(edit_label, true);

	int id = 20 + cur_index;
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_obj_get_parent(edit_label), id);
	if (obj == NULL)
	{
		return;
	}
	lv_obj_set_hidden(obj, false);

	lv_task_del(task_t);
}

bool setting_password_input_string(lv_obj_t *parent, const char *string, int max_edit, bool passwd_mode)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}

	int edit_index = *((int *)label->user_data);
	if (edit_index >= max_edit)
	{
		return false;
	}
	int id = 10 + edit_index;
	lv_obj_t *edit_label = lv_obj_get_child_form_id(parent, id);
	if (edit_label == NULL)
	{
		printf("dind id:%d failed \n", id);
		return false;
	}

	setting_password_line_set_cheked(parent, 30 + edit_index, max_edit);

	lv_obj_set_hidden(edit_label, false);
	lv_label_set_text(edit_label, string);
	(*((int *)label->user_data))++;
		printf("---------------->>>>>>>>>>>%d \n",(*((int *)label->user_data)));
	if(passwd_mode == true)
	{
		lv_layout_task_create(setting_password_input_string_task, 500, LV_TASK_PRIO_MID, edit_label);
	}
	return true;
}



lv_obj_t * dialog_msg_cont_creat()
{
	lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont_bg,1024,600);
	lv_obj_set_pos(cont_bg,0,0);
	lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
	
	lv_obj_t *cont = lv_page_create(cont_bg, NULL);
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
	return cont;
}

static void* network_init_restart_task(void* arg)
{
	network_init_restart(user_data_get()->other.network_device);
	return NULL;
}

/************************************************************
* @Description: 网络重置
* @Author: xiaoxiao
* @Date: 2022-12-12 18:15:14
* @param: 
* @explain: 
************************************************************/
lv_obj_t * network_init_reset_func(void)
{
	//网络重置
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_init_restart_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	
	lv_obj_t * cont = dialog_msg_cont_creat();

	// char *src[language_total] = {"Network device resetting","网络设备重置中","сброс сетевого устройства","Réinitialisation du périphérique réseau","Se está restableciendo el dispositivo de red","Netzwerkgerät zurücksetzen","إعادة تعيين جهاز الشبكة","Resetování síťového zařízení","מתקן רשת מתקן מחדש","Redefinição do dispositivo de rede","Ripristino del dispositivo di rete"};
	lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SETCOMMON_LANG_NETWORKRESET_ID));
	lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -50);
	lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	lv_obj_t *preload = lv_spinner_create(cont, NULL);
	lv_obj_set_size(preload, 80, 80);

	lv_obj_align(preload, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -30);

	lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0xB8, 0x8D, 0x56));
	lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
	lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, 8);
	lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, 8);

	return cont;
}