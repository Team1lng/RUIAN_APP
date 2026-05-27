#include "layout_define.h"
#define LAYOUT_CUSTOMIZ_RING_CONT_ID 0x01
#define LAYOUT_CUSTOMIZ_RING_CONT_PAGE_ID 0x01
#define LAYOUT_CUSTOMIZ_RING_MSGBOX_BG_ID 0x02
//壁纸设置完成的标志
static bool customize_ring_set_flg = false;

extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

char *ring_name = NULL;
static void layout_ring_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);

    lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	// char *str[language_total] = {"Set ring", "铃声设置", "Звенеть", "La cloche", "Tintineo", "Ring", "ضبط الرنين", "Nastavit zvonění", "תכנות צלצול","Tom de toque","Imposta Suoneria"};
	lv_label_set_text(obj, str_get(LAYOUT_RING_LANG_SETRING_ID));

    
	
}

/************************************************************
* @Description: 设置此音乐为开门铃声,并且保存到本地
* @Author: xiaoxiao
* @Date: 2023-01-31 14:23:14
* @param: 
* @explain: 
************************************************************/
static void * layout_customize_ring_set_func(void * arg)
{
	customize_ring_set_flg = false;
    //退出保存选择的铃声到本地
    if(ring_name != NULL)
    {
        if(access(LOCAL_RING_FILE_PATH, F_OK) != 0)
        {
            system("mkdir "LOCAL_RING_FILE_PATH);
        }
        printf("ring_name is %s===========\n",ring_name); 
        char cmd[256];
        sprintf(cmd,"cp -rf %s " LOCAL_RING7_FILE_PATH,ring_name);
        system(cmd);
        printf("cmd is %s===========\n",cmd); 

        user_data_get()->audio.door1_ring = 6;
        user_data_save();
    }
	customize_ring_set_flg = true;
	return NULL;
}

static void layout_ring_settinh_fail_task(lv_task_t * task)
{

    goto_layout(pLAYOUT(ring));
	
}

static void layout_ring_settinh_success_display_task(lv_task_t * task)
{
    if(customize_ring_set_flg)
	{
		goto_layout(pLAYOUT(ring));
        return;
    }
}

static void layout_ring_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void layout_ring_back_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);

    if(ring_name != NULL)
    {
        unsigned int file_size = file_size_get(ring_name);
        if(file_size == -1)
        {
            printf("文件为空\n");
        }
        else if(file_size > 1048576)
        {
            // char *src[language_total] = {"Setting failed!!!\nThe file cannot be larger than 1M","设置失败\n文件不能大于1M","Ошибка настройки\nфайл не может быть больше 1M","La configuration a échoué\nle fichier ne peut pas dépasser 1M",\
            // "¡Configuración fallida!!! El archivo no puede ser mayor de 1M","Einstellung fehlgeschlagen!!!\nDie Datei kann nicht größer als 1M sein","فشل الإعداد ! \n  الملف لا يمكن أن يكون أكبر من 1 متر",\
            // "Nastavení selhalo!!!\nSoubor nemůže být větší než 1M","התקנה נכשלה!!! \n קובצים לא יכולים להיות גדולים יותר מ1M","A configuração falhou!!! \n Os ficheiros não podem ser maiores do que 1M","Impostazione fallita!!! \nIl file non può essere più grande di 1M"};
            layout_customiz_file_set_display_create(LAYOUT_CUSTOMIZ_RING_CONT_ID,LAYOUT_CUSTOMZERING_LANG_SETTINGFAILED_ID,false,false);
            lv_layout_task_create(layout_ring_settinh_fail_task, 2000, LV_TASK_PRIO_MID, NULL);
            
        }else
        {
            ak_pthread_t thread_id;
            ak_thread_create(&thread_id, layout_customize_ring_set_func,NULL, ANYKA_THREAD_NORMAL_STACK_SIZE,-1);
            ak_thread_detach(thread_id);
            
            // char *src[language_total] = {"Setting","正在设置","Устанавливается","Mise en place","Configuración en curso","Einstellung","الإعداد","Nastavení","מתקן","A Configurar","Impostazioni"};
            layout_customiz_file_set_display_create(LAYOUT_CUSTOMIZ_RING_CONT_ID,LAYOUT_BACKGROUND_LANG_SETTING_ID,true,true);
            lv_layout_task_create(layout_ring_settinh_success_display_task, 1000, LV_TASK_PRIO_MID, NULL);
        }

    }
    else
    {
        goto_layout(pLAYOUT(ring));

    }
	
}

static lv_obj_t* layout_ring_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(layout_ring_back_btn_down, layout_ring_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}



