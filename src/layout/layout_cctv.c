#include "layout_define.h"
#include "leo_api.h"
#include <fcntl.h>

extern bool wifi;


#define CCTV_CHANNEL_CONT_ID 0X01
#define CCTV_CHANNEL_LABEL_ID 0X02
#define CCTV_RECORD_VIDEO_BTN_ID 0X03
#define CCTV_SNAP_PHOTO_BTN_ID 0x04

static lv_task_t* cctv_exit_ptask = NULL;

static void cctv_channel_label_display(void);
extern bool is_video_recording(void);
extern int current_cctv_online_cannel[8];
static void cctv_video_mode_open(void)
{
    // monitor_channel_set(MON_CH_CCTV_1);
    fb_video_mode_enable(true);
    monitor_open();//s监控画面开启
    
    
    lv_area_t area[] = {{58,  33,  58 + 146,  33 + 59},
                        {820, 33,  820 + 146, 33 + 59},
                        {274, 463, 274 + 116, 463 + 116},
                        {394, 463, 394 + 116, 463 + 116},
                        {514, 463, 514 + 116, 463 + 116},
                        {634, 463, 634 + 116, 463 + 116}
    };
    gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

    system_bg_fill_color(0x00, 0, 0, 1024, 600);
}

static void cctv_video_mode_close(void)
{
    fb_video_mode_enable(false);
    usleep(100 * 1000);
    monitor_close();
}

static void cctv_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
    btn_data *pdata = (btn_data *) obj->user_data;
    lv_obj_t * children = (lv_obj_t *) pdata->user_data;
    lv_obj_set_state(children, state);
}

static void cctv_btn_img_transform_set(lv_obj_t *obj)
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

    // lv_obj_t* obj_parent = lv_obj_get_parent(obj);
    // lv_obj_set_style_local_bg_opa(obj_parent,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
}

static lv_obj_t *cctv_btn_create(int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);

    lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
    lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

    lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img, img_src);

    lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);

    lv_label_set_long_mode(label,LV_LABEL_LONG_CROP);
	lv_obj_set_size(label,w,30);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(label, string);
	lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

    cctv_btn_img_transform_set(img);
    lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, -10);

    btn_pdata->user_data = img;
    btn->user_data = btn_pdata;
    btn_touch_event_listen(btn);

    return btn;
}
static void cctv_swap_btn_down(lv_obj_t *obj)
{
    cctv_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_swap_btn_up(lv_obj_t *obj)
{

    //为避免cctv通道切换时视频录制出现故障，在通道切换前结束录制
    if (is_video_recording() == true)
    {
        cctv_btn_state_set(obj, LV_STATE_DEFAULT);
        record_video_stop(0x00);
    }

    cctv_btn_state_set(obj, LV_STATE_DEFAULT);
	MONITOR_CH ch = monitor_channel_get();
    for(int i = 0;i<8;i++){
            if(ch<27){
                ch++;
            }else{
                ch = 19;
            }
            printf("\n======%d=>%d========\n",i,current_cctv_online_cannel[ch-19]);
            if(current_cctv_online_cannel[ch-19] == 1){
            //hlf:上一个通道的画面会和新通道的ui叠加，如cctv1和cctv2文本叠加，所以切换通道把背景不透明
            #ifndef _PLATFORM_800_1280
            lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
            lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
            #endif

	        monitor_channel_set(ch);
            monitor_open();
	        cctv_channel_label_display();
            // goto_layout(pLAYOUT(cctv));
            return;
        }
    }
}
static void cctv_swap_btn_create(void)
{
    static btn_data btn_data = btn_data_create(cctv_swap_btn_down, cctv_swap_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SWITCH_PNG);
	char *src[language_total] = {"Switch","切换","Переключить","Echange","Intercambio","Austausch"};
	cctv_btn_create(274, 463, 116, 116, str_get(LAYOUT_CCTV_LANG_SWITCH_ID), &btn_data, &info);
}


static void cctv_record_jpeg_callback(unsigned long arg1, unsigned long arg2)
{
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), CCTV_SNAP_PHOTO_BTN_ID);
	if(obj == NULL)
	{
		return ;
	}
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
}

