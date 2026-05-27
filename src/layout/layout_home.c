#if 0
#include "layout_define.h"
#include "layout_setting_common.h"
#include "leo_api.h"

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LAYOUT_HOME_OUTDOOR_DISABLE_ADJ_ID 1001
#define LAYOUT_HOME_DEVICE_MODE_ADJ_ID 888
#define LAYOUT_HOME_MEDIA_NEQW_ADJ_ID 100
#define LAYOUT_HOME_CH_CONT_BG_ADJ_ID 80
extern int wpa_cli_scan_wifi(bool *continue_flag);
extern bool wpa_cli_wlan_status(bool *continue_flag); 

extern void outdoor_motion_event_register(event_pro_callback handle);
extern void motion_call_extern_func(unsigned long arg1, unsigned long arg2);

static void home_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
    btn_data *pdata = (btn_data *) obj->user_data;
    lv_obj_t * children = (lv_obj_t *) pdata->user_data;
    lv_obj_set_state(children, state);
}

static void home_btn_img_transform_set(lv_obj_t *obj)
{
    lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 256);
    lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 300);

    lv_obj_set_style_local_transition_prop_1(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_ZOOM);
    lv_obj_set_style_local_transition_prop_2(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_STYLE_TRANSFORM_ZOOM);

    lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 200);
    lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 200);

    static lv_anim_path_t path;
    path.cb = lv_anim_path_overshoot, path.user_data = NULL;
    lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &path);
    lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, &path);


    //lv_obj_t* obj_parent = lv_obj_get_parent(obj);
    //lv_obj_set_style_local_bg_opa(obj_parent,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);

}

static lv_obj_t *home_btn_create(int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src, bool bg_color)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);

    if (bg_color == true)
    {
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);
    }
    else
    {
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    }

    lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, img_src);
//	lv_obj_set_click(img,true);

    if (string != NULL)
    {
        lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
		
        lv_label_set_text(label, string);
        lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }

    home_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_IN_TOP_MID, 0, 20);

    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);
    return btn;
}


static lv_obj_t *home_btn_create1(int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src, bool bg_color)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);

    if (bg_color == true)
    {
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);
    }
    else
    {
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    }

    lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, img_src);
//	lv_obj_set_click(img,true);

    if (string != NULL)
    {
        lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
		lv_obj_set_id(label,LAYOUT_HOME_DEVICE_MODE_ADJ_ID);
        lv_label_set_text(label, string);
        lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }

    home_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_IN_TOP_MID, 0, 20);

    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);
    return btn;
}

extern bool net_online_device[DEVICE_TOTAL];
void layout_home_door_dispaly_creat();
static lv_obj_t *monitor_btn= NULL;
static void home_montior_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}

static int outdoor_online_count_get(int * valid_index)
{
    int valid_ch_count = 0;

    for(int i=0 ;i<18;i++)
	{
        if(net_online_device[DEVICE_UNIT_OUTDOOR_1+i] && (outdoor_call_mask_get(DEVICE_UNIT_OUTDOOR_1+i) == 0) && (	monitor_data_busy_get(DEVICE_UNIT_OUTDOOR_1 + i) == 0))
        {
            valid_ch_count ++ ;
            if(valid_index != NULL)
            *valid_index = i;
        }
	}
    return valid_ch_count;
}

static void home_montior_btn_up(lv_obj_t *obj)
{
    int valid_ch_count = 0;
    int valid_index = 0;
    valid_ch_count = outdoor_online_count_get(&valid_index);

    if(valid_ch_count == 1)
    {
        network_cmd_data_init(data);
        data.cmd = NET_COMMON_CMD_SOUND;
        data.arg1 = 3| user_data_get()->user_language << 16;
        data.arg2 = 1;
        data.device = DEVICE_UNIT_OUTDOOR_1+valid_index;
        network_send_cmd_data(&data);
        monitor_channel_set(get_channel_by_outdoor_device(DEVICE_UNIT_OUTDOOR_1 + valid_index));
        monitor_enter_flag_set(MON_ENTER_MANUAL_DOOR);
        goto_layout(pLAYOUT(monitor));//页面跳转
        return ;
    }
    else
    {
        layout_home_door_dispaly_creat();
    }

}


//创建按钮
static void home_monitor_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_montior_btn_down, home_montior_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_MONITOR_PNG);
    // char *language[language_total] = {"Monitor","监控","Монитор","Moniteur","Monitor","Monitor","شاشة","Monitor", "צפייה","Monitor","Monitor"};
   	monitor_btn =  home_btn_create(210, 448, 150, 120, str_get(LAYOUT_HOME_LANG_MONITOR_ID), &btn_data, &info, true);
  	lv_obj_set_click(monitor_btn,false );
    lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_id(img,LAYOUT_HOME_OUTDOOR_DISABLE_ADJ_ID);
	static rom_bin_info img_info = rom_bin_info_get(ROM_RES_HOME_MONITOR_DISABLE_PNG);
	lv_img_set_src(img, &img_info);
	lv_obj_align(img, monitor_btn, LV_ALIGN_IN_TOP_RIGHT, -8, 8);
	//lv_obj_set_hidden(img, true);
}

/* static void home_cctv_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}
 */

/* static void home_cctv_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);

    system_bg_data_backup();
    monitor_channel_set(MON_CH_CCTV_1);
    goto_layout(pLAYOUT(cctv));
} */


/* static void home_cctv_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_cctv_btn_down, home_cctv_btn_up, NULL);
    rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_CCTV_PNG);
    home_btn_create(370, 448, 138, 120, "CCTV", &btn_data, &info, true);
} */


static void home_interphone_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}


static void home_interphone_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);

    goto_layout(pLAYOUT(interphone));
}


static void home_interphone_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_interphone_btn_down, home_interphone_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_INTERCOM_PNG);
// char *language[language_total] = {"Intercom","内线通话","Домофон","Interphone","Interfono","Sprechanlage","الاتصال الداخلي","Interkom", "חיוג למסך נוסף","Intercomunicador","Interfono"};
		
	home_btn_create(364, 448, 150, 120, str_get(LAYOUT_HOME_LANG_INTERCOM_ID), &btn_data, &info, true);
}

static void home_playback_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}


static void home_playback_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);

    extern void playback_thumb_parameter_init(void);
    playback_thumb_parameter_init();
    goto_layout(pLAYOUT(playback));
}

//static lv_obj_t *sd_icon = NULL;

static void home_playback_new_icon_display(void)
{

	lv_obj_t *child = lv_obj_get_child_form_id(lv_scr_act(), 8);
	if(child)
		lv_obj_del(child);			


    int new_total = 0;
    if (is_sdcard_insert() == false)//sd卡拔出
    {
        new_total = media_file_total_get(FILE_TYPE_FLASH_PHOTO, true);
		
		lv_obj_t *sd_icon = lv_img_create(lv_scr_act(), NULL);
		lv_obj_set_id(sd_icon, 8);
	
		static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_NOINSERT_H_PNG);
		lv_img_set_src(sd_icon, &sd_info);
		lv_obj_set_pos(sd_icon, 890, 45);
		
    }
    else if (is_sdcard_insert() == true)//sd卡插入
    {
        new_total = media_file_total_get(FILE_TYPE_SD_MIXED, true);

		lv_obj_t *sd_icon = lv_img_create(lv_scr_act(), NULL);
		lv_obj_set_id(sd_icon, 8);

		struct statfs diskInfo;
	      
	    statfs(SD_BASE_PATH, &diskInfo);
	    unsigned long long blocksize = diskInfo.f_bsize; //每个block里包含的字节数
	   // unsigned long long totalsize = blocksize * diskInfo.f_blocks; //总的字节数，f_blocks为block的数目
	
	      
	    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小
	   // unsigned long long availableDisk = diskInfo.f_bavail * blocksize; //可用空间大小
	   

		if((freeDisk < 100*1024*1024)){
			static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_FULL_H_PNG);
			lv_img_set_src(sd_icon, &sd_info);

		}else if((freeDisk >= 100*1024*1024)){
			static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_INSERT_H_PNG);
			lv_img_set_src(sd_icon, &sd_info);
		}


		lv_obj_set_pos(sd_icon, 890, 45);
    }
	

    lv_obj_t * obj = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_MEDIA_NEQW_ADJ_ID);
    if (obj != NULL)
    {
        lv_obj_set_hidden(obj, new_total ? false : true);
    }
}


static void home_playback_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_playback_btn_down, home_playback_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_PLAYBACK_PNG);
   
	// char *language[language_total] = {"Playback","浏览","Просмотр","Playback","Reproducción","Wiedergabe","عرض التسجيلات","Záznamy", "הקלטות","Reprodução","Riproduzione"};
	lv_obj_t * btn = home_btn_create(518, 448, 150, 120, str_get(LAYOUT_HOME_LANG_PLAYBACK_ID), &btn_data, &info, true);
	
    lv_obj_t * obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_id(obj, LAYOUT_HOME_MEDIA_NEQW_ADJ_ID);
    lv_obj_set_size(obj, 16, 16);
    lv_obj_align(obj, btn, LV_ALIGN_IN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xDD, 0x3D, 0x3D));
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 45);
	

    home_playback_new_icon_display();
	
}

/*
static void home_light_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}

static bool light_status_on = false;

bool is_light_status_on(void)
{
    return light_status_on;
}

void light_status_set(bool is_on)
{
    light_status_on = is_on;
}

//
 static void home_light_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);
    light_status_set(is_light_status_on() ? false : true);

    btn_data *pdata = (btn_data *) obj->user_data;
    lv_obj_t * img = (lv_obj_t *) pdata->user_data;

    if (is_light_status_on() == true)
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_ON_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb2, 0x2d, 0x00));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 15);
    }
    else
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
    }
    

}

 static void home_light_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_light_btn_down, home_light_btn_up, NULL);
    if (is_light_status_on() == true)
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_ON_PNG);
        lv_obj_t * obj = home_btn_create(654, 448, 138, 120, "Light", &btn_data, &info, true);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb2, 0x2d, 0x00));

        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 15);
        lv_obj_set_auto_realign(img, true);
    }
    else
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_PNG);
        lv_obj_t * obj = home_btn_create(656, 448, 138, 120, "Light", &btn_data, &info, true);
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
        lv_obj_set_auto_realign(img, true);
    }
} */



static void home_products_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}


 static void home_products_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);
    goto_layout(pLAYOUT(product_introduction));

}

static void home_products_btn_display()
{
    lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),20);
    lv_obj_t *img =lv_obj_get_child_form_id(lv_scr_act(),21);

    if (is_sdcard_insert() == false)//sd卡拔出
    {

        if((btn != NULL) &&((img != NULL)))	
        {
            lv_obj_set_hidden(btn,true);
            lv_obj_set_hidden(img,true);
            lv_obj_set_click(btn,false);
        }
        else
        {
        }
    }
    else if (is_sdcard_insert() == true)//sd卡插入
    {
        int picture_total = media_file_total_get(FILE_TYPE_SD_PRODUCTS_PICTURE, 0);
        if(picture_total > 0)
        {
            if((btn != NULL) &&((img != NULL)))	
            {
                lv_obj_set_hidden(btn,false);
                lv_obj_set_hidden(img,false);
                lv_obj_set_click(btn,true);
            }

        } else
        {
            lv_obj_set_hidden(btn,true);
            lv_obj_set_hidden(img,true);
            lv_obj_set_click(btn,false);
        }    
    }
	
}

static void home_products_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_products_btn_down, home_products_btn_up, NULL);
    
    rom_bin_info info = rom_bin_info_get(ROM_RES_PRODUCT_PNG);
    lv_obj_t * obj = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_pos(obj, 125, 35);
    lv_obj_set_size(obj, 50, 50);
    lv_obj_set_id(obj,20);

    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));
    lv_obj_set_style_local_radius(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,90);
    lv_obj_set_style_local_radius(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,90);
    
    // lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, &info);
    lv_obj_set_id(img,21);

    btn_data.user_data = img;
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);

    lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 15);
    lv_obj_set_auto_realign(img, true);


    home_products_btn_display();
}




static void home_mode_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}


int is_mode_status_on(void)
{
    return user_data_get()->other.mode;
}

void mode_status_set(void)
{
	if(user_data_get()->other.mode < 2)
		user_data_get()->other.mode++;
	else
		user_data_get()->other.mode = 0;

	user_data_save();
    all_device_mode_sync();
    
}

static void absent_mode_btn_display(lv_task_t * task)
{
    lv_obj_t * obj = task->user_data ;
    if(obj == NULL)
    {
        return ;
    }
    
    home_btn_state_set(obj, LV_STATE_DEFAULT);
  
    btn_data *pdata = (btn_data *) obj->user_data;
    lv_obj_t * img = (lv_obj_t *) pdata->user_data;
	
	lv_obj_t * label = lv_obj_get_child_form_id(lv_scr_act(),LAYOUT_HOME_DEVICE_MODE_ADJ_ID);

    if (is_mode_status_on() == 0)
    {
        // char *language[language_total] = {"Dormancy","休眠模式","Спячка","Dormance","Inactividad","Schlafzustand","وضع السكون","Nerušit", "בית שקט","Não Incomodar","Inattività"};
        static rom_bin_info info = rom_bin_info_get(ROM_RES_SLEEP_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x40, 0));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);

		lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_DORMANCY_ID));
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
		
    }
    else if(is_mode_status_on() == 1)
    {
    	// char *language[language_total] = {"At Home","居家模式","Дома","À la maison","En casa","Zuhause","فى المنزل","Doma", "בבית","Em Casa","In Casa"};
        static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_AT_HOME_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
		lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_ATHOME_ID));
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }   
	else if(is_mode_status_on() == 2)
    {
    	// char *language[language_total] = {"Leave Home","离家模式","Выйти из дома","Quitter la maison","Salir de casa","Zuhause verlassen","خارج المنزل","Mimo domov", "לא בבית","Ausente","Fuori Casa"};
        static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LEAVE_HOME_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);

		lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID));
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }
	
    lv_obj_set_auto_realign(img, true);
}
static void home_mode_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);
  
	mode_status_set();
	
    btn_data *pdata = (btn_data *) obj->user_data;
    lv_obj_t * img = (lv_obj_t *) pdata->user_data;
	
	lv_obj_t * label = lv_obj_get_child_form_id(lv_scr_act(),LAYOUT_HOME_DEVICE_MODE_ADJ_ID);

    if (is_mode_status_on() == 0)
    {
    	        // char *language[language_total] = {"Dormancy","休眠模式","Спячка","Dormance","Inactividad","Schlafzustand","وضع السكون","Nerušit", "בית שקט","Não Incomodar","Inattività"};
    	
        static rom_bin_info info = rom_bin_info_get(ROM_RES_SLEEP_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x40, 0));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);

		lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_DORMANCY_ID));
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
		
    }
    else if(is_mode_status_on() == 1)
    {
    	// char *language[language_total] = {"At Home","居家模式","Дома","À la maison","En casa","Zuhause","فى المنزل","Doma", "בבית","Em Casa","In Casa"};
        static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_AT_HOME_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
		lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_ATHOME_ID));
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }   
	else if(is_mode_status_on() == 2)
    {
    	// char *language[language_total] = {"Leave Home","离家模式","Выйти из дома","Quitter la maison","Salir de casa","Zuhause verlassen","خارج المنزل","Mimo domov", "לא בבית","Ausente","Fuori Casa"};
        static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LEAVE_HOME_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);

		lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID));
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }
	
    lv_obj_set_auto_realign(img, true);

}

