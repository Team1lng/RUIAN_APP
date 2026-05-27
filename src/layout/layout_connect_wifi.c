#include "layout_define.h"
#include "tuya_ipc_api.h"

extern lv_obj_t *addwifi_back_btn_create(void);
extern lv_obj_t *input_textarea_create(lv_obj_t *parent, int x, int y, int w, int h, bool pwd_mode, const char *txt);
extern char connectwifi_name[24];
extern const char **kb_map[];
extern const lv_btnmatrix_ctrl_t *kb_ctrl[];
extern int connectwifi_index;
extern lv_obj_t *connect_wifi_cb(void);
extern void set_msg_text(lv_obj_t *msg, int state);
extern int wifi_connection_status_sucess(void);

extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);
extern bool wpa_cli_wlan_status(bool *continue_flag);

static lv_task_t *msg_ui_ptask = NULL;
static bool connect_falge = false;

static lv_obj_t *msg = NULL;

lv_obj_t *mask_layer_cont = NULL;

#define CONNECTING 0
#define CONNECT_SUCCESS 1
#define CONNECT_FAIL 2
#define WIFINAME_EMPTY 3
#define WIFIPWD_SHOT 4
#define NOT_FIND 5

static int task_timer = 0;
static void msg_task(struct _lv_task_t *task_t)
{
	if (connect_falge)
	{
		// // 删除透明遮罩层
		// if(mask_layer_cont)
		// 	lv_obj_del(mask_layer_cont);
		if (task_timer <= 12)
		{
		
			task_timer++;
			int connect_ret = wifi_connection_status_sucess();
			if (connect_ret == 1 && (task_timer > 6))
			{ // 连接成功

				system("rm -rf /etc/config/wpa_supplicant.conf &");
				system("cp -rf /tmp/wpa_supplicant.conf /etc/config/ &");
				system("sync");
				user_data_get()->wifi.wifi_connect_flag = true;
				user_data_save();
				goto_layout(pLAYOUT(set_wifi));
				lv_obj_t *msg1 = connect_wifi_cb();
				printf("msgbox1 is %p\n", msg1);
				// // 删除透明遮罩层
				// if(mask_layer_cont)
				// 	lv_obj_del(mask_layer_cont);
				if (msg1)
					set_msg_text(msg1, CONNECT_SUCCESS);
				task_timer = 0;
				connect_falge = false;
				tuya_ipc_reconnect_wifi();
			}
		}
		else
		{ // 连接失败
			connect_falge = false;
			task_timer = 0;
			lv_obj_t *msg1 = connect_wifi_cb();
			set_msg_text(msg1, CONNECT_FAIL);
			// 删除透明遮罩层
			if(mask_layer_cont)
				lv_obj_del(mask_layer_cont);
			if (msg)
				lv_obj_del(msg);
			printf("\n杀死wpa进程开始\n");
			system("killall wpa_supplicant &");
			system("killall udhcpc &");
			system("rm -rf /tmp/wpa_supplicant.conf &");
			printf("\n杀死wpa进程完毕\n");
			usleep(1000*500);
			printf("\n打开wpa进程开始\n");
			system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B &");
			printf("\n打开wpa进程完毕\n");

			system("udhcpc -i wlan0 -n 4 -R &");
			lv_keyboard_ext_t *ext = lv_obj_get_ext_attr(task_t->user_data);
			lv_textarea_set_text(ext->ta, "");
		}
		// lv_obj_t *mask_layer_cont = lv_obj_get_child_form_id(lv_scr_act(), 238);
	}
	else
		return;
}

// 创建标题
static void connectwifi_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);


	lv_label_set_text(obj, connectwifi_name);
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
}

// 创建一个透明大容器防止在连接的时候用户造成误触
static void mask_layer_create(void)
{
	mask_layer_cont = lv_obj_create(lv_scr_act(), NULL);
	set_location(mask_layer_cont, 0, 0, 1024, 600);
	lv_obj_set_id(mask_layer_cont, 238);
	lv_obj_set_style_local_bg_opa(mask_layer_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_10);
	lv_obj_set_style_local_bg_color(mask_layer_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	// lv_obj_set_style_local_bg_opa(mask_layer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_0);
	// usleep(500*1000);
}

// connect键盘的回调函数
static void lv_keyboard_event_cb(lv_obj_t *kb)
{
	LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

	// if(event != LV_EVENT_VALUE_CHANGED) return;

	lv_keyboard_ext_t *ext = lv_obj_get_ext_attr(kb);
	uint16_t btn_id = lv_btnmatrix_get_active_btn(kb);

	if (btn_id == LV_BTNMATRIX_BTN_NONE)
		return;
	if (lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_HIDDEN | LV_BTNMATRIX_CTRL_DISABLED))
		return;
	// if(lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT) return;

	const char *txt = lv_btnmatrix_get_active_btn_text(kb); // 获得活跃的按钮的文本
	if (txt == NULL)
		return;

	/*Do the corresponding action according to the text of the button*/
	if (strcmp(txt, "abc") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_TEXT_LOWER;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_LOWER]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_LOWER]);

		lv_keyboard_img_set(kb, 10, NULL);

		static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
		lv_keyboard_img_set(kb, 11, &info11);

		static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
		lv_keyboard_img_set(kb, 22, &info22);

		return;
	}
