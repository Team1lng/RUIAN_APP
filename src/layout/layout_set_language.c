#include "layout_define.h"
extern bool wifi;
extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);

static void language_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);

	// char *str[language_total] = {"Language","语言","Язык","Langues","Idioma","Sprache","اللغة","Jazyk", "שפה","Idioma","Lingua"};
	lv_label_set_text(obj, str_get(LAYOUT_SETLANGUAGE_LANG_LANGUAGE_ID));
	lv_label_set_long_mode(obj, LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj, 1024, 60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj, LV_LAYOUT_CENTER);
}
static void language_back_btn_down(lv_obj_t *obj)
{

	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void language_back_btn_up(lv_obj_t *obj)
{

	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(setting));
}

static lv_obj_t *language_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(language_back_btn_down, language_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t *btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);

	return btn;
}

static void language_btnmatrix_up(lv_obj_t *obj)
{
	uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
	if (btn_id == LV_BTNMATRIX_BTN_NONE)
		return;

	if (lv_btnmatrix_get_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_DISABLED))
		return;

	user_data_get()->user_language = btn_id;
	tuya_set_current_language(btn_id);
	user_data_save();

	// 刷新
	goto_layout(pLAYOUT(set_language));
}

static void language_page_create(lv_obj_t *parent)
{
	lv_obj_t *language_cont = lv_cont_create(parent, NULL);
	set_location(language_cont, 0, 100, 1024, 500);
	lv_cont_set_layout(language_cont, LV_LAYOUT_OFF);

	lv_obj_set_style_local_bg_color(language_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(language_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_t *btnm = lv_btnmatrix_create(language_cont, NULL);
	lv_obj_set_style_local_bg_opa(btnm, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	lv_obj_set_style_local_pad_left(btnm, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_right(btnm, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_top(btnm, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_bottom(btnm, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_inner(btnm, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 32);

	lv_obj_set_style_local_bg_opa(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_bg_color(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x262626));
	lv_obj_set_style_local_bg_color(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0x262626));

	lv_obj_set_style_local_bg_color(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_make(0xCF, 0xA0, 0x75));

	lv_obj_set_style_local_bg_opa(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, LV_OPA_TRANSP);

	lv_btnmatrix_ext_t *ext = lv_obj_get_ext_attr(btnm);
	ext->pattern_align = LV_LABEL_ALIGN_CENTER;

	// static const char *map[] = {"English", "中文", "Русский", "Français", "Español", "Deutsch","\n",  "عربي", "Čeština", "עברית", "Português","Italiano","...", ""};
	static const char *map[] = {"English", "中文", "Русский", "Français", "Español","\n",
								"Deutsch", "عربي", "Čeština", "עברית", "Português", "\n",
								"Italiano", "Polish", "GREEK", "Türkçe", "Nederlands", ""};
	lv_btnmatrix_set_map(btnm, map);
	lv_btnmatrix_set_one_check(btnm, true);
	lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKABLE);

	lv_btnmatrix_set_align(btnm, LV_LABEL_ALIGN_CENTER, 0, 55);
	lv_obj_set_x(btnm, 160);
	lv_obj_set_y(btnm, 20);
	lv_obj_set_width(btnm, 800);
	lv_obj_set_height(btnm, 440);

	lv_obj_align_x(btnm, NULL, LV_ALIGN_CENTER, 0);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SET_LANGUAGE_PNG);
	lv_btnmatrix_set_pattern_image(btnm, 0, &info);
	lv_btnmatrix_set_pattern_image(btnm, 1, &info);
	lv_btnmatrix_set_pattern_image(btnm, 2, &info);
	lv_btnmatrix_set_pattern_image(btnm, 3, &info);
	lv_btnmatrix_set_pattern_image(btnm, 4, &info);
	lv_btnmatrix_set_pattern_image(btnm, 5, &info);
	lv_btnmatrix_set_pattern_image(btnm, 6, &info);
	lv_btnmatrix_set_pattern_image(btnm, 7, &info);
	lv_btnmatrix_set_pattern_image(btnm, 8, &info);
	lv_btnmatrix_set_pattern_image(btnm, 9, &info);
	lv_btnmatrix_set_pattern_image(btnm, 10, &info);
	lv_btnmatrix_set_pattern_image(btnm, 11, &info);
	lv_btnmatrix_set_pattern_image(btnm, 12, &info);
	lv_btnmatrix_set_pattern_image(btnm, 13, &info);
	lv_btnmatrix_set_pattern_image(btnm, 14, &info);
	lv_btnmatrix_set_pattern_align(btnm, 1);

	// lv_btnmatrix_set_btn_ctrl(btnm,11,LV_BTNMATRIX_CTRL_DISABLED);	/* 禁用 */

	switch (user_data_get()->user_language)
	{
	case 0:
		lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 1:
		lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 2:
		lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 3:
		lv_btnmatrix_set_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 4:
		lv_btnmatrix_set_btn_ctrl(btnm, 4, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 5:
		lv_btnmatrix_set_btn_ctrl(btnm, 5, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 6:
		lv_btnmatrix_set_btn_ctrl(btnm, 6, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 7:
		lv_btnmatrix_set_btn_ctrl(btnm, 7, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 8:
		lv_btnmatrix_set_btn_ctrl(btnm, 8, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 9:
		lv_btnmatrix_set_btn_ctrl(btnm, 9, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 10:
		lv_btnmatrix_set_btn_ctrl(btnm, 10, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 11:
		lv_btnmatrix_set_btn_ctrl(btnm, 11, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 12:
		lv_btnmatrix_set_btn_ctrl(btnm, 12, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 13:
		lv_btnmatrix_set_btn_ctrl(btnm, 13, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	case 14:
	    lv_btnmatrix_set_btn_ctrl(btnm, 14, LV_BTNMATRIX_CTRL_CHECK_STATE);
		break;
	default:
		break;
	}
	static btn_data btn_data = btn_data_up_create(language_btnmatrix_up);
	btnm->user_data = &btn_data;
	btn_touch_event_listen(btnm);
}

static void LAYOUT_ENETER_FUNC(set_language)
{

	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false); // 系统背景使能设置

	language_head_label_create();
	language_back_btn_create();

	language_page_create(lv_scr_act());
}

static void LAYOUT_QUIT_FUNC(set_language)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
}

CREATE_LAYOUT(set_language);
