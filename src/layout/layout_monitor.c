
#include "layout_define.h"
#include "leo_api.h"
#include "../api/video/video_decode.h"
#define TEST 0
extern void playback_sd_status_change_callback(unsigned long arg1, unsigned long arg2);
static bool play_ring_falg = true;

extern bool wifi;

static bool recording = false;

extern bool talking;

typedef enum
{
	LV_MONITOR_UNLOCK_OBJ_ID,
	LAYOUT_MONITOR_HEAD_CONT_ID,

} layout_monitor_obj_id;

extern void network_event_register(event_pro_callback handle);
extern void network_event_register(event_pro_callback handle);
extern void outdoor_motion_event_register(event_pro_callback handle);
extern void motion_call_extern_func(unsigned long arg1, unsigned long arg2);
extern bool occupy_resource[DEVICE_TOTAL];
extern bool media_thumb_wait_thread_quit(void);

bool is_light_status_on(void);

void light_status_set(bool is_on);

bool monitor_auto_record_flag = false;
extern bool video_auto_record_falg;
extern bool tuya_sent_flag;

extern int is_mode_status_on(void);
extern bool sent_tuya_start(char mode, MONITOR_CH video_channel);

void monitor_call_extern_func(unsigned long arg1, unsigned long arg2);
static void key_call_extern_func(unsigned long arg1, unsigned long arg2);
// static int test_times = 0;

/*
static void network_event_extern_proc(unsigned long arg1, unsigned long arg2);
static void network_event_inside_proc(unsigned int arg1, unsigned int arg2);
static void tuya_event_extern_proc(unsigned int arg1, unsigned int arg2);
*/
static void monitor_call_inside_func(unsigned long ar1g, unsigned long arg2);
//铃声播放时间到，终止铃声
static void ring_play_time_stop(void);
static void monitor_channel_label_display(void);
static void door_ring_callback(void);

extern bool is_video_recording(void);
extern void gui_raw_clear(void);

static int monitor_timeout_val = 60;
lv_task_t *monitor_timer_ptask = NULL;
lv_task_t *door_ring_play_task = NULL;
static lv_obj_t *outdoor_img = NULL;
static lv_obj_t *outdoor_slider = NULL;
static lv_obj_t *indoor_img = NULL;
static lv_obj_t *indoor_slider = NULL;
static bool sound_flag = true;
static unsigned long long door_ring_timesmp = 0;

//hlf:发送楼层号
void net_send_floor_number(void)
{
	network_cmd_data_init(data);
	data.cmd = NET_COMMON_CMD_FLOOR;
	MONITOR_CH monitor_ch = monitor_channel_get();
	network_device device = get_outdoor_device_by_channel(monitor_ch);
	data.device = device;
	data.arg1 = 0;
	data.arg2 = atoi(user_data_get()->floor);
	network_send_cmd_data(&data);
}

/***
** 函数作用：通过监控通道获取门口设备
** 返回参数说明：
***/
network_device get_outdoor_device_by_channel(MONITOR_CH CH)
{
	return (network_device)(CH + (DEVICE_UNIT_OUTDOOR_1 - MON_CH_UNIT_DOOR_1));
}

/***
** 函数作用：通过设备获取对应的通道
** 返回参数说明：
***/
MONITOR_CH get_channel_by_outdoor_device(network_device device)
{
	return (MONITOR_CH)(device - (DEVICE_UNIT_OUTDOOR_1 - MON_CH_UNIT_DOOR_1));
}

static bool is_monitor_data_busy[DEVICE_ALL];

bool monitor_data_busy_get(network_device device)
{
	return is_monitor_data_busy[device];
}

void monitor_data_busy_enable(network_device device, bool en)
{
	is_monitor_data_busy[device] = en;
}

// static bool is_tuya_app_busy = false;
static void monitor_timer_set(int8_t time)
{
	monitor_timeout_val = time;
}

static void sound_set(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		sound_flag = sound_flag == true ? false : true;

		if ((outdoor_img != NULL) && (outdoor_img->parent != NULL) && (user_data_get()->other.network_device == 1))
		{
			MONITOR_CH ch = monitor_channel_get();
			if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2) || (ch == MON_CH_DOOR_0))
			{
				printf("outdoor slider adjust\n");
				lv_obj_set_hidden(outdoor_img->parent, sound_flag);
			}
		}

		if ((indoor_img != NULL) && (indoor_img->parent != NULL))
		{
			lv_obj_set_hidden(indoor_img->parent, sound_flag);
		}

		ring_play_time_stop();
	}
}

/**
 * 打开监控视频模式
 * 初始化视频监控界面，配置显示区域，准备接收和显示视频流
 */
static void monitor_video_mode_open(void)
{
    gui_raw_clear();                              // 清除GUI原始缓冲区
    system_bg_fill_color(0x00, 0, 0, 1024, 600); // 填充背景为黑色(0,0,0)
    
    monitor_open();                               // 打开指定通道的监控，开始接收视频数据
    fb_video_mode_enable(true);                   // 启用监控画面显示功能
    
#if 1       // 控件的位置定义区域
    // 定义界面各元素的显示区域坐标
    lv_area_t area[] = {
        {0, 0, 1024, 0 + 48},                   // 顶部区域(标题栏等)
        // {58, 33, 58 + 146, 33 + 59},
        // {820, 33, 820 + 146, 33 + 59},
        {60, 105, 60 + 65, 105 + 40},           // 左上角按钮区域
        {68, 160, 68 + 40, 160 + 290},          // 左侧边栏区域
        {914, 160, 914 + 40, 160 + 290},        // 右侧边栏区域
        {900, 105, 900 + 70, 105 + 40},         // 右上角按钮区域
        // {467, 33, 467 + 90, 33 + 46},
        {152, 463, 152 + 116, 463 + 116},       // 底部按钮1
        {272, 463, 272 + 116, 463 + 116},       // 底部按钮2
        {392, 463, 392 + 116, 463 + 116},       // 底部按钮3
        {512, 463, 512 + 116, 463 + 116},       // 底部按钮4
        {632, 463, 632 + 116, 463 + 116},       // 底部按钮5
        //{754, 463, 754 + 116, 463 + 116},
        {752, 463, 752 + 116, 463 + 116},       // 底部按钮6
        {312, 150, 312 + 400, 150 + 300},       // 中央视频显示区域
        {0, 0, 0 + 1024, 0 + 600}                // 全屏区域
    };
    
    // 设置GUI绘制区域，用于布局界面元素
    gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));
#endif

    // 启用屏幕点击事件处理
    lv_obj_set_click(lv_scr_act(), true);

    // 创建按钮数据结构，绑定声音设置回调函数
    static btn_data btn_data = btn_data_anything_create(sound_set);
    
    // 将按钮数据设置为当前屏幕的用户数据
    lv_scr_act()->user_data = &btn_data;
    
    // 注册屏幕触摸事件监听器
    btn_touch_event_listen(lv_scr_act());

    // 再次填充背景为黑色，确保视觉一致性
    system_bg_fill_color(0x00, 0, 0, 1024, 600); 
}

static void monitor_video_mode_close(void)
{
	fb_video_mode_enable(false);

	usleep(100 * 1000);
	monitor_close();
}

static void monitor_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
	btn_data *pdata = (btn_data *)

						  obj->user_data;
	lv_obj_t *children = (lv_obj_t *)

							 pdata->user_data;
	lv_obj_set_state(children, state);
}

static void monitor_btn_img_transform_set(lv_obj_t *obj)
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

static lv_obj_t *monitor_btn_create(int x, int y, int w, int h, char *string, btn_data *btn_pdata, const void *img_src)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);

	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));
	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_30);

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_img_set_src(img, img_src);

	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
	lv_obj_set_size(label, w, 30);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_label_set_text(label, string);
	lv_obj_align(label, btn, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

	monitor_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, -10);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);

	return btn;
}

extern bool net_online_device[DEVICE_TOTAL];

static lv_obj_t *monitor_record_photo_btn;

static void monitor_record_jpeg_callback(unsigned long arg1, unsigned long arg2)
{
	lv_obj_set_state(monitor_record_photo_btn, LV_STATE_DEFAULT);
	lv_obj_set_style_local_bg_color(monitor_record_photo_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,
									LV_COLOR_MAKE(39, 39, 39));
}

static void monitor_snap_photo_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);
}

static void monitor_snap_photo_btn_up(lv_obj_t *obj)
{
	if (lv_obj_get_state(obj, LV_BTN_PART_MAIN) & LV_STATE_CHECKED)
	{
		return;
	}
	if (record_pictrue_start(REC_MODE_MANUAL, monitor_channel_get()) == true) //手动从当前通道拍照
	{
		lv_obj_set_state(obj, LV_STATE_CHECKED);

		lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_CHECKED, LV_COLOR_MAKE(255, 0, 0));
	}
}

static void monitor_snap_photo_btn_create(void)
{
	static btn_data btn_data = btn_data_create(monitor_snap_photo_btn_down, monitor_snap_photo_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_PHOTO_PNG);
	// char *src[language_total] = {"Photo", "拍照", "Фото", "Photo", "Foto", "Foto", "صور ", "Fotografie", "תמונה","Fotos","Foto"};
	monitor_record_photo_btn = monitor_btn_create(152, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_PHOTO_ID), &btn_data, &info);
}

// static bool record_flag = false;
// static unsigned long long record_timesmp = 0;

static lv_obj_t *monitor_record_video_btn;
static void monitor_record_video_callback(unsigned long arg1, unsigned long arg2)
{
	lv_obj_set_style_local_bg_color(monitor_record_video_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,
									LV_COLOR_MAKE(39, 39, 39));
}

static void monitor_record_video_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);
}

extern bool audio_play_open(bool is_mobilephone);

static void monitor_record_video_btn_up(lv_obj_t *obj)
{
	if (recording == false)
	{ //如果已经未开始录像
		if (is_audio_talk_open() == false)
		{
			MONITOR_CH monitor_ch = monitor_channel_get();
			network_device device = get_outdoor_device_by_channel(monitor_ch);
			audio_talk_open(device, true);
		}

		recording = true;
		if (record_video_start(REC_MODE_MANUAL, 0x01, monitor_channel_get()) == true)
		{ //那么开始录像

			lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));
		}
	}
	else
	{
		record_video_stop(0x00);
		recording = false;
		if (talking == false)
			audio_talk_close();
		// record_flag = false;
	}
	monitor_btn_state_set(obj, LV_STATE_DEFAULT);
}

static void monitor_record_video_btn_create(void)
{
	static btn_data btn_data = btn_data_create(monitor_record_video_btn_down, monitor_record_video_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_VIDEO_PNG);
	// char *src[language_total] = {"Record", "录像", "Запись", "Record", "Registro", "Datensatz", "تسجيل ", "Záznam obrazu", "הקלטת וידאו","Vídeos","Registrazione"};
	monitor_record_video_btn = monitor_btn_create(272, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_RECORD_ID), &btn_data, &info);
}

static void monitor_talk_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);
}

