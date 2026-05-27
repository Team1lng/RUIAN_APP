#if 1
#include "layout_define.h"

//跳转下一个页面标识
static CCTV_BRAND_CH brand_inter_flag;

extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

//标题
static void layout_ring_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	

	lv_label_set_text(obj,str_get(LAYOUT_CCTV_LANG_SETBRAND_ID));
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	
}

static void layout_brand_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void layout_brand_back_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
    if(brand_inter_flag == CCTV_BRAND_CH_ADD){
	    goto_layout(pLAYOUT(add_cctv));
    }else{
        goto_layout(pLAYOUT(write_cctv));
    }
	
}

//返回键创建
static lv_obj_t* layout_ring_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(layout_brand_back_btn_down, layout_brand_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
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
        lv_obj_t * btnm_cctv_brand = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),1),0xff),0xffffff),1);
        if(btnm == obj)
        {
           	lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECK_STATE);
		    lv_btnmatrix_set_btn_ctrl(btnm, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
            if(btnm_cctv_brand != NULL)
            {
                lv_btnmatrix_clear_btn_ctrl_all(btnm_cctv_brand, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            user_data_get()->cctv_brand_Index = btn_id +6;
        }
        else
        {
            if(btnm != NULL)
            {
                lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECK_STATE);
            }
            lv_btnmatrix_clear_btn_ctrl_all(btnm_cctv_brand, LV_BTNMATRIX_CTRL_CHECK_STATE);
		    lv_btnmatrix_set_btn_ctrl(btnm_cctv_brand, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
            user_data_get()->cctv_brand_Index = btn_id;
        }
        printf("\n\n======%d\n\n",user_data_get()->cctv_brand_Index);
		user_data_save();
	}
}
static void layout_ring_page_create(lv_obj_t *parent)
{
	//创建容器
	lv_obj_t *cctv_brand_cont = lv_cont_create(parent,NULL);
    lv_obj_set_pos(cctv_brand_cont, 0, 100);
	lv_obj_set_size(cctv_brand_cont, 1024, 500);
    lv_obj_set_id(cctv_brand_cont,1);
	lv_cont_set_layout(cctv_brand_cont, LV_LAYOUT_OFF);

	lv_obj_set_style_local_bg_color(cctv_brand_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(cctv_brand_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

	//创建按钮

    static rom_bin_info info_off = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
    static rom_bin_info info_on = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);

    lv_obj_t *page = lv_page_create(cctv_brand_cont, NULL);
	lv_obj_set_size(page,1024,460);
	lv_obj_set_pos(page,0,0);
    // lv_obj_set_size(page,1024,(access(LOCAL_RING7_FILE_PATH,F_OK) == 0)?200:360);
	// lv_obj_set_pos(page,0,(access(LOCAL_RING7_FILE_PATH,F_OK) == 0)?300:140);
    lv_obj_set_id(page,0xff);
    lv_page_set_scrollable_fit4(page, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_MAX, LV_FIT_MAX);
    lv_cont_set_fit(page, LV_FIT_NONE);
    lv_page_set_scrollbar_mode(page,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_local_pad_top(page, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,10);


    	lv_obj_t *btnm_cctv_brand = lv_btnmatrix_create(page, NULL);

        lv_page_glue_obj(btnm_cctv_brand,true);
        if(brand_inter_flag == CCTV_BRAND_CH_ADD){
            lv_btnmatrix_set_map(btnm_cctv_brand, brand_str_get());
        }else{
        static char *cctv_brand_str[] = {"MAZi","\n","Hikvision","\n","Dahua","\n","XM","\n","Urmet","\n","Onvif","\n",""};//Urmet
        // cctv_brand_str[8] = str_get(LAYOUT_SETTING_LANG_OTHER_ID);
            lv_btnmatrix_set_map(btnm_cctv_brand, cctv_brand_str);
        }
        lv_btnmatrix_set_align(btnm_cctv_brand, LV_LABEL_ALIGN_LEFT, 70, 0);
        lv_obj_set_x(btnm_cctv_brand, 40);
        lv_obj_set_y(btnm_cctv_brand, 0);
        lv_obj_set_id(btnm_cctv_brand,1);
        lv_obj_set_width(btnm_cctv_brand, 1024);
        lv_obj_set_height(btnm_cctv_brand, 540);//450

        lv_obj_set_style_local_bg_opa(btnm_cctv_brand, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);

        lv_obj_set_style_local_pattern_image(btnm_cctv_brand, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off);
        lv_obj_set_style_local_pattern_image(btnm_cctv_brand, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on);

        static btn_data btn_data = btn_data_anything_create(screensaver_btnmatrix_up);
        btnm_cctv_brand->user_data = &btn_data;
        btn_touch_event_listen(btnm_cctv_brand);


        lv_btnmatrix_clear_btn_ctrl_all(btnm_cctv_brand, LV_BTNMATRIX_CTRL_CHECK_STATE);
        lv_btnmatrix_set_btn_ctrl(btnm_cctv_brand, user_data_get()->cctv_brand_Index, LV_BTNMATRIX_CTRL_CHECK_STATE);


}


//获取cctv品牌跳转页面
CCTV_BRAND_CH get_layout_cctv_brand_ch(){
    return brand_inter_flag;
}

//设置cctv品牌跳转页面
CCTV_BRAND_CH set_layout_cctv_brand_ch(CCTV_BRAND_CH my_layout){
    brand_inter_flag = my_layout;
}

static void LAYOUT_ENETER_FUNC(cctv_brand)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	layout_ring_head_label_create();
	layout_ring_back_btn_create();

	//创建显示系统信息的容器
	layout_ring_page_create(lv_scr_act());
}


static void LAYOUT_QUIT_FUNC(cctv_brand)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
}


CREATE_LAYOUT(cctv_brand);

#endif