static void home_mode_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_mode_btn_down, home_mode_btn_up, NULL);

	lv_obj_t * obj = NULL;	

    if (is_mode_status_on() == 0)
    {
   	 	// char *language[language_total] = {"Dormancy","休眠模式","Спячка","Dormance","Inactividad","Schlafzustand","وضع السكون","Nerušit", "בית שקט","Não Incomodar","Inattività"};
        static rom_bin_info info = rom_bin_info_get(ROM_RES_SLEEP_PNG);
		obj = home_btn_create1(672, 448, 150, 120, str_get(LAYOUT_HOME_LANG_DORMANCY_ID), &btn_data, &info, true);
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x40, 0));
        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
        lv_obj_set_auto_realign(img, true);
    }
   	else if(is_mode_status_on() == 1)
    {
    	// char *language[language_total] = {"At Home","居家模式","Дома","À la maison","En casa","Zuhause","فى المنزل","Doma", "בבית","Em Casa","In Casa"};
        static  rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_AT_HOME_PNG);
        obj = home_btn_create1(672, 448, 150, 120, str_get(LAYOUT_HOME_LANG_ATHOME_ID), &btn_data, &info, true);
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
        lv_obj_set_auto_realign(img, true);
    }
	else if(is_mode_status_on() == 2)
    {
    	// char *language[language_total] = {"Leave Home","离家模式","Выйти из дома","Quitter la maison","Salir de casa","Zuhause verlassen","خارج المنزل","Mimo domov", "לא בבית","Ausente","Fuori Casa"};
        static  rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LEAVE_HOME_PNG);
        obj = home_btn_create1(672, 448, 150, 120, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID), &btn_data, &info, true);
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,lv_color_hex(0x4386d7));

        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
        lv_obj_set_auto_realign(img, true);
    }
    lv_layout_task_create(absent_mode_btn_display, 1000, LV_TASK_PRIO_MID, obj);
   
}


static void home_setting_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}

static void home_setting_btn_up(lv_obj_t *obj)
{
    set_enter_setting_page_which(0);
    home_btn_state_set(obj, LV_STATE_DEFAULT);

    goto_layout(pLAYOUT(setting));
}


static void home_setting_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_setting_btn_down, home_setting_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_SET_PNG);
    home_btn_create(20, 20, 100, 100, NULL, &btn_data, &info, false);
}


static lv_task_t *home_timer_ptask = NULL;
static lv_obj_t *time_obj_group[6] = {NULL};

static void home_time_display(struct tm *time)
{
    static rom_bin_info time_res_group[11] = {rom_bin_info_get(ROM_RES_HOME_0_PNG), rom_bin_info_get(ROM_RES_HOME_1_PNG), rom_bin_info_get(ROM_RES_HOME_2_PNG), rom_bin_info_get(ROM_RES_HOME_3_PNG), rom_bin_info_get(ROM_RES_HOME_4_PNG), rom_bin_info_get(ROM_RES_HOME_5_PNG), rom_bin_info_get(ROM_RES_HOME_6_PNG), rom_bin_info_get(ROM_RES_HOME_7_PNG), rom_bin_info_get(ROM_RES_HOME_8_PNG), rom_bin_info_get(ROM_RES_HOME_9_PNG), rom_bin_info_get(ROM_RES_HOME_DOT_PNG)};

    lv_img_set_src(time_obj_group[0], &time_res_group[time->tm_hour / 10]);
    lv_img_set_src(time_obj_group[1], &time_res_group[time->tm_hour % 10]);

    lv_img_set_src(time_obj_group[2], &time_res_group[10]);

    lv_img_set_src(time_obj_group[3], &time_res_group[time->tm_min / 10]);
    lv_img_set_src(time_obj_group[4], &time_res_group[time->tm_min % 10]);

}


static int home_date_get_week(struct tm *time)
{

    int m = time->tm_mon + 1;
    int y = time->tm_year + 1900;
    int d = time->tm_mday;//+1;
    if (m == 1 || m == 2)
    {
        m += 12;
        y--;
    }
    return (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7 + 1;
}

static void home_date_display(struct tm *tm)
{      
	
// 	char *week_str[language_total][7] = {	{"Monday",    "Tuesday",    "Wednesday",	"Thursday",	 "Friday",   "Saturday",     "Sunday"},
// 											{"周一",	  "周二",  "周三",  "周四",  "周五",  "周六",  "周天"},
// 											{"Понедельник",	  "Вторник", "Среда",		"Четверг", 	 "Пятница",	  "Суббота",	 "Воскресенье"},	
// 											{"Lundi",	  "Mardi",    "Mercredi",	"Jeudi",	 "Vendredi",	  "Samedi",    "Dimanche"},
// 											{"Lunes",    "Martes",    "Miércoles",	"Jueves",	 "Viernes",    "Sábado",    "Domingo"},
											      
// 											{"Montag",	  "Dienstag",    "Mittwoch",	"Donnerstag",	 "Freitag",	  "Samstag",    "Sonntag"},
// 											{"اثنين", "ثلاثاء", "اربعاء", "خميس",  "جمعة",  "سبت",   "احد"},
// 											{"Pondělí" ,"Úterý", "Středa", "Čtvrtek", "Pátek", "Sobota", "Neděle"},
//                                             {"שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת", "ראשון"},
//                                             };

// /*	
// 	char *month_str[language_total][12] = { {"January",    "February",  "March",  "April",  "May",  "June",   "July",    "August",  "September",   "October",  		"November",  	"December"},
// 											{"1月",	       "2月",	    "3月",    "4月",    "5月",  "6月",    "7月",     "8月", 		  "9月",         "10月",	    		"11月",	     	"12月"},
// 											{"Январь",     "Февраль",   "Март",   "Апрель", "Май",  "Июль",   "Июль",    "Август",  "Сентябрь",    "Октябрь",  		"Ноябрь",    	"Декабрь"},
// 											{"Janvier",    "Février",   "Mars",   "Avril",  "Mai",  "Juin",   "Juillet", "Août",    "Septembre",   "Octobre",  		"Novembre",  	"Décembre "},
// 											{"Enero",      "Febrero",   "Marzo",  "Abril",  "Mayo", "Junio",  "Julio",   "Agosto",  "Septiembre",  "Octubre",  		"Noviembre",	"Diciembre"},
// 											{"Januar",     "Februar",   "März",   "April",  "Mai",  "Juni",   "Juli",    "August",  "September",   "Oktober",  		"November",  	"Dezember"},
// 											//{"كانون الثاني","شباط",      "اذار",     "نيسان",  "ايار",  "حزيران", "تموز",     "اب",	"ايلول",	   "تشرين الاول ",	"تشربن الثاني ","كانون الاول "}
// 											{"01","02","03","04","05","06","07","08","09","10","11","12"}};
// */

//     char date_str[100] = {0};
//     int week = home_date_get_week(time) - 1; 
// 	//if(user_data_get()->user_language == language_english)
// 		//sprintf(date_str, "%s, %s %d, %04d", week_str[user_data_get()->user_language][week], month_str[user_data_get()->user_language][time->tm_mon], time->tm_mday, time->tm_year + 1900);

// 	sprintf(date_str, "%04d - %02d - %02d   %s", time->tm_year + 1900, time->tm_mon+1, time->tm_mday, week_str[user_data_get()->user_language][week]);
// 	/*
// 	else if(user_data_get()->user_language == language_chinese)
// 		sprintf(date_str, "%04d年 %s%02d日 %s",  time->tm_year + 1900,month_str[user_data_get()->user_language][time->tm_mon],time->tm_mday, week_str[user_data_get()->user_language][week]);

// 	else if(user_data_get()->user_language == language_russian)
// 			sprintf(date_str, "%s, %d %s %04d года", week_str[user_data_get()->user_language][week], time->tm_mday, month_str[user_data_get()->user_language][time->tm_mon], time->tm_year + 1900);
	
// 	else if(user_data_get()->user_language == language_french)
// 			sprintf(date_str, "%s, Publié le %d %s %04d", week_str[user_data_get()->user_language][week], time->tm_mday, month_str[user_data_get()->user_language][time->tm_mon], time->tm_year + 1900);

// 	else if(user_data_get()->user_language == language_spanish)
// 			sprintf(date_str, "%s, %d de %s de %04d", week_str[user_data_get()->user_language][week], time->tm_mday, month_str[user_data_get()->user_language][time->tm_mon], time->tm_year + 1900);
	
// 	else if(user_data_get()->user_language == language_german)
// 			sprintf(date_str, "%s, Am %d. %s %04d", week_str[user_data_get()->user_language][week], time->tm_mday, month_str[user_data_get()->user_language][time->tm_mon], time->tm_year + 1900);

// 	else if(user_data_get()->user_language == language_arabic)
// 			sprintf(date_str, "%04d-%s-%d  %s", time->tm_year + 1900, month_str[user_data_get()->user_language][time->tm_mon], time->tm_mday,week_str[user_data_get()->user_language][week]   );

// */
// 	lv_label_set_text(time_obj_group[5], date_str);

	// char *week_str_total[language_total][7] = {	{"Monday",    "Tuesday",    "Wednesday",	"Thursday",	 "Friday",   "Saturday",     "Sunday"},
	// 										{"周一",	  "周二",  "周三",  "周四",  "周五",  "周六",  "周天"},
	// 										{"Понедельник",	  "Вторник", "Среда",	"Четверг", 	 "Пятница",	  "Суббота",	 "Воскресенье"},	
	// 										{"Lundi",	  "Mardi",    "Mercredi",	"Jeudi",	 "Vendredi",	  "Samedi",    "Dimanche"},
	// 										{"Lunes",    "Martes",    "Miércoles",	"Jueves",	 "Viernes",    "Sábado",    "Domingo"},
											      
	// 										{"Montag",	  "Dienstag",    "Mittwoch",	"Donnerstag",	 "Freitag",	  "Samstag",    "Sonntag"},
	// 										{"اثنين", "ثلاثاء", "اربعاء", "خميس",  "جمعة",  "سبت",   "احد"},
	// 										{"Pondělí" ,"Úterý", "Středa", "Čtvrtek", "Pátek", "Sobota", "Neděle"},
    //                                         {"שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת", "ראשון"}, 
    //                                         {"Seg ","Ter","Qua","Qui","Sex","Sáb","Dom"},
    //                                         {"Lunedi ","Martedi","Mercoledi","Giovedi","Venerdi","Sabato","Domenica"},
    //                                         };

    int week = home_date_get_week(tm) - 1; 
    char * week_str =  week_str_get(week);

	
	// char *month_str[language_total][12] = { {"January",    "February",  "March",  "April",  "May",  "June",   "July",    "August",  "September",   "October",  		"November",  	"December"},
	// 										{"1月",	       "2月",	    "3月",    "4月",    "5月",  "6月",    "7月",     "8月", 		  "9月",         "10月",	    		"11月",	     	"12月"},
	// 										{"Январь",     "Февраль",   "Март",   "Апрель", "Май",  "Июнь",   "Июль",    "Август",  "Сентябрь",    "Октябрь",  		"Ноябрь",    	"Декабрь"},
	// 										{"Janvier",    "Février",   "Mars",   "Avril",  "Mai",  "Juin",   "Juillet", "Août",    "Septembre",   "Octobre",  		"Novembre",  	"Décembre "},
	// 										{"Enero",      "Febrero",   "Marzo",  "Abril",  "Mayo", "Junio",  "Julio",   "Agosto",  "Septiembre",  "Octubre",  		"Noviembre",	"Diciembre"},
	// 										{"Januar",     "Februar",   "März",   "April",  "Mai",  "Juni",   "Juli",    "August",  "September",   "Oktober",  		"November",  	"Dezember"},
    //                                         {"يناير", "فبراير","مارس","أبريل","مايو","يونيو","يوليو","أغسطس","سبتمبر","اكتوبر","نوفمبر","ديسمبر"},
    //                                         {"Leden","Únor","Březen","Duben","Květen","Červen","Červenec","Srpen","Září","Říjen","Listopad","Prosinec"},
    //                                         {"בינואר","פברואר","מרץ","אפריל","מאי","יוני","יולי","אוגוסט","בספטמבר","אוקטובר","נובמבר","דצמבר"},
    //                                         {"Janeiro","Fevereiro","Março","Abril","Maio","Junho","Julho","Agosto","Setembro","Outubro","Novembro","Dezembro"},
    //                                         {"Gennaio","Febbraio","Marzo","Aprile","Maggio","Giugno","Luglio","Agosto","Settembre","Ottobre","Novembre","Dicembre"}
    //                                         };
    int mon = tm->tm_mon + 1 - 1;	/*HOME_LANG_ID_MONTH_1 */
    char * mon_str = mon_str_get(mon); 
	switch (user_data_get()->user_language)
    {
    case language_english:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %s %d, %04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;
    case language_chinese:
		lv_label_set_text_fmt(time_obj_group[5], "%04d年%d月%d日, %s", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, week_str);
        break;

     case language_russian:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %s, %d,%04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;

     case language_french:
        lv_label_set_text_fmt(time_obj_group[5], "%s %d  %s %04d", week_str,tm->tm_mday,  mon_str ,tm->tm_year + 1900 );
        break;

     case language_spanish:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d-%s-%04d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;

     case language_german:
        lv_label_set_text_fmt(time_obj_group[5], "%s %d  %s %04d", week_str,tm->tm_mday,  mon_str ,tm->tm_year + 1900 );
        break;

     case language_arabic:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %s %d, %04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;

     case language_czech:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s %04d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;

     case language_hebrew:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %s %d, %04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;

     case language_portugal:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d de %s de %04d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;

     case language_italy:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d-%s-%04d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;

     case language_polish:
        lv_label_set_text_fmt(time_obj_group[5],  "%s, %s %d, %04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;
    default:
        break;
    }

}


static void home_time_display_task(struct _lv_task_t *task_t)
{
    time_t seconds = time(NULL);
    struct tm tm = {0};
    localtime_r(&seconds, &tm);
    printf("========year is %d\n",tm.tm_year);
    printf("========mon is %d\n",tm.tm_mon);
    printf("========day is %d\n",tm.tm_mday);
    home_time_display(&tm);
    home_date_display(&tm);

#ifdef LEO_FUNC_TEST
    if(task_t != NULL)
    {
        monitor_channel_set(MON_CH_DOOR_1);
        goto_layout(pLAYOUT(monitor));
    }
#endif
}


static void home_time_touch_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(standby));
}