static lv_task_t *montior_talk_task_t = NULL;
static void monitor_talk_open_task(lv_task_t *task)
{
	if (audio_output_buffer_query() > 0)
	{
		return;
	}
	lv_obj_t *btn_obj = (lv_obj_t *)task->user_data;
	btn_data *pdata = (btn_data *)btn_obj->user_data;
	lv_obj_t *img_obj = (lv_obj_t *)pdata->user_data;

	MONITOR_CH talk_ch = *((MONITOR_CH *)img_obj->user_data);

	MONITOR_CH monitor_ch = monitor_channel_get();
	if (talk_ch == monitor_ch)
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_ON_PNG);
		lv_img_set_src(img_obj, &info);
		lv_obj_align(img_obj, btn_obj, LV_ALIGN_CENTER, 0, -10);

		lv_obj_set_style_local_bg_color(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));

		network_device device = get_outdoor_device_by_channel(monitor_ch);

		// if(recording == false)
		//	audio_talk_open(device, false);
		// else
		//	audio_play_open(true);
		if (audio_talk_open(device, false) == false)
			audio_play_open(true);

		audio_volume_set(get_sound_val(user_data_get()->audio.door_talk_val));
	}
	lv_task_del(task);
	montior_talk_task_t = NULL;
}

static void monitor_talk_btn_up(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_DEFAULT);
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *img = (lv_obj_t *)pdata->user_data;

	if (talking == false) //未通话
	{

		if (montior_talk_task_t == NULL) //如果通话任务空
		{
			talking = true;
			monitor_timer_set(60);

			audio_play_stop_set(); //停止声音播放

			static MONITOR_CH ch = MON_CH_NONE;

			ch = monitor_channel_get();
			img->user_data = &ch;
			montior_talk_task_t = lv_task_create(monitor_talk_open_task, 100, LV_TASK_PRIO_MID, obj);

			monitor_talk_open_task(montior_talk_task_t);
			/*
			rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_ON_PNG);
			lv_img_set_src(img, &info);
			lv_obj_align(img, obj, LV_ALIGN_CENTER, 0, -10);
			lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb2, 0x2d, 0x00));

			MONITOR_CH ch = monitor_channel_get();
			network_device device = ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
			audio_talk_open(device);
			*/
			//改变图

			static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_TALK_PNG);

			lv_img_set_src(outdoor_img, &info1);
			lv_img_set_src(indoor_img, &info1);

			lv_slider_set_value(indoor_slider, user_data_get()->audio.door_talk_val, LV_ANIM_OFF);

			lv_slider_set_range(outdoor_slider, 0, 100);
			lv_slider_set_value(outdoor_slider, user_data_get()->audio.outdoor_talk_val, LV_ANIM_OFF);
			network_device device = get_outdoor_device_by_channel(ch);
			printf("device=====is====%d", device);

			occupy_resource[device] = true;
			network_cmd_data_init(data);

			if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2))
			{
				data.device = DEVICE_GROUP;
			}
			else
			{
				data.device = DEVICE_ALL;
			}
			data.cmd = NET_COMMON_CMD_OUTDOOR_TALK;
			data.arg1 = ch;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);

			data.cmd = NET_COMMON_CMD_DATA_BUSY;
			data.arg1 = device;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);

			data.cmd = NET_COMMON_CMD_SOUND;
			data.arg1 = 1 | user_data_get()->user_language << 16;
			data.arg2 = get_sound_val(user_data_get()->audio.outdoor_talk_val);
			data.device = device;
			network_send_cmd_data(&data);
		}
	}
	else if (talking == true)
	{

		audio_talk_close(); //关闭通话
		goto_layout(pLAYOUT(home));

		return;
	}

	lv_obj_align(outdoor_img, outdoor_img->parent, LV_ALIGN_IN_TOP_MID, 0, 10);
	lv_obj_align(indoor_img, indoor_img->parent, LV_ALIGN_IN_TOP_MID, 0, 10);
	lv_obj_set_auto_realign(img, true); //图片重新对齐
}

static lv_obj_t *talk_btn = NULL;
static void monitor_talk_btn_create(void)
{
	static btn_data btn_data = btn_data_create(monitor_talk_btn_down, monitor_talk_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_OFF_PNG);
	// char *src[language_total] = {"Talk", "通话", "Говорить", "Parler", "Hablar", "Reden", "تكلم ", "Hovořit", "דיבור","Falar","Parla"};
	talk_btn = monitor_btn_create(392, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_TALK_ID), &btn_data, &info);
}

static void monitor_door1_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);
}

static lv_task_t *monitor_unlock_task_t = NULL;

static void monitor_unlock_task(lv_task_t *task_t)
{
	lv_obj_t *child = lv_obj_get_child_form_id(lv_scr_act(), LV_MONITOR_UNLOCK_OBJ_ID);
	if (child != NULL)
	{
		lv_obj_set_hidden(child, true);
	}
	lv_obj_t *obj = task_t->user_data;
	if (obj == NULL)
	{
		printf(" task_t->user_data is null \n");
	}
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));

	if (monitor_unlock_task_t != NULL)
	{
		lv_task_del(monitor_unlock_task_t);
		monitor_unlock_task_t = NULL;
	}
	if (tuya_ipc_get_register_status() != 0)
	{
		// tuya_dp_148_response_accessory_lock(false);
		// tuya_dp_239_response_accessory_lock(false);
	}

	else
		printf("tuya is no!\n");
}

static void monitor_door1_btn_up(lv_obj_t *obj)
{
	// monitor_timeout_val = 60; //hlf:开锁取消重置
	monitor_btn_state_set(obj, LV_STATE_DEFAULT);

	if (monitor_unlock_task_t != NULL)
	{
		return;
	}

	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));

	lv_obj_t *child = lv_obj_get_child_form_id(lv_scr_act(), LV_MONITOR_UNLOCK_OBJ_ID);

	if (child != NULL)
	{
		lv_obj_set_hidden(child, false);
	}

	network_cmd_data_init(data);
	data.cmd = NET_COMMON_CMD_UNLOCK;
	MONITOR_CH monitor_ch = monitor_channel_get();
	network_device device = get_outdoor_device_by_channel(monitor_ch);
	data.device = device;
	data.arg1 = 1 | user_data_get()->user_language << 16;
	data.arg2 = user_data_get()->door1_delay | device << 16;
	network_send_cmd_data(&data); //发送开始命令
	net_send_floor_number();

	monitor_unlock_task_t = lv_task_create(monitor_unlock_task, 1500, LV_TASK_PRIO_HIGH, obj);
	open_door_ring_play(30);
}

static void monitor_door2_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);
}

static void monitor_door2_btn_up(lv_obj_t *obj)
{
	// monitor_timeout_val = 60; //hlf
	monitor_btn_state_set(obj, LV_STATE_DEFAULT);

	if (monitor_unlock_task_t != NULL)
	{
		return;
	}

	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));

	lv_obj_t *child = lv_obj_get_child_form_id(lv_scr_act(), LV_MONITOR_UNLOCK_OBJ_ID);

	if (child != NULL)
	{
		lv_obj_set_hidden(child, false);
	}

	network_cmd_data_init(data);

	MONITOR_CH monitor_ch = monitor_channel_get();
	network_device device = get_outdoor_device_by_channel(monitor_ch);
	data.device = device;
	data.cmd = NET_COMMON_CMD_UNLOCK;
	data.arg1 = 2 | user_data_get()->user_language << 16;
	data.arg2 = user_data_get()->door2_delay | device << 16;
	network_send_cmd_data(&data);
	net_send_floor_number();

	monitor_unlock_task_t = lv_task_create(monitor_unlock_task, 1500, LV_TASK_PRIO_HIGH, obj);
	open_door_ring_play(30);
}

static lv_obj_t *door_btn = NULL;
static void monitor_door1_btn_create(void)
{
	static btn_data btn_data_door2 = btn_data_create(monitor_door1_btn_down, monitor_door1_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK_SMALL_PNG);
	// char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür", "باب ", "Dveře", "פתיחת דלת 1","Porta","Porta"};
	door_btn = monitor_btn_create(512, 463, 116, 116, str_get(LAYOUT_HOME_LANG_DOOR_ID), &btn_data_door2, &info);
}

static void monitor_door2_btn_create(void)
{
	static btn_data btn_data_door1 = btn_data_create(monitor_door2_btn_down, monitor_door2_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK_BIG_PNG);
	// char * src[language_total] = {"Garage", "车库门", "Гараж", "Garage", "Garaje", "Garage","كراج","Garáž","פתיחת דלת 2","Garagem","Box auto"};
	// monitor_btn_create_1(692, 463, 116, 116, src[user_data_get()->user_language], &btn_data_door1, &info, 10086);

	monitor_btn_create(632, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_GARAGE_ID), &btn_data_door1, &info);
}

/* static void monitor_light_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);

} */

/* static void monitor_light_status_display(lv_obj_t *obj)
{
	btn_data *data = obj->user_data;
	lv_obj_t *img = data->user_data;
	if (is_light_status_on() == true)
	{
		rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_ON_PNG);
		lv_img_set_src(img, &info);

		lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb2, 0x2d, 0x00));

		lv_obj_align(img, obj, LV_ALIGN_CENTER, 0, -10);
	}
	else
	{
		rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_OFF_PNG);
		lv_img_set_src(img, &info);
		lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
		lv_obj_align(img, obj, LV_ALIGN_CENTER, 0, -10);
	}
	lv_obj_set_auto_realign(img, true);
} */

/* static void monitor_light_btn_up(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_DEFAULT);

	light_status_set(is_light_status_on() ? false : true);
	monitor_light_status_display(obj);
} */

/* static void monitor_light_btn_create(void)
{
	static btn_data btn_data = btn_data_create(monitor_light_btn_down, monitor_light_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_OFF_PNG);
	lv_obj_t *obj = monitor_btn_create(754, 463, 116, 116, "Light", &btn_data, &info);
	monitor_light_status_display(obj);
} */
static void monitor_home_btn_down(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_PRESSED);
}

static void monitor_home_btn_up(lv_obj_t *obj)
{
	monitor_btn_state_set(obj, LV_STATE_DEFAULT);
	monitor_quit_mask_set(MON_QUIT_MANUAL_DOOR);
	goto_layout(pLAYOUT(home));
}

static lv_obj_t *home_btn = NULL;
static void monitor_home_btn_create(void)
{
	static btn_data btn_data = btn_data_create(monitor_home_btn_down, monitor_home_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_EXIT_PNG);
	// char * src[language_total] = {"Exit", "退出", "Выход", "Sortie", "Salida", "Ausgang", "خروج", "Ukončit", "יציאה","Sair","Uscita"};
	home_btn = monitor_btn_create(752, 463, 116, 116, str_get(LAYOUT_MONITOR_LANG_EXIT_ID), &btn_data, &info);
}

static lv_obj_t *montior_channel_label = NULL;