static lv_obj_t *ring_btn_create(lv_obj_t * parent,int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src_off, const void *img_src_on,int id)
{
    lv_obj_t * btn = lv_btn_create(parent, NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);

    lv_obj_set_id(btn,id);

    lv_page_glue_obj(btn,true);


    lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	// lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	// lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	// lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED , lv_color_make(0x30, 0x30, 0x30));
	// lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_CHECKED , lv_color_make(0x30, 0x30, 0x30));



    lv_obj_set_style_local_pattern_image(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,img_src_off);

    lv_obj_set_style_local_pattern_image(btn,LV_OBJ_PART_MAIN,LV_STATE_CHECKED,img_src_on);
//	lv_obj_set_click(img,true);

    if (string != NULL)
    {
        lv_obj_t * label = lv_label_create(parent, NULL);
		
        lv_label_set_text(label, string);
        lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 80, 0);
    }


  //  lv_obj_align(img, btn, LV_ALIGN_IN_LEFT_MID, 70, -20);


    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);

    return btn;
}
/************************************************************
* @Description: 铃声按键点击事件
* @Author: xiaoxiao
* @Date: 2023-01-31 14:25:58
* @param: 
* @explain: 
************************************************************/
static void ring_select_btn_up(lv_obj_t *obj)
{
    int total =  media_file_total_get(FILE_TYPE_SD_RING, 0);
    for(int i =1;i<=total;i++)
    {
        lv_obj_set_state(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),LAYOUT_CUSTOMIZ_RING_CONT_ID),LAYOUT_CUSTOMIZ_RING_CONT_PAGE_ID),0xFFFFFF),i),LV_STATE_DEFAULT);
    }
    lv_obj_set_state(obj,LV_STATE_CHECKED);
    int id = lv_obj_get_id(obj);

    media_info *info = media_info_get(FILE_TYPE_SD_RING,id -1);

    if(ring_name == NULL)
    {
        ring_name = (char *)malloc(128);
    }
	bzero(ring_name,128);
	sprintf(ring_name,"%s%s",SD_RING_PATH,info->file_name);

    printf("ring_name is %s\n",ring_name);
    custom_door_ring_play(ring_name, get_sound_val(user_data_get()->audio.door_ring_val), NULL, NULL);


}

static void layout_ring_page_create(lv_obj_t *parent)
{
	//创建容器
	lv_obj_t *ring_cont = lv_cont_create(parent,NULL);
    lv_obj_set_id(ring_cont,LAYOUT_CUSTOMIZ_RING_CONT_ID);
    lv_obj_set_pos(ring_cont, 0, 100);
	lv_obj_set_size(ring_cont, 1024, 500);
	lv_cont_set_layout(ring_cont, LV_LAYOUT_OFF);

	lv_obj_set_style_local_bg_color(ring_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(ring_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

    int total =  media_file_total_get(FILE_TYPE_SD_RING, 0);

    if(total ==0)
    {
        return;
    }
    lv_obj_t *page = lv_page_create(ring_cont, NULL);
	lv_obj_set_size(page,1024,500);
	lv_obj_set_pos(page,0,0);
    lv_obj_set_id(page,LAYOUT_CUSTOMIZ_RING_CONT_PAGE_ID);
    lv_page_set_scrollable_fit4(page, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_MAX, LV_FIT_MAX);
    lv_cont_set_fit(page, LV_FIT_NONE);
    lv_page_set_scrollbar_mode(page,LV_SCROLLBAR_MODE_OFF);

    int x = 40;
    int y =-80;
    for(int i =1;i<=total;i++)
    {
        y+=80;
        char buf[64];
        // char *str[language_total] = {"Ring","铃声","Звонок","Anneau","Anillo","Ring","رنين","Zvonění","צלצול","Toque","Suoneria"};
        sprintf(buf,"%s %d",str_get(LAYOUT_SETTING_LANG_RING_ID),i);
        static rom_bin_info info_on = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);
        static rom_bin_info info_off = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
        static btn_data btn_data = btn_data_create(NULL, ring_select_btn_up, NULL);
        ring_btn_create(page,x, y, 1024, 80, buf, &btn_data, &info_off,&info_on,i);
    }
    

}

/*
* arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
* arg2:arg1= 1,arg2参数无意义
*	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
*	   arg1=3,arg2=1:内存正常，2，内存已满
*/
static void layout_customize_ring_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
    if (arg1 == 1)
    {	
    	 goto_layout(pLAYOUT(home));
    }
}


static void LAYOUT_ENETER_FUNC(customize_ring)
{

    customize_ring_set_flg = false;
    ring_name = NULL;
    
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	layout_ring_head_label_create();
	layout_ring_back_btn_create();

	//创建显示系统信息的容器
	layout_ring_page_create(lv_scr_act());

    sdcard_event_register(layout_customize_ring_sd_status_change_callback);
}


static void LAYOUT_QUIT_FUNC(customize_ring)
{
    if(ring_name != NULL)
    {
        free(ring_name);
        ring_name = NULL;
    }   
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);    

}


CREATE_LAYOUT(customize_ring);