#if LV_USE_ARABIC_PERSIAN_CHARS == 1
	else if (strcmp(txt, "أب") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_TEXT_ARABIC;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_ARABIC]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_ARABIC]);
		return;
	}
#endif
	else if (strcmp(txt, "ABC") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_TEXT_UPPER;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_UPPER]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_UPPER]);

		// 对图片进行调整
		lv_keyboard_img_set(kb, 10, NULL);

		static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
		lv_keyboard_img_set(kb, 11, &info11);

		static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
		lv_keyboard_img_set(kb, 22, &info22);
		return;
	}
	else if (strcmp(txt, "1#") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_SPECIAL;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_SPECIAL]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_SPECIAL]);

		static rom_bin_info info10 = rom_bin_info_get(ROM_RES_KB_DELETL_PNG);
		lv_keyboard_img_set(kb, 10, &info10);

		lv_keyboard_img_set(kb, 11, NULL);

		lv_keyboard_img_set(kb, 22, NULL);
		return;
	}

	/****************************************设置模式************************************************************************/
	/*Add the characters to the text area if set*/

	if (ext->ta == NULL)
		return;

	if (ext->btnm.pattern_p[btn_id])
	{

		if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_CANCEL_PNG)
		{ /*LV_SYMBOL_CLOSE*/

			lv_textarea_set_text(ext->ta, ""); /*De-assign the text area to hide it cursor if needed*/
			return;
		}
		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_APPLY_PNG)
		{ /*LV_SYMBOL_OK*/
			// 创建一个透明大容器防止在连接的时候用户造成误触
			mask_layer_create();
			// usleep(1000*500);
			if (strlen(lv_textarea_get_text(ext->ta)) < 8)
			{
				lv_obj_t *msg = connect_wifi_cb();
				set_msg_text(msg, WIFIPWD_SHOT);
				lv_textarea_set_text(ext->ta, "");
				lv_obj_del(mask_layer_cont);
				return;
			}
			connect_falge = true;

			const char *txt = lv_textarea_get_text(ext->ta);

			wpa_cli_connect_new_wifi(connectwifi_name, txt);

			msg = connect_wifi_cb();
			set_msg_text(msg, CONNECTING);
		
		}
		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_ENTER_PNG) /* LV_SYMBOL_NEW_LINE*/
		{
		}

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_LEFT_PNG) /*LV_SYMBOL_LEFT*/
		{
		}

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_RIGHT_PNG) /*LV_SYMBOL_RIGHT*/
		{
		}

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_DELETL_PNG || ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_DELETS_PNG) /*LV_SYMBOL_BACKSPACE*/
			lv_textarea_del_char(ext->ta);

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_SPACE_PNG) /*LV_SYMBOL_SPACE*/
			lv_textarea_add_char(ext->ta, ' ');
	}
	else if (strcmp(txt, "+/-") == 0)
	{
		uint16_t cur = lv_textarea_get_cursor_pos(ext->ta);
		const char *ta_txt = lv_textarea_get_text(ext->ta);
		if (ta_txt[0] == '-')
		{
			lv_textarea_set_cursor_pos(ext->ta, 1);
			lv_textarea_del_char(ext->ta);
			lv_textarea_add_char(ext->ta, '+');
			lv_textarea_set_cursor_pos(ext->ta, cur);
		}
		else if (ta_txt[0] == '+')
		{
			lv_textarea_set_cursor_pos(ext->ta, 1);
			lv_textarea_del_char(ext->ta);
			lv_textarea_add_char(ext->ta, '-');
			lv_textarea_set_cursor_pos(ext->ta, cur);
		}
		else
		{
			lv_textarea_set_cursor_pos(ext->ta, 0);
			lv_textarea_add_char(ext->ta, '-');
			lv_textarea_set_cursor_pos(ext->ta, cur + 1);
		}
	}
	else
	{
		lv_textarea_add_text(ext->ta, txt);
	}
}