static void monitor_channel_label_display(void)
{
	MONITOR_CH ch = monitor_channel_get();

	// char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür", "باب ", "Dveře", "פתיחת דלת 1","Porta","Porta"};
	char ch_num[1] = {0};
	if (ch > MON_CH_UNIT_DOOR_2)
	{
		ch -= 3;
	}

	sprintf(ch_num, "%d", ch);

	char *ch_name = (char *)malloc(strlen(ch_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
	strcpy(ch_name, str_get(LAYOUT_HOME_LANG_DOOR_ID));
	strcat(ch_name, ch_num);
	lv_label_set_text(montior_channel_label, ch_name);

	/*
	else if (ch == MON_CH_CCTV_1) {
		lv_label_set_text(montior_channel_label, "CCTV 1");
	}
	else if (ch == MON_CH_CCTV_2) {
		lv_label_set_text(montior_channel_label, "CCTV 2");
	}
*/
}

void monitor_channel_label_create(void)
{
	lv_obj_t *obj = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_pos(obj, 38, 0);
	lv_obj_set_size(obj, 146, 48);
	lv_obj_set_style_local_bg_color(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x39, 0x39, 0x39));
	lv_obj_set_style_local_bg_opa(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	// lv_obj_set_style_local_radius(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 40);

	montior_channel_label = lv_label_create(obj, NULL);
	lv_label_set_align(montior_channel_label, LV_LABEL_ALIGN_LEFT);

	monitor_channel_label_display();
	lv_obj_align(montior_channel_label, obj, LV_ALIGN_CENTER, 0, 0);
}

static void monitor_record(void)
{
	if (user_data_get()->motion.record == 0)
	{

		sent_tuya_start(REC_MODE_MANUAL, monitor_channel_get());
		return;
	}
	else if (user_data_get()->motion.record == 1)
	{ //

		if (record_pictrue_start(REC_MODE_MANUAL, monitor_channel_get()) == true)
		{
			lv_obj_set_style_local_bg_color(monitor_record_photo_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));
		}
	}
	else if (user_data_get()->motion.record == 2)
	{ //判断sd卡是否插入
		if (is_sdcard_insert() == false)
		{ //未插入 改变用户数据
			sent_tuya_start(REC_MODE_MANUAL, monitor_channel_get());
			user_data_get()->motion.record = 0;
			user_data_save();
			return;
		}
		else if (is_sdcard_insert() == true)
		{
			/*自动录像启动*/
			sent_tuya_start(REC_MODE_MANUAL, monitor_channel_get());
			if (recording == false) //如果已经未开始录像
			{
				if (is_audio_talk_open() == false)
				{
					MONITOR_CH monitor_ch = monitor_channel_get();
					network_device device = get_outdoor_device_by_channel(monitor_ch);
					audio_talk_open(device, true);
				}
				recording = true;

				if (record_video_start(REC_MODE_MANUAL, 0x01, monitor_channel_get()) == true)
				{ //那么开始录像
					lv_obj_set_style_local_bg_color(monitor_record_video_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));
				}
			}
		}
	}
}




/**
 * 监控定时器任务函数 - 负责更新显示时间、处理自动记录和超时逻辑
 * 
 * 此函数由LVGL定时器周期性调用，主要功能包括：
 * 1. 更新倒计时显示
 * 2. 更新当前时间显示
 * 3. 处理自动记录逻辑
 * 4. 监控超时处理
 * 5. 根据特定条件触发消息发送、语音通话和录像功能
 * 
 * @param task_t LVGL任务结构体指针，包含用户数据
 */
//======================cyy:无人接通处理的地方（插卡才能用）============================
static void monitor_timer_task(struct _lv_task_t *task_t)
{
    // 更新倒计时显示标签
    lv_obj_t *timer_label = (lv_obj_t *)task_t->user_data;
    lv_label_set_text_fmt(timer_label, "%02d", monitor_timeout_val);
    lv_obj_align(timer_label, timer_label->parent, LV_ALIGN_CENTER, 0, 0);

    // 更新当前时间显示标签
    lv_obj_t *time_label = (lv_obj_t *)timer_label->user_data;
    time_t seconds = time(NULL);

    struct tm tm = {0};
    localtime_r(&seconds, &tm);
    lv_label_set_text_fmt(time_label, "%02d:%02d", tm.tm_hour, tm.tm_min);
    lv_obj_align(time_label, time_label->parent, LV_ALIGN_CENTER, 0, 0);

    // 自动拍照逻辑 - 当视频自动记录和监控自动记录标志都启用时执行
    if ((video_auto_record_falg == true) && (monitor_auto_record_flag == true))
    {
        monitor_record(); // 执行自动拍照
        monitor_auto_record_flag = false; // 重置自动记录标志，避免重复触发
    }

    // 秒级计时器 - 确保每秒只执行一次关键逻辑
    static unsigned long sec = 0;

    struct ak_timeval timeval;
    ak_get_ostime(&timeval);

    // 每秒执行一次的逻辑块
    if (timeval.sec != sec)
    {
        sec = timeval.sec; // 更新秒计数器

        // 监控超时处理 - 当倒计时归零时返回主界面
        if (monitor_timeout_val == 0)
        {
            goto_layout(pLAYOUT(home)); // 跳转到主界面
        }
        else
            monitor_timeout_val--; // 否则减少倒计时值

        // 检查SD卡插入状态
        if (is_sdcard_insert())
        {

		//==============================cyy:处理三种模式下，门口机对应对应的处理方式===========================

            // 条件1: 倒计时30秒且满足特定条件时的处理
            if (monitor_timeout_val == 30 && talking == false && monitor_enter_flag_get() != MON_ENTER_MANUAL_DOOR && is_mode_status_on() != 2)
            {
                // 网络设备消息发送
                if (user_data_get()->other.network_device == 1 && user_data_get()->nobody_message)
                {
                    network_cmd_data_init(data);
                    MONITOR_CH monitor_ch = monitor_channel_get();
                    network_device device = get_outdoor_device_by_channel(monitor_ch);
                    data.cmd = NET_COMON_CMD_MESSACG;
                    data.arg1 = user_data_get()->user_language;
                    data.arg2 = 0;
                    data.device = device;
                    network_send_cmd_data(&data); // 发送网络消息命令
                }                
                // 避免重复录制
                if (recording)
                    return;
                // 开启语音通话
                if (is_audio_talk_open() == false)
                {
                    MONITOR_CH monitor_ch = monitor_channel_get();
                    network_device device = get_outdoor_device_by_channel(monitor_ch);
                    audio_talk_open(device, true); // 打开语音通话
                }
                // 开始录像
                recording = true;
                if (record_video_start(REC_MODE_MANUAL, 0x01, monitor_channel_get()) == true)
                { // 启动手动模式录像
                    lv_obj_set_style_local_bg_color(monitor_record_video_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));
                }
            }
            // 条件2: 模式状态为2且倒计时40秒时的处理 (与条件1逻辑类似)
            else if (is_mode_status_on() == 2 && monitor_timeout_val == 58 && talking == false && monitor_enter_flag_get() != MON_ENTER_MANUAL_DOOR)
            {
                // 网络设备消息发送
                if (user_data_get()->other.network_device == 1 && user_data_get()->nobody_message)
                {
                    printf("\n=================发送message命令=====================\n");
                    network_cmd_data_init(data);
                    MONITOR_CH monitor_ch = monitor_channel_get();
                    network_device device = get_outdoor_device_by_channel(monitor_ch);
                    data.cmd = NET_COMON_CMD_MESSACG;
                    data.arg1 = user_data_get()->user_language;
                    data.arg2 = 0;
                    data.device = device;
                    network_send_cmd_data(&data); // 发送网络消息命令
                }
                
                // 避免重复录制
                if (recording)
                    return;

                // 开启语音通话
                if (is_audio_talk_open() == false)
                {
                    MONITOR_CH monitor_ch = monitor_channel_get();
                    network_device device = get_outdoor_device_by_channel(monitor_ch);
                    audio_talk_open(device, true); // 打开语音通话
                }
                
                // 开始录像
                recording = true;
                if (record_video_start(REC_MODE_MANUAL, 0x01, monitor_channel_get()) == true)
                { // 启动手动模式录像
                    lv_obj_set_style_local_bg_color(monitor_record_video_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4386d7));
                }
            }
           
        }
    }
}

static void monitor_top_cont_create(void)
{
	if (monitor_enter_flag_get() != MON_ENTER_TUYA)
	{
		lv_obj_t *top_cont = lv_cont_create(lv_scr_act(), NULL);
		lv_cont_set_layout(top_cont, LV_LAYOUT_OFF);
		lv_obj_set_id(top_cont, LAYOUT_MONITOR_HEAD_CONT_ID);
		lv_obj_set_pos(top_cont, 0, 0);
		lv_obj_set_size(top_cont, 1024, 48);
		lv_obj_set_style_local_bg_color(top_cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x39, 0x39, 0x39));
		lv_obj_set_style_local_bg_opa(top_cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
	}
}

static void monitor_time_label_create(void)
{
	if (monitor_enter_flag_get() != MON_ENTER_TUYA)
	{

		lv_obj_t *time = lv_cont_create(lv_scr_act(), NULL);

		lv_cont_set_layout(time, LV_LAYOUT_OFF);
		lv_obj_set_pos(time, 840, 0);
		lv_obj_set_size(time, 146, 48);
		// lv_obj_set_style_local_bg_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_MAKE(0x39, 0x39, 0x39));
		lv_obj_set_style_local_bg_opa(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		// lv_obj_set_style_local_radius(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 40);

		lv_obj_t *time_label = lv_label_create(time, NULL);

		lv_label_set_long_mode(time_label, LV_LABEL_LONG_EXPAND);
		lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(time_label, time, LV_ALIGN_CENTER, 0, 0);

		lv_obj_t *timer = lv_cont_create(lv_scr_act(), time);

		lv_obj_set_pos(timer, 467, 0);
		lv_obj_set_size(timer, 90, 36);
		lv_obj_set_style_local_bg_opa(timer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_text_font(timer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));

		lv_obj_t *timer_label = lv_label_create(timer, NULL);
		timer_label->user_data = time_label;

		lv_label_set_align(timer_label, LV_LABEL_ALIGN_CENTER);
		lv_label_set_long_mode(timer_label, LV_LABEL_LONG_EXPAND);
		lv_obj_set_style_local_text_color(timer_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));

#if TEST
		lv_obj_t *times_label = lv_label_create(time, NULL);
		lv_obj_set_style_local_text_color(times_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0000ff));
		lv_obj_align(times_label, time, LV_ALIGN_IN_RIGHT_MID, 0, 0);
		lv_label_set_text_fmt(times_label, "%d", test_times);

#endif
		if (monitor_timer_ptask != NULL)
		{
			lv_task_del(monitor_timer_ptask);
		}

		monitor_timer_ptask = lv_task_create(monitor_timer_task, 500, LV_TASK_PRIO_HIGH, timer_label);
		lv_task_ready(monitor_timer_ptask);

		monitor_timer_task(monitor_timer_ptask);
	}

	/*创建sd的容器*/
	lv_obj_t *sd_cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(sd_cont, 900, 105);
	lv_obj_set_size(sd_cont, 65, 40);
	lv_cont_set_layout(sd_cont, LV_LAYOUT_CENTER);

	lv_obj_t *sd_img = lv_img_create(sd_cont, NULL);

	if (is_sdcard_insert() == false)
	{

		static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_NOINSERT_PNG);
		lv_img_set_src(sd_img, &sd_info);
	}
	else if (is_sdcard_insert() == true)
	{

		struct statfs diskInfo;

		statfs(SD_BASE_PATH, &diskInfo);
		unsigned long long blocksize = diskInfo.f_bsize; //每个block里包含的字节数

		unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小

		if ((freeDisk < 100 * 1024 * 1024))
		{
			static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_FULL_PNG);
			lv_img_set_src(sd_img, &sd_info);
		}
		else if ((freeDisk >= 100 * 1024 * 1024))
		{
			static rom_bin_info sd_info = rom_bin_info_get(ROM_RES_SD_INSERT_PNG);
			lv_img_set_src(sd_img, &sd_info);
		}
	}
}