extern linked_info link_info;

static void home_time_display_create(void)
{
    if (home_timer_ptask != NULL)
    {
        lv_task_del(home_timer_ptask);
    }


    time_obj_group[0] = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_pos(time_obj_group[0], 349, 150);
    lv_obj_set_size(time_obj_group[0], 74, 110);

    time_obj_group[1] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[1], 349 + 74);

    time_obj_group[2] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[2], 349 + 74 * 2);
    lv_obj_set_width(time_obj_group[2], 29);

    time_obj_group[3] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[3], 349 + 74 * 2 + 29);
    time_obj_group[4] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[4], 349 + 74 * 3 + 29);

    time_obj_group[5] = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(time_obj_group[5], LV_LABEL_LONG_CROP);
    lv_obj_set_pos(time_obj_group[5], 310, 270);
    lv_obj_set_size(time_obj_group[5], 378, 49);
	lv_obj_set_style_local_text_color(time_obj_group[5], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));

	
    lv_label_set_align(time_obj_group[5], LV_LABEL_ALIGN_CENTER);
	
    home_timer_ptask = lv_task_create(home_time_display_task, 60000, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(home_timer_ptask);
	
    home_time_display_task(home_timer_ptask);

    lv_obj_t * obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_pos(obj, 349, 150);
    lv_obj_set_size(obj, 74 * 4 + 29, 110);
    static btn_data btn_data = btn_data_up_create(home_time_touch_up);
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);

	if(user_data_get()->wifi.wifi_open_flag){
		wpa_cli_scan_wifi(&user_data_get()->wifi.wifi_open_flag);
		wpa_cli_wlan_status(&user_data_get()->wifi.wifi_open_flag);
	
		if( link_info.completed == 1){
			lv_obj_t *wifi_icon = lv_img_create(lv_scr_act(), NULL);
			lv_obj_set_pos(wifi_icon, 936, 34);
			lv_obj_set_size(wifi_icon,  50, 50);
			static rom_bin_info wifi_info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
			lv_img_set_src(wifi_icon, &wifi_info);
		}
	}
}


/*
* arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
* arg2:arg1= 1,arg2参数无意义
*	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
*	   arg1=3,arg2=1:内存正常，2，内存已满
*/

static void home_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
    if (arg1 == 1)
    {	
        home_playback_new_icon_display();
        home_products_btn_display();
    }
}

#if 0
static void sat_canvas_btn_down(lv_obj_t *obj,lv_event_t event)
{
    static lv_point_t last_point = {0};
   
    lv_point_t point;
    lv_indev_get_point(lv_indev_get_next(NULL),&point);
    if(event == LV_EVENT_PRESSED)
    {
        last_point = point; 
    }
    else if(event == LV_EVENT_PRESSING)
    {
        lv_obj_t* line = lv_obj_get_child_form_id(obj, 0);
        
        static lv_point_t line_points[2] = {{0},{0}};

        line_points[0] = last_point;
        line_points[1] = point;
        lv_line_set_points(line,line_points,2);
       
    }
}
static void sat_canvas_create(void)
{
    lv_obj_t *obj = lv_scr_act();//lv_obj_create(lv_scr_act(), NULL);

    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);

    static btn_data btn_data = btn_data_anything_create(sat_canvas_btn_down);
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);

    lv_obj_set_click(obj,true);

    lv_obj_t* line = lv_line_create(lv_scr_act(),NULL);
    lv_obj_set_id(line,0);

    lv_obj_set_style_local_line_opa(line,LV_LINE_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_obj_set_style_local_line_width(line,LV_LINE_PART_MAIN,LV_STATE_DEFAULT,3);
    lv_obj_set_style_local_line_color(line,LV_LINE_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0xFF,0xFF,0xFF));
}

#endif
static lv_task_t *check_outdoot_ptask = NULL;
static void ch_select_cont_cancel_btn_up(lv_obj_t* obj)
{	
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
	if(cont != NULL)
	{
		lv_obj_del(cont);
	}
}
static lv_obj_t *door_monitoring_btn_create(lv_obj_t * parent,int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src, bool bg_color,unsigned int id)
{
    lv_obj_t * btn = lv_btn_create(parent, NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
	lv_obj_set_id(btn,id);
    if (bg_color == true)
    {
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x32,0xD7,0x4B));
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));


    }
    else
    {
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
    	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));
    }
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);
    lv_obj_t * img = lv_img_create(parent, NULL);
    lv_img_set_src(img, img_src);

	lv_obj_t * img2 = lv_img_create(parent, NULL);
	lv_obj_set_id(img2,id+100);
	static rom_bin_info img_info = rom_bin_info_get(ROM_RES_HOME_MONITOR_DISABLE_PNG);
	lv_img_set_src(img2, &img_info);
    lv_obj_set_hidden(img2,true);
	lv_obj_align(img2, btn, LV_ALIGN_IN_TOP_RIGHT, -8, 8);

//	lv_obj_set_click(img,true);

    if (string != NULL)
    {
        lv_obj_t * label = lv_label_create(parent, NULL);
		
        lv_label_set_text(label, string);
        lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    }

    home_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_IN_TOP_MID, 0, 20);

    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);
    return btn;
}
	
extern  bool net_online_device[DEVICE_TOTAL];

static void layout_home_valid_ch_btn_up(lv_obj_t* obj)
{
	unsigned int id = lv_obj_get_id(obj);
	if(net_online_device[id -3])
	{
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		data.arg1 = 3| user_data_get()->user_language << 16;
		data.device = id -3;
		network_send_cmd_data(&data);
		monitor_channel_set(id - 9);
		monitor_enter_flag_set(MON_ENTER_MANUAL_DOOR);
		goto_layout(pLAYOUT(monitor));//页面跳转
	}

}

static void outdoot_device_online_check_task(struct _lv_task_t *task_t)
{
  //实时动态刷新监控是否在线  
    int valid_ch_count =  outdoor_online_count_get(NULL);
    lv_obj_t*obj = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_OUTDOOR_DISABLE_ADJ_ID);
    if(valid_ch_count == 0)
    {
        lv_obj_set_click(monitor_btn, false);
        lv_obj_set_hidden(obj, false);
        return ;
    }
    else
    {
        lv_obj_set_click(monitor_btn, true);
        lv_obj_set_hidden(obj, true);
    }

//实时动态刷新在线资源的可用性
    lv_obj_t * contbg = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
    if(contbg)
    {
        lv_obj_t * cont = lv_obj_get_child_form_id(contbg, 80);
        if(cont)
        {
            for(int i =0;i<18;i++)
            {
                lv_obj_t * monitor_ch_btn = lv_obj_get_child_form_id(lv_obj_get_child_form_id(cont,0xffffff), i +10);
                if(monitor_ch_btn != NULL)
                {
                    lv_obj_t * obj = lv_obj_get_child_form_id(lv_obj_get_child_form_id(cont,0xffffff), i +10+100);
                    if(obj != NULL)
                    {
                        if((outdoor_call_mask_get(DEVICE_UNIT_OUTDOOR_1 + i) == 0x00) && (monitor_data_busy_get(DEVICE_UNIT_OUTDOOR_1 + i) == false))
                        {
                            lv_obj_set_click(monitor_ch_btn, true);
                            lv_obj_set_hidden(obj, true);
                        }
                        else
                        {
                            lv_obj_set_click(monitor_ch_btn, false);
                            lv_obj_set_hidden(obj, false);
                        }
                    }
                }
            }
        }
    }
}

void layout_home_door_dispaly_creat()
{

	lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(cont_bg,LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
	lv_obj_set_size(cont_bg,1024,600);
	lv_obj_set_pos(cont_bg,0,0);
	lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
	
	lv_obj_t *cont = lv_page_create(cont_bg, NULL);
	lv_obj_set_size(cont,750,360);
	lv_obj_set_pos(cont,137,120);
    lv_obj_set_id(cont,80);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
	lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
	lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);

    int door_num = 0;
    static rom_bin_info info_carm = rom_bin_info_get(ROM_RES_CARM_PNG);
    for(int i=0 ;i<18;i++)
	{
        if(net_online_device[DEVICE_UNIT_OUTDOOR_1+i])
        {
            static btn_data btn_data = btn_data_up_create(layout_home_valid_ch_btn_up);
            // char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür","باب","Dveře","שער","Porta","Porta"};
            char str_num[1] = {0};
            if(i<2)
            {
                sprintf(str_num,"%d",i+1);
            }
            else
            {
                sprintf(str_num,"%d",i-2);
            }
            char *name = (char *) malloc(strlen(str_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
            strcpy(name, str_get(LAYOUT_HOME_LANG_DOOR_ID));
            strcat(name, str_num);
            door_monitoring_btn_create(cont,30 + (door_num)*160, 20, 138, 120, name, &btn_data, &info_carm, false ,i+10);
            door_num ++;
            free(name);	
        }
	}

	lv_obj_t* cancle_btn = lv_btn_create(cont_bg, NULL);
	lv_obj_set_style_local_bg_opa(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_size(cancle_btn, 220, 70);
	lv_obj_align(cancle_btn, cont, LV_ALIGN_IN_BOTTOM_MID, 0,-30);
	
	
	lv_obj_t* label = lv_label_create(cancle_btn,NULL);
	
	// char * src1[language_total] = {"Cancel","取消","Отмена","Annuler","Cancelar","Absagen","الغاء","Zrušit","בוטל","Cancelar","Annulla"};
	lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_CANCEL_ID));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(label, cancle_btn, LV_ALIGN_CENTER, 0,0);
	
	
	static btn_data btn_data = btn_data_up_create(ch_select_cont_cancel_btn_up);
	cancle_btn->user_data = &btn_data;
	btn_touch_event_listen(cancle_btn);
}


static void outdoot_device_online_check_create(void){
    if (check_outdoot_ptask != NULL)
    {
        lv_task_del(check_outdoot_ptask);
    }
	
	check_outdoot_ptask = lv_task_create(outdoot_device_online_check_task, 500, LV_TASK_PRIO_MID, NULL);
	lv_task_ready(check_outdoot_ptask);
	   
	outdoot_device_online_check_task(check_outdoot_ptask);

}

/************************************************************
* @Description: 监控可用资源状态变化回调函数
* @Author: xiaoxiao
* @Date: 2023-02-16 14:39:58
* @param: 
* @explain: 
************************************************************/
void outdoor_status_change_func(unsigned long arg1, unsigned long arg2)
{
    lv_obj_t * bg_cont = lv_obj_get_child_form_id(lv_scr_act(),LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
    if(bg_cont != NULL)
    {
        lv_obj_del(bg_cont);
    }   

}

static void LAYOUT_ENETER_FUNC(home)
{

    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	// system_bg_data_recovery();//背景颜色处理函数
	//sat_canvas_create();return;
    home_monitor_btn_create();//创建监控按钮

   // home_cctv_btn_create();
    home_interphone_btn_create();//创建内线通话的按钮

    home_playback_btn_create();//创建回放的按钮

    home_products_btn_create();

    home_mode_btn_create();//创建模式设置按钮

    home_setting_btn_create();//创建设置按钮

    home_time_display_create();//创建时间显示

    /*进入playback 页面处理*/

    media_thumb_device_close();//关闭媒体设备

    sdcard_event_register(home_sd_status_change_callback);//sd卡状态改变 即有新的照片 显示点

	get_linked_wifi_info(&link_info);
	

	outdoor_motion_event_register(motion_call_extern_func);


    outdoot_device_online_check_create();

    outdoor_status_change_event_register(outdoor_status_change_func);
	
}


static void LAYOUT_QUIT_FUNC(home)
{
//    background_close();
    if (home_timer_ptask != NULL)
    {
        lv_task_del(home_timer_ptask);
        home_timer_ptask = NULL;
    }
	if (check_outdoot_ptask != NULL)
    {
        lv_task_del(check_outdoot_ptask);
        check_outdoot_ptask = NULL;
    }
    sdcard_event_register(NULL);
	outdoor_motion_event_register(NULL);
    outdoor_status_change_event_register(NULL);

//    system_bg_data_backup();
}


CREATE_LAYOUT(home);
#endif

#if 1
#include "layout_define.h"
#include "layout_setting_common.h"
#include "leo_api.h"
#include "../include/tuya/tuya_sdk.h"

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LAYOUT_HOME_OUTDOOR_DISABLE_ADJ_ID 1001
#define LAYOUT_HOME_DEVICE_MODE_ADJ_ID 888
#define LAYOUT_HOME_MEDIA_NEQW_ADJ_ID 100
#define LAYOUT_HOME_CH_CONT_BG_ADJ_ID 80

#define LAYOUT_HOME_WEATHER_IMG 70
#define LAYOUT_HOME_TEMP_LABEL 60
#define LAYOUT_HOME_LH_TEMP_LABEL 50
#define LAYOUT_HOME_LINE 51
#define LAYOUT_HOME_WEATHER_LABEL 52
#define LAYOUT_HOME_PRESSURE_LABEL 53
#define LAYOUT_HOME_PM25_LABEL 54

int current_cctv_online_cannel[8]; // 当前在线的CCTV通道数组

extern int wpa_cli_scan_wifi(bool *continue_flag);    // 扫描WiFi网络
extern bool wpa_cli_wlan_status(bool *continue_flag); // 获取WiFi连接状态

extern void outdoor_motion_event_register(event_pro_callback handle);        // 注册室外移动事件回调
extern void motion_call_extern_func(unsigned long arg1, unsigned long arg2); // 调用外部移动事件处理函数

/**
 * @brief 设置主页按钮状态
 * @param obj 按钮对象
 * @param state 要设置的状态
 * @note 该函数通过用户数据获取按钮相关信息并设置子对象状态
 */
static void home_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
    btn_data *pdata = (btn_data *)obj->user_data;      // 获取按钮用户数据
    lv_obj_t *children = (lv_obj_t *)pdata->user_data; // 获取按钮子对象
    lv_obj_set_state(children, state);                 // 设置子对象状态
}

/**
 * @brief 设置主页按钮图像变换效果
 * @param obj 按钮对象
 * @note 配置按钮在默认状态和按下状态的缩放动画效果
 */
static void home_btn_img_transform_set(lv_obj_t *obj)
{
    // 设置默认状态和按下状态的缩放比例
    lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 256);
    lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 300);

    // 设置过渡属性为缩放变换
    lv_obj_set_style_local_transition_prop_1(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_ZOOM);
    lv_obj_set_style_local_transition_prop_2(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_STYLE_TRANSFORM_ZOOM);

    // 设置过渡时间
    lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 200);
    lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 200);

    // 设置过渡动画路径为过冲效果
    static lv_anim_path_t path;
    path.cb = lv_anim_path_overshoot, path.user_data = NULL;
    lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &path);
    lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, &path);
}
/**
 * @brief 创建主页按钮
 * @param x,y 按钮位置坐标
 * @param w,h 按钮宽度和高度
 * @param string 按钮标签文本
 * @param btn_pdata 按钮数据结构
 * @param img_src 按钮图像资源
 * @param bg_color 是否显示背景颜色
 * @return 创建的按钮对象
 * @note 封装了按钮创建的通用逻辑，包括图像、标签和样式设置
 */