// 对键盘进行设置布局
void keyboard_layout(lv_obj_t *obj)
{
	static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
	lv_keyboard_img_set(obj, 11, &info11);

	static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
	lv_keyboard_img_set(obj, 22, &info22);

	static rom_bin_info info35 = rom_bin_info_get(ROM_RES_KB_CANCEL_PNG);
	lv_keyboard_img_set(obj, 35, &info35);

	static rom_bin_info info36 = rom_bin_info_get(ROM_RES_KB_LEFT_PNG);
	lv_keyboard_img_set(obj, 36, &info36);

	static rom_bin_info info37 = rom_bin_info_get(ROM_RES_KB_SPACE_PNG);
	lv_keyboard_img_set(obj, 37, &info37);

	static rom_bin_info info38 = rom_bin_info_get(ROM_RES_KB_RIGHT_PNG);
	lv_keyboard_img_set(obj, 38, &info38);

	static rom_bin_info info39 = rom_bin_info_get(ROM_RES_KB_APPLY_PNG);
	lv_keyboard_img_set(obj, 39, &info39);
}

// 创建键盘
static lv_obj_t *keybord_create(lv_obj_t *parent, lv_obj_t *ta)
{
	lv_obj_t *kb = lv_keyboard_create(parent, NULL);

	set_location(kb, 0, 200, 1024, 400);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_keyboard_set_textarea(kb, ta);
	lv_keyboard_set_cursor_manage(kb, false);

	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x191919));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_pad_top(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_pad_bottom(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 5);
	lv_obj_set_style_local_pad_left(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_pad_right(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_pad_inner(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);

	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x242424));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_text_font(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE(31));

	connect_falge = false;

	keyboard_layout(kb);

	static btn_data kb_data = btn_data_up_create(lv_keyboard_event_cb);
	kb->user_data = &kb_data;
	btn_touch_event_listen(kb);

	if (msg_ui_ptask != NULL)
	{
		lv_task_del(msg_ui_ptask);
	}
	msg_ui_ptask = lv_task_create(msg_task, 300, LV_TASK_PRIO_HIGH, kb);
	lv_task_ready(msg_ui_ptask);

	msg_task(msg_ui_ptask);

	return kb;
}

