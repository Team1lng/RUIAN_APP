#include "layout_background.h"

extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

static int cur_picture_index = 0;
static media_type picture_file_type = FILE_TYPE_SD_PICTURE;
static int picture_total;
static bool bg_img_set_flg = false;
char *picture_name = NULL;

static int background_parameter_init(void)
{
	picture_file_type = FILE_TYPE_SD_PICTURE;
	cur_picture_index = 0;
	picture_total = 0;
	picture_total = media_file_total_get(picture_file_type, 0);
    printf("========picture_total %d\n",picture_total);

	return picture_total;
}

static void layout_bg_info_label_display(void)
{

	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), layout_bg_info_adj_id);

	lv_label_set_text_fmt(label, "[%02d/%02d]",cur_picture_index + 1, picture_total);
} 
 

void layout_media_info_label_create(lv_obj_t *parent, int obj_id)
{
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_id(label, obj_id);
	lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(label, 800, 30);
	lv_obj_set_size(label, 310, 47);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

	layout_bg_info_label_display();
}

 


extern bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);
static void layout_bg_thumb_dsiplay(void)
{
    unsigned char * picture_data = picture_data_get();
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    fb_video_mode_enable(false);
	
    if(cur_picture_index >= picture_total)
	{
		cur_picture_index = 0;
	}
	media_info *info = media_info_get(picture_file_type,cur_picture_index);
	if(info == NULL)
	{
		return;
	}
	if (picture_data == NULL)
	{
		picture_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, 1024 * 600 * 4);

	}
    static rom_bin_info img = rom_bin_raw_get();
	rom_bin_raw_init(img, picture_data, 1024, 600);
	bzero(picture_name,sizeof(PICTURE_NAM_LENGTH));
	sprintf(picture_name,"%s%s",SD_PICTURE_PATH,info->file_name);
	lv_jpg_decode_data(picture_name, &img, 1024, 600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_image(lv_disp_get_default(), &img);

}

static void layout_bg_btn_state_set(lv_obj_t* obj,lv_state_t state)
{
	btn_data* pdata = (btn_data*)obj->user_data;
	lv_obj_t* children = (lv_obj_t*)pdata->user_data;
	lv_obj_set_state( children, state);
}


static void layout_bg_left_page_btn_down(lv_obj_t* obj)
{
	layout_bg_btn_state_set(obj,LV_STATE_PRESSED);
}


static void layout_bg_left_page_btn_up(lv_obj_t* obj)
{
	layout_bg_btn_state_set(obj,LV_STATE_DEFAULT);

	cur_picture_index--;
    if(cur_picture_index < 0)
	{
		cur_picture_index = picture_total -1;
	}
	

	layout_bg_thumb_dsiplay();

	buttom_aimmation_play(false);

	layout_bg_info_label_display();
}

static void layout_bg_btn_img_transform_set(lv_obj_t* obj)
{
	lv_obj_set_style_local_transform_zoom(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,256);
	lv_obj_set_style_local_transform_zoom(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,300);
	
	lv_obj_set_style_local_transition_prop_1(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_STYLE_TRANSFORM_ZOOM);
	lv_obj_set_style_local_transition_prop_2(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_STYLE_TRANSFORM_ZOOM);

	lv_obj_set_style_local_transition_time(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,200);
	lv_obj_set_style_local_transition_time(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,200);

	static lv_anim_path_t path ;
	path.cb = lv_anim_path_overshoot,
	path.user_data = NULL;
	lv_obj_set_style_local_transition_path(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,&path);
	lv_obj_set_style_local_transition_path(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,&path);
}

static lv_obj_t*  layout_bg_btn_create(int x,int y,int w,int h,btn_data* btn_pdata,const void* img_src,bool bg_color)
{
	lv_obj_t* btn = lv_btn_create(lv_scr_act(),NULL);
	lv_obj_set_pos(btn, x,y);
	lv_obj_set_size(btn,w,h);

	if(bg_color == true)
	{
		lv_obj_set_style_local_bg_color(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
		lv_obj_set_style_local_bg_color(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(0x4d,0x7a,0xFF));

		
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_OPA_70);
	

		lv_obj_set_style_local_radius(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);
		lv_obj_set_style_local_radius(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,45);
	
	}
	else
	{
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_OPA_TRANSP);

	}
	
	lv_obj_t* img = lv_img_create(lv_scr_act(),NULL);
	lv_img_set_src(img, img_src);


	layout_bg_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	
	return btn;
}
static void layout_bg_left_page_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(layout_bg_left_page_btn_down, layout_bg_left_page_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* obj = layout_bg_btn_create(0,307,54,54,&btn_data,&info,true);
	lv_obj_set_style_local_bg_opa(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(57,57,57));
}

static void layout_bg_right_page_btn_down(lv_obj_t* obj)
{
	layout_bg_btn_state_set(obj,LV_STATE_PRESSED);
}