static void cctv_snap_photo_btn_down(lv_obj_t *obj)
{
    cctv_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_snap_photo_btn_up(lv_obj_t *obj)
{
    cctv_btn_state_set(obj, LV_STATE_DEFAULT);
    if (record_pictrue_start(REC_MODE_MANUAL, monitor_channel_get()) == true)
    {
        lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));
    }
}

static void cctv_snap_photo_btn_create(void)
{
    static btn_data btn_data = btn_data_create(cctv_snap_photo_btn_down, cctv_snap_photo_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_PHOTO_PNG);
	// char *src[language_total] = {"Photo","拍照","фото","Photos","Fotos","Foto"};
    lv_obj_t* obj = cctv_btn_create(394, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_PHOTO_ID), &btn_data, &info);
	lv_obj_set_id(obj,CCTV_SNAP_PHOTO_BTN_ID);

}




static void cctv_record_video_callback(unsigned long arg1, unsigned long arg2)
{
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), CCTV_RECORD_VIDEO_BTN_ID);
	if(obj == NULL)
	{
		return ;
	}
	
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
}

static void cctv_record_video_btn_down(lv_obj_t *obj)
{
    cctv_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_record_video_btn_up(lv_obj_t *obj)
{
    if (is_video_recording() == false)
    {
        if (record_video_start(REC_MODE_MANUAL, 0x00, monitor_channel_get()) == true)
        {
            lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));
        }
    }
    else
    {
        record_video_stop(0x00);
    }
    cctv_btn_state_set(obj, LV_STATE_DEFAULT);
}

static void cctv_record_video_btn_create(void)
{
    static btn_data btn_data = btn_data_create(cctv_record_video_btn_down, cctv_record_video_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_VIDEO_PNG);
	// char *src[language_total] = {"Video","录像","видеозапись","Vidéo","Grabación de vídeo","Video"};
    lv_obj_t* obj = cctv_btn_create(514, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_RECORD_ID), &btn_data, &info);
	lv_obj_set_id(obj, CCTV_RECORD_VIDEO_BTN_ID);

}



static void cctv_home_btn_down(lv_obj_t *obj)
{
    cctv_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_home_btn_up(lv_obj_t *obj)
{
    cctv_btn_state_set(obj, LV_STATE_DEFAULT);

    goto_layout(pLAYOUT(home));
}

static void cctv_home_btn_create(void)
{
    static btn_data btn_data = btn_data_create(cctv_home_btn_down, cctv_home_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_HOME_PNG);
	// char *src[language_total] = {"home","桌面","рабочий стол","Bureau","Interfaz principal","Desktop"};
    cctv_btn_create(634, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_EXIT_ID), &btn_data, &info);

}


static void cctv_channel_label_display(void)
{
	lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), CCTV_CHANNEL_CONT_ID);
	if(parent == NULL)
	{
		printf("find CCTV_CHANNEL_CONT_ID error \n");
		return ;
	}

	lv_obj_t* label = lv_obj_get_child_form_id(parent, CCTV_CHANNEL_LABEL_ID);
	if(label == NULL)
	{
		printf("find CCTV_CHANNEL_LABEL_ID error \n");
		return ;
	}
    MONITOR_CH ch = monitor_channel_get();
    if (ch == MON_CH_CCTV_1)
    {
        lv_label_set_text(label, "CCTV 1");
    }
    else if (ch == MON_CH_CCTV_2)
    {
        lv_label_set_text(label, "CCTV 2");
    }
    else if (ch == MON_CH_CCTV_3)
    {
        lv_label_set_text(label, "CCTV 3");
    }
    else if (ch == MON_CH_CCTV_4)
    {
        lv_label_set_text(label, "CCTV 4");
    }else if (ch == MON_CH_CCTV_5)
    {
        lv_label_set_text(label, "CCTV 5");
    }else if (ch == MON_CH_CCTV_6)
    {
        lv_label_set_text(label, "CCTV 6");
    }else if (ch == MON_CH_CCTV_7)
    {
        lv_label_set_text(label, "CCTV 7");
    }else if (ch == MON_CH_CCTV_8)
    {
        lv_label_set_text(label, "CCTV 8");
    }
}