static lv_obj_t *home_btn_create(int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src, bool bg_color)
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
    }
    else
    {
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    }

    lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, img_src);
    //	lv_obj_set_click(img,true);

    if (string != NULL)
    {
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);

        lv_label_set_text(label, string);
        lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
        lv_obj_set_size(label, w, 30);
        lv_label_set_align(label, LV_LAYOUT_CENTER);
        lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    }

    home_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_IN_TOP_MID, 0, 20);

    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);

    return btn;
}

static lv_obj_t *home_btn_create1(int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src, bool bg_color)
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
    }
    else
    {
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    }

    lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, img_src);
    //	lv_obj_set_click(img,true);

    if (string != NULL)
    {
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
        lv_obj_set_id(label, LAYOUT_HOME_DEVICE_MODE_ADJ_ID);
        lv_label_set_text(label, string);
        lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    }

    home_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_IN_TOP_MID, 0, 20);

    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);
    return btn;
}

extern bool net_online_device[DEVICE_TOTAL]; // 外部声明设备在线状态数组
void layout_home_door_dispaly_creat();       // 声明多设备选择布局创建函数
static lv_obj_t *monitor_btn = NULL;         // 静态声明监控按钮对象(初始化为空)

static void home_montior_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED); // 设置按钮状态为按下状态
}

/**
 * @brief 获取在线室外设备数量
 * @param valid_index 用于存储第一个有效设备索引的指针
 * @return 在线设备数量
 * @note 遍历检查室外设备的在线状态、呼叫掩码和忙状态
 */
static int outdoor_online_count_get(int *valid_index)
{
    int valid_ch_count = 0; // 初始化有效设备计数器

    for (int i = 0; i < 18; i++) // 遍历18个室外设备单元
    {
        // 检查设备在线且未被呼叫且数据不忙
        if (net_online_device[DEVICE_UNIT_OUTDOOR_1 + i] &&
            (outdoor_call_mask_get(DEVICE_UNIT_OUTDOOR_1 + i) == 0) &&
            (monitor_data_busy_get(DEVICE_UNIT_OUTDOOR_1 + i) == 0))
        {
            valid_ch_count++; // 有效设备计数加1
            if (valid_index != NULL)
                *valid_index = i; // 记录第一个有效设备的索引
        }
    }
    return valid_ch_count; // 返回有效设备总数
}

/**
 * @brief 监控按钮释放处理函数
 * @param obj 按钮对象
 * @note 根据在线室外设备数量决定操作：单设备直接进入监控，多设备显示选择界面
 */
static void home_montior_btn_up(lv_obj_t *obj)
{
    int valid_ch_count = 0;                                  // 初始化有效设备计数器
    int valid_index = 0;                                     // 初始化有效设备索引
    valid_ch_count = outdoor_online_count_get(&valid_index); // 获取有效设备数量和首个索引

    if (valid_ch_count == 1) // 只有一个有效设备时
    {
        network_cmd_data_init(data);                                                             // 初始化网络命令数据结构
        data.cmd = NET_COMMON_CMD_SOUND;                                                         // 设置命令类型为声音控制
        data.arg1 = 3;                                                                           // 设置声音参数
        data.device = DEVICE_UNIT_OUTDOOR_1 + valid_index;                                       // 设置目标设备ID
        network_send_cmd_data(&data);                                                            // 发送网络命令
        monitor_channel_set(get_channel_by_outdoor_device(DEVICE_UNIT_OUTDOOR_1 + valid_index)); // 设置监控通道
        monitor_enter_flag_set(MON_ENTER_MANUAL_DOOR);                                           // 设置手动进入监控标志
        goto_layout(pLAYOUT(monitor));                                                           // 跳转到监控页面
        return;                                                                                  // 结束函数执行
    }
    else // 有多个有效设备或无有效设备时
    {
        layout_home_door_dispaly_creat(); // 创建多设备选择布局
    }
}

/**
 * @brief 创建监控按钮
 * @note 根据是否存在CCTV设备调整按钮位置，添加状态图标
 */
static void home_monitor_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_montior_btn_down, home_montior_btn_up, NULL); // 初始化按钮事件回调
    static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_MONITOR_ICON_PNG);                 // 获取按钮图标资源

    // 根据CCTV设备数量调整按钮位置
    if (user_data_get()->onvif_dev_count > 0)
    {
        monitor_btn = home_btn_create(136, 448, 150, 120, str_get(LAYOUT_HOME_LANG_MONITOR_ID), &btn_data, &info, true); // 有CCTV时的位置
    }
    else
    {
        monitor_btn = home_btn_create(210, 448, 150, 120, str_get(LAYOUT_HOME_LANG_MONITOR_ID), &btn_data, &info, true); // 无CCTV时的位置
    }

    lv_obj_set_click(monitor_btn, false); // 禁用按钮点击

    // 创建状态图标
    lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_id(img, LAYOUT_HOME_OUTDOOR_DISABLE_ADJ_ID);
    static rom_bin_info img_info = rom_bin_info_get(ROM_RES_HOME_MONITOR_DISABLE_PNG);
    lv_img_set_src(img, &img_info);                                                                        // 设置图标资源
    lv_obj_set_style_local_image_recolor(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF)); // 设置图标颜色
    lv_obj_align(img, monitor_btn, LV_ALIGN_IN_TOP_RIGHT, -8, 8);                                          // 对齐图标位置
    lv_obj_set_style_local_bg_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));      // 设置背景颜色

    // lv_obj_set_hidden(img, true);  // 注释掉的隐藏图标代码
}

/**
 * @brief CCTV按钮按下事件处理
 * @param obj 按钮对象
 * @note 设置按钮为按下状态
 */
static void home_cctv_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED); // 设置按钮状态为按下
}

/**
 * @brief 检测CCTV设备在线状态
 * @return 在线设备数量
 * @note 遍历用户注册的CCTV设备，统计在线数量
 */
static int cctv_online_check()
{
    int online_flag = 0;

    if (user_data_get()->onvif_dev_count < 1) // 检查是否有注册设备
    {
        return 0;
    }

    for (int i = 0; i < user_data_get()->onvif_dev_count; i++) // 遍历所有注册设备
    {
        printf("\n======检测已注册cctv是否在线==========\n");

        if (user_data_get()->onvif_dev[i].cannel == 1) // 检查通道状态(1表示在线)
        {
            online_flag++; // 在线设备计数加1
        }
    }

    return online_flag; // 返回在线设备总数
}

/**
 * @brief CCTV按钮释放事件处理
 * @param obj 按钮对象
 * @note 根据在线CCTV设备数量决定操作：单设备直接进入，多设备显示选择界面，无设备提示错误
 */
static void home_cctv_btn_up(lv_obj_t *obj)
{
    printf("\n=======onvif_dev_count=>%d=========\n", user_data_get()->onvif_dev_count); // 打印注册设备数量
    int online_flag = cctv_online_check();                                               // 获取在线设备数量

    if (online_flag == 1 && user_data_get()->onvif_dev_count == 1) // 只有一个在线设备时
    {
        monitor_channel_set(MON_CH_CCTV_1); // 设置监控通道为CCTV1
        goto_layout(pLAYOUT(cctv));         // 跳转到CCTV页面
    }
    else if (user_data_get()->onvif_dev_count > 1) // 有多个注册设备时
    {
        layout_home_cctv_dispaly_create(); // 创建CCTV选择布局
    }
    else // 无在线设备时
    {
        printf("\n======当前没有cctv在线==========\n");
        lv_obj_t *msg = connect_wifi_cb();                                  // 创建消息框
        lv_msgbox_set_text(msg, str_get(LAYOUT_CCTV_LANG_CCTVNOONLINE_ID)); // 设置消息内容
        lv_msgbox_start_auto_close(msg, 1000);                              // 自动关闭消息框(1秒)
    }
}
// static lv_obj_t *cctv_btn= NULL;

// 创建cctv按钮
static void home_cctv_btn_create(void)
{
    if (user_data_get()->onvif_dev_count > 0) // 检查是否存在已注册的CCTV设备
    {
        static btn_data btn_data = btn_data_create(home_cctv_btn_down, home_cctv_btn_up, NULL); // 创建按钮事件回调（按下/释放/长按）
        // static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_CCTV_PNG);
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_CCTV_ICON_PNG);           // 获取4线制CCTV图标资源
        lv_obj_t *btn = home_btn_create(290, 448, 150, 120, "CCTV", &btn_data, &info, true); // 创建按钮（位置、尺寸、文本、图标、背景色）
    }
}

/* static void home_cctv_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}
 */

/* static void home_cctv_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);

    system_bg_data_backup();
    monitor_channel_set(MON_CH_CCTV_1);
    goto_layout(pLAYOUT(cctv));
} */

/* static void home_cctv_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_cctv_btn_down, home_cctv_btn_up, NULL);
    rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_CCTV_PNG);
    home_btn_create(370, 448, 138, 120, "CCTV", &btn_data, &info, true);
} */

/**
 * @brief 内线电话按钮按下事件处理
 * @param obj 按钮对象
 * @note 设置按钮状态为按下
 */
static void home_interphone_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED); // 设置按钮为按下状态
}

/**
 * @brief 内线电话按钮释放事件处理
 * @param obj 按钮对象
 * @note 设置按钮状态为默认并跳转至内线电话页面
 */
static void home_interphone_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT); // 设置按钮为默认状态
    goto_layout(pLAYOUT(interphone));          // 跳转到内线电话页面
}

/**
 * @brief 创建内线电话按钮
 * @note 根据CCTV设备存在与否调整按钮位置，设置图标和文本
 */
static void home_interphone_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_interphone_btn_down, home_interphone_btn_up, NULL); // 创建按钮事件回调
    static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_TELL_ICON_PNG);                          // 获取4线制通话图标资源
    printf("%x", ROM_RES_HOME_INTERCOM_PNG);                                                            // 打印图标资源地址

    // 根据CCTV设备数量调整按钮位置
    if (user_data_get()->onvif_dev_count > 0)
    {
        home_btn_create(444, 448, 150, 120, str_get(LAYOUT_HOME_LANG_INTERCOM_ID), &btn_data, &info, true); // 有CCTV时的位置
    }
    else
    {
        home_btn_create(364, 448, 150, 120, str_get(LAYOUT_HOME_LANG_INTERCOM_ID), &btn_data, &info, true); // 无CCTV时的位置
    }
}

/**
 * @brief 回放按钮按下事件处理
 * @param obj 按钮对象
 * @note 设置按钮状态为按下
 */
static void home_playback_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED); // 设置按钮为按下状态
}

/**
 * @brief 回放按钮释放事件处理
 * @param obj 按钮对象
 * @note 初始化回放参数并跳转至回放页面
 */
static void home_playback_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT); // 设置按钮为默认状态
    extern void playback_thumb_parameter_init(void);
    playback_thumb_parameter_init(); // 初始化回放缩略图参数
    goto_layout(pLAYOUT(playback));  // 跳转到回放页面
}

/**
 * @brief 显示回放新文件图标
 * @note 根据SD卡状态（插入/拔出/空间不足）显示对应图标，并更新新文件提示
 */