static void layout_bg_right_page_btn_up(lv_obj_t* obj)
{
	layout_bg_btn_state_set(obj,LV_STATE_DEFAULT);


	cur_picture_index ++;
	if(cur_picture_index >= picture_total)
	{
		cur_picture_index = 0;
	}

	layout_bg_thumb_dsiplay();
	
	buttom_aimmation_play(true);

	layout_bg_info_label_display();
}



static void layout_bg_right_page_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(layout_bg_right_page_btn_down, layout_bg_right_page_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_RIGHT_PNG);
	lv_obj_t* obj = layout_bg_btn_create(970,307,54,54,&btn_data,&info,true);
	lv_obj_set_style_local_bg_opa(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);	
	lv_obj_set_style_local_bg_color(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(57,57,57));
}


static void cancel_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

static void layout_bg_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(NULL, cancel_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
}

 void * layout_bg_bg_img_set_task(void * arg)
{
	bg_img_set_flg = false;
	char cmd[128];
    if(access(BACKGROUND_FILE_PATH, F_OK) != 0)
	{
		system("mkdir "BACKGROUND_FILE_PATH);
	}
	system("rm -rf " BACKGROUND_FILE_PATH "frame.jpg");
    sprintf(cmd,"cp -v %s " BACKGROUND_FILE_PATH "frame.jpg",(char *)arg);
	printf("cmd is %s\n",cmd);
    system(cmd);
	bg_img_set_flg = true;
	return NULL;
}

static void layout_bg_settinh_fail_task(lv_task_t * task)
{
	lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(),layout_bg_setting_tips_cont_adi_id));
	lv_task_del(task);
}


static void layout_bg_settinh_success_task(lv_task_t * task)
{
	if(bg_img_set_flg)
	{
		goto_layout(pLAYOUT(home));
		return;
	}
}

static void layout_bg_bg_img_set_btn_up(lv_obj_t *obj)
{
    media_info *info = media_info_get(picture_file_type,cur_picture_index);
	bzero(picture_name,sizeof(PICTURE_NAM_LENGTH));
    sprintf(picture_name,"%s%s",SD_PICTURE_PATH,info->file_name);

	
	unsigned int file_size = file_size_get(picture_name);
	if(file_size == -1)
	{
		printf("文件为空\n");
	}
	else if(file_size > 2097152)
	{
            // char *src[language_total] = {"Setting failed!!!\nThe file cannot be larger than 2M","设置失败\n文件不能大于2M","Ошибка настройки\nфайл не может быть больше 2M","La configuration a échoué\nle fichier ne peut pas dépasser 2M",\
            // "¡Configuración fallida!!! El archivo no puede ser mayor de 2M","Einstellung fehlgeschlagen!!!\nDie Datei kann nicht größer als 2M sein","فشل الإعداد ! \n  الملف لا يمكن أن يكون أكبر من 1 متر",\
            // "Nastavení selhalo!!!\nSoubor nemůže být větší než 2M","התקנה נכשלה!!! \n קובצים לא יכולים להיות גדולים יותר מ2M","A configuração falhou!!! \n Os ficheiros não podem ser maiores do que 2M","Impostazione fallita!!! \nIl file non può essere più grande di 2M"};
		layout_customiz_file_set_display_create(layout_bg_setting_tips_cont_adi_id,LAYOUT_BACKGROUND_LANG_SETTINGFAILED_ID,false,false);
		lv_layout_task_create(layout_bg_settinh_fail_task, 2000, LV_TASK_PRIO_MID, NULL);
		
	}
	else
	{

		ak_pthread_t thread_id;
		ak_thread_create(&thread_id, layout_bg_bg_img_set_task, (void *)picture_name, ANYKA_THREAD_NORMAL_STACK_SIZE,-1);
		ak_thread_detach(thread_id);

		// char *src[language_total] = {"Setting","正在设置","Устанавливается","Mise en place","Configuración en curso","Einstellung","الإعداد","Nastavení","מתקן","A Configurar","Impostazioni"};
		layout_customiz_file_set_display_create(layout_bg_setting_tips_cont_adi_id,LAYOUT_BACKGROUND_LANG_SETTING_ID,true,true);
		lv_layout_task_create(layout_bg_settinh_success_task, 1000, LV_TASK_PRIO_MID, NULL);
	}

}

 
static void layout_layout_bg_bg_img_set_btn_create(void)
{
	lv_obj_t* obj = lv_btn_create(lv_scr_act(),NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xff,0xff,0xff));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));
	if(picture_total >0)
	{
		lv_obj_set_x(obj,150);
	}
	else
	{
		lv_obj_set_hidden(obj,true);
	}
	lv_obj_set_y(obj, 440);
	lv_obj_set_size(obj, 300, 70);
	lv_obj_set_style_local_radius(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);	
	
	// char *src[language_total] = {"Set as wallpaper", "设为壁纸", "Установить обои", "Définir comme papier peint", "Como fondo de pantalla", "Als Hintergrundbild festlegen", "تعيين خلفية", "Nastavit jako tapetu", "קבע כנייר קרקע","Definir como papel de parede","Imposta come sfondo"};

	lv_obj_t* label = lv_label_create(lv_scr_act(),NULL);
	lv_label_set_text(label, str_get(LAYOUT_BACKGROUND_LANG_SETWALLPAPER_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_auto_realign(label, true);
	lv_obj_set_style_local_text_color(label, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));

	static btn_data btn_data = btn_data_up_create(layout_bg_bg_img_set_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);

}

 void recovery_system_bg_btn_up()
{
	bzero(picture_name,sizeof(PICTURE_NAM_LENGTH));
	sprintf(picture_name, BACKGROUND_FILE_PATH "system_bg.jpg");

	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, layout_bg_bg_img_set_task, (void *)picture_name, ANYKA_THREAD_NORMAL_STACK_SIZE,-1);
	ak_thread_detach(thread_id);

	// char *src[language_total] = {"Setting","正在设置","Устанавливается","Mise en place","Configuración en curso","Einstellung","الإعداد","Nastavení","מתקן","A Configurar","Impostazioni"};
	layout_customiz_file_set_display_create(layout_bg_setting_tips_cont_adi_id,LAYOUT_BACKGROUND_LANG_SETTING_ID,true,true);
	lv_layout_task_create(layout_bg_settinh_success_task, 1000, LV_TASK_PRIO_MID, NULL);
	

}


