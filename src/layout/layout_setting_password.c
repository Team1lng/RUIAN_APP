#include "layout_define.h"
#include "layout_setting_common.h"
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT 0X01
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT 0X02
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_NUM 0X03

#define SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT 0X04

#define SETTING_PASSWORD_OBJ_ID_MSG_CONT 0X05
#define SETTING_PASSWORD_OBJ_ID_APPLE_BTN 0X06

static PASSWD_CH passwd_channel = PASSWD_CH_NONE;
void enter_layout_passwd_ch(PASSWD_CH ch)
{
	passwd_channel = ch;
}
PASSWD_CH get_layout_passwd_ch()
{
	return passwd_channel;
}
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
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
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}


/***
** 日期: 2022-05-09 11:54
** 作者: leo.liu
** 函数作用：复位输入框
** 返回参数说明：
***/
static void setting_password_not_match_task(lv_task_t *task_t)
{
    // char *src[language_total] = {"Please input a password","请输入密码","Увядзіце пароль","Veuillez saisir le mot de passe","Introduzca la contraseña","Bitte geben Sie ein Passwort ein","الرجاء إدخال كلمة السر","Zadejte prosím heslo","אנא הכנס סיסמה","Introduza por favor uma senha","Inserisci password"};
	lv_obj_t *label = (lv_obj_t *)task_t->user_data;
	lv_label_set_text(label, str_get(LAYOUT_SETPASSWORD_LANG_PLEASEINPUTPASSWORD_ID));
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);
	setting_password_input_reset(parent, 4);

	lv_task_del(task_t);
}

/***
** 日期: 2022-05-09 15:19
** 作者: leo.liu
** 函数作用：检测密码是否很简单
** 返回参数说明：
***/
static bool setting_password_check_passowrd_easy(lv_obj_t *parent)
{
	char easy_password[3][4] = {{"0000"}, {"1234"}, {"4321"}};
	char buffer[9] = {0};
	memset(buffer,0,sizeof(buffer));
	setting_password_get_string(parent, buffer);


	
	for (int i = 0; i < 3; i++)
	{
		if (strncmp(easy_password[i], buffer, 4) == 0)
		{
			return true;
		}
	}
	return false;
}
/***
** 日期: 2022-05-09 14:16
** 作者: leo.liu
** 函数作用：输入新密码按键事件
** 返回参数说明：
***/
static void setting_password_keyborad_new_password_btn_up(lv_obj_t *obj, lv_event_t ev)
{
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);
	if (parent == NULL)
	{
		return;
	}

	if (ev != LV_EVENT_VALUE_CHANGED)
	{
		return;
	}

	const char *txt = lv_btnmatrix_get_active_btn_text(obj);
	if (txt[0] == ' ')
	{
		setting_password_del_string(parent, 8);
		if (setting_password_edit_index_get(parent) == 7)
		{
			lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
			lv_obj_set_state(btn, LV_STATE_DISABLED);
		}
	}
	else if (setting_password_input_string(parent, txt, 8 , true) == true)
	{
		if ((setting_password_edit_index_get(parent) == 8) && (setting_password_check_passowrd_easy(parent) == false))
		{
			lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
			lv_obj_set_state(btn, LV_STATE_DEFAULT);
		}
	}
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
	else if (setting_password_input_string(parent, txt, 4,true) == true)
	{
		if (setting_password_edit_index_get(parent) == 4)
		{
			char buffer[5] = {0};
			setting_password_get_string(parent, buffer);
			printf("password:%c%c%c%c \n", user_data_get()->other.password[0], user_data_get()->other.password[1], user_data_get()->other.password[2], user_data_get()->other.password[3]);
			if ((strncmp(buffer, user_data_get()->other.password, 4) == 0) || (strncmp(buffer, user_data_get()->super_password, 4) == 0))
			{
				if(get_layout_passwd_ch() == PASSWD_CH_SETTING_FLOOR)
				{
					goto_layout(pLAYOUT(room_select));
				}
				else if(get_layout_passwd_ch() == PASSWD_CH_CCTV_INFORMATION){
					goto_layout(pLAYOUT(cctv_information));
				}
				else{
					printf("input password success ! \n");
					lv_obj_t *new_parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);
					lv_obj_set_hidden(parent, true);
					lv_obj_set_hidden(new_parent, false);

					btn_data *click_data = (btn_data *)obj->user_data;
					click_data->anything_func = setting_password_keyborad_new_password_btn_up;
					setting_password_input_reset(new_parent, 8);	

					lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
					lv_obj_set_state(btn, LV_STATE_DISABLED);
					lv_obj_set_hidden(btn, false);

				}

			}
			else
			{
				printf("input password failed ! \n");
				lv_obj_t *label = lv_obj_get_child_form_id(parent, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT);
				// char *src[language_total] = {"Incorrect password","密码不正确","Неправільны пароль","Incorrect password","Contraseña incorrecta","Falsches Passwort","كلمة السر غير صحيحة","Nesprávné heslo","סיסמה לא נכונה","Senha incorrecta","Password errata"};
				lv_label_set_text(label, str_get(LAYOUT_SETPASSWORD_LANG_INCORRECTPASSWORD_ID));
				lv_layout_task_create(setting_password_not_match_task, 1000, LV_TASK_PRIO_MID, label);
			}

		}
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
    // char *src[language_total] = {"Please input a password","请输入密码","Увядзіце пароль","Veuillez saisir le mot de passe","Introduzca la contraseña","Bitte geben Sie Ihr Passwort ein","الرجاء إدخال كلمة السر","Zadejte prosím heslo","אנא הכנס סיסמה","Introduza por favor uma senha","Inserisci password"};
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);

	lv_obj_set_pos(cont, 540, 210);
	lv_obj_set_size(cont, 312, 167);

	lv_obj_t *obj = lv_label_create(cont, NULL);
	lv_obj_set_id(obj, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT);
	lv_label_set_long_mode(obj, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(obj, 0, 0);
	lv_obj_set_size(obj, 312, 56);
	lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_text(obj,str_get(LAYOUT_SETPASSWORD_LANG_PLEASEINPUTPASSWORD_ID));
	setting_password_input_label_create(cont, 0, 91, 1);
}

