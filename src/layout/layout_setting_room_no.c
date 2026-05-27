#include "layout_define.h"
#include "layout_setting_common.h"
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT 0X01
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT 0X02
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_NUM 0X03

#define SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT 0X04

#define SETTING_PASSWORD_OBJ_ID_MSG_CONT 0X05
#define SETTING_PASSWORD_OBJ_ID_APPLE_BTN 0X06

extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

#define LAYOUT_ROOM_NO_SETING_BG_ADJ_ID 0X07

static void reset_network_init_check_task(lv_task_t * task)
{
	if(network_inited_status_get() == true)
	{
		lv_task_del(task);
		lv_obj_del(lv_obj_get_parent((lv_obj_t *)(task->user_data)));
		goto_layout(pLAYOUT(setting));
	}

}


//返回设置页面按钮
static void setting_password_cancel_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void setting_password_cancel_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(setting));
	
}

static lv_obj_t* setting_password_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(setting_password_cancel_btn_down, setting_password_cancel_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	printf("%x",ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}
/***
** 日期: 2022-05-09 11:55
** 作者: leo.liu
** 函数作用：输入键盘事件
** 返回参数说明：
***/
static void setting_password_keyboard_enter_password_btn_up(lv_obj_t *obj, lv_event_t ev)
{
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);
	if (parent == NULL)
	{
		return;
	}

	// if (ev != LV_EVENT_VALUE_CHANGED)
	// {
	// 	return;
	// }
	if (ev != LV_EVENT_RELEASED)
	{
		return;
	}
	const char *txt = lv_btnmatrix_get_active_btn_text(obj);
	if (txt[0] == ' ')
	{
		setting_password_del_string(parent, 4);
	}
	else if (txt[0] == '#')
	{
		if (setting_password_edit_index_get(parent) == 4)
		{
			char buffer[5] = {0};
			setting_password_get_string(parent, buffer);
			if(strncmp(buffer,"0000",4) != 0)
			{
				// user_data_get()->room_id = atoi(buffer);
				// int floor_id = atoi(user_data_get()->floor);
				// user_data_get()->other.family_id = (floor_id << 16) || 

				//hlf
				// short int new_family_id = 0;
				// sscanf(buffer,"%04hu",&new_family_id);

				// network_cmd_data_init(data);
				// data.cmd = NET_COMMON_CMD_RESET_OUTDOOR_NETWORK;
				// data.arg1 = 0;
				// data.arg2 = new_family_id;
				// data.device = DEVICE_GROUP;
				// network_send_cmd_data(&data);

				sscanf(buffer,"%04hu",&user_data_get()->other.family_id);
				user_data_save();


				if(network_inited_status_get() == true)
				{
					lv_obj_t * obj = network_init_reset_func();
					network_inited_status_set(false);
					lv_layout_task_create(reset_network_init_check_task, 2000, LV_TASK_PRIO_MID, obj);
				}

			}

		}

	}
	else
	{
		setting_password_input_string(parent, txt, 4,false);
	}	
}

/***
** 日期: 2022-05-07 14:05
** 作者: leo.liu
** 函数作用：数字键盘创建
** 返回参数说明：
***/
static void setting_password_keyboard_btn_create(void)
{
	static btn_data click_data = btn_data_anything_create(setting_password_keyboard_enter_password_btn_up);
	click_data.anything_func = setting_password_keyboard_enter_password_btn_up;
	setting_passowrd_num_keyboard_create(NULL, 100, 100, 280, 384, &click_data);
}

/***
** 日期: 2022-05-07 16:04
** 作者: leo.liu
** 函数作用：创建输入密码容器
** 返回参数说明：
***/
static void setting_password_enter_pwd_cont_create(void)
{
    // char *src1[language_total] = {"Your current room number","你当前的房号","ваш текущий номер","Votre numéro de chambre actuel","Su número de habitación actual","Ihre aktuelle Zimmernummer","رقم غرفتك الحالية","Vaše aktuální číslo pokoje","מספר החדר הנוכחי שלך","O seu número actual do quarto","Il tuo attuale numero di camera"};
	// char *src2[language_total] = {"Please enter the room number to be set\nPress # to confirm","请输入要设置的房号\n按#确认","Введите номер комнаты\nНажмите кнопку #, чтобы подтвердит","Veuillez saisir le numéro de chambre à définir\nAppuyez sur le  # pour confirmer",\
	// "Introduzca el número de habitación que desea establecer\nConfirme con #","Bitte geben Sie die Zimmernummer ein,\ndie Sie einstellen möchten.\nDrücken Sie # zur Bestätigung.",\
	// "أدخل رقم الغرفة التي تريد تعيين\nاضغط على # للتأكيد","Zadejte prosím číslo pokoje\nkteré má být nastaveno\nStiskněte # pro potvrzení","אנא הכנס את מספר החדר שיקבע\nלחץ # כדי לאשר","Introduza o número do quarto a definir\nPressione # para confirmar","Inserire numero camera da impostare \n premere # per confermare"};
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);

	lv_obj_set_pos(cont, 540, 210);
	lv_obj_set_size(cont,312 , 412);

	lv_obj_t *obj = lv_label_create(cont, NULL);
	lv_obj_set_id(obj, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT);
	lv_label_set_long_mode(obj, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(obj, 0, 0);
	lv_obj_set_size(obj, 312, 56);
	lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_text(obj,str_get(LAYOUT_SETROOMNO_LANG_YOURCURRETROOM_ID));

	lv_obj_t * obj2 = lv_label_create(cont, obj);
	lv_obj_set_id(obj2, 0x03);
	lv_label_set_long_mode(obj2, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(obj2, 0, 155);
	lv_obj_set_size(obj2, 312, 56);
	lv_label_set_align(obj2, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(obj2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_text(obj2,str_get(LAYOUT_SETROOMNO_LANG_ENTERSETNUMBER_ID));

    int device = user_data_get()->other.family_id;
    char buffer[4] = {0};
    sprintf(buffer,"%04d",device);
    lv_obj_t *label = lv_label_create(cont, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_FONT_SIZE_31);
    lv_label_set_text(label, buffer);
    lv_obj_set_pos(label, 0, 41  );
    lv_obj_set_size(label, 312, 60);
    lv_obj_set_style_local_text_letter_space(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 65);




    lv_obj_t *line = lv_obj_create(cont, NULL);
    lv_obj_set_size(line, 60, 4);
    lv_obj_set_style_local_bg_opa(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0x0096ff));
    lv_obj_set_style_local_radius(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 60);
    lv_obj_set_pos(line, 0, 101  );

    line = lv_obj_create(cont, line);
    lv_obj_set_x(line, 0 + (60 + 24) * 1);

    line = lv_obj_create(cont, line);
    lv_obj_set_x(line, 0 + (60 + 24) * 2);

    line = lv_obj_create(cont, line);
    lv_obj_set_x(line, 0 + (60 + 24) * 3);
	

	setting_password_input_label_create(cont, 0, 206, 1);
}



static void LAYOUT_ENETER_FUNC(room_select)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置
	setting_password_cancel_btn_create();

	setting_password_keyboard_btn_create();

	setting_password_enter_pwd_cont_create();

}

static void LAYOUT_QUIT_FUNC(room_select)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
}

CREATE_LAYOUT(room_select);