static void home_playback_new_icon_display(void)
{
    // 删除已存在的SD卡图标
    lv_obj_t *child = lv_obj_get_child_form_id(lv_scr_act(), 8);
    if (child)
        lv_obj_del(child);

    int new_total = 0;
    if (is_sdcard_insert() == false)
    {                                                                  // SD卡未插入
        new_total = media_file_total_get(FILE_TYPE_FLASH_PHOTO, true); // 获取闪存照片文件总数

        // 创建SD卡未插入图标
        lv_obj_t *sd_icon = lv_img_create(lv_scr_act(), NULL);
        lv_obj_set_id(sd_icon, 8);
        static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_NOINSERT_H_PNG);
        lv_img_set_src(sd_icon, &sd_info);
        lv_obj_set_pos(sd_icon, 890, 45);
    }
    else if (is_sdcard_insert() == true)
    {                                                               // SD卡已插入
        new_total = media_file_total_get(FILE_TYPE_SD_MIXED, true); // 获取SD卡混合文件总数

        // 创建SD卡状态图标
        lv_obj_t *sd_icon = lv_img_create(lv_scr_act(), NULL);
        lv_obj_set_id(sd_icon, 8);

        struct statfs diskInfo;
        statfs(SD_BASE_PATH, &diskInfo);                            // 获取SD卡文件系统信息
        unsigned long long blocksize = diskInfo.f_bsize;            // 块大小
        unsigned long long freeDisk = diskInfo.f_bfree * blocksize; // 剩余空间

        // 根据剩余空间选择图标（满/正常）
        if (freeDisk < 100 * 1024 * 1024)
        {
            static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_FULL_H_PNG);
            lv_img_set_src(sd_icon, &sd_info);
        }
        else
        {
            static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_INSERT_H_PNG);
            lv_img_set_src(sd_icon, &sd_info);
        }
        lv_obj_set_pos(sd_icon, 890, 45);
    }

    // 更新新文件提示显示状态（有新文件时显示红点）
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_MEDIA_NEQW_ADJ_ID);
    if (obj != NULL)
    {
        lv_obj_set_hidden(obj, new_total ? false : true); // 有新文件时显示，否则隐藏
    }
}

/**
 * @brief 创建回放按钮
 * @note 根据CCTV设备存在与否调整按钮位置，添加新文件提示红点
 */
static void home_playback_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_playback_btn_down, home_playback_btn_up, NULL); // 创建按钮事件回调（按下/释放/长按）
    static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_MEDIA_ICON_PNG);                     // 获取4线制媒体图标资源
    lv_obj_t *btn = NULL;

    // 根据CCTV设备数量调整按钮位置
    if (user_data_get()->onvif_dev_count > 0)
    {
        btn = home_btn_create(598, 448, 150, 120, str_get(LAYOUT_HOME_LANG_PLAYBACK_ID), &btn_data, &info, true); // 有CCTV时的位置
    }
    else
    {
        btn = home_btn_create(518, 448, 150, 120, str_get(LAYOUT_HOME_LANG_PLAYBACK_ID), &btn_data, &info, true); // 无CCTV时的位置
    }

    // 创建新文件提示红点（默认隐藏）
    lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_id(obj, LAYOUT_HOME_MEDIA_NEQW_ADJ_ID);                                                         // 设置唯一ID
    lv_obj_set_size(obj, 16, 16);                                                                              // 设置红点尺寸
    lv_obj_align(obj, btn, LV_ALIGN_IN_TOP_RIGHT, -10, 10);                                                    // 对齐到按钮右上角
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);                      // 设置背景不透明度
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xDD, 0x3D, 0x3D)); // 设置背景颜色为红色
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 45);                                // 设置圆角（完全圆形）

    home_playback_new_icon_display(); // 调用函数更新SD卡状态图标和红点显示状态
}

/*
static void home_light_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}

static bool light_status_on = false;

bool is_light_status_on(void)
{
    return light_status_on;
}

void light_status_set(bool is_on)
{
    light_status_on = is_on;
}

//
 static void home_light_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);
    light_status_set(is_light_status_on() ? false : true);

    btn_data *pdata = (btn_data *) obj->user_data;
    lv_obj_t * img = (lv_obj_t *) pdata->user_data;

    if (is_light_status_on() == true)
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_ON_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb2, 0x2d, 0x00));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 15);
    }
    else
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_PNG);
        lv_img_set_src(img, &info);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
    }


}

 static void home_light_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_light_btn_down, home_light_btn_up, NULL);
    if (is_light_status_on() == true)
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_ON_PNG);
        lv_obj_t * obj = home_btn_create(654, 448, 138, 120, "Light", &btn_data, &info, true);

        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb2, 0x2d, 0x00));

        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 15);
        lv_obj_set_auto_realign(img, true);
    }
    else
    {
        rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_PNG);
        lv_obj_t * obj = home_btn_create(656, 448, 138, 120, "Light", &btn_data, &info, true);
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);
        lv_obj_set_auto_realign(img, true);
    }
} */

/**
 * @brief 产品介绍按钮按下事件处理
 * @param obj 按钮对象
 * @note 设置按钮状态为按下
 */
static void home_products_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED); // 设置按钮为按下状态
}

/**
 * @brief 产品介绍按钮释放事件处理
 * @param obj 按钮对象
 * @note 设置按钮状态为默认并跳转至产品介绍页面
 */
static void home_products_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT);  // 设置按钮为默认状态
    goto_layout(pLAYOUT(product_introduction)); // 跳转到产品介绍页面
}

/**
 * @brief 显示产品介绍按钮状态
 * @note 根据SD卡状态和产品图片数量决定按钮显示/隐藏及可点击性
 */
static void home_products_btn_display()
{
    // 获取按钮和图标对象
    lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 20);
    lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 21);

    if (is_sdcard_insert() == false)
    { // SD卡未插入
        // 隐藏按钮和图标，禁用点击
        if ((btn != NULL) && (img != NULL))
        {
            lv_obj_set_hidden(btn, true);
            lv_obj_set_hidden(img, true);
            lv_obj_set_click(btn, false);
        }
    }
    else if (is_sdcard_insert() == true)
    {                                                                               // SD卡已插入
        int picture_total = media_file_total_get(FILE_TYPE_SD_PRODUCTS_PICTURE, 0); // 获取SD卡产品图片数量
        if (picture_total > 0)
        { // 存在产品图片
            if ((btn != NULL) && (img != NULL))
            {
                lv_obj_set_hidden(btn, false);
                lv_obj_set_hidden(img, false);
                lv_obj_set_click(btn, true); // 显示按钮和图标，启用点击
            }
        }
        else
        { // 无产品图片
            lv_obj_set_hidden(btn, true);
            lv_obj_set_hidden(img, true);
            lv_obj_set_click(btn, false); // 隐藏按钮和图标，禁用点击
        }
    }
}

/**
 * @brief 创建产品介绍按钮
 * @note 设置按钮样式、图标，并根据SD卡状态更新显示
 */
static void home_products_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_products_btn_down, home_products_btn_up, NULL); // 创建按钮事件回调

    rom_bin_info info = rom_bin_info_get(ROM_RES_PRODUCT_PNG); // 获取产品图标资源
    lv_obj_t *obj = lv_btn_create(lv_scr_act(), NULL);         // 创建按钮对象
    lv_obj_set_pos(obj, 125, 35);                              // 设置按钮位置
    lv_obj_set_size(obj, 50, 50);                              // 设置按钮尺寸
    lv_obj_set_id(obj, 20);                                    // 设置按钮ID

    // 设置按钮样式（默认状态和按下状态的背景色、圆角）
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));
    lv_obj_set_style_local_radius(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 90);
    lv_obj_set_style_local_radius(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 90);

    // 创建并设置按钮图标
    lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, &info);
    lv_obj_set_id(img, 21); // 设置图标ID

    // 关联用户数据并监听触摸事件
    btn_data.user_data = img;
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);

    // 对齐图标并启用自动重对齐
    lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 15);
    lv_obj_set_auto_realign(img, true);

    home_products_btn_display(); // 更新按钮显示状态
}

/**
 * @brief 模式按钮按下事件处理
 * @param obj 按钮对象
 * @note 设置按钮状态为按下
 */
static void home_mode_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED); // 设置按钮为按下状态
}

/**
 * @brief 获取当前模式状态
 * @return 当前模式值（0:休眠 1:居家 2:离家）
 * @note 从用户数据中获取模式状态
 */
int is_mode_status_on(void)
{
    return user_data_get()->other.mode; // 返回用户数据中的模式状态
}

/**
 * @brief 设置模式状态（循环切换：休眠->居家->离家->休眠）
 * @note 保存模式状态并同步到所有设备
 */
void mode_status_set(void)
{
    if (user_data_get()->other.mode < 2)
    {
        user_data_get()->other.mode++; // 模式值递增
    }
    else
    {
        user_data_get()->other.mode = 0; // 模式值循环
    }

    user_data_save(); // 保存用户数据

    all_device_mode_sync(); // 同步所有设备模式
}

/**
 * @brief 缺席模式按钮显示任务
 * @param task 任务句柄
 * @note 根据当前模式更新按钮图标、背景色和标签文本，每秒执行一次
 */
static void absent_mode_btn_display(lv_task_t *task)
{
    lv_obj_t *obj = task->user_data; // 获取任务关联的按钮对象
    if (obj == NULL)
    {
        return;
    }

    home_btn_state_set(obj, LV_STATE_DEFAULT); // 设置按钮为默认状态

    btn_data *pdata = (btn_data *)obj->user_data; // 获取按钮用户数据
    lv_obj_t *img = (lv_obj_t *)pdata->user_data; // 获取按钮图标对象

    // 获取模式标签对象
    lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_DEVICE_MODE_ADJ_ID);

    // 根据当前模式更新按钮显示
    if (is_mode_status_on() == 0)
    {
        // 休眠模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_DORMANCY_PNG);
        lv_img_set_src(img, &info); // 设置休眠模式图标

        // 设置按钮背景色为橙色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x40, 0));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);              // 对齐图标
        lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_DORMANCY_ID)); // 设置标签文本
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);        // 对齐标签
    }
    else if (is_mode_status_on() == 1)
    {
        // 居家模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_AT_HOME_PNG);
        lv_img_set_src(img, &info); // 设置居家模式图标

        // 设置按钮背景色为灰色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);            // 对齐图标
        lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_ATHOME_ID)); // 设置标签文本
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);      // 对齐标签
    }
    else if (is_mode_status_on() == 2)
    {
        // 离家模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_LEAVE_HOME_PNG);
        lv_img_set_src(img, &info); // 设置离家模式图标

        // 设置按钮背景色为蓝色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);               // 对齐图标
        lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID)); // 设置标签文本
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);         // 对齐标签
    }

    lv_obj_set_auto_realign(img, true); // 启用图标自动重对齐
}

/**
 * @brief 模式按钮释放事件处理
 * @param obj 按钮对象
 * @note 切换模式状态并更新按钮显示
 */
static void home_mode_btn_up(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_DEFAULT); // 设置按钮为默认状态
    mode_status_set();                         // 切换模式状态

    btn_data *pdata = (btn_data *)obj->user_data; // 获取按钮用户数据
    lv_obj_t *img = (lv_obj_t *)pdata->user_data; // 获取按钮图标对象

    // 获取模式标签对象
    lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_DEVICE_MODE_ADJ_ID);

    // 根据当前模式更新按钮显示
    if (is_mode_status_on() == 0)
    {
        // 休眠模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_DORMANCY_PNG);
        lv_img_set_src(img, &info); // 设置休眠模式图标

        // 设置按钮背景色为橙色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x40, 0));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);              // 对齐图标
        lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_DORMANCY_ID)); // 设置标签文本
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);        // 对齐标签
    }
    else if (is_mode_status_on() == 1)
    {
        // 居家模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_AT_HOME_PNG);
        lv_img_set_src(img, &info); // 设置居家模式图标

        // 设置按钮背景色为灰色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);            // 对齐图标
        lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_ATHOME_ID)); // 设置标签文本
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);      // 对齐标签
    }
    else if (is_mode_status_on() == 2)
    {
        // 离家模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_LEAVE_HOME_PNG);
        lv_img_set_src(img, &info); // 设置离家模式图标

        // 设置按钮背景色为蓝色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));

        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20);               // 对齐图标
        lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID)); // 设置标签文本
        lv_obj_align(label, obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);         // 对齐标签
    }

    lv_obj_set_auto_realign(img, true); // 启用图标自动重对齐
}

/**
 * @brief 居家模式按钮
 * @note 根据当前模式创建按钮，并启动状态更新任务
 */
static void home_mode_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_mode_btn_down, home_mode_btn_up, NULL); // 创建按钮事件回调

    lv_obj_t *obj = NULL;

    // 根据当前模式创建按钮
    if (is_mode_status_on() == 0)
    {
        // 休眠模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_DORMANCY_PNG);
        if (user_data_get()->onvif_dev_count > 0)
        {
            obj = home_btn_create1(752, 448, 150, 120, str_get(LAYOUT_HOME_LANG_DORMANCY_ID), &btn_data, &info, true);
        }
        else
        {
            obj = home_btn_create1(672, 448, 150, 120, str_get(LAYOUT_HOME_LANG_DORMANCY_ID), &btn_data, &info, true);
        }
        // 设置按钮背景色为橙色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x40, 0));
        lv_obj_t *img = (lv_obj_t *)btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20); // 对齐图标
        lv_obj_set_auto_realign(img, true);                 // 启用图标自动重对齐
    }
    else if (is_mode_status_on() == 1)
    {
        // 居家模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_AT_HOME_PNG);
        if (user_data_get()->onvif_dev_count > 0)
        {
            obj = home_btn_create1(752, 448, 150, 120, str_get(LAYOUT_HOME_LANG_ATHOME_ID), &btn_data, &info, true);
        }
        else
        {
            obj = home_btn_create1(672, 448, 150, 120, str_get(LAYOUT_HOME_LANG_ATHOME_ID), &btn_data, &info, true);
        }
        // 设置按钮背景色为灰色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
        lv_obj_t *img = (lv_obj_t *)btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20); // 对齐图标
        lv_obj_set_auto_realign(img, true);                 // 启用图标自动重对齐
    }
    else if (is_mode_status_on() == 2)
    {
        // 离家模式
        static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_LEAVE_HOME_PNG);
        if (user_data_get()->onvif_dev_count > 0)
        {
            obj = home_btn_create1(752, 448, 150, 120, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID), &btn_data, &info, true);
        }
        else
        {
            obj = home_btn_create1(672, 448, 150, 120, str_get(LAYOUT_HOME_LANG_LEAVEHOME_ID), &btn_data, &info, true);
        }
        // 设置按钮背景色为蓝色
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));
        lv_obj_t *img = (lv_obj_t *)btn_data.user_data;
        lv_obj_align(img, obj, LV_ALIGN_IN_TOP_MID, 0, 20); // 对齐图标
        lv_obj_set_auto_realign(img, true);                 // 启用图标自动重对齐
    }

    // 创建定期更新按钮状态的任务（1000ms执行一次）
    lv_layout_task_create(absent_mode_btn_display, 1000, LV_TASK_PRIO_MID, obj);
}
#if 0