/***
** 日期: 2022-05-09 11:59
** 作者: leo.liu
** 函数作用：输入新密码
** 返回参数说明：
***/
static void setting_password_input_new_pwd_cont_create(void)
{
    // char *src1[language_total] = {"Password reset", "密码重置","Сброс пароля", "Réinitialisation du mot de passe",  "restablecer la contraseña", "Passwort zurücksetzen","إعادة تعيين كلمة المرور","Obnovení hesla","מחדש את הסיסמה","Repor a Senha","Reset Password"};
	/***** SETTING_PASSWORD_LANG_ID_NEW_PWD *****/
	// char *src2[language_total] = {"New password", "新密码", "Новый пароль", "Nouveau mot de passe","Nueva Schwester", "Neues Passwort", "كلمة المرور الجديدة","Nové heslo","סיסמה חדשה","Nova senha","Nuova password"};
	/***** SETTING_PASSWORD_LANG_ID_CONFIRM_PWD *****/
	// char *src3[language_total] = {"Confirm Password", "确认密码", "Подтвердить пароль", "Confirmer le mot de passe","Confirmar contraseña", "Bestätigen Sie Ihr Passwort","تأكيد كلمة المرور","Potvrzení hesla","אישור את הסיסמה","Confirmar a Senha","Conferma password"};

	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);

	lv_obj_set_pos(cont, 540, 80);
	lv_obj_set_size(cont, 312, 388);
	lv_obj_set_hidden(cont, true);

	lv_obj_t *obj = lv_obj_create(cont, NULL);
	lv_obj_set_pos(obj, 0, 0);
	lv_obj_set_size(obj, 312, 56);
	lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_MID);
	lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SETPASSWORD_LANG_PASSWORDRESET_ID));

	obj = lv_obj_create(cont, obj);
	lv_obj_set_y(obj, 44);
	lv_obj_set_height(obj, 40);
	lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SETPASSWORD_LANG_NEWPASSWORD_ID));

	obj = lv_obj_create(cont, obj);
	lv_obj_set_y(obj, 254);
	lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SETPASSWORD_LANG_CONFIRMPASSWORD_ID));

	setting_password_input_label_create(cont, 0, 142, 2);
}