static void layout_bg_recovery_system_bg_btn_create(void)
{
	lv_obj_t* obj = lv_btn_create(lv_scr_act(),NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xff,0xff,0xff));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));
	lv_obj_set_y(obj, 440);
	if(picture_total >0)
	{
		lv_obj_set_x(obj,580);
	}
	else
	{
		lv_obj_set_x(obj,362);
	}

	lv_obj_set_size(obj, 300, 70);
	lv_obj_set_style_local_radius(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);	
	
	// char *src[language_total] = {"Restore factory wallpaper", "恢复出厂壁纸", "Восстановление заводских обоев", "Restaurer le fond d'écran d'usine", "Restaurar el papel pintado de fábrica", "Werkshintergrund wiederherstellen", "استعادة خلفية المصنع", "Obnovení tapety z výroby", "תחזיר נייר קרן מפעל","Restaurar o papel de parede de fábrica","Ripristinare lo sfondo di fabbrica"};
	
	lv_obj_t* label = lv_label_create(lv_scr_act(),NULL);
	lv_label_set_text(label, str_get(LAYOUT_BACKGROUND_LANG_RESTOREWALLPAPER_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);
	lv_obj_set_auto_realign(label, true);
	lv_obj_set_style_local_text_color(label, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));

	static btn_data btn_data = btn_data_up_create(recovery_system_bg_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);

}

/*
* arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
* arg2:arg1= 1,arg2参数无意义
*	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
*	   arg1=3,arg2=1:内存正常，2，内存已满
*/
static void layout_bg_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
    if (arg1 == 1)
    {	
    	 goto_layout(pLAYOUT(home));
    }
}



static void LAYOUT_ENETER_FUNC(background)
{
	bg_img_set_flg = false;
	picture_name = (char *)malloc(PICTURE_NAM_LENGTH);
    background_parameter_init();

	if(picture_total > 0)
	{
		layout_media_info_label_create(lv_scr_act(),layout_bg_info_adj_id);
		 if(picture_total > 1)
		{
			dot_animation_param_init(&cur_picture_index,&picture_total);
			buttom_annimation_dot_create();
			layout_bg_left_page_btn_create();//上一页
			layout_bg_right_page_btn_create();//下一页
		}

		layout_layout_bg_bg_img_set_btn_create();
		layout_bg_thumb_dsiplay();
	}

	layout_bg_recovery_system_bg_btn_create();
    layout_bg_cancel_btn_create();

	sdcard_event_register(layout_bg_sd_status_change_callback);



}


static void LAYOUT_QUIT_FUNC(background)
{
    unsigned char * picture_data = picture_data_get();
	// if(picture_name != NULL)
	// {
	// 		free(picture_name);
	// 		picture_name = NULL;
	// }

	if(picture_data == NULL)
	{
		printf("fffffffffff\n");
	}

	// lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    // fb_video_mode_enable(false);
    // static rom_bin_info img = rom_bin_raw_get();
	// rom_bin_raw_init(img, picture_data, 1024, 600);

	// lv_jpg_decode_data(SYSTEM_BG_FILE_PATH, &img, 1024, 600);
	// lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	// lv_disp_set_bg_image(lv_disp_get_default(), &img);
	sdcard_event_register(NULL);


}

CREATE_LAYOUT(background);