static void monitor_unlock_icon_create(void)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_id(cont, LV_MONITOR_UNLOCK_OBJ_ID);
	lv_obj_set_pos(cont, 437, 190);
	lv_obj_set_size(cont, 150, 150);

	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(39, 39, 39));

	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
	lv_obj_set_style_local_radius(cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 360);

	lv_obj_t *img = lv_img_create(cont, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MON_UNLOCK_ICON_PNG);

	lv_img_set_src(img, &info);
	lv_obj_align(img, cont, LV_ALIGN_CENTER, 0, 0);

	if (cont != NULL)
		lv_obj_set_hidden(cont, true);
}

static void monitor_outdoor_slider_event(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{

		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		MONITOR_CH monitor_ch = monitor_channel_get();
		network_device device = get_outdoor_device_by_channel(monitor_ch);
		data.device = device;
		data.arg1 = 2 | user_data_get()->user_language << 16;
		if (talking)
		{
			user_data_get()->audio.outdoor_talk_val = lv_slider_get_value(obj);
			// printf("slider value is %d\n",lv_slider_get_value(obj));
			data.arg2 = get_sound_val(user_data_get()->audio.outdoor_talk_val);
		}
		else
		{
			user_data_get()->audio.outdoor_ring_val = lv_slider_get_value(obj);
			// printf("slider value is %d\n",lv_slider_get_value(obj));
			data.arg2 = get_sound_val(user_data_get()->audio.outdoor_ring_val);
		}
		printf("=============outdoor_talk_val:%d\n", data.arg2);
		network_send_cmd_data(&data);
		user_data_save();
	}
}

static void monitor_indoor_slider_event(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		//	MONITOR_CH ch = monitor_channel_get();

		//未通话 改变的是door铃声大小
		if (talking == false)
		{
			user_data_get()->audio.door_ring_val = lv_slider_get_value(obj);

			door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, NULL);
		}
		else if (talking == true)
		{ //通话中 改变的是对应通道的声音大小

			user_data_get()->audio.door_talk_val = lv_slider_get_value(obj);
			audio_volume_set(get_sound_val(user_data_get()->audio.door_talk_val));
		}
		printf("=================door_talk_val:%d\n", user_data_get()->audio.door_talk_val);

		user_data_save();
	}
}

//创建调整户外机声音的滑块
static lv_obj_t *monitor_outdoor_slider_create(void)
{
	//创建容器
	lv_obj_t *outdoor_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_cont_set_layout(outdoor_cont, LV_FIT_NONE);
	lv_obj_set_size(outdoor_cont, 40, 290);
	lv_obj_set_pos(outdoor_cont, 68, 160);
	lv_obj_set_style_local_bg_color(outdoor_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x393939));
	lv_obj_set_style_local_bg_opa(outdoor_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
	lv_obj_set_style_local_radius(outdoor_cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 30);

	//创建图标
	outdoor_img = lv_img_create(outdoor_cont, NULL);

	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RING_PNG);
	lv_img_set_src(outdoor_img, &info1);

	lv_obj_align(outdoor_img, outdoor_cont, LV_ALIGN_IN_TOP_MID, 0, 10);

	//创建户外机图标
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_OUTDOOR_PNG);
	lv_obj_t *outdoor = lv_img_create(outdoor_cont, NULL);

	lv_img_set_src(outdoor, &info2);
	lv_obj_align(outdoor, outdoor_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

	outdoor_slider = lv_slider_create(outdoor_cont, NULL);
	lv_obj_set_id(outdoor_slider, 0);
	lv_obj_set_style_local_bg_color(outdoor_slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT,
									lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(outdoor_slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT,
									lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(outdoor_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
	lv_obj_align(outdoor_slider, outdoor_cont, LV_ALIGN_CENTER, 0, 0);
	lv_slider_set_range(outdoor_slider, 5, 35);

	//创建滑块

	lv_slider_set_value(outdoor_slider, user_data_get()->audio.outdoor_ring_val, LV_ANIM_OFF);

	lv_obj_set_size(outdoor_slider, 5, 200);
	lv_obj_align(outdoor_slider, outdoor_cont, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_ext_click_area(outdoor_slider, 22, 22, 10, 10);
	outdoor_slider->event_cb = monitor_outdoor_slider_event;

	lv_obj_set_hidden(outdoor_cont, true);

	return outdoor_cont;
}

//创建调整室内机声音的滑块
static lv_obj_t *monitor_indoor_slider_create(void)
{
	//创建容器
	lv_obj_t *indoor_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_cont_set_layout(indoor_cont, LV_FIT_NONE);
	lv_obj_set_size(indoor_cont, 40, 290);
	lv_obj_set_pos(indoor_cont, 914, 160);
	lv_obj_set_style_local_bg_color(indoor_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x393939));
	lv_obj_set_style_local_bg_opa(indoor_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
	lv_obj_set_style_local_radius(indoor_cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 30);

	//创建图标
	indoor_img = lv_img_create(indoor_cont, NULL);

	if (monitor_enter_flag_get() == MON_ENTER_TUYA)
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RING_PNG);
		lv_img_set_src(indoor_img, &info);
	}
	else
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RING_PNG);
		lv_img_set_src(indoor_img, &info);
	}
	lv_obj_align(indoor_img, indoor_cont, LV_ALIGN_IN_TOP_MID, 0, 10);

	//创建室内机机图标
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_PNG);
	lv_obj_t *indoor = lv_img_create(indoor_cont, NULL);

	lv_img_set_src(indoor, &info2);
	lv_obj_align(indoor, indoor_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

	//创建滑块
	indoor_slider = lv_slider_create(indoor_cont, NULL);
	lv_obj_set_id(indoor_slider, 1);
	lv_obj_set_style_local_bg_color(indoor_slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT,
									lv_color_make(0x66, 0x69, 0x68));
	lv_obj_set_style_local_bg_color(indoor_slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT,
									lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(indoor_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT,
									lv_color_make(0xCF, 0xA0, 0x75));
	lv_slider_set_range(indoor_slider, 0, 100);

	if (monitor_enter_flag_get() != MON_ENTER_TUYA)
	{
		lv_slider_set_value(indoor_slider, user_data_get()->audio.door_ring_val, LV_ANIM_OFF);
	}
	else
	{
		lv_slider_set_value(indoor_slider, user_data_get()->audio.door_ring_val, LV_ANIM_OFF);
	}

	lv_obj_set_size(indoor_slider, 5, 200);
	lv_obj_align(indoor_slider, indoor_cont, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_ext_click_area(indoor_slider, 22, 22, 10, 10);
	indoor_slider->event_cb = monitor_indoor_slider_event;

	lv_obj_set_hidden(indoor_cont, true);
	return indoor_cont;
}

/**
 * 监控界面按键命令处理函数
 * 处理监控模式下的按键输入，包括返回、切换通道、开锁和通话等功能
 * @param arg1 按键设备类型
 * @param arg2 附加参数(未使用)
 */
static void monitor_key_cmd_func(unsigned long arg1, unsigned long arg2)
{
    touch_sound_play();                             // 播放触摸反馈音
    MONITOR_CH ch = monitor_channel_get();        // 获取当前监控通道
    network_device device = get_outdoor_device_by_channel(ch); // 获取对应网络设备
    key_device key = (key_device)arg1;             // 获取按下的按键值
    
    // 返回键处理 - 返回到主界面
    if (key == key_return)                           
    {
        goto_layout(pLAYOUT(home));
        return;
    }
    // 监控切换键处理 - 切换监控通道
    else if (key == key_monitor)                    
    {
        // 通话或录制状态下不切换
        if (talking == true)                        
            return;
        if (recording == true)
            return;
        // 仅在涂鸦进入模式下允许切换通道
        if (monitor_enter_flag_get() != MON_ENTER_TUYA)
        {
            return;
        }

        // 查找下一个在线的室外设备并切换到对应通道
        for (int i = 0; i < 18; i++)
        {
            if ((net_online_device[DEVICE_UNIT_OUTDOOR_1 + i]) && (DEVICE_UNIT_OUTDOOR_1 + i != device))
            {
                monitor_channel_set(get_channel_by_outdoor_device(DEVICE_UNIT_OUTDOOR_1 + i));
                break;
            }
            if (i == 17)  // 没有找到可用通道
            {
                return;
            }
        }
        
        // 更新通道信息并发送声音提示命令
        ch = monitor_channel_get();                 
        device = get_outdoor_device_by_channel(ch);
        network_cmd_data_init(data);
        data.cmd = NET_COMMON_CMD_SOUND;
        data.arg1 = 3 | user_data_get()->user_language << 16;
        data.device = device;
        network_send_cmd_data(&data);

        // 设置监控超时时间，打开新通道并更新显示
        monitor_timer_set(60);                      
        monitor_channel_set(ch);                   
        monitor_open();                             
        monitor_channel_label_display();           
    }
    // 开锁键处理 - 触发开锁操作
    else if (key == key_lock)                       
    {
        // 已有解锁任务时不重复处理
        if (monitor_unlock_task_t != NULL)         
        {
            return;
        }
        
        // 显示解锁提示界面
        lv_obj_t *child = lv_obj_get_child_form_id(lv_scr_act(), LV_MONITOR_UNLOCK_OBJ_ID);
        if (child != NULL)
        {
            lv_obj_set_hidden(child, false);
        }
        
        // 发送开锁命令
        network_cmd_data_init(data);
        data.device = device;
        data.cmd = NET_COMMON_CMD_UNLOCK;
        data.arg1 = 1 | user_data_get()->user_language << 16;
        data.arg2 = user_data_get()->door1_delay | device << 16;
        network_send_cmd_data(&data);             
        net_send_floor_number();                  

        // 创建解锁任务，设置门铃声并播放
        if (monitor_unlock_task_t == NULL)         
        {
            monitor_unlock_task_t = lv_task_create(monitor_unlock_task, 1500, LV_TASK_PRIO_HIGH, door_btn);
        }
        open_door_ring_play(70);                   
    }
    // 通话键处理 - 控制双向通话
    else if (key == key_talk)                      
    {
        // 设置通话按钮状态
        monitor_btn_state_set(talk_btn, LV_STATE_DEFAULT);
        btn_data *pdata = (btn_data *)talk_btn->user_data;
        lv_obj_t *img = (lv_obj_t *)pdata->user_data;
        
        // 未处于通话状态，开启通话
        if (talking == false)                       
        {
            // 确保通话任务未创建
            if (montior_talk_task_t == NULL)        
            {
                talking = true;
                monitor_timer_set(60);             // 设置通话超时时间
                audio_play_stop_set();             // 停止当前播放的声音
                
                // 获取当前通道和设备信息
                static MONITOR_CH ch = MON_CH_NONE;
                ch = monitor_channel_get();
                network_device device = get_outdoor_device_by_channel(ch);

                // 更新UI显示，准备通话
                img->user_data = &ch;
                montior_talk_task_t = lv_task_create(monitor_talk_open_task, 100, LV_TASK_PRIO_MID, talk_btn);
                static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_TALK_PNG);
                lv_img_set_src(indoor_img, &info1);
                lv_slider_set_value(indoor_slider, user_data_get()->audio.door_talk_val, LV_ANIM_OFF);

                // 发送开启通话命令
                network_cmd_data_init(data);
                if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2))
                {
                    data.device = DEVICE_GROUP;    // 单元门设备组
                }
                else
                {
                    data.device = DEVICE_ALL;      // 所有设备
                }
                data.cmd = NET_COMMON_CMD_OUTDOOR_TALK;
                data.arg2 = user_data_get()->other.family_id;
                data.arg1 = ch;
                network_send_cmd_data(&data);

                // 发送设备忙状态和通话提示音命令
                data.cmd = NET_COMMON_CMD_DATA_BUSY;
                data.arg1 = device;
                data.arg2 = user_data_get()->other.family_id;
                network_send_cmd_data(&data);

                data.cmd = NET_COMMON_CMD_SOUND;
                data.arg1 = 1 | user_data_get()->user_language << 16;
                data.arg2 = get_sound_val(user_data_get()->audio.outdoor_talk_val);
                data.device = device;
                network_send_cmd_data(&data);
            }
        }
        // 已处于通话状态，关闭通话
        else if (talking == true)                   
        {
            audio_talk_close();                    // 关闭音频通话
            goto_layout(pLAYOUT(home));            // 返回主界面
            return;
        }

        // 重新对齐通话界面元素
        lv_obj_align(outdoor_img, outdoor_img->parent, LV_ALIGN_IN_TOP_MID, 0, 10);
        lv_obj_align(indoor_img, indoor_img->parent, LV_ALIGN_IN_TOP_MID, 0, 10);
        lv_obj_set_auto_realign(img, true);        // 启用自动重新对齐
    }
}