void createFolder(char *path, int depth) {  
    if (depth == 0) {  
        char filename[50];  
        FILE *file;  
        for (int i = 0; i < 1000; i++) { // 1000 files per folder  
            snprintf(filename, sizeof(filename), "%s/test.conf", path);  
            file = fopen(filename, "wb");  
            if (file == NULL) {  
                perror("Error opening file");  
                return;  
            }  
            fseek(file, 1024 * 1024, SEEK_SET); // Seek to 1MB  
            fclose(file);  
        }  
    } else {  
        char foldername[50];  
        snprintf(foldername, sizeof(foldername), "%s/%02d", path, depth);  
        mkdir(foldername, 0755); // Create folder with permissions  
        createFolder(foldername, depth - 1); // Recursive call  
    }  
}  


void traverseAndPrint(char *path) {  
    struct dirent *entry;  
    DIR *dir = opendir(path);  
    if (dir == NULL) {  
        perror("Error opening directory");  
        return;  
    }  
    while ((entry = readdir(dir)) != NULL) {  
        if (entry->d_type == DT_DIR) { // Skip if not a directory  
            char subpath[100];  
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);  
            traverseAndPrint(subpath); // Recursive call for subdirectories  
        } else if (entry->d_type == DT_REG && strcmp(entry->d_name, "test.conf") == 0) { // Found a test.conf file  
            printf("Found test.conf at: %s\n", entry->d_name);  
        }  
    }  
    closedir(dir);  
}  

int test_func() {  
    srand(time(NULL)); // Seed random number generator once at the start of the program.  
    for (int i = 2020; i <= 2030; i++) { // Create folders from 2020 to 2030  
        char path[50];  
        snprintf(path, sizeof(path), "/mnt/tf/test/%d", i); //文件夹的名字是2020到2030，例如folder_2020，folder_2021等。  
        mkdir(path, 0755); // Create folder with permissions  
        createFolder(path, 3); // Create files in each folder using createFolder function. 3 is the depth level.   
    }  
    traverseAndPrint("folder_2020"); // Start traversal from folder_2020. You can change this to any other folder.   
    return 0;  
}

#endif

static void home_setting_btn_down(lv_obj_t *obj)
{
    home_btn_state_set(obj, LV_STATE_PRESSED);
}

static void home_setting_btn_up(lv_obj_t *obj)
{
    set_enter_setting_page_which(0);
    home_btn_state_set(obj, LV_STATE_DEFAULT);

#if 0


    // FILE *file = fopen("/mnt/tf/test.conf", "wb");  
    // if (file == NULL) {  
    //     printf("Error opening file\n");  
        
    // }else{
    //      srand(time(NULL));
    //       unsigned char buffer[1024]; // Buffer for random data  
    //     for (int i = 0; i < (1024 * 1024); i++) {  
    //         // Fill the buffer with random data  
    //         for (int j = 0; j < 1024; j++) {  
    //             buffer[j] = rand() % 256; // Generate a random byte  
    //         }  
    //         fwrite(buffer, sizeof(unsigned char), 1024, file); // Write the buffer to the file  
    //     }  
  
    // fclose(file); // Close the file  
    // }


    // for(int i = 1;i<100;i++){
    //     char temp_path[64];
    //     char temp_cp[64];
    //     sprintf(temp_path,"mkdir /mnt/tf/media/test_floder_%d",i);
    //     system(temp_path);
    //     sprintf(temp_cp,"cp /mnt/tf/test.conf /mnt/tf/media/test_floder_%d/",i);
    //     system(temp_cp);
    // }




    FILE *file;  
    long long int start_time = 0, end_time = 0;  
    int count = 0;  
  
    // 获取开始时间戳  
    clock_t start = clock(); // 使用clock()函数获取当前时间戳（以秒为单位）  
    start_time = start;  
    int temp_data = 20;
    // 循环读取文件100次  
    for(int i = 1;i<100;i++){
        char filename[64]; 
        sprintf(filename,"/mnt/tf/media/test_floder_%d/test.conf",i);
        char buffer[200*1024]; // 缓冲区用于存储读取的文本数据  
        // 打开文件  
        file = fopen(filename, "r");  
        if (file == NULL) {  
            printf("Error opening file\n");  
            return 1;  
        }  
            size_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), file); // 读取文件内容到缓冲区中  
            count++; // 增加计数器  
        // 关闭文件  
        fclose(file);  
        }  
    // 获取结束时间戳并计算时间差  
    clock_t end = clock(); // 再次获取当前时间戳  
    end_time = end;  
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC; // 计算经过的时间（以秒为单位）  
  
    // 打印时间戳和时间差  
    printf("Start time: %lld\n", start_time);  
    printf("End time: %lld\n", end_time);  
    printf("Elapsed time: %f seconds\n", elapsed_time);  
    printf("count : %d count\n", count);  
    printf("data : %dk \n", temp_data);

#endif
    // char *temp_ip = "192.168.170.110";
    // char *temp_eth = "wlan0";
    // get_mac_address_by_ip(temp_ip,temp_eth);

    goto_layout(pLAYOUT(setting));
}

/**
 * @brief 创建设置按钮
 * @note 使用指定图标创建设置按钮，不显示文本，背景透明
 */
static void home_setting_btn_create(void)
{
    static btn_data btn_data = btn_data_create(home_setting_btn_down, home_setting_btn_up, NULL); // 创建按钮事件回调
    static rom_bin_info info = rom_bin_info_get(ROM_RES_4_WIRE_SETTING_PNG);                      // 获取4线制设置图标
    home_btn_create(20, 20, 100, 100, NULL, &btn_data, &info, false);                             // 创建按钮（位置、尺寸、无文本、图标、透明背景）
}

static lv_task_t *home_timer_ptask = NULL;   // 时间显示任务句柄
static lv_obj_t *time_obj_group[6] = {NULL}; // 时间显示对象数组

/**
 * @brief 显示当前时间
 * @param time 时间结构指针
 * @note 更新时间显示对象的图标资源
 */
static void home_time_display(struct tm *time)
{
    // 时间数字图标资源数组（0-9和冒号）
    static rom_bin_info time_res_group[11] = {
        rom_bin_info_get(ROM_RES_HOME_HOME_0_PNG), rom_bin_info_get(ROM_RES_HOME_HOME_1_PNG),
        rom_bin_info_get(ROM_RES_HOME_HOME_2_PNG), rom_bin_info_get(ROM_RES_HOME_HOME_3_PNG),
        rom_bin_info_get(ROM_RES_HOME_HOME_4_PNG), rom_bin_info_get(ROM_RES_HOME_HOME_5_PNG),
        rom_bin_info_get(ROM_RES_HOME_HOME_6_PNG), rom_bin_info_get(ROM_RES_HOME_HOME_7_PNG),
        rom_bin_info_get(ROM_RES_HOME_HOME_8_PNG), rom_bin_info_get(ROM_RES_HOME_HOME_9_PNG),
        rom_bin_info_get(ROM_RES_HOME_DOT_PNG)};

    // 设置小时十位图标
    lv_img_set_src(time_obj_group[0], &time_res_group[time->tm_hour / 10]);
    // 设置小时个位图标
    lv_img_set_src(time_obj_group[1], &time_res_group[time->tm_hour % 10]);
    // 设置冒号图标
    lv_img_set_src(time_obj_group[2], &time_res_group[10]);
    // 设置分钟十位图标
    lv_img_set_src(time_obj_group[3], &time_res_group[time->tm_min / 10]);
    // 设置分钟个位图标
    lv_img_set_src(time_obj_group[4], &time_res_group[time->tm_min % 10]);
}

/**
 * @brief 计算日期对应的星期
 * @param time 时间结构指针
 * @return 星期几（1-7）
 * @note 使用蔡勒公式计算星期
 */
static int home_date_get_week(struct tm *time)
{
    int m = time->tm_mon + 1;     // 月份（1-12）
    int y = time->tm_year + 1900; // 年份
    int d = time->tm_mday;        // 日期

    // 调整1月和2月为上一年的13月和14月
    if (m == 1 || m == 2)
    {
        m += 12;
        y--;
    }
    // 蔡勒公式计算星期
    return (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7 + 1;
}

/**
 * @brief 显示当前日期（多语言支持）
 * @param tm 时间结构指针
 * @note 根据用户语言设置显示对应的日期格式
 */
static void home_date_display(struct tm *tm)
{
    int week = home_date_get_week(tm) - 1; // 获取星期索引（0-6）
    char *week_str = week_str_get(week);   // 获取星期字符串

    int mon = tm->tm_mon + 1 - 1;     // 获取月份索引
    char *mon_str = mon_str_get(mon); // 获取月份字符串

    // 根据用户语言设置日期格式
    switch (user_data_get()->user_language)
    {
    case language_english:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %s %d, %04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;
    case language_chinese:
        lv_label_set_text_fmt(time_obj_group[5], "%04d年%d月%d日, %s", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, week_str);
        break;
    case language_russian:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %s, %d,%04d", week_str, mon_str, tm->tm_mday, tm->tm_year + 1900);
        break;
    case language_french:
        lv_label_set_text_fmt(time_obj_group[5], "%s %d %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_spanish:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d de %s de %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_german:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d de %s de %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;    
    case language_arabic:
        lv_label_set_text_fmt(time_obj_group[5], "%s، %d %s ، %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_czech:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d. %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_hebrew:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s, %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_portugal:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d de %s de %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_italy:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_polish:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_greek:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_turkey:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;
    case language_nederlands:
        lv_label_set_text_fmt(time_obj_group[5], "%s, %d %s %d", week_str, tm->tm_mday, mon_str, tm->tm_year + 1900);
        break;

    default:
        break;
    }
}

/**
 * @brief 时间显示任务
 * @param task_t 任务句柄
 * @note 每秒更新一次时间和日期显示
 */
static void home_time_display_task(struct _lv_task_t *task_t)
{
    time_t seconds = time(NULL); // 获取当前时间戳
    struct tm tm = {0};
    localtime_r(&seconds, &tm); // 转换为本地时间结构

    // 打印时间信息
    printf("========year is %d\n", tm.tm_year);
    printf("========mon is %d\n", tm.tm_mon);
    printf("========day is %d\n", tm.tm_mday);

    home_time_display(&tm); // 更新时间显示
    home_date_display(&tm); // 更新日期显示

    // 测试功能（条件编译）
#ifdef LEO_FUNC_TEST
    if (task_t != NULL)
    {
        monitor_channel_set(MON_CH_DOOR_1);
        goto_layout(pLAYOUT(monitor));
    }
#endif
}

/**
 * @brief 时间显示区域触摸释放处理
 * @param obj 触摸对象
 * @note 触摸时间显示区域时跳转至待机页面
 */
static void home_time_touch_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(standby)); // 跳转至待机页面
}

extern linked_info link_info; // 外部声明WiFi连接信息

/**
 * @brief 创建时间显示
 * @note 初始化时间显示对象，启动时间更新任务
 */
static void home_time_display_create(void)
{
    // 删除已存在的时间显示任务
    if (home_timer_ptask != NULL)
    {
        lv_task_del(home_timer_ptask);
    }

    int off_y = 10;
    // 创建小时十位图标
    time_obj_group[0] = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_pos(time_obj_group[0], 349, 150 + off_y);
    lv_obj_set_size(time_obj_group[0], 74, 110);

    // 创建小时个位图标
    time_obj_group[1] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[1], 349 + 74);

    // 创建冒号图标
    time_obj_group[2] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[2], 349 + 74 * 2);
    lv_obj_set_width(time_obj_group[2], 29);

    // 创建分钟十位图标
    time_obj_group[3] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[3], 349 + 74 * 2 + 29);
    // 创建分钟个位图标
    time_obj_group[4] = lv_img_create(lv_scr_act(), time_obj_group[0]);
    lv_obj_set_x(time_obj_group[4], 349 + 74 * 3 + 29);

    // 创建日期标签
    time_obj_group[5] = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(time_obj_group[5], LV_LABEL_LONG_CROP);
    lv_obj_set_pos(time_obj_group[5], 310, 270 + off_y);
    lv_obj_set_size(time_obj_group[5], 430, 49);
    lv_obj_set_style_local_text_color(time_obj_group[5], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    lv_label_set_align(time_obj_group[5], LV_LABEL_ALIGN_CENTER);

    // 创建时间更新任务（60000ms执行一次）
    home_timer_ptask = lv_task_create(home_time_display_task, 60000, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(home_timer_ptask);

    home_time_display_task(home_timer_ptask); // 初始化时间显示

    // 创建时间显示区域触摸对象
    lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_pos(obj, 349, 160);
    lv_obj_set_size(obj, 74 * 4 + 29, 110);
    static btn_data btn_data = btn_data_up_create(home_time_touch_up);
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);

    // 显示WiFi状态图标
    if (user_data_get()->wifi.wifi_open_flag)
    {
        wpa_cli_scan_wifi(&user_data_get()->wifi.wifi_open_flag);
        wpa_cli_wlan_status(&user_data_get()->wifi.wifi_open_flag);

        if (link_info.completed == 1)
        {
            lv_obj_t *wifi_icon = lv_img_create(lv_scr_act(), NULL);
            lv_obj_set_pos(wifi_icon, 936, 34);
            lv_obj_set_size(wifi_icon, 50, 50);
            static rom_bin_info wifi_info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
            lv_img_set_src(wifi_icon, &wifi_info);
        }
    }
}

/*
 * arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
 * arg2:arg1= 1,arg2参数无意义
 *	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
 *	   arg1=3,arg2=1:内存正常，2，内存已满
 */