static void cctv_channel_label_create(void)
{
    lv_obj_t * obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, CCTV_CHANNEL_CONT_ID);
    lv_obj_set_pos(obj, 58, 33);
    lv_obj_set_size(obj, 146, 59);
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
    lv_obj_set_style_local_bg_opa(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
    lv_obj_set_style_local_radius(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 40);

    lv_obj_t* label = lv_label_create(obj, NULL);
	lv_obj_set_id(label, CCTV_CHANNEL_LABEL_ID);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, obj, LV_ALIGN_CENTER, -10, 0);
    cctv_channel_label_display();
}

static lv_task_t *cctv_time_ptask = NULL;

static void cctv_time_task(struct _lv_task_t *task_t)
{
    lv_obj_t * label = (lv_obj_t *) task_t->user_data;

    time_t seconds = time(NULL);
    struct tm tm = {0};
    localtime_r(&seconds, &tm);

    char buffer[32] = {0};
    sprintf(buffer, "%02d:%02d", tm.tm_hour, tm.tm_min);
    lv_label_set_text(label, buffer);

}

static void cctv_exit_task()
{
    if(cctv_exit_ptask != NULL){
        printf("\n=========存在则删除============\n");
        lv_task_del(cctv_exit_ptask);
        cctv_exit_ptask = NULL;
        printf("\n=========删除成功============\n");
    }
    goto_layout(pLAYOUT(home));

}

static void cctv_time_label_create(void)
{
    lv_obj_t * obj = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_pos(obj, 820, 33);
    lv_obj_set_size(obj, 146, 59);
    lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
    lv_obj_set_style_local_bg_opa(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    lv_obj_set_style_local_radius(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 40);

    lv_obj_t * label = lv_label_create(obj, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, obj, LV_ALIGN_CENTER, -10, 0);

    if (cctv_time_ptask != NULL)
    {
        lv_task_del(cctv_time_ptask);
    }
    cctv_time_ptask = lv_task_create(cctv_time_task, 10000, LV_TASK_PRIO_MID, label);
    lv_task_ready(cctv_time_ptask);

    cctv_time_task(cctv_time_ptask);
	
 
}



static void LAYOUT_ENETER_FUNC(cctv)
{
    //定时退出任务
    cctv_exit_ptask = lv_task_create(cctv_exit_task, 60*1000, LV_TASK_PRIO_MID, &clock); 
	// lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
    #ifdef _PLATFORM_800_1280
        lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
        lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    #else
        //在刷出第一诊后，才在视频现实函数设置显示器和屏幕透明
        lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
        lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    #endif

	cctv_video_mode_open();  //显示cctv画面

	cctv_swap_btn_create(); //切换通道按钮

	cctv_snap_photo_btn_create(); //拍照按钮

	cctv_record_video_btn_create(); //录像按钮

	cctv_home_btn_create(); //返回主页面按钮
    
	cctv_channel_label_create(); //通道显示标签创建

	cctv_time_label_create(); //时间标签创建

	
    record_jpeg_event_register(cctv_record_jpeg_callback);
    record_video_event_register(cctv_record_video_callback);

	standby_timer_close();
}

static void LAYOUT_QUIT_FUNC(cctv)
{
   
    MONITOR_CH test1_ch = monitor_channel_get();
    printf("\n\n==退出cctv时的通道1====%d==========\n\n",test1_ch);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	cctv_video_mode_close();

	if (cctv_time_ptask != NULL)
    {
        lv_task_del(cctv_time_ptask);
        cctv_time_ptask = NULL;
    }
    record_video_stop(0x00);
	// usleep(1000*500);
    record_jpeg_event_register(NULL);
    record_video_event_register(NULL);
    // MONITOR_CH test_ch2 = monitor_channel_get();
    // printf("\n\n==退出cctv时的通道2====%d==========\n\n",test_ch2);
    monitor_channel_set(test1_ch);
    standby_timer_open(-1, NULL);
     if(cctv_exit_ptask != NULL){
        // printf("\n=========存在则删除============\n");
        lv_task_del(cctv_exit_ptask);
        cctv_exit_ptask = NULL;
        // printf("\n=========删除成功============\n");
    }
}

CREATE_LAYOUT(cctv);