static void monitor_indoor_cmd_func(unsigned long arg1, unsigned long arg2)
{
	char cmd = (arg1 >> 8) & 0xFF;
	network_device out_device = arg2 & 0xFF;
	unsigned int family_id = arg2 >> 8;
	MONITOR_CH ch = monitor_channel_get();
	MONITOR_CH out_ch = get_channel_by_outdoor_device(out_device);

	switch (cmd)
	{
	case 1: /* in device 与 out_device 通话 */
		if (family_id != user_data_get()->other.family_id)
		{
			if ((out_device == DEVICE_UNIT_OUTDOOR_1) || (out_device == DEVICE_UNIT_OUTDOOR_2))
			{
				break;
			}
		}
		if ((talking == true) || (out_ch != ch))
		{
			/*不做任何处理*/
		}
		else
		{
			printf("=======%s=========%d=====\n", __func__, __LINE__);
			goto_layout(pLAYOUT(home));
		}

		break;

	case 2:
	{
	}
	break;

	default:
		printf("Parameter error :%d\n\r", cmd);
		break;
	}
}

void tuya_event_inside_proc(unsigned long arg1, unsigned long arg2)
{
	tuya_event ev = (tuya_event)arg1;
	// if ((monitor_enter_flag_get() != MON_ENTER_TUYA))
	// {
	// 	if (ev != TUYA_EVENT_MONITOR_ENTER)
	// 	{
	// 		return;
	// 	}
	// }
	if (ev == TUYA_EVENT_OPEN_DOOR || ev == TUYA_EVENT_OPEN_DOOR2)
    {
    }
    else if ((monitor_enter_flag_get() != MON_ENTER_TUYA))
    {
        if (ev != TUYA_EVENT_MONITOR_ENTER)
            return;
    }

	switch (ev)
	{

	/*切换监控*/
	case TUYA_EVENT_MONITOR_SWAP:
	{
		network_device device = get_outdoor_device_by_channel(arg2);
		if ((talking) || (monitor_data_busy_get(device) == true)) //正在通话或者，监控资源被其他户占用就不允许切换
		{
			break;
		}
		printf("arg2 = %ld\n", arg2);
		tuya_ipc_ring_buffer_video_release_data();
		MONITOR_CH ch = monitor_channel_get(); //获取当前通道
		network_device cur_device = get_outdoor_device_by_channel(ch);

		if (ch == arg2)
		{
			tuya_switch_channel_upload_results(ch);
			break;
		}
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		data.arg1 = 0 | user_data_get()->user_language << 16;
		data.arg2 = 0;
		data.device = cur_device;
		data.cmd = NET_COMMON_CMD_SOUND;
		network_send_cmd_data(&data);
		data.arg1 = 3 | user_data_get()->user_language << 16;
		data.arg2 = 1;
		data.device = device;
		network_send_cmd_data(&data);
		monitor_channel_set(arg2); //设置通道
		monitor_open();			   //打开该通道的监控
		audio_talk_close();
		audio_talk_open(device, true);
		// monitor_channel_label_display();	//通道的文本显示
		tuya_switch_channel_upload_results(arg2);
		break;
	}

		/*开锁*/

	case TUYA_EVENT_OPEN_DOOR:
	printf("==============%d================\n",  __LINE__);

	{
		MONITOR_CH monitor_ch = monitor_channel_get();
		network_device device = get_outdoor_device_by_channel(monitor_ch);
		printf("tuya_dp_148_response_accessory_lock %lu\n", arg2);
		if (arg2 != 0)
		{
			// tuya_dp_148_response_accessory_lock(true);
			network_cmd_data_init(data);
			data.device = device;
			data.cmd = NET_COMMON_CMD_UNLOCK;
			data.arg1 = 1 | user_data_get()->user_language << 16;
			data.arg2 = user_data_get()->door1_delay | device << 16;
			network_send_cmd_data(&data);
			net_send_floor_number();
		}
		else
		{
			network_cmd_data_init(data);
			data.device = device;
			data.cmd = NET_COMMON_CMD_LOCK;
			data.arg1 = 1 | device << 16;
			data.arg2 = 1;
			network_send_cmd_data(&data);
			// tuya_dp_148_response_accessory_lock(false);
		}

		break;
	}
	case TUYA_EVENT_OPEN_DOOR2:
	{
		MONITOR_CH monitor_ch = monitor_channel_get();
		network_device device = get_outdoor_device_by_channel(monitor_ch);
		if (arg2 != 0)
		{
			// tuya_dp_239_response_accessory_lock(true);

			network_cmd_data_init(data);
			data.device = device;

			data.cmd = NET_COMMON_CMD_UNLOCK;
			data.arg1 = 2 | user_data_get()->user_language << 16;
			data.arg2 = user_data_get()->door2_delay | device << 16;
			network_send_cmd_data(&data);
			net_send_floor_number();
		}
		else
		{
			network_cmd_data_init(data);
			data.device = device;

			data.cmd = NET_COMMON_CMD_LOCK;
			data.arg1 = 1 | device << 16;
			data.arg2 = 2;
			network_send_cmd_data(&data);
			// tuya_dp_239_response_accessory_lock(false);
		}
		break;
	}

		/*通话*/
	case TUYA_EVENT_TALK:
	{
		MONITOR_CH monitor_ch = monitor_channel_get();
		network_device device = get_outdoor_device_by_channel(monitor_ch);
		if (arg2 == true)
		{
			audio_play_stop_set(); //停止声音播放
			talking = true;

			if (audio_talk_open(device, true) == false)
			{
				printf("talk_device already is opened\n");
			}
			audio_volume_set(get_sound_val(user_data_get()->audio.door_talk_val));

			network_cmd_data_init(data);
			data.cmd = NET_COMMON_CMD_SOUND;
			data.arg1 = 1 | user_data_get()->user_language << 16;
			data.arg2 = 255;
			data.device = device;
			network_send_cmd_data(&data);

			if ((monitor_ch == MON_CH_UNIT_DOOR_1) || (monitor_ch == MON_CH_UNIT_DOOR_2))
			{
				data.device = DEVICE_GROUP;
			}
			else
			{
				data.device = DEVICE_ALL;
			}
			data.cmd = NET_COMMON_CMD_OUTDOOR_TALK;
			data.arg1 = monitor_ch;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);

			data.cmd = NET_COMMON_CMD_DATA_BUSY;
			data.arg1 = device;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);
			occupy_resource[device] = true;
		}
		else
		{
			talking = false;
			occupy_resource[device] = false;
			network_cmd_data_init(data);
			data.cmd = NET_COMMON_CMD_DATA_RELEASE;
			data.arg1 = device;
			data.arg2 = user_data_get()->other.family_id;
			if ((monitor_ch == MON_CH_UNIT_DOOR_1) || (monitor_ch == MON_CH_UNIT_DOOR_2))
			{
				data.device = DEVICE_GROUP;
			}
			else
			{
				data.device = DEVICE_ALL;
			}

			data.cmd = NET_COMMON_CMD_SOUND;
			data.arg1 = 0 | user_data_get()->user_language << 16;
			data.arg2 = 1;
			data.device = device;
			network_send_cmd_data(&data);
#if 0
				audio_talk_close(); //音频关闭
#endif
		}

		break;
	}

	/*进入监控*/
	case TUYA_EVENT_MONITOR_ENTER:
		/*Don't do anything*/
		{
			audio_talk_close();
			monitor_enter_flag_set(MON_ENTER_TUYA);
			play_ring_falg = false;
			audio_volume_set(get_sound_val(user_data_get()->audio.door_talk_val));

			MONITOR_CH monitor_ch = monitor_channel_get();
			network_device device = get_outdoor_device_by_channel(monitor_ch);
			tuya_switch_channel_upload_results(monitor_ch);
			network_cmd_data_init(data);
			data.cmd = NET_COMMON_CMD_SOUND;
			data.arg1 = 3 | user_data_get()->user_language << 16;
			data.arg2 = 1;
			data.device = device;
			network_send_cmd_data(&data);

			data.cmd = NET_COMMON_CMD_LOCK_STATE;
			data.arg1 = 2;
			data.arg2 = device;
			data.device = device;
			network_send_cmd_data(&data);
			occupy_resource[device] = false;

			if (audio_talk_open(device, true) == false)
			{
				printf("talk_device already is opened\n");
			}

#if 0
				data.cmd = NET_COMMON_CMD_DATA_BUSY_RELEASE;
				data.arg1 = device;
				data.arg2 = 0;
				if((monitor_ch == MON_CH_UNIT_DOOR_1) || (monitor_ch == MON_CH_UNIT_DOOR_2))
				{
					data.device = DEVICE_GROUP;
				}
				else
				{
					data.device = DEVICE_ALL;
				}
				network_send_cmd_data(&data);
#endif
		}

		break;

	/*退出监控*/
	case TUYA_EVENT_MONITOR_QUIT:
	{
		MONITOR_CH ch = monitor_channel_get();

		network_device device = get_outdoor_device_by_channel(ch);

		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		data.arg1 = 0 | user_data_get()->user_language << 16;
		data.arg2 = 1;
		data.device = device;
		network_send_cmd_data(&data);

		goto_layout(pLAYOUT(home));
	}
	break;

	/*设置离家模式*/
	case TUYA_EVENT_ABSENT_MODE:
	{

		user_data_get()->other.mode = 2;
		all_device_mode_sync();
		user_data_save();
	}
	break;
		/*设置居家模式*/
	case TUYA_EVENT_HOME_MODE:
	{
		user_data_get()->other.mode = 1;
		all_device_mode_sync();
		user_data_save();
	}
	break;

		/*设置睡眠模式*/
	case TUYA_EVENT_SLEEP_MODE:
	{
		user_data_get()->other.mode = 0;
		all_device_mode_sync();
		user_data_save();
	}
	break;

	default:

		/*Don't do anything*/
		break;
	}

	return;
}

