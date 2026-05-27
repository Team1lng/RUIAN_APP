#include "layout_define.h"
// extern bool wifi;
extern int current_cctv_index;
extern int connecting_cctv_ipaddr;
// extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);
// extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
// extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
// extern char tuya_uid[64];
// extern char tuya_key[64];
// static int test_times = 0;
static void system_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	

	lv_label_set_text(obj, str_get(LAYOUT_SETTING_LANG_SYSTEM_ID));
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	
}

static void system_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void system_back_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(set_cctv));
	
}


static lv_obj_t* system_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(system_back_btn_down, system_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}



static void ccctv_delete_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_delete_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	printf("\n下标=>%d\n",current_cctv_index);
	// 将保存onvif数组的当前索引的下标删除
	// for(int i = current_cctv_index;i<user_data_get()->onvif_dev_count-1;i++){
		// user_data_get()->onvif_dev[current_cctv_index] = user_data_get()->onvif_dev[current_cctv_index+1];
	// }
	if((current_cctv_index < 8) && current_cctv_index != 7){
		memmove(&user_data_get()->onvif_dev[current_cctv_index],&user_data_get()->onvif_dev[current_cctv_index+1],sizeof(user_onvif_info)*(7-current_cctv_index));
	}else if(current_cctv_index == 7)
	{
		memset(&user_data_get()->onvif_dev[current_cctv_index],0,sizeof(user_onvif_info));
	}

	user_data_get()->onvif_dev_count--;
	user_data_save();
	goto_layout(pLAYOUT(set_cctv));

	
}


static lv_obj_t* cctv_delete_btn_create(void)
{
	static btn_data btn_data = btn_data_create(ccctv_delete_btn_down, cctv_delete_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_DEL_ALL_PNG);
	lv_obj_t* btn = setting_btn_create(917, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}



static lv_obj_t* system_btn_create(lv_obj_t *parent, int x, int y, int w, int h, const char * src)
{
	lv_obj_t *cont = lv_cont_create(parent, NULL);
	set_location(cont,x,y,w,h);
	
	lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

	lv_obj_set_style_local_border_side(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	
	lv_obj_t *label = lv_label_create(cont, NULL);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(label, src);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, cont, LV_ALIGN_IN_LEFT_MID, 0, 0);
	
	return cont;
}


static void system_page_create(lv_obj_t *parent)
{
	//创建容器
	lv_obj_t *system_cont = lv_cont_create(parent,NULL);
	set_location(system_cont, 0, 100, 1024,500);
	lv_cont_set_layout(system_cont, LV_LAYOUT_OFF);

	lv_obj_set_style_local_bg_color(system_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(system_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

	//创建按钮
	// const char *str1[language_total] = {"Software version","软件版本","Версия программного обеспечения","Une version de logiciel","Versión del software","Softwareversion","اصدار البرنامج","Verze softwaru", "גירסת תוכנה","Versão do Software","Versione software"};
	lv_obj_t *cont1 = system_btn_create(system_cont, 70, 0 ,877, 99,str_get(LAYOUT_CCTV_LANG_IPADDRESS_ID));
	// cont1->event_cb = test_enter_cb;
	
	lv_obj_t *label1 = lv_label_create(cont1, NULL);
	lv_obj_set_style_local_text_color(label1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));
	lv_label_set_text_fmt(label1, "%s",user_data_get()->onvif_dev[current_cctv_index].ip);

	lv_obj_align(label1, cont1, LV_ALIGN_IN_LEFT_MID, 690, 0);


	// const char *str2[language_total] = {"Release date","发布日期","Дата выпуска","Date de sortie","Fecha de lanzamiento","Veröffentlichungsdatum","تاريخ الاصدار","Datum vydání", "תאריך שחרור","Data de Lançamento","Data di rilascio"};
	lv_obj_t *cont2 = system_btn_create(system_cont, 70, 100 ,877, 99,str_get(LAYOUT_CCTV_LANG_ACCOUNT_ID));
	
	lv_obj_t *label2 = lv_label_create(cont2, NULL);
	lv_obj_set_style_local_text_color(label2, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));
	lv_label_set_text_fmt(label2, "%s", user_data_get()->onvif_dev[current_cctv_index].user);

	lv_obj_align(label2, cont2, LV_ALIGN_IN_LEFT_MID, 690, 0);


	// const char *str3[language_total] = {"SD remain space","SD卡剩余空间","SD остается место","SD reste de l'espace","SD queda espacio","SD bleibt Platz","المساحة المتبقية من بطاقة التخزين","Zbývající místo na SD kartě", "נפח פנוי ב SD","Espaço Livre no Cartão SD","Spazio rimanente SD"};
	lv_obj_t *cont3 = system_btn_create(system_cont, 70, 200 ,877, 99,str_get(LAYOUT_CCTV_LANG_PASSWORD_ID));
	lv_obj_t *label3 = lv_label_create(cont3, NULL);
	lv_obj_set_style_local_text_color(label3, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));
	lv_label_set_text_fmt(label3, "%s", user_data_get()->onvif_dev[current_cctv_index].pswd);			
	lv_obj_align(label3, cont3, LV_ALIGN_IN_LEFT_MID, 690, 0);

	

	// const char *str4[language_total] = {"UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID","UUID"};
	lv_obj_t *cont4 = system_btn_create(system_cont, 70, 300 ,877, 99,"Main code stream");
	

	lv_obj_t *label4 = lv_label_create(cont4, NULL);
	lv_obj_set_style_local_text_color(label4, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));

	// if(strlen(tuya_uid))
	// {
	// 	lv_label_set_text_fmt(label4, "*%s",&tuya_uid[11]);
	// }
	// else
	// {
		lv_label_set_text(label4, user_data_get()->onvif_dev[current_cctv_index].url);
		printf("\n=========user_data_get()->onvif_dev[current_cctv_index].url=>%s=========\n",user_data_get()->onvif_dev[current_cctv_index].url);
		char *abc = sat_ipcamera_ipaddr_get(0);
		printf("=========sat_ipcamera_ipaddr_get=>%s======",abc);
	// }
			
	lv_obj_align(label4, cont4, LV_ALIGN_IN_LEFT_MID, 200, 0);

#if 0	
	static char sub_string[64] = {0};
	memset(sub_string,0,sizeof(sub_string));
	char serial[64] = {0};
	const char *str5[language_total] = {"Serial number","Serial number","Serial number","Serial number","Serial number","Serial number","Serial number"};
	if (tuya_serial_number_get(serial) == true)
	{
		strcat(sub_string, serial);
	}
	lv_obj_t *cont5 = system_btn_create(system_cont, 70, 400 ,877, 99,str5[user_data_get()->user_language]);
	
	lv_obj_t *label5 = lv_label_create(cont5, NULL);
	lv_obj_set_style_local_text_color(label5, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));

			
	lv_obj_align(label5, cont5, LV_ALIGN_IN_LEFT_MID, 700, 0);

	lv_label_set_text_fmt(label5, "%s",sub_string);
#endif



}

static void LAYOUT_ENETER_FUNC(cctv_information)
{
// 	//判断mipi版本和rgb版本
// 	screen_check();
// 	test_times = 0;
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

// 	system_head_label_create();
	system_back_btn_create();
	cctv_delete_btn_create();
// 	//创建显示系统信息的容器
	system_page_create(lv_scr_act());
}


static void LAYOUT_QUIT_FUNC(cctv_information)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	// test_times = 0;
}


CREATE_LAYOUT(cctv_information);