/**
 * @brief SD卡状态变化回调函数
 * @param arg1 SD卡状态标志（1:状态变化 0:其他）
 * @param arg2 预留参数
 * @note 当SD卡状态发生变化时，更新回放图标和产品按钮显示
 */
static void home_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
    if (arg1 == 1) // SD卡状态发生变化
    {
        home_playback_new_icon_display(); // 更新回放新文件图标显示
        home_products_btn_display();      // 更新产品介绍按钮显示
    }
}

#if 0
static void sat_canvas_btn_down(lv_obj_t *obj,lv_event_t event)
{
    static lv_point_t last_point = {0};
   
    lv_point_t point;
    lv_indev_get_point(lv_indev_get_next(NULL),&point);
    if(event == LV_EVENT_PRESSED)
    {
        last_point = point; 
    }
    else if(event == LV_EVENT_PRESSING)
    {
        lv_obj_t* line = lv_obj_get_child_form_id(obj, 0);
        
        static lv_point_t line_points[2] = {{0},{0}};

        line_points[0] = last_point;
        line_points[1] = point;
        lv_line_set_points(line,line_points,2);
       
    }
}
static void sat_canvas_create(void)
{
    lv_obj_t *obj = lv_scr_act();//lv_obj_create(lv_scr_act(), NULL);

    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);

    static btn_data btn_data = btn_data_anything_create(sat_canvas_btn_down);
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);

    lv_obj_set_click(obj,true);

    lv_obj_t* line = lv_line_create(lv_scr_act(),NULL);
    lv_obj_set_id(line,0);

    lv_obj_set_style_local_line_opa(line,LV_LINE_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_obj_set_style_local_line_width(line,LV_LINE_PART_MAIN,LV_STATE_DEFAULT,3);
    lv_obj_set_style_local_line_color(line,LV_LINE_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0xFF,0xFF,0xFF));
}

#endif
static lv_task_t *check_outdoot_ptask = NULL; // 室外设备在线检查任务句柄

/**
 * @brief 通道选择容器取消按钮释放处理
 * @param obj 按钮对象
 * @note 删除通道选择容器
 */
static void ch_select_cont_cancel_btn_up(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_CH_CONT_BG_ADJ_ID); // 获取通道选择背景容器
    if (cont != NULL)
    {
        lv_obj_del(cont); // 删除容器
    }
}

/**
 * @brief 创建门监控按钮
 * @param parent 父容器对象
 * @param x,y 按钮位置坐标
 * @param w,h 按钮宽度和高度
 * @param string 按钮标签文本
 * @param btn_pdata 按钮数据结构
 * @param img_src 按钮图像资源
 * @param bg_color 是否显示背景颜色
 * @param id 按钮ID
 * @return 创建的按钮对象
 * @note 封装门监控按钮创建逻辑，包括图标、标签和状态指示
 */
static lv_obj_t *door_monitoring_btn_create(lv_obj_t *parent, int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src, bool bg_color, unsigned int id)
{
    lv_obj_t *btn = lv_btn_create(parent, NULL); // 创建按钮
    lv_obj_set_pos(btn, x, y);                   // 设置位置
    lv_obj_set_size(btn, w, h);                  // 设置尺寸
    lv_obj_set_id(btn, id);                      // 设置ID

    // 根据参数设置按钮背景颜色
    if (bg_color == true)
    {
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x32, 0xD7, 0x4B)); // 绿色背景
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF)); // 蓝色按下背景
    }
    else
    {
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));       // 灰色背景
        lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF)); // 蓝色按下背景
    }

    // 设置背景透明度
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

    // 创建并设置按钮图标
    lv_obj_t *img = lv_img_create(parent, NULL);
    lv_img_set_src(img, img_src);

    // 创建状态图标（默认隐藏）
    lv_obj_t *img2 = lv_img_create(parent, NULL);
    lv_obj_set_id(img2, id + 100);
    static rom_bin_info img_info = rom_bin_info_get(ROM_RES_HOME_MONITOR_DISABLE_PNG);
    lv_img_set_src(img2, &img_info);
    lv_obj_set_hidden(img2, true);                         // 初始隐藏
    lv_obj_align(img2, btn, LV_ALIGN_IN_TOP_RIGHT, -8, 8); // 对齐到右上角

    // 如果有标签文本，创建并设置标签
    if (string != NULL)
    {
        lv_obj_t *label = lv_label_create(parent, NULL);
        lv_label_set_text(label, string);
        lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -10); // 对齐到按钮底部
    }

    // 应用按钮图像变换效果并对齐
    home_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_IN_TOP_MID, 0, 20);

    // 设置用户数据并监听触摸事件
    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);

    return btn;
}

extern bool net_online_device[DEVICE_TOTAL]; // 设备在线状态数组

/**
 * @brief 门监控有效通道按钮释放处理
 * @param obj 按钮对象
 * @note 根据按钮ID获取设备并进入监控页面
 */
static void layout_home_valid_ch_btn_up(lv_obj_t *obj)
{
    unsigned int id = lv_obj_get_id(obj); // 获取按钮ID
    if (net_online_device[id - 3])        // 检查设备是否在线
    {
        network_cmd_data_init(data);     // 初始化网络命令
        data.cmd = NET_COMMON_CMD_SOUND; // 设置命令类型为声音
        data.arg1 = 3;                   // 设置声音参数
        data.device = id - 3;            // 设置目标设备ID
        network_send_cmd_data(&data);    // 发送网络命令

        monitor_channel_set(id - 9);                   // 设置监控通道
        monitor_enter_flag_set(MON_ENTER_MANUAL_DOOR); // 设置监控进入标志
        goto_layout(pLAYOUT(monitor));                 // 跳转到监控页面
    }
}

/**
 * @brief CCTV通道选择按钮释放处理
 * @param obj 按钮对象
 * @note 根据按钮ID设置监控通道并跳转
 */
static void layout_home_choice_cctv_btn_up(lv_obj_t *obj)
{
    printf("\n=========id=>%d===========\n", lv_obj_get_id(obj)); // 打印按钮ID（调试）
    switch (lv_obj_get_id(obj))
    {
    case 10:
        monitor_channel_set(MON_CH_CCTV_1);
        break; // 设置CCTV1通道
    case 11:
        monitor_channel_set(MON_CH_CCTV_2);
        break; // 设置CCTV2通道
    case 12:
        monitor_channel_set(MON_CH_CCTV_3);
        break; // 设置CCTV3通道
    case 13:
        monitor_channel_set(MON_CH_CCTV_4);
        break; // 设置CCTV4通道
    case 14:
        monitor_channel_set(MON_CH_CCTV_5);
        break; // 设置CCTV5通道
    case 15:
        monitor_channel_set(MON_CH_CCTV_6);
        break; // 设置CCTV6通道
    case 16:
        monitor_channel_set(MON_CH_CCTV_7);
        break; // 设置CCTV7通道
    case 17:
        monitor_channel_set(MON_CH_CCTV_8);
        break; // 设置CCTV8通道
    default:
        break;
    }
    goto_layout(pLAYOUT(cctv)); // 跳转到CCTV页面
}

/**
 * @brief 室外设备在线检查任务
 * @param task_t 任务句柄
 * @note 实时更新室外设备在线状态和按钮可用性
 */
static void outdoot_device_online_check_task(struct _lv_task_t *task_t)
{
    // 刷新监控设备在线状态
    int valid_ch_count = outdoor_online_count_get(NULL); // 获取在线设备数量
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_OUTDOOR_DISABLE_ADJ_ID);
    if (valid_ch_count == 0)
    {
        lv_obj_set_click(monitor_btn, false); // 无在线设备时禁用监控按钮
        lv_obj_set_hidden(obj, false);        // 显示禁用图标
        return;
    }
    else
    {
        lv_obj_set_click(monitor_btn, true); // 有在线设备时启用监控按钮
        lv_obj_set_hidden(obj, true);        // 隐藏禁用图标
    }

    // 刷新在线资源可用性
    lv_obj_t *contbg = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
    if (contbg)
    {
        lv_obj_t *cont = lv_obj_get_child_form_id(contbg, 80);
        if (cont)
        {
            // 遍历更新每个通道按钮状态
            for (int i = 0; i < 18; i++)
            {
                lv_obj_t *monitor_ch_btn = lv_obj_get_child_form_id(lv_obj_get_child_form_id(cont, 0xffffff), i + 10);
                if (monitor_ch_btn != NULL)
                {
                    lv_obj_t *status_obj = lv_obj_get_child_form_id(lv_obj_get_child_form_id(cont, 0xffffff), i + 10 + 100);
                    if (status_obj != NULL)
                    {
                        // 检查设备是否可使用（未被呼叫且不忙）
                        if ((outdoor_call_mask_get(DEVICE_UNIT_OUTDOOR_1 + i) == 0x00) &&
                            (monitor_data_busy_get(DEVICE_UNIT_OUTDOOR_1 + i) == false))
                        {
                            lv_obj_set_click(monitor_ch_btn, true); // 启用按钮点击
                            lv_obj_set_hidden(status_obj, true);    // 隐藏状态图标
                        }
                        else
                        {
                            lv_obj_set_click(monitor_ch_btn, false); // 禁用按钮点击
                            lv_obj_set_hidden(status_obj, false);    // 显示状态图标
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 创建家庭门监控显示布局
 * @note 生成可选择的门监控设备列表
 */
void layout_home_door_dispaly_creat()
{
    // 创建背景容器
    lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont_bg, LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
    lv_obj_set_size(cont_bg, 1024, 600);
    lv_obj_set_pos(cont_bg, 0, 0);
    lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色背景
    lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);                // 60%透明度

    // 创建内容容器
    lv_obj_t *cont = lv_page_create(cont_bg, NULL);
    lv_obj_set_size(cont, 750, 360);
    lv_obj_set_pos(cont, 137, 120);
    lv_obj_set_id(cont, 80);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色背景
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);               // 不透明
    // 设置边框样式
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);

    int door_num = 0;
    static rom_bin_info info_carm = rom_bin_info_get(ROM_RES_CARM_PNG); // 获取门图标资源
    // 遍历创建在线门设备按钮
    for (int i = 0; i < 18; i++)
    {
        if (net_online_device[DEVICE_UNIT_OUTDOOR_1 + i]) // 检查设备是否在线
        {
            static btn_data btn_data = btn_data_up_create(layout_home_valid_ch_btn_up); // 创建按钮释放回调

            // 生成设备名称（门1、门2等）
            char str_num[1] = {0};
            if (i < 2)
            {
                sprintf(str_num, "%d", i + 1);
            }
            else
            {
                sprintf(str_num, "%d", i - 2);
            }
            char *name = (char *)malloc(strlen(str_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
            strcpy(name, str_get(LAYOUT_HOME_LANG_DOOR_ID));
            strcat(name, str_num);

            // 创建门监控按钮
            door_monitoring_btn_create(cont, 30 + (door_num) * 160, 20, 138, 120, name, &btn_data, &info_carm, false, i + 10);
            door_num++;

            free(name); // 释放内存
            name = NULL;
        }
    }

    // 创建取消按钮
    lv_obj_t *cancle_btn = lv_btn_create(cont_bg, NULL);
    lv_obj_set_style_local_bg_opa(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);                      // 完全不透明
    lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26, 0x26, 0x26)); // 深灰色背景
    lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75)); // 棕黄色按下背景
    lv_obj_set_size(cancle_btn, 220, 70);                                                                             // 设置尺寸
    lv_obj_align(cancle_btn, cont, LV_ALIGN_IN_BOTTOM_MID, 0, -30);                                                   // 对齐到内容容器底部中央

    // 创建取消按钮标签
    lv_obj_t *label = lv_label_create(cancle_btn, NULL);
    lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_CANCEL_ID)); // 设置取消文本
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);              // 居中对齐
    lv_obj_align(label, cancle_btn, LV_ALIGN_CENTER, 0, 0);        // 对齐到按钮中央

    // 设置取消按钮回调
    static btn_data btn_data = btn_data_up_create(ch_select_cont_cancel_btn_up);
    cancle_btn->user_data = &btn_data;
    btn_touch_event_listen(cancle_btn); // 监听触摸事件
}

/**
 * @brief 创建CCTV通道选择窗口
 * @note 生成可选择的CCTV通道列表，显示在线设备
 */
void layout_home_cctv_dispaly_create()
{
    // 创建背景容器
    lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont_bg, LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
    lv_obj_set_size(cont_bg, 1024, 600);
    lv_obj_set_pos(cont_bg, 0, 0);
    lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色背景
    lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);                // 60%透明度

    // 创建内容容器
    lv_obj_t *cont = lv_page_create(cont_bg, NULL);
    lv_obj_set_size(cont, 750, 360);
    lv_obj_set_pos(cont, 137, 120);
    lv_obj_set_id(cont, 80);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色背景
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);               // 不透明
    // 设置边框样式
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);

    int door_num = 0;
    static rom_bin_info info_carm = rom_bin_info_get(ROM_RES_CARM_PNG);        // 获取摄像头图标资源
    memset(current_cctv_online_cannel, 0, sizeof(current_cctv_online_cannel)); // 初始化在线通道数组

    // 遍历创建在线CCTV设备按钮
    for (int i = 0; i < user_data_get()->onvif_dev_count; i++)
    {
        static btn_data btn_data = btn_data_up_create(layout_home_choice_cctv_btn_up); // 创建按钮释放回调

        // 生成CCTV设备名称（CCTV 1、CCTV 2等）
        char str_num[8];
        sprintf(str_num, "CCTV %d", i + 1);

        // 检查设备通道是否可用
        if (user_data_get()->onvif_dev[i].cannel == 1)
        {
            current_cctv_online_cannel[i] = 1; // 标记为在线通道
            // 创建CCTV监控按钮
            door_monitoring_btn_create(cont, 30 + (door_num) * 160, 20, 138, 120, str_num, &btn_data, &info_carm, false, i + 10);
            door_num++;
        }
        printf("\n=online_cannel===%d==>%d==========\n", i, current_cctv_online_cannel[i]); // 打印通道状态（调试）
    }

    // 创建取消按钮
    lv_obj_t *cancle_btn = lv_btn_create(cont_bg, NULL);
    lv_obj_set_style_local_bg_opa(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);                      // 完全不透明
    lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26, 0x26, 0x26)); // 深灰色背景
    lv_obj_set_style_local_bg_color(cancle_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75)); // 棕黄色按下背景
    lv_obj_set_size(cancle_btn, 220, 70);                                                                             // 设置尺寸
    lv_obj_align(cancle_btn, cont, LV_ALIGN_IN_BOTTOM_MID, 0, -30);                                                   // 对齐到内容容器底部中央

    // 创建取消按钮标签
    lv_obj_t *label = lv_label_create(cancle_btn, NULL);
    lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_CANCEL_ID)); // 设置取消文本
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);              // 居中对齐
    lv_obj_align(label, cancle_btn, LV_ALIGN_CENTER, 0, 0);        // 对齐到按钮中央

    // 设置取消按钮回调
    static btn_data btn_data = btn_data_up_create(ch_select_cont_cancel_btn_up);
    cancle_btn->user_data = &btn_data;
    btn_touch_event_listen(cancle_btn); // 监听触摸事件
}