void tuya_event_extern_proc(unsigned long arg1, unsigned long arg2)
{
	tuya_event ev = (tuya_event)arg1;
	switch (ev)
	{
	/*切换监控*/
	case TUYA_EVENT_MONITOR_SWAP:
		// tuya_ipc_ring_buffer_video_release_data();
		// tuya_switch_camera(arg2);
		//	tuya_ipc_ring_buffer_video_release_data();
		break;

	/*开锁*/
	case TUYA_EVENT_OPEN_DOOR:
	printf("==============%d================\n",  __LINE__);
	{
		network_device device = DEVICE_UNIT_OUTDOOR_1;
		printf("tuya_dp_148_response_accessory_lock %lu\n", arg2);
		if (arg2 != 0)
		{
			// tuya_dp_148_response_accessory_lock(true);
			network_cmd_data_init(data);
			data.device = device;
			data.cmd = NET_COMMON_CMD_UNLOCK;
			data.arg1 = 1 | user_data_get()->user_language << 16;
			data.arg2 = user_data_get()->door1_delay | device << 16;
			network_send_cmd_data(&data);
			net_send_floor_number();
		}
		else
		{
			network_cmd_data_init(data);
			data.device = device;
			data.cmd = NET_COMMON_CMD_LOCK;
			data.arg1 = 1 | device << 16;
			data.arg2 = 1;
			network_send_cmd_data(&data);
			// tuya_dp_148_response_accessory_lock(false);
		}

		break;
	}

	case TUYA_EVENT_OPEN_DOOR2:
	{
		network_device device = DEVICE_UNIT_OUTDOOR_1;
		printf("tuya_dp_239_response_accessory_lock %lu\n", arg2);
		if (arg2 != 0)
		{
			network_cmd_data_init(data);
			data.device = device;
			data.cmd = NET_COMMON_CMD_UNLOCK;
			data.arg1 = 1 | user_data_get()->user_language << 16;
			data.arg2 = user_data_get()->door1_delay | device << 16;
			network_send_cmd_data(&data);
			net_send_floor_number();
		}
		else
		{
			network_cmd_data_init(data);
			data.device = device;
			data.cmd = NET_COMMON_CMD_LOCK;
			data.arg1 = 1 | device << 16;
			data.arg2 = 1;
			network_send_cmd_data(&data);
		}

		break;
	}
	/*通话*/
	case TUYA_EVENT_TALK:
		break;

	/*进入监控*/
	case TUYA_EVENT_MONITOR_ENTER:
	{
		audio_talk_close();
		play_ring_falg = false;
		monitor_enter_flag_set(MON_ENTER_TUYA);
		for (int i = 0; i < 18; i++)
		{
			//判断设备是否在线，并且其他户有无在占用监控资源
			if ((net_online_device[DEVICE_UNIT_OUTDOOR_1 + i] == true) && (monitor_data_busy_get(DEVICE_UNIT_OUTDOOR_1 + i) == false))
			{
				MONITOR_CH monitor_ch = get_channel_by_outdoor_device(DEVICE_UNIT_OUTDOOR_1 + i);
				monitor_channel_set(monitor_ch);
				tuya_switch_channel_upload_results(monitor_ch);
				printf("=====monitor_ch is %d\n", monitor_ch);
				goto_layout(pLAYOUT(monitor));
				break;
			}
			if (i == 17)
			{
				return;
			}
		}
		MONITOR_CH monitor_ch = monitor_channel_get();
		network_device device = get_outdoor_device_by_channel(monitor_ch);
		tuya_switch_channel_upload_results(monitor_ch);
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		data.arg1 = 3 | user_data_get()->user_language << 16;
		data.arg2 = 1;
		data.device = device;
		network_send_cmd_data(&data);

		audio_volume_set(get_sound_val(user_data_get()->audio.door_talk_val));
		if (audio_talk_open(device, true) == false)
		{
			printf("talk_device already is opened\n");
		}

		data.cmd = NET_COMMON_CMD_LOCK_STATE;
		data.arg1 = 2;
		data.arg2 = device;
		data.device = device;
		network_send_cmd_data(&data);
		// occupy_resource[DEVICE_ALL] = true;

		key_call_event_register(NULL);
	}
	break;

	/*退出监控*/
	case TUYA_EVENT_MONITOR_QUIT:

		/*Don't do anything*/
		break;

	/*设置离家模式*/
	case TUYA_EVENT_ABSENT_MODE:
	{

		user_data_get()->other.mode = 2;
		all_device_mode_sync();
		user_data_save();
	}
	break;
		/*设置居家模式*/
	case TUYA_EVENT_HOME_MODE:
	{
		user_data_get()->other.mode = 1;
		all_device_mode_sync();
		user_data_save();
	}
	break;

		/*设置睡眠模式*/
	case TUYA_EVENT_SLEEP_MODE:
	{
		user_data_get()->other.mode = 0;
		all_device_mode_sync();
		user_data_save();
	}
	break;

	default:

		/*Don't do anything*/
		break;
	}

	return;
}

static void request_key_frame(struct _lv_task_t *task_t)
{
	//hlf:进监控获取关键帧
	for(int i=0;i<2;i++)
	{
		network_cmd_data_init(data);
		MONITOR_CH ch = monitor_channel_get() ;
		network_device device = get_outdoor_device_by_channel(ch);
		data.device = device;
		data.cmd = NET_COMMON_CMD_KEY_FRAME;
		data.arg1 = 1;
		data.arg2 = 0;
		network_send_cmd_data(&data);
	}
	lv_task_del(task_t);
}

static lv_task_t *check_key_ptask = NULL;

static lv_task_t *tuya_check_task_t = NULL;
static void tuya_task(struct _lv_task_t *task_t)
{
	if (tuya_ipc_get_client_online_num() >= 1 && (monitor_enter_flag_get() == MON_ENTER_TUYA))
	{

		play_ring_falg = false;

		fb_video_mode_enable(false);
		video_decode_close();

		lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
		lv_obj_set_size(cont, 1024, 600);
		lv_obj_set_pos(cont, 0, 0);
		lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0X000000));
		lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

		lv_obj_t *img = lv_img_create(cont, NULL);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TUYA_USING_PNG);

		lv_img_set_src(img, &info);
		lv_obj_align(img, cont, LV_ALIGN_CENTER, 0, 0);
		if (monitor_timer_ptask != NULL)
		{
			lv_task_del(monitor_timer_ptask);
			monitor_timer_ptask = NULL;
		}

		if (montior_talk_task_t != NULL)
		{
			lv_task_del(montior_talk_task_t);
			montior_talk_task_t = NULL;
		}

		if (check_key_ptask != NULL)
		{
			lv_task_del(check_key_ptask);
			check_key_ptask = NULL;
		}

		if (tuya_check_task_t != NULL)
		{
			lv_task_del(tuya_check_task_t);
			tuya_check_task_t = NULL;
		}
	}
}

void check_tuya(void)
{
	if (tuya_check_task_t != NULL)
		lv_task_del(tuya_check_task_t);

	tuya_check_task_t = lv_task_create(tuya_task, 1000, LV_TASK_PRIO_HIGH, NULL);
	lv_task_ready(tuya_check_task_t);
	tuya_task(tuya_check_task_t);
}

static void monitor_enter_parameter_init(void)
{
	monitor_timer_set(60);

	record_jpeg_event_register(monitor_record_jpeg_callback);
	record_video_event_register(monitor_record_video_callback);
	outdoor_call_event_register(monitor_call_inside_func);
	indoor_cmd_event_register(monitor_indoor_cmd_func);
	tuya_event_register(tuya_event_inside_proc);
}

#define DEVICE_KEY_CALL_TIME 1500

#if 0

static void ckeck_key_task(struct _lv_task_t *task_t){
	
	static key_device key_last = 0;
	key_device key = det_key_pin_call();
				
	if(key != false && key_last != key)
	{
		key_last = key;
	}
	else if(key == false && key_last != false)
	{
		printf("key value = %d\n",key_last);
		extern bool key_call_event_push(char arg1,char arg2);
		key_call_event_push(key_last,0);
		key_last = key;

	}
}

static void check_key(void){
	if(check_key_ptask != NULL)
		lv_task_del(check_key_ptask);
	
	check_key_ptask = lv_task_create(ckeck_key_task, 50, LV_TASK_PRIO_HIGH, NULL);
	lv_task_ready(check_key_ptask);
	ckeck_key_task(check_key_ptask);
}
#endif
static void layout_monitor_interphohe_call_func(unsigned long arg1, unsigned long arg2)
{
	if (talking)
	{
		if (arg1 == 1) //被呼叫
		{
			//呼叫本机的对象设备
			network_device interphone_call_mastar_device;
			interphone_call_mastar_device = (network_device)(arg2); // call的
			network_cmd_data_init(data);
			data.device = interphone_call_mastar_device;
			data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			data.arg1 = 5;
			data.arg2 = user_data_get()->other.network_device;
			network_send_cmd_data(&data);
		}
	}
	else
	{
		interphone_call_event_extern_func(arg1, arg2);
	}
}



