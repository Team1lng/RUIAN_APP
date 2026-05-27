#include "layout_define.h"



extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
static void layout_ring_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	
    lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	// char *str[language_total] = {"Set ring", "铃声设置", "Звенеть", "La cloche", "Tintineo", "Ring", "ضبط الرنين", "Nastavit zvonění", "תכנות צלצול","Definir Toque","Imposta Suoneria"};
	lv_label_set_text(obj, str_get(LAYOUT_RING_LANG_SETRING_ID));
	
}

static void layout_ring_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void layout_ring_back_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(setting));
	
}


static lv_obj_t* layout_ring_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(layout_ring_back_btn_down, layout_ring_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}


static lv_obj_t* layout_ring_btn_create(lv_obj_t *parent, int x, int y, int w, int h, const char * src,bool en)
{
	lv_obj_t *cont = lv_cont_create(parent, NULL);
	lv_obj_set_pos(cont, x, y);
	lv_obj_set_size(cont, w, h);
	
	lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));

    if(en)
    {   
        lv_obj_set_style_local_border_side(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	    lv_obj_set_style_local_border_width(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	    lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    }

	lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	
	lv_obj_t *label = lv_label_create(cont, NULL);



    lv_label_set_long_mode(label,LV_LABEL_LONG_CROP);
	lv_obj_set_size(label,500,30);
	lv_label_set_text(label, src);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, cont, LV_ALIGN_IN_LEFT_MID, 0, 0);
	
	return cont;
}



static void custom_ring_btn_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	if(event == LV_EVENT_CLICKED )
	{
        goto_layout(pLAYOUT(customize_ring));
    }


}