static void setting_password_msg_cont_create(void)
{
    // char *src[language_total] = {"Passwords do not match","密码不匹配", "Неверный пароль", "Le mot de passe ne correspond pas","La contraseña no coincide", "Passwörter stimmen nicht überein", "كلمة المرور غير متطابقة", "Hesla se neshodují","הסיסמאות לא מתאימות","As senhas não correspondem","le password non corrispondono"};
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(cont, 0, 0);
	lv_obj_set_size(cont, 1024, 600);
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

	lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_MSG_CONT);
	lv_obj_set_hidden(cont, true);

	lv_obj_t *obj = lv_obj_create(cont, NULL);
	lv_obj_set_size(obj, 550, 160);
	lv_obj_align(obj, cont, LV_ALIGN_CENTER, 0, 0);

	lv_obj_set_style_local_radius(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SETPASSWORD_LANG_PASSWORDNOMATCH_ID));
}

static void lv_setting_password_msg_task(lv_task_t *task_t)
{
	lv_obj_t *cont = (lv_obj_t *)task_t->user_data;
	lv_obj_set_hidden(cont, true);
	lv_task_del(task_t);
}

static void setting_password_apple_btn_up(lv_obj_t *obj)
{
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_MSG_CONT);
	if (lv_obj_get_hidden(cont) == false)
	{
		return;
	}

	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);
	if (parent == NULL)
	{
		return;
	}

	if (setting_password_edit_index_get(parent) == 8)
	{
		char buffer[9] = {0};
		setting_password_get_string(parent, buffer);
		if (strncmp(&buffer[0], &buffer[4], 4) == 0)
		{
			strncpy(user_data_get()->other.password, buffer, 4);
			user_data_save();
			goto_layout(pLAYOUT(setting));
		}
		else
		{
			lv_obj_set_hidden(cont, false);
			lv_layout_task_create(lv_setting_password_msg_task, 1000, LV_TASK_PRIO_MID, cont);
		}
	}
}
/***
** 日期: 2022-05-09 14:42
** 作者: leo.liu
** 函数作用：创建应用按钮
** 返回参数说明：
***/
static void setting_password_apple_btn_create(void)
{
    // char *src[language_total] =  {"apply",  "应用", "Применить", "appliquer","Aplicar", "verwenden", "تأكيد","použite","אפליקציה","aplicar","Applica"};
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_id(btn, SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
	lv_obj_set_pos(btn, 0, 516);
	lv_obj_set_size(btn, 1024, 84);
	lv_obj_set_hidden(btn, true);

	static btn_data click_data = btn_data_up_create(setting_password_apple_btn_up);
	btn->user_data = &click_data;
	btn_touch_event_listen(btn);

	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0096ff));
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x001E33));
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0069b3));

	lv_obj_set_style_local_value_font(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x888888));
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SETPASSWORD_LANG_APPLY_ID));

	lv_obj_set_state(btn, LV_STATE_DISABLED);
}

static void LAYOUT_ENETER_FUNC(setting_password)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置
	setting_password_cancel_btn_create();

	setting_password_keyboard_btn_create();

	setting_password_enter_pwd_cont_create();

	setting_password_input_new_pwd_cont_create();

	setting_password_apple_btn_create();

	setting_password_msg_cont_create();
}

static void LAYOUT_QUIT_FUNC(setting_password)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
}

CREATE_LAYOUT(setting_password);