static void LAYOUT_ENETER_FUNC(monitor)
{
	//hlf:在视频回放页call机会导致dma申请不了，所以关闭媒体缩略图释放部分dma.
	media_thumb_device_close();
	//将页面恢复为有线网卡的路由，恢复设备和设备之间的通信
	system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
	media_thumb_wait_thread_quit();
	printf("enter the page monitor\n");
#ifdef _PLATFORM_800_1280
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
#else
	//在刷出第一诊后，才在视频现实函数设置显示器和屏幕透明
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
#endif

	// test_times++;

	monitor_video_mode_open();

	monitor_enter_parameter_init(); //倒计时参数初始化
	
	standby_timer_close();

	interphone_call_event_register(layout_monitor_interphohe_call_func);

	if (monitor_enter_flag_get() != MON_ENTER_TUYA)
	{

		sdcard_event_register(playback_sd_status_change_callback);
		key_call_event_register(monitor_key_cmd_func);

		//	check_key();

		monitor_top_cont_create();

		monitor_time_label_create(); //时间文本创建

		monitor_snap_photo_btn_create(); //抓拍按钮创建

		monitor_record_video_btn_create(); //录像按钮创建

		monitor_talk_btn_create(); //通话按钮创建

		monitor_door1_btn_create(); //门1按钮创建

		monitor_door2_btn_create(); //门2按钮创建

		// monitor_light_btn_create();
		monitor_home_btn_create(); //返回home界面按钮创建

		monitor_channel_label_create(); //通道文本创建

		monitor_unlock_icon_create(); //解锁图标创建

		//解码
		monitor_outdoor_slider_create(); //创建调整户外机声音块

		monitor_indoor_slider_create(); //创建调整室内机声音的滑块
	}

	lv_task_create(request_key_frame, 100, LV_TASK_PRIO_LOW, NULL);

	play_ring_falg = true;
	sound_flag = true;
	recording = false;

	check_tuya();
}

//退出
static void LAYOUT_QUIT_FUNC(monitor)
{

	//将页面恢复为用户选择的路由，恢复设备和设备之间的通信
	if (user_data_get()->cctv_connect_mode == 0)
	{
		system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
	}
	else
	{
		system("route del -net 224.0.0.0 netmask 224.0.0.0 br0");
	}
	MONITOR_CH ch = monitor_channel_get();
	network_device device = get_outdoor_device_by_channel(ch);
	memset(occupy_resource, 0, DEVICE_TOTAL);
	network_cmd_data_init(data);
	data.cmd = NET_COMMON_CMD_SOUND;
	data.arg1 = 0 | user_data_get()->user_language << 16;
	data.arg2 = talking ? 1 : 0;
	data.device = device;
	network_send_cmd_data(&data);

	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	unsigned int mask = outdoor_call_mask_get(device);
	if (talking)
	{
		printf("=======send NET_COMMON_CMD_DATA_RELEASE\n");
		if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2))
		{
			data.device = DEVICE_GROUP;
		}
		else
		{
			data.device = DEVICE_ALL;
		}
		data.cmd = NET_COMMON_CMD_DATA_RELEASE;
		data.arg1 = device;
		data.arg2 = user_data_get()->other.family_id;
		network_send_cmd_data(&data);
	}
	else if (monitor_enter_flag_get() == MON_ENTER_CALL)
	{
		printf("mask is 0x%x\n", mask);
		if (mask == 0x00)
		{
			if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2))
			{
				data.device = DEVICE_GROUP;
			}
			else
			{
				data.device = DEVICE_ALL;
			}
			data.cmd = NET_COMMON_CMD_DATA_RELEASE;
			data.arg1 = device;
			data.arg2 = user_data_get()->other.family_id;
			network_send_cmd_data(&data);
		}
		else
		{
			data.cmd = NET_COMMON_CMD_DATA_BUSY_RELEASE;
			data.arg1 = device;
			data.arg2 = 0;
			if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2))
			{
				data.device = DEVICE_GROUP;
			}
			else
			{
				data.device = DEVICE_ALL;
			}
			network_send_cmd_data(&data);
		}
	}
	ring_play_time_stop();
	play_ring_falg = false;
	monitor_video_mode_close(); //监控和声音关闭

	if (monitor_timer_ptask != NULL) //
	{
		lv_task_del(monitor_timer_ptask); //
		monitor_timer_ptask = NULL;		  //
	}

	if (monitor_unlock_task_t != NULL) //解锁任务未退出
	{
		lv_task_del(monitor_unlock_task_t); //删除该任务
		monitor_unlock_task_t = NULL;		//置空指针
	}

	if (montior_talk_task_t != NULL) //对话任务未退出
	{
		lv_task_del(montior_talk_task_t); //删除对话人物
		montior_talk_task_t = NULL;		  //置空任务
	}

	//判断是否在线

	if (tuya_check_task_t != NULL) //对话任务未退出
	{
		lv_task_del(tuya_check_task_t); //删除对话人物
		tuya_check_task_t = NULL;		//置空任务
	}

	if (check_key_ptask != NULL) //
	{
		lv_task_del(check_key_ptask); //删除对话人物
		check_key_ptask = NULL;		  //置空任务
	}

	/*将此控件指向空,因为退出此页面，所有控件将被销毁*/
	outdoor_img = NULL;
	indoor_img = NULL;
	//	montior_busy_msg_box = NULL;

	sdcard_event_register(NULL);

	record_video_stop(0x00); //停止录像

	audio_talk_close();	   //音频关闭
	audio_play_stop_set(); //停止声音播放

	standby_timer_open(-1, NULL); //获取系统时间

	record_jpeg_event_register(NULL); //关闭所以注册表
	record_video_event_register(NULL);
	indoor_cmd_event_register(NULL);
	network_event_register(NULL);

	outdoor_call_event_register(monitor_call_extern_func); //继续回调
	tuya_event_register(tuya_event_extern_proc);		   //涂鸦
	key_call_event_register(key_call_extern_func);
	interphone_call_event_register(interphone_call_event_extern_func);

	monitor_auto_record_flag = false;
	tuya_sent_flag = false;
	recording = false;

	lv_obj_set_click(lv_scr_act(), false);
	lv_scr_act()->user_data = NULL;

	talking = false;
	monitor_enter_flag_set(MON_ENTER_NONE);

}

static void bell_press_finish_func(lv_task_t *task)
{
	door_chrime_en(false);
	lv_task_del(task);
}

static void door_chirm_callback_func(void)
{
	door_chrime_en(true);
	lv_task_create(bell_press_finish_func, 2000, LV_TASK_PRIO_MID, NULL);
}

//铃声播放时间到，终止铃声
static void ring_play_time_stop(void)
{
	if (door_ring_play_task != NULL)
	{
		lv_task_del(door_ring_play_task);
		door_ring_play_task = NULL;
	}
	audio_play_stop_set();
}

//外部呼叫的回调函数
void monitor_call_extern_func(unsigned long arg1, unsigned long arg2)
{
	network_device device = (network_device)arg1; //确定呼叫的门口机设备

	if ((device < DEVICE_UNIT_OUTDOOR_1) || (device > DEVICE_OUTDOOR_15)) //不是门口机 返回
	{
		return;
	}
	if (user_data_get()->other.family_id != arg2)
	{
		return;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	door_ring_timesmp = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	//根据选择对应通道的铃声和音量
	if (is_mode_status_on() != 0)
	{
		door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, door_ring_callback);
		if (door_ring_play_task != NULL)
		{
			lv_task_del(door_ring_play_task);
			door_ring_play_task = NULL;
		}
		door_ring_play_task = lv_task_create(ring_play_time_stop, user_data_get()->ring_time * 1000, LV_TASK_PRIO_MID, NULL);
	}
	MONITOR_CH ch = get_channel_by_outdoor_device(device);
	monitor_channel_set(ch);
	network_cmd_data_init(data);
#if 0
	if((device == DEVICE_UNIT_OUTDOOR_1) && (device ==  DEVICE_UNIT_OUTDOOR_2))
	{
		/*==========门口机呼叫时繁忙响应，通知组内其他室内机，当前设备响应了门口机呼叫=======*/
		data.device = DEVICE_GROUP;
	}
	else if((device >= DEVICE_OUTDOOR_0) && (device <= DEVICE_OUTDOOR_15))
	{
		data.device = DEVICE_ALL;
	}
	data.cmd = NET_COMMON_CMD_DATA_BUSY;
	data.arg1 = device;
	data.arg2 = user_data_get()->other.family_id;
	network_send_cmd_data(&data);
	occupy_resource[device] = true;
#endif
	data.cmd = NET_COMMON_CMD_SOUND;
	data.arg1 = 3 | user_data_get()->user_language << 16;
	data.device = device;

	printf("========arg1 is 0x%x\n", data.arg1);
	network_send_cmd_data(&data);

	monitor_auto_record_flag = true;
	tuya_sent_flag = true;
	monitor_enter_flag_set(MON_ENTER_CALL);
	door_chirm_callback_func();
	goto_layout(pLAYOUT(monitor));
}

static void key_call_extern_func(unsigned long arg1, unsigned long arg2)
{
	key_device key = (key_device)arg1; //获取到活动的按键值

	if (key == key_monitor)
	{
		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		data.arg1 = 3 | user_data_get()->user_language << 16;

		for (int i = 0; i < 18; i++)
		{
			if (net_online_device[DEVICE_UNIT_OUTDOOR_1 + i] == true)
			{
				monitor_channel_set(MON_CH_UNIT_DOOR_1 + i);
				data.device = DEVICE_UNIT_OUTDOOR_1 + i;
				break;
			}
			if (i == 16)
			{
				return;
			}
		}
		network_send_cmd_data(&data);
		monitor_enter_flag_set(MON_ENTER_MANUAL_DOOR);
		goto_layout(pLAYOUT(monitor));
	}
}

/************************************************************
 * @Description: 在监控页面内收到门口机call机
 * @Author: xiaoxiao
 * @Date: 2022-12-02 09:10:25
 * @LastEditTime: 2022-12-02
 * @explain:
 ************************************************************/