static void screensaver_btnmatrix_up(lv_obj_t* obj,lv_event_t event)
{
	if(event == LV_EVENT_CLICKED){
        uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
        if (btn_id == LV_BTNMATRIX_BTN_NONE)
        return;
	
		if (lv_btnmatrix_get_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_DISABLED))
		return;

        lv_obj_t * btnm = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),1),1);
        lv_obj_t * btnm2 = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),1),0xff),0xffffff),1);
        if(btnm == obj)
        {
           	lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECK_STATE);
		    lv_btnmatrix_set_btn_ctrl(btnm, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
            if(btnm2 != NULL)
            {
                lv_btnmatrix_clear_btn_ctrl_all(btnm2, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            user_data_get()->audio.door1_ring = btn_id +6;
        }
        else
        {
            if(btnm != NULL)
            {
                lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            lv_btnmatrix_clear_btn_ctrl_all(btnm2, LV_BTNMATRIX_CTRL_CHECK_STATE);
		    lv_btnmatrix_set_btn_ctrl(btnm2, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
            user_data_get()->audio.door1_ring = btn_id;
        }
		user_data_save();
        door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, NULL);

	}
}
static void layout_ring_page_create(lv_obj_t *parent)
{
	//创建容器
	lv_obj_t *ring_cont = lv_cont_create(parent,NULL);
    lv_obj_set_pos(ring_cont, 0, 100);
	lv_obj_set_size(ring_cont, 1024, 500);
    lv_obj_set_id(ring_cont,1);
	lv_cont_set_layout(ring_cont, LV_LAYOUT_OFF);

	lv_obj_set_style_local_bg_color(ring_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(ring_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

	//创建按钮
	// const char *str1[language_total] = {"Custom ringtones","自定义铃声","Настроить колокольчик","Sonnerie personnalisée","Timbre personalizado","Klingeltöne anpassen","نغمات مخصصة","Upravit vyzváněcí tóny","תואם צלצולים","Toques Personalizados","Suonerie personalizzate"};
	lv_obj_t *cont1 = layout_ring_btn_create(ring_cont, 70, 0 ,877, 69,str_get(LAYOUT_RING_LANG_CUSTOMRINGTONES_ID),true);
	cont1->event_cb = custom_ring_btn_cb;

    static rom_bin_info info_off = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
    static rom_bin_info info_on = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);

    if(access(LOCAL_RING7_FILE_PATH,F_OK) == 0)
    {
        printf("========LOCAL_RING7_FILE_PATH=======\n",LOCAL_RING7_FILE_PATH);
        //创建按钮
        // const char *str2[language_total] = {"Local ringtone","本地铃声","Местный колокольчик","Une version de logicielSonnerie locale","Timbre local","Lokaler Klingelton","النغمات المحلية","Místní vyzvánění", "צלצול מקומי","Toque Local","Suoneria locale"};
        layout_ring_btn_create(ring_cont, 70, 70 ,877, 69,str_get(LAYOUT_RING_LANG_LOCALRINGTONES_ID),true);
        lv_obj_t *btnm = lv_btnmatrix_create(ring_cont, NULL);
        lv_obj_set_id(btnm,1);

        lv_obj_set_style_local_bg_opa(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);
        lv_obj_set_style_local_pattern_image(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off);

        lv_obj_set_style_local_pattern_image(btnm, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on);

            // static const char *map[language_total][12] = {
            //     {"Ring 7",""},
            //     {"铃声 7", ""},
            //     {"Звонок 7",""},
            //     {"Anneau 7",""},
            //     {
            //         "Anillo 7",""
            //     },
            //     {"Ring 7",""},
            //     {"رنين 7",""},
            //     {"Zvonění 7",""},
            //     {"7",""},
            //     {"Toque 7",""},
            //     {"Suoneria 7", ""}
            // };
                
        lv_btnmatrix_set_map(btnm, btnmatrix7_str_get());
        lv_btnmatrix_set_align(btnm, LV_LABEL_ALIGN_LEFT, 70, 0);
        lv_obj_set_x(btnm, 70);
        lv_obj_set_y(btnm, 140);
        lv_obj_set_width(btnm, 1024);
        lv_obj_set_height(btnm, 90);

        static btn_data btn_func = btn_data_anything_create(screensaver_btnmatrix_up);
        btnm->user_data = &btn_func;
        btn_touch_event_listen(btnm);

        lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECK_STATE);
        if(user_data_get()->audio.door1_ring == 6)
        {
            lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);
        }
    }



    // const char *str3[language_total] = {"System ring","系统铃声","Системный звонок","Sonnerie du système","Timbre del sistema","Systemglocke","نظام الرنين","Systémové vyzvánění", "פעמון מערכת","Toque do Sistema","suoneria di sistema"};
	layout_ring_btn_create(ring_cont, 70, (access(LOCAL_RING7_FILE_PATH,F_OK) == 0)?230 :70,877, 69,str_get(LAYOUT_RING_LANG_SYSTEMRINGTONES_ID),true);

    lv_obj_t *page = lv_page_create(ring_cont, NULL);
	lv_obj_set_size(page,1024,(access(LOCAL_RING7_FILE_PATH,F_OK) == 0)?200:360);
	lv_obj_set_pos(page,0,(access(LOCAL_RING7_FILE_PATH,F_OK) == 0)?300:140);
    lv_obj_set_id(page,0xff);
    lv_page_set_scrollable_fit4(page, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_MAX, LV_FIT_MAX);
    lv_cont_set_fit(page, LV_FIT_NONE);
    lv_page_set_scrollbar_mode(page,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_local_pad_top(page, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,10);

    // int x = 0;
    // int y = -120;
    // for(int i=1;i<=6;i++)
    // {
    //     x = 0;
    //     y += 100;
    //     char str_ring[32] ;
    //     const char *ring[language_total] = {"Ring", "Zvonění", "Zvonenie", "Csengő", "Anill ", "Ring","رنين","Zvonění","טבעות"};

    //     sprintf(str_ring, "%s %d",ring[user_data_get()->user_language],i);

    //     lv_obj_t * obj = layout_ring_btn_create(page, x+70, y ,877, 99, str_ring,true);
    //     lv_page_glue_obj(obj,true);
    // }

    	lv_obj_t *btnm2 = lv_btnmatrix_create(page, NULL);

        lv_page_glue_obj(btnm2,true);
        // lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        // lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
        // lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_COVER);
        // lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
        
        // lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        // lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_BORDER_SIDE_BOTTOM);

        // lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
        // lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_make(0, 0, 0));
        
        // lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
        // lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 1);




        // static const char *map2[language_total][12] = {
        //     {"Ring 1", "\n", "Ring 2",  "\n","Ring 3", "\n", "Ring 4",  "\n","Ring 5", "\n","Ring 6",""},
        //     {"铃声 1", "\n", "铃声 2",  "\n","铃声 3", "\n", "铃声 4",  "\n","铃声 5", "\n","铃声 6",""},
		//     {"Звонок 1", "\n", "Звонок 2", "\n", "Звонок 3", "\n", "Звонок 4", "\n", "Звонок 5", "\n","Звонок 6",""},
        //     {"Anneau 1","\n", "Anneau 2","\n", "Anneau 3","\n", "Anneau 4","\n", "Anneau 5","\n", "Anneau 6",""},
        //     {
        //         "Anillo 1","\n",
        //         "Anillo 2","\n",
        //         "Anillo 3","\n",
        //         "Anillo 4","\n",
        //         "Anillo 5","\n",
        //         "Anillo 6",""
        //     },
        //     {"Ring 1","\n", "Ring 2","\n", "Ring 3","\n", "Ring 4","\n", "Ring 5","\n", "Ring 6",""},
        //     {"رنين 1","\n", "رنين 2", "\n","رنين 3","\n", "رنين 4", "\n","رنين 5","\n", "رنين 6",""},
        //     {"Zvonění 1","\n", "Zvonění 2","\n", "Zvonění 3","\n", "Zvonění 4","\n", "Zvonění 5","\n", "Zvonění 6",""},
        //     {"1","\n", "2","\n", "3","\n", "4","\n", "5", "\n","6",""},
        //     {"Toque 1", "\n", "Toque 2",  "\n","Toque 3", "\n", "Toque 4",  "\n","Toque 5", "\n","Toque 6",""},
        //     {"Suoneria 1", "\n", "Suoneria 2",  "\n","Suoneria 3", "\n", "Suoneria 4",  "\n","Suoneria 5", "\n","Suoneria 6",""},
        // };
        lv_btnmatrix_set_map(btnm2, btnmatrix16_str_get());
        lv_btnmatrix_set_align(btnm2, LV_LABEL_ALIGN_LEFT, 70, 0);
        lv_obj_set_x(btnm2, 40);
        lv_obj_set_y(btnm2, 0);
        lv_obj_set_id(btnm2,1);
        lv_obj_set_width(btnm2, 1024);
        lv_obj_set_height(btnm2, 450);

        lv_obj_set_style_local_bg_opa(btnm2, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);

        lv_obj_set_style_local_pattern_image(btnm2, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off);

    
        lv_obj_set_style_local_pattern_image(btnm2, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on);

        static btn_data btn_data = btn_data_anything_create(screensaver_btnmatrix_up);
        btnm2->user_data = &btn_data;
        btn_touch_event_listen(btnm2);


        lv_btnmatrix_clear_btn_ctrl_all(btnm2, LV_BTNMATRIX_CTRL_CHECK_STATE);
        lv_btnmatrix_set_btn_ctrl(btnm2, user_data_get()->audio.door1_ring, LV_BTNMATRIX_CTRL_CHECK_STATE);


}


static void LAYOUT_ENETER_FUNC(ring)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	layout_ring_head_label_create();
	layout_ring_back_btn_create();

	//创建显示系统信息的容器
	layout_ring_page_create(lv_scr_act());
}


static void LAYOUT_QUIT_FUNC(ring)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
}


CREATE_LAYOUT(ring);