/**
 * @brief 创建室外设备在线检查任务
 * @note 初始化并启动定时检查室外设备在线状态的任务
 */
static void outdoot_device_online_check_create(void)
{
    // 删除已存在的检查任务
    if (check_outdoot_ptask != NULL)
    {
        lv_task_del(check_outdoot_ptask);
    }

    // 创建新的检查任务（每500ms执行一次，中等优先级）
    check_outdoot_ptask = lv_task_create(outdoot_device_online_check_task, 500, LV_TASK_PRIO_MID, NULL);
    lv_task_ready(check_outdoot_ptask); // 立即执行一次任务

    outdoot_device_online_check_task(check_outdoot_ptask); // 手动触发一次任务执行
}

/************************************************************
 * @Description: 监控可用资源状态变化回调函数
 * @Author: xiaoxiao
 * @Date: 2023-02-16 14:39:58
 * @param:
 * @explain:
 ************************************************************/
void outdoor_status_change_func(unsigned long arg1, unsigned long arg2)
{
    // 获取并删除通道选择背景容器
    lv_obj_t *bg_cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_CH_CONT_BG_ADJ_ID);
    if (bg_cont != NULL)
    {
        lv_obj_del(bg_cont);
    }
}

lv_task_t *tuya_weather_get_task = NULL;                                                                            // 天气获取任务句柄
uint8_t find_day[3] = {19, 120, 146};                                                                               // 晴天天气代码
uint8_t rainy_day[18] = {101, 107, 108, 111, 112, 113, 118, 122, 123, 125, 134, 136, 137, 139, 141, 143, 144, 145}; // 雨天天气代码
uint8_t snowy_day[11] = {104, 105, 115, 124, 126, 127, 128, 130, 131, 133, 138};                                    // 雪天天气代码

/**
 * @brief 天气获取任务
 * @param task_t 任务句柄
 * @note 从Tuya API获取天气信息并更新UI显示
 */
static void weather_get_task(struct _lv_task_t *task_t)
{
    // 检查Tuya API是否在线
    if (tuya_api_online_check() == false)
    {
        return;
    }

    // 获取天气图标资源
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_FIND_PNG);   // 晴天图标
    static rom_bin_info info2 = rom_bin_info_get(ROM_RES_HOME_RAINY_PNG);  // 雨天图标
    static rom_bin_info info3 = rom_bin_info_get(ROM_RES_HOME_SNOWY_PNG);  // 雪天图标
    static rom_bin_info info4 = rom_bin_info_get(ROM_RES_HOME_CLOUDY_PNG); // 阴天图标

    // 获取UI元素
    lv_obj_t *line = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_LINE);               // 分隔线
    lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_WEATHER_IMG);         // 天气图标
    lv_obj_t *wet_label = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_WEATHER_LABEL); // 天气标签
    lv_obj_t *temp = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_TEMP_LABEL);         // 温度标签
    lv_obj_t *lh_temp = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_LH_TEMP_LABEL);   // 高低温标签
    lv_obj_t *pm25_label = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_HOME_PM25_LABEL);   // PM2.5标签

    // 获取天气数据
    tuya_api_weather tuya_weather = tuya_weather_get();

    // 如果获取到有效天气数据
    if (tuya_weather.condition != 0)
    {
        // 调整时间显示位置（为天气显示腾出空间）
        int off_x = 150 - 40;
        lv_obj_set_pos(time_obj_group[0], 349 - off_x, 160);
        lv_obj_set_x(time_obj_group[1], 349 - off_x + 74);
        lv_obj_set_x(time_obj_group[2], 349 - off_x + 74 * 2);
        lv_obj_set_x(time_obj_group[3], 349 - off_x + 74 * 2 + 29);
        lv_obj_set_x(time_obj_group[4], 349 - off_x + 74 * 3 + 29);
        lv_obj_set_pos(time_obj_group[5], 310 - off_x, 270);

        // 显示天气相关UI元素
        lv_obj_set_hidden(line, false);
        lv_obj_set_hidden(pm25_label, false);
        lv_obj_set_hidden(img, false);
        lv_obj_set_hidden(temp, false);
        lv_obj_set_hidden(lh_temp, false);
        lv_obj_set_hidden(wet_label, false);
    }

    uint8_t weather = 0; // 1:晴天， 2：雨天， 3：雪天， 4：阴天或其他
    uint8_t i = 0;

    // 判断天气类型
    for (i = 0; i < 3; i++)
    {
        if (tuya_weather.condition == find_day[i])
        {
            weather = 1; // 晴天
            goto end;
        }
    }

    for (i = 0; i < 18; i++)
    {
        if (tuya_weather.condition == rainy_day[i])
        {
            weather = 2; // 雨天
            goto end;
        }
    }

    for (i = 0; i < 11; i++)
    {
        if (tuya_weather.condition == snowy_day[i])
        {
            weather = 3; // 雪天
            goto end;
        }
    }

    weather = 4; // 阴天或其他

end:
    // 根据天气类型更新UI显示
    if (weather == 1) // 晴天
    {
        lv_img_set_src(img, &info1);                                                                    // 设置晴天图标
        lv_label_set_text_fmt(wet_label, "%s  %dhPa", str_get(LAYOUT_SUNNY_ID), tuya_weather.pressure); // 设置天气标签（晴天+气压）
    }
    else if (weather == 2) // 雨天
    {
        lv_img_set_src(img, &info2);                                                                   // 设置雨天图标
        lv_label_set_text_fmt(wet_label, "%s  %dhPa", str_get(LAYOUT_RAIN_ID), tuya_weather.pressure); // 设置天气标签（雨天+气压）
    }
    else if (weather == 3) // 雪天
    {
        lv_img_set_src(img, &info3);                                                                   // 设置雪天图标
        lv_label_set_text_fmt(wet_label, "%s  %dhPa", str_get(LAYOUT_SNOW_ID), tuya_weather.pressure); // 设置天气标签（雪天+气压）
    }
    else if (weather == 4) // 阴天或其他
    {
        lv_img_set_src(img, &info4);                                                                    // 设置阴天图标
        lv_label_set_text_fmt(wet_label, "%s  %dhPa", str_get(LAYOUT_CLOUD_ID), tuya_weather.pressure); // 设置天气标签（阴天+气压）
    }

    // 更新温度、高低温和PM2.5显示
    lv_label_set_text_fmt(temp, "%d℃", tuya_weather.temp);                              // 当前温度
    lv_label_set_text_fmt(lh_temp, "%d℃ ~ %d℃", tuya_weather.tlow, tuya_weather.thigh); // 最低温度~最高温度
    lv_label_set_text_fmt(pm25_label, "PM2.5: %d", tuya_weather.pm25);                  // PM2.5值
}

/**
 * @brief 创建天气显示界面元素
 * @note 初始化天气相关UI组件，包括分隔线、温度显示、天气图标等
 */
void weather_display_create()
{
    int x = 80;

    // 创建分隔线
    lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj, 2, 93);
    lv_obj_set_pos(obj, 520 + x, 190);
    lv_obj_set_id(obj, LAYOUT_HOME_LINE);
    lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C)); // 设置线条颜色
    lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);             // 完全不透明
    lv_obj_set_hidden(obj, true);                                                                     // 初始隐藏

    // 创建当前温度标签
    lv_obj_t *temp_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_id(temp_label, LAYOUT_HOME_TEMP_LABEL);
    lv_obj_set_pos(temp_label, 550 + x, 152 + 7);
    lv_label_set_text(temp_label, "");
    lv_obj_set_style_local_text_font(temp_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(36));           // 设置字体大小
    lv_obj_set_style_local_text_color(temp_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C)); // 设置文本颜色
    lv_obj_set_hidden(temp_label, true);                                                                         // 初始隐藏

    // 创建PM2.5标签
    lv_obj_t *pm25_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_id(pm25_label, LAYOUT_HOME_PM25_LABEL);
    lv_obj_set_pos(pm25_label, 550 + x, 185 + 20);
    lv_label_set_text(pm25_label, "");
    lv_obj_set_style_local_text_font(pm25_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(pm25_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    lv_obj_set_hidden(pm25_label, true); // 初始隐藏

    // 创建天气图标
    lv_obj_t *weather_img = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_size(weather_img, 72, 70);
    lv_obj_set_pos(weather_img, 645 + x, 170 - 5);
    lv_obj_set_id(weather_img, LAYOUT_HOME_WEATHER_IMG);
    lv_obj_set_hidden(weather_img, true); // 初始隐藏

    // 创建天气标签
    lv_obj_t *wet_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_id(wet_label, LAYOUT_HOME_WEATHER_LABEL);
    lv_obj_set_pos(wet_label, 550 + x, 218 + 20);
    lv_label_set_text(wet_label, "");
    lv_obj_set_style_local_text_font(wet_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(wet_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    lv_obj_set_hidden(wet_label, true); // 初始隐藏

    // 创建高低温标签
    lv_obj_t *lh_temp_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_id(lh_temp_label, LAYOUT_HOME_LH_TEMP_LABEL);
    lv_obj_set_pos(lh_temp_label, 550 + x, 251 + 20);
    lv_label_set_text(lh_temp_label, "");
    lv_obj_set_style_local_text_font(lh_temp_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(lh_temp_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
    lv_obj_set_hidden(lh_temp_label, true); // 初始隐藏

    // 创建天气获取任务（每1000ms执行一次，中等优先级）
    tuya_weather_get_task = lv_task_create(weather_get_task, 1000, LV_TASK_PRIO_MID, NULL);
}

/**
 * @brief 创建Dosa图像显示
 * @note 在指定位置显示Dosa图像
 */
void dosa_image_create()
{
    lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_size(img, 800, 200);
    lv_obj_set_pos(img, 122, 150);
    static rom_bin_info img_info = rom_bin_info_get(ROM_RES_ZOSA_PNG);
    lv_img_set_src(img, &img_info); // 设置图像资源
}

lv_task_t *door_version_get_task = NULL; // 门版本获取任务句柄

/**
 * @brief 获取门版本信息任务
 * @param task_t 任务句柄
 * @note 发送网络命令获取门设备版本信息
 */
static void get_door_version_task(struct _lv_task_t *task_t)
{
    network_cmd_data_init(data);            // 初始化网络命令数据
    data.cmd = NET_COMMON_CMD_DOOR_VERSION; // 设置命令类型为获取门版本
    data.arg1 = 0;
    data.device = DEVICE_GROUP;   // 设置目标设备为设备组
    network_send_cmd_data(&data); // 发送网络命令
}

/**
 * @brief 进入home布局的回调函数
 * @note 初始化home界面所有元素，注册事件，创建任务
 */
static void LAYOUT_ENETER_FUNC(home)
{
    background_open();                                                                                // 打开背景
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP); // 设置背景透明

    // 创建各类功能按钮
    home_monitor_btn_create();    // 创建监控按钮
    home_cctv_btn_create();       // 创建CCTV监控按钮
    home_interphone_btn_create(); // 创建内线通话按钮
    home_playback_btn_create();   // 创建回放按钮
    home_products_btn_create();   // 创建产品介绍按钮
    home_mode_btn_create();       // 创建模式设置按钮
    home_setting_btn_create();    // 创建设置按钮

    home_time_display_create(); // 创建时间显示

    // 媒体设备相关操作
    media_thumb_device_close(); // 关闭媒体设备

    // 注册SD卡状态变化回调
    sdcard_event_register(home_sd_status_change_callback); // SD卡状态改变时更新界面

    // 获取WiFi连接信息
    get_linked_wifi_info(&link_info);

    // 注册外部事件回调
    outdoor_motion_event_register(motion_call_extern_func);           // 室外移动事件回调
    outdoot_device_online_check_create();                             // 创建室外设备在线检查任务
    outdoor_status_change_event_register(outdoor_status_change_func); // 室外状态变化回调

    weather_display_create(); // 创建天气显示

    // 创建获取门版本任务（每1000ms执行一次，中等优先级）
    door_version_get_task = lv_task_create(get_door_version_task, 1000, LV_TASK_PRIO_MID, NULL);
}

/**
 * @brief 退出home布局的回调函数
 * @note 清理home界面资源，删除任务，注销事件
 */
static void LAYOUT_QUIT_FUNC(home)
{
    background_close(); // 关闭背景

    // 删除各类任务
    if (home_timer_ptask != NULL)
    {
        lv_task_del(home_timer_ptask);
        home_timer_ptask = NULL;
    }
    if (check_outdoot_ptask != NULL)
    {
        lv_task_del(check_outdoot_ptask);
        check_outdoot_ptask = NULL;
    }
    if (tuya_weather_get_task != NULL)
    {
        lv_task_del(tuya_weather_get_task);
        tuya_weather_get_task = NULL;
    }
    if (door_version_get_task != NULL)
    {
        lv_task_del(door_version_get_task);
        door_version_get_task = NULL;
    }

    // 注销事件回调
    sdcard_event_register(NULL);
    outdoor_motion_event_register(NULL);
    outdoor_status_change_event_register(NULL);
}

CREATE_LAYOUT(home); // 创建home布局

#endif