static void monitor_call_inside_func(unsigned long arg1, unsigned long arg2)
{
	network_device device = (network_device)arg1; //确定呼叫的门口机设备
	MONITOR_CH ch = monitor_channel_get();
	network_device cur_outdoor = get_outdoor_device_by_channel(ch);

	if ((device < DEVICE_UNIT_OUTDOOR_1) || (device > DEVICE_OUTDOOR_15)) //不是门口机 返回
	{
		return;
	}

	if (user_data_get()->other.family_id != arg2)
	{
		if ((device == DEVICE_UNIT_OUTDOOR_1) || (device == DEVICE_UNIT_OUTDOOR_2))
		{
			return;
		}
		else
		{
			if (cur_outdoor == device)
			{
				goto_layout(pLAYOUT(home));
			}
			return;
		}
	}
	else if (monitor_enter_flag_get() == MON_ENTER_TUYA)
	{
		goto_layout(pLAYOUT(home));
		monitor_call_extern_func(arg1, arg2);
		return;
	}

	if (get_channel_by_outdoor_device(device) != ch)
	{
		if ((device == DEVICE_UNIT_OUTDOOR_1) || (device == DEVICE_UNIT_OUTDOOR_2))
		{
			if (talking == true)
			{
				return;
			}
			else
			{
				printf("\nanyone else callllllllllllllllllllllllllllll\n");
				network_cmd_data_init(data);
				data.cmd = NET_COMMON_CMD_SOUND;
				data.arg1 = 0 | user_data_get()->user_language << 16;
				data.arg2 = talking;
				data.device = cur_outdoor;
				network_send_cmd_data(&data);
			}
		}
		else
		{
			if (talking == true)
			{
				audio_talk_close(); //音频关闭
				monitor_btn_state_set(talk_btn, LV_STATE_DEFAULT);
				btn_data *pdata = (btn_data *)talk_btn->user_data;
				lv_obj_t *img = (lv_obj_t *)pdata->user_data;

				rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_OFF_PNG);
				lv_img_set_src(img, &info);
				lv_obj_align(img, talk_btn, LV_ALIGN_CENTER, 0, -10);
				lv_obj_set_style_local_bg_color(talk_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0XFF, 0x3d, 0x3d));

				static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RING_PNG);

				lv_img_set_src(outdoor_img, &info1);
				lv_img_set_src(indoor_img, &info1);

				lv_slider_set_value(indoor_slider, user_data_get()->audio.door_ring_val, LV_ANIM_OFF);
				lv_obj_align(outdoor_img, outdoor_img->parent, LV_ALIGN_IN_TOP_MID, 0, 10);
				lv_obj_align(indoor_img, indoor_img->parent, LV_ALIGN_IN_TOP_MID, 0, 10);
				lv_obj_set_auto_realign(img, true); //图片重新对齐

				network_cmd_data_init(data);
				data.cmd = NET_COMMON_CMD_SOUND;
				data.arg1 = 0 | user_data_get()->user_language << 16;
				data.arg2 = talking;
				data.device = cur_outdoor;
				network_send_cmd_data(&data);

				talking = false;
				occupy_resource[cur_outdoor] = false;
				
				if ((ch == MON_CH_UNIT_DOOR_1) || (ch == MON_CH_UNIT_DOOR_2))
				{
					data.device = DEVICE_GROUP;
				}
				else
				{
					data.device = DEVICE_ALL;
				}
				data.cmd = NET_COMMON_CMD_DATA_RELEASE;
				data.arg1 = cur_outdoor;
				data.arg2 = user_data_get()->other.family_id;
				network_send_cmd_data(&data);
			}
				
		}
		record_video_stop(0x00);
		recording = false;
		audio_talk_close();
		monitor_channel_set(get_channel_by_outdoor_device(device));
		monitor_open();
		monitor_channel_label_display();
		// usleep(500 * 1000);
	}

	network_cmd_data_init(data);
	data.cmd = NET_COMMON_CMD_SOUND;
	data.arg1 = 3 | user_data_get()->user_language << 16;
	data.arg2 = talking;
	data.device = device;
	network_send_cmd_data(&data);
	audio_play_stop_set();
	monitor_timer_set(60);
	monitor_auto_record_flag = true;
	tuya_sent_flag = true;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	door_ring_timesmp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if (is_mode_status_on() != 0 && talking == false)
	{
		if (audio_output_buffer_query() == 0)
			door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, door_ring_callback);
		if (door_ring_play_task != NULL)
		{
			lv_task_del(door_ring_play_task);
			door_ring_play_task = NULL;
		}
		door_ring_play_task = lv_task_create(ring_play_time_stop, user_data_get()->ring_time * 1000, LV_TASK_PRIO_MID, NULL);
	}
	door_chirm_callback_func();
#if 0
	network_cmd_data_init(data);
	if((device == DEVICE_UNIT_OUTDOOR_1) && (device ==  DEVICE_UNIT_OUTDOOR_2))
	{
		/*==========门口机呼叫时繁忙响应，通知组内其他室内机，当前设备响应了门口机呼叫=======*/
		data.device = DEVICE_GROUP;
	}
	else if((device >= DEVICE_OUTDOOR_0) && (device <= DEVICE_OUTDOOR_15))
	{
		data.device = DEVICE_ALL;
	}

	data.cmd = NET_COMMON_CMD_DATA_BUSY;
	data.arg1 = device;
	data.arg2 = user_data_get()->other.family_id;
	network_send_cmd_data(&data);
	occupy_resource[device] = true;
#endif
	monitor_enter_flag_set(MON_ENTER_CALL);
}

void layout_monitor_init(void)
{
	outdoor_call_event_register(monitor_call_extern_func); //户外机呼叫事件注册表 回调
	key_call_event_register(key_call_extern_func);

	/*	if (tuya_event_extern_proc == NULL)
	{
	Debug("=========layout_monitor_init=========>>>>\n\n\n\n\n\n\n");
	}

	Debug("=========layout_monitor_init=========>>>>\n\n\n\n\n\n\n"); */
	for (int i = 0; i < 18; i++)
	{
		if (net_online_device[DEVICE_UNIT_OUTDOOR_1 + i])
		{
			set_tuya_channel_state(i + 1, true); //使能涂鸦通道的状态
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (user_data_get()->onvif_dev[i].url[0] != 0)
		{
			set_tuya_channel_state(i + 1, true); //使能涂鸦通道的状态
		}
	}
	tuya_event_register(tuya_event_extern_proc); //涂鸦时间注册表
}

static void door_ring_callback(void)
{
	if (play_ring_falg == false)
		return;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timesmp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if (abs(timesmp - door_ring_timesmp) < (user_data_get()->ring_time - 2) * 1000)
	{
		//根据选择对应通道的铃声和音量
		if (is_mode_status_on() != 0)
		{
			printf("\n铃声播放\n");
			printf("\n铃声播放--------------\n");
			door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, door_ring_callback);
			printf("\n铃声播放--------%d\n",user_data_get()->audio.door1_ring);
		}
	}
}

CREATE_LAYOUT(monitor);

void monitor_call_motion_func(unsigned long arg1, unsigned long arg2)
{
	network_device device = (network_device)arg1; //确定呼叫的门口机设备

	if ((device < DEVICE_UNIT_OUTDOOR_1) || (device > DEVICE_OUTDOOR_15)) //不是门口机 返回
	{
		return;
	}
	if (user_data_get()->other.family_id != arg2)
	{
		return;
	}
	goto_layout(pLAYOUT(close)); //创建一个空白layout，为了把监控的一些状态请0
	struct timeval tv;
	gettimeofday(&tv, NULL);
	door_ring_timesmp = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	//根据选择对应通道的铃声和音量 
	if (is_mode_status_on() != 0)
	{
		door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, door_ring_callback);
	}
	door_chirm_callback_func();
#if 0
	network_cmd_data_init(data);
	if((device == DEVICE_UNIT_OUTDOOR_1) && (device ==  DEVICE_UNIT_OUTDOOR_2))
	{
		/*==========门口机呼叫时繁忙响应，通知组内其他室内机，当前设备响应了门口机呼叫=======*/
		data.device = DEVICE_GROUP;
	}
	else if((device >= DEVICE_OUTDOOR_0) && (device <= DEVICE_OUTDOOR_15))
	{
		data.device = DEVICE_ALL;
	}

	data.cmd = NET_COMMON_CMD_DATA_BUSY;
	data.arg1 = device;
	data.arg2 = user_data_get()->other.family_id;
	network_send_cmd_data(&data);
	occupy_resource[device] = true;
#endif
	monitor_auto_record_flag = true;
	tuya_sent_flag = true;
	monitor_enter_flag_set(MON_ENTER_CALL);
	MONITOR_CH ch = get_channel_by_outdoor_device(device);
	monitor_channel_set(ch);
	goto_layout(pLAYOUT(monitor));
}

void tuya_event_motion_proc(unsigned long arg1, unsigned long arg2)
{
	tuya_event ev = (tuya_event)arg1;
	switch (ev)
	{
	/*切换监控*/
	case TUYA_EVENT_MONITOR_SWAP:
		// tuya_ipc_ring_buffer_video_release_data();
		// tuya_switch_camera(arg2);
		//	tuya_ipc_ring_buffer_video_release_data();
		break;

	/*开锁*/
	case TUYA_EVENT_OPEN_DOOR:
		// start_door_unlock();
		printf("==============%d================\n",  __LINE__);
		break;
	case TUYA_EVENT_OPEN_DOOR2:
		// start_door_unlock();
		break;

	/*通话*/
	case TUYA_EVENT_TALK:
		break;

	/*进入监控*/
	case TUYA_EVENT_MONITOR_ENTER:
	{
		MONITOR_CH monitor_ch = monitor_channel_get();
		tuya_switch_channel_upload_results(monitor_ch);
		network_device device = get_outdoor_device_by_channel(monitor_ch);
		monitor_enter_flag_set(MON_ENTER_TUYA);
		tuya_ipc_ring_buffer_video_release_data();
		record_video_stop(0x00);

		network_cmd_data_init(data);
		data.cmd = NET_COMMON_CMD_SOUND;
		data.arg1 = 3 | user_data_get()->user_language << 16;
		data.arg2 = 1;
		data.device = device;
		network_send_cmd_data(&data);
		if (audio_talk_open(device, true) == false)
		{
			printf("talk_device already is opened\n");
		}

		data.cmd = NET_COMMON_CMD_LOCK_STATE;
		data.arg1 = 2;
		data.arg2 = device;
		data.device = device;
		network_send_cmd_data(&data);
		//	occupy_resource[DEVICE_ALL] = true;

		tuya_event_register(tuya_event_inside_proc);

	}
	break;

	/*退出监控*/
	case TUYA_EVENT_MONITOR_QUIT:

		/*Don't do anything*/
		break;

	default:

		/*Don't do anything*/
		break;
	}

	return;
}
static lv_task_t *bell_press_default_event_task_t = NULL;
static int temp_bell_time;
static void bell_press_default_event_again()
{
	if (temp_bell_time > 2)
	{
		door_ring_play(user_data_get()->audio.door1_ring, get_sound_val(user_data_get()->audio.door_ring_val), NULL, bell_press_default_event_again);
	}
}

void bell_press_default_event_task_bd()
{
	printf("\n===1==temp_bell_time=%d=======\n", temp_bell_time);
	if (temp_bell_time > 0)
	{
		temp_bell_time--;
	}
	else
	{
		if (bell_press_default_event_task_t != NULL)
		{
			lv_task_del(bell_press_default_event_task_t);
			bell_press_default_event_task_t = NULL;
		}
		audio_play_stop_set();
		audio_play_stop_set();
	}
}

/************************************************************
** 函数说明: 门钟按压事件回调
** 作者: xiaoxiao
** 日期: 2023-04-24 21:08:25
** 参数说明:
** 注意事项:
************************************************************/
void bell_press_default_event(unsigned long arg1, unsigned long arg2)
{
	door_chirm_callback_func();
	struct timeval tv;
	gettimeofday(&tv, NULL);
	door_ring_timesmp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if (bell_press_default_event_task_t == NULL)
	{
		temp_bell_time = user_data_get()->ring_time;
		bell_press_default_event_again();
		bell_press_default_event_task_t = lv_task_create(bell_press_default_event_task_bd, 1000, LV_TASK_PRIO_MID, NULL);
	}
}