// 创建两个输入框
lv_obj_t *input_textarea_create(lv_obj_t *parent, int x, int y, int w, int h, bool pwd_mode, const char *txt)
{
	lv_obj_t *textarea = lv_textarea_create(parent, NULL);
	set_location(textarea, x, y, w, h);

	lv_textarea_set_text(textarea, "");

	lv_textarea_set_placeholder_text(textarea, txt);
	lv_textarea_set_max_length(textarea, 30);
	lv_textarea_set_one_line(textarea, true);
	lv_textarea_set_text_align(textarea, LV_LABEL_ALIGN_LEFT);
	lv_textarea_set_scrollbar_mode(textarea, LV_SCROLLBAR_MODE_OFF);

	lv_textarea_set_cursor_hidden(textarea, true);

	lv_textarea_set_pwd_mode(textarea, pwd_mode);
	lv_textarea_set_pwd_show_time(textarea, 200);

	lv_textarea_set_cursor_blink_time(textarea, 500);

	lv_obj_set_style_local_bg_color(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_text_font(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_style_local_text_letter_space(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 5);

	lv_obj_set_style_local_border_side(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_color(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x424542));
	lv_obj_set_style_local_border_width(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_side(textarea, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_color(textarea, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0x424542));
	lv_obj_set_style_local_border_width(textarea, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, 1);

	lv_obj_set_style_local_text_color(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, lv_color_hex(0x424542));
	lv_obj_set_style_local_text_font(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_style_local_pad_left(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, 40);
	lv_obj_set_style_local_text_letter_space(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, 2);

	return textarea;
}

static lv_obj_t *pwd_ta = NULL;

// 改变密码模式
static void change_pwd_mode(lv_obj_t *obj)
{

	static rom_bin_info info_pwd = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
	static rom_bin_info info_inv = rom_bin_info_get(ROM_RES_SETWIFI_VISIBLE_PNG);

	if (lv_textarea_get_pwd_mode(pwd_ta))
	{
		lv_img_set_src(obj, &info_inv);
		lv_textarea_set_pwd_mode(pwd_ta, false);
	}
	else
	{
		lv_img_set_src(obj, &info_pwd);
		lv_textarea_set_pwd_mode(pwd_ta, true);
	}
}

static void connectwifi_page_create(lv_obj_t *parent)
{
	// 如果是免费wifi
	wifi_info info;
	memset(&info, 0, sizeof(wifi_info));
	get_scanned_wifi_info(connectwifi_index, &info);

	lv_obj_t *wifipwd_cont = lv_cont_create(lv_scr_act(), NULL);
	set_location(wifipwd_cont, 0, 100, 1024, 500);
	lv_cont_set_layout(wifipwd_cont, LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_color(wifipwd_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(wifipwd_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	// 创建一个输入框

	// char *src[language_total] = {"Please input wifi password","请输入wifi密码", "Пожалуйста, введите пароль Wi-Fi",
	// 							 "Veuillez saisir le mot de passe Wi-Fi", "Por favor ingrese la contraseña wifi",
	// 							 "Bitte geben Sie das WLAN-Passwort ein", "الرجاء ادخال كلمة مرور واي فاي",
	// 							 "Zadejte heslo pro WiFi", "נא הכנס סיסמת WIFI", "Introduza Senha Wi-Fi Por Favor","Inserire la password Wi-Fi"};
	pwd_ta = input_textarea_create(wifipwd_cont, 40, 0, 942, 77, true, str_get(LAYOUT_CONNECTWIFI_LANG_INPUTWIFIPASSWORD_ID));
	// 创建密码模式切换 密码模式进去一定是不可见的 可见只是当时可见 退出再进任然不可见

	lv_obj_t *pwd_mode = lv_img_create(wifipwd_cont, NULL);
	// pwd_mode->user_data = ta;
	set_location(pwd_mode, 932, 15, 48, 48);
	static rom_bin_info pwd_info = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
	lv_img_set_src(pwd_mode, &pwd_info);
	// lv_obj_set_event_cb(pwd_mode, change_pwd_mode);
	lv_obj_set_click(pwd_mode, true);
	lv_obj_set_ext_click_area(pwd_mode, 15, 15, 15, 15);

	static btn_data img_data = btn_data_up_create(change_pwd_mode);
	pwd_mode->user_data = &img_data;
	btn_touch_event_listen(pwd_mode);

	// 创建一个键盘 并且绑定这个输入框

	lv_obj_t *kb = keybord_create(wifipwd_cont, pwd_ta);

	lv_obj_set_pos(kb, 0, 100);
	// 在键盘的确认按钮的回调上是将密码匹配wifi名称并且尝试连接 连接成功 在用户数据中保存 失败 直接清空密码 并且提示
}

static lv_task_t *set_scan_wifi_ptask = NULL;

static void scan_wifi_task(struct _lv_task_t *task_t)
{

	wpa_cli_scan_wifi(&user_data_get()->wifi.wifi_open_flag);
	wpa_cli_wlan_status(&user_data_get()->wifi.wifi_open_flag);
}

static void scan_wifi(void)
{
	if (user_data_get()->wifi.wifi_open_flag == false)
		return;

	if (set_scan_wifi_ptask != NULL)
	{
		lv_task_del(set_scan_wifi_ptask);
	}

	set_scan_wifi_ptask = lv_task_create(scan_wifi_task, 2000, LV_TASK_PRIO_MID, NULL);
	lv_task_ready(set_scan_wifi_ptask);

	scan_wifi_task(set_scan_wifi_ptask);
}

static void LAYOUT_ENETER_FUNC(connect_wifi)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false); // 系统背景使能设置

	connectwifi_head_label_create(); // 创建标题
	addwifi_back_btn_create();
	connectwifi_page_create(lv_scr_act()); // 创建连接wifi的页面
	scan_wifi();
}

static void LAYOUT_QUIT_FUNC(connect_wifi)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	if (msg_ui_ptask != NULL) // 计时任务未退出
	{
		lv_task_del(msg_ui_ptask); // 退出计时任务
		msg_ui_ptask = NULL;	   // 置空指针
	}

	if (set_scan_wifi_ptask != NULL) // 计时任务未退出
	{
		lv_task_del(set_scan_wifi_ptask); // 退出计时任务
		set_scan_wifi_ptask = NULL;		  // 置空指针
	}

	if (connect_falge)
	{
		system("killall wpa_supplicant ");
		system("killall udhcpc ");
		system("rm -rf /tmp/wpa_supplicant.conf ");

		system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B &");
		system("udhcpc -i wlan0 -n 4 -R &");
	}
}

CREATE_LAYOUT(connect_wifi);
