#include "layout_define.h"
#include "leo_tuya_key_check.h"
#include <math.h>

extern bool wifi;

extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);
extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
extern char tuya_uid[64];
extern char tuya_key[64];
static int test_times = 0;
static char screen_str[8]; //用来存放屏的版本

static void system_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	
	// char *str[language_total] = {"System","系统","Система","Système","Sistema","System","النظام","Systém", "מערכת","Sistema","Sistema"};
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
	goto_layout(pLAYOUT(setting));
	
}


static lv_obj_t* system_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(system_back_btn_down, system_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
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

extern unsigned long long test_call_timesmp;

extern bool net_online_device[DEVICE_TOTAL];


static lv_task_t * test_ptask = NULL;

static void test_task(struct _lv_task_t *task_t){
	printf("testing\n");
	network_cmd_data_init(data);
	data.cmd = NET_COMMON_CMD_SOUND;
	data.arg1 = 3| user_data_get()->user_language << 16;

	for(int i = 0; i<18; i++)
	{
		if(net_online_device[DEVICE_UNIT_OUTDOOR_1 + i] == true)
		{
			monitor_channel_set(MON_CH_UNIT_DOOR_1 + i);
			data.device = DEVICE_UNIT_OUTDOOR_1 + i;
			break;
		}
		if(i == 16)
		{
			return ;
		}
	}

	network_send_cmd_data(&data);
	monitor_enter_flag_set(MON_ENTER_MANUAL_DOOR) ;
	goto_layout(pLAYOUT(monitor));

	
}


static void test_enter_cb(struct _lv_obj_t * obj, lv_event_t event)
{
	if(event == LV_EVENT_CLICKED )
	{
		test_times++;
		if(test_times == 5)
		{
			test_flag = true;

			if (test_ptask != NULL) 
				lv_task_del(test_ptask);
			test_ptask = lv_task_create(test_task, 68000, LV_TASK_PRIO_HIGH, NULL);

			//lv_task_ready(test_ptask);
			test_task(test_ptask);
			
		}
	}
}

static const char *network_device_link_level(int dev_id)
{
    char buf[64] = {0};
    static char result[32] = {0};
    memset(result, 0, sizeof(result)); 
    strcpy(result, "0");
    
    int level = 0;
    int quality = 0;
    double ratio = 0.0;
    bool is_link_down = false;
    // printf("[Wire Grade] dev_id=%d 开始计算\n", dev_id);

    sprintf(buf, "cat /proc/eth%d", dev_id);
    FILE *fp = popen(buf, "r");
    if (fp == NULL)
    {
        // printf("[Wire Grade] dev_id=%d popen失败\n", dev_id);
        return result;
    }

    
    while (fgets(buf, sizeof(buf), fp) > 0)
    {
        // printf("[Wire Grade] dev_id=%d 读取内容：%s", dev_id, buf);
        
        if (strstr(buf, "link_down"))
        {
            is_link_down = true;
            break;
        }
        if (strstr(buf, "quality"))
        {
            int ret = sscanf(buf, "quality: %d", &level); 
            printf("[Wire Grade] dev_id=%d 解析level：%d（sscanf返回值：%d）\n", dev_id, level, ret);
            break;
        }
    }
    pclose(fp);

    if (is_link_down)
    {
        strcpy(result, "0");
        // printf("[Wire Grade] dev_id=%d 链路断开，返回0\n", dev_id);
        return result;
    }
    if (level == 0)
    {
        strcpy(result, "0");
        // printf("[Wire Grade] dev_id=%d level=0，返回0\n", dev_id);
        return result;
    }

    ratio = 32768.0 / (3.0 * level);
    // printf("[Wire Grade] dev_id=%d level=%d → ratio=%.2f\n", dev_id, level, ratio);
    
    if (ratio <= 0)
    { 
        quality = 0;
    }else{
        double quality_double = 10.0 * log10(ratio); // 先存浮点值，方便打印
        quality = (int)quality_double; // 显式截断，便于调试
        // printf("[Wire Grade] dev_id=%d 浮点quality=%.2f → 截断后=%d\n", dev_id, quality_double, quality);
    }

    if (quality < 0) quality = 0;
    if (quality > 100) quality = 100;

    sprintf(result, "%d", quality);
    printf("[Wire Grade] dev_id=%d 最终返回：%s\n", dev_id, result);

    return result;
}

/**
 * 创建系统设置页面
 * 
 * 功能描述：
 * 构建系统设置界面，包含版本信息、发布日期、存储状态等系统参数显示
 * 
 * 参数：
 * @parent - 父容器对象指针
 * 
 * 页面布局：
 * 1. 顶部标题栏区域
 * 2. 可滚动内容区域，包含多个信息项
 * 3. 每项左侧显示名称，右侧显示对应值
 * 
 * 注意事项：
 * 1. 所有文本显示均支持多语言切换
 * 2. 版本号和发布日期支持动态更新
 * 3. 信息项通过胶水对象固定在页面上，确保滚动时不丢失
 */
static void system_page_create(lv_obj_t *parent)
{
    // 创建主内容容器，使用页面控件支持滚动
    lv_obj_t *system_cont = lv_page_create(parent,NULL);
    set_location(system_cont, 0, 100, 1024,500);  // 设置容器位置和大小
    lv_page_set_scrollable_fit4(system_cont, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_MAX, LV_FIT_MAX);  // 设置可滚动区域
    lv_page_set_scrollbar_mode(system_cont, LV_SCROLLBAR_MODE_DRAG);  // 设置滚动条模式为拖拽式
    // 设置容器内边距为0，优化空间利用
    lv_obj_set_style_local_pad_top(system_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(system_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_right(system_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_bottom(system_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    // 设置容器背景为深灰色
    lv_obj_set_style_local_bg_color(system_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
    lv_obj_set_style_local_bg_opa(system_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

    // 创建软件版本信息项
    // 左侧显示"Software version"标签，右侧显示具体版本号
    lv_obj_t *cont1 = system_btn_create(system_cont, 70, 0 ,877, 99,str_get(LAYOUT_SYSTEM_LANG_SOFTVERSION_ID));
    cont1->event_cb = test_enter_cb;  // 设置点击事件回调，用于进入测试模式
    lv_page_glue_obj(cont1, true);  // 将对象固定在页面上，防止滚动时丢失
    
    // 创建版本号标签
    lv_obj_t *label1 = lv_label_create(cont1, NULL);
    lv_obj_set_style_local_text_color(label1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    // 设置版本号文本，格式为"屏类型-软件版本"
    lv_label_set_text_fmt(label1, "%s%s", screen_str,"RAR41-TIP-V3.6.0_IPS");//RAR41-TIP-V3.5.8_IPS//RAR41-TIP-V3.5.2_IPS
    // 对齐到容器右侧中间位置
    lv_obj_align(label1, cont1, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    // 创建发布日期信息项
    // 左侧显示"Release date"标签，右侧显示具体日期
    lv_obj_t *cont2 = system_btn_create(system_cont, 70, 100 ,877, 99,str_get(LAYOUT_SYSTEM_LANG_RELEASEDATE_ID));
    lv_page_glue_obj(cont2, true);  // 将对象固定在页面上
    
    // 创建日期标签
    lv_obj_t *label2 = lv_label_create(cont2, NULL);
    lv_obj_set_style_local_text_color(label2, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    // 设置发布日期文本
    lv_label_set_text_fmt(label2, "%s", "2026-05-26");
    // 对齐到容器右侧中间位置
    lv_obj_align(label2, cont2, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    // 创建SD卡剩余空间信息项
    // 左侧显示"SD remain space"标签，右侧显示具体剩余空间
    lv_obj_t *cont3 = system_btn_create(system_cont, 70, 200 ,877, 99,str_get(LAYOUT_SYSTEM_LANG_SDSPACE_ID));
    lv_page_glue_obj(cont3, true);  // 将对象固定在页面上
    
    // 创建空间信息标签
    lv_obj_t *label3 = lv_label_create(cont3, NULL);
    lv_obj_set_style_local_text_color(label3, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    
    // 检查SD卡是否插入
    if(is_sdcard_insert())
    {
        // 获取SD卡状态信息
        struct statfs diskInfo;
        statfs(SD_BASE_PATH, &diskInfo);
        
        // 计算SD卡总容量（以GB为单位）
        unsigned long long blocksize = diskInfo.f_bsize;        // 每个block包含的字节数
        unsigned long long totalsize = blocksize * diskInfo.f_blocks;  // 总字节数
        int a = totalsize/107374182;  // 转换为GB（107374182 = 1024*1024*1024/10）
        
        // 计算SD卡剩余容量（以GB为单位）
        unsigned long long freeDisk = diskInfo.f_bfree * blocksize;  // 剩余空间字节数
        int b = freeDisk/107374182;  // 转换为GB
        
        // 设置显示格式为"剩余容量/总容量"，精确到小数点后1位
        lv_label_set_text_fmt(label3, "%d.%dG/%d.%dG", b/10, b%10, a/10, a%10);
    }
    else
    {
        // SD卡未插入时显示0.0G/0.0G
        lv_label_set_text_fmt(label3, "%s", "0.0G/0.0G");
    }
    // 对齐到容器右侧中间位置
    lv_obj_align(label3, cont3, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    // 创建UUID信息项
    // 左侧显示"UUID"标签，右侧显示设备唯一标识符
    #if 1
    static char sub_string[64] = {0};
    memset(sub_string,0,sizeof(sub_string));
    char serial[64] = {0};
    
    // 获取设备序列号
    if (tuya_serial_number_get(serial) == true)
    {
        strcat(sub_string, serial);
    }
    
    // 创建UUID容器和标签
    lv_obj_t *cont4 = system_btn_create(system_cont, 70, 300 ,877, 99,str_get(LAYOUT_SYSTEM_LANG_UUID_ID));
    lv_page_glue_obj(cont4, true);
    
    lv_obj_t *label4 = lv_label_create(cont4, NULL);
    lv_obj_set_style_local_text_color(label4, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    
    // 如果UUID存在，则显示"序列号+UUID"格式
    if(strlen(tuya_uid))
    {
        lv_label_set_text_fmt(label4, "(%s)%s",sub_string,tuya_uid);
    }
    else
    {
        lv_label_set_text(label4, "");  // UUID不存在时显示空
    }
    // 对齐到容器右侧中间位置
    lv_obj_align(label4, cont4, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    // 创建屏幕分辨率信息项
    // 左侧显示"Resolution"标签，右侧显示具体分辨率
    lv_obj_t *cont5 = system_btn_create(system_cont, 70, 400 ,877, 99,str_get(LAYOUT_SCREEN_RESOLETION_ID));
    lv_obj_t *label5 = lv_label_create(cont5, NULL);
    lv_page_glue_obj(cont5, true);
    lv_obj_set_style_local_text_color(label5, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    
    // 根据编译选项设置不同的分辨率显示
    #ifndef _PLATFORM_800_1280
        lv_label_set_text(label5, "1024 * 600");  // 默认分辨率
    #else
        lv_label_set_text(label5, "1280 * 800");  // 高分辨率平台
    #endif
    
    // 对齐到容器右侧中间位置
    lv_obj_align(label5, cont5, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    #endif

    // 创建Door1版本信息项
    // 左侧显示"Door1 version"标签，右侧显示Door1设备版本
    lv_obj_t *cont6 = system_btn_create(system_cont, 70, 500 ,877, 99, "Door1 version");
    lv_obj_t *label6 = lv_label_create(cont6, NULL);
    lv_page_glue_obj(cont6, true);
    lv_obj_set_style_local_text_color(label6, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    
    // 从函数获取Door1版本信息并显示
    lv_label_set_text(label6, door1_version_get());
    // 对齐到容器右侧中间位置
    lv_obj_align(label6, cont6, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    // 创建Door2版本信息项
    // 左侧显示"Door2 version"标签，右侧显示Door2设备版本
    lv_obj_t *cont7 = system_btn_create(system_cont, 70, 600 ,877, 99, "Door2 version");
    lv_obj_t *label7 = lv_label_create(cont7, NULL);
    lv_page_glue_obj(cont7, true);
    lv_obj_set_style_local_text_color(label7, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    
    // 从函数获取Door2版本信息并显示
    lv_label_set_text(label7, door2_version_get());
    // 对齐到容器右侧中间位置
    lv_obj_align(label7, cont7, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    // 创建线材等级
    lv_obj_t *cont8 = system_btn_create(system_cont, 70, 700 ,877, 99, "Wire grade1");
    lv_obj_t *label8 = lv_label_create(cont8, NULL);
    lv_page_glue_obj(cont8, true);
    lv_obj_set_style_local_text_color(label8, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色

    // lv_label_set_text(label8, network_device_link_level(0));
    lv_obj_align(label8, cont8, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    const char *wire_grade1 = network_device_link_level(0);
    lv_label_set_text(label8, wire_grade1);
    // 【新增：强制刷新LVGL界面】
    lv_obj_refresh_style(label8, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
    lv_obj_invalidate(label8); // 标记label需要重绘
    lv_obj_invalidate(cont8); // 标记父容器需要重绘
    lv_obj_align(label8, cont8, LV_ALIGN_IN_RIGHT_MID, 0, 0);





    lv_obj_t *cont9 = system_btn_create(system_cont, 70, 800 ,877, 99, "Wire grade2");
    lv_obj_t *label9 = lv_label_create(cont9, NULL);
    lv_page_glue_obj(cont9, true);
    lv_obj_set_style_local_text_color(label9, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));  // 设置文本颜色为灰色
    const char *wire_grade2 = network_device_link_level(1);

    lv_label_set_text(label9, wire_grade2);
    lv_obj_align(label9, cont9, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    lv_obj_refresh_style(label9, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
    lv_obj_invalidate(label9); // 标记label需要重绘
    lv_obj_invalidate(cont9); // 标记父容器需要重绘
    lv_obj_align(label9, cont9, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    #if 0	
    static char sub_string[64] = {0};
    memset(sub_string,0,sizeof(sub_string));
    char serial[64] = {0};
    const char *str5[language_total] = {"Serial number","Serial number","Serial number","Serial number","Serial number","Serial number","Serial number"};
    
    if (tuya_serial_number_get(serial) == true)
    {
        strcat(sub_string, serial);
    }
    
    // 创建序列号信息项（当前未启用）
    lv_obj_t *cont5 = system_btn_create(system_cont, 70, 400 ,877, 99,str5[user_data_get()->user_language]);
    lv_obj_t *label5 = lv_label_create(cont5, NULL);
    lv_obj_set_style_local_text_color(label5, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x787878));
    lv_obj_align(label5, cont5, LV_ALIGN_IN_LEFT_MID, 700, 0);
    lv_label_set_text_fmt(label5, "%s",sub_string);
    #endif
}
static void screen_check(){
	// system("cd /sys/firmware/devicetree/base/soc/lcdc@20010000/lcd-panel@0");
	//通过查询节点的状态来判断mipi屏和rgb屏，lcd-panel@0状态为okay是rgb屏，lcd-panel@9状态为okay是mipi屏
	FILE *pf = popen("cat /sys/firmware/devicetree/base/soc/lcdc@20010000/lcd-panel@9/status","r");
	char buffer[128] = {0};
	fgets(buffer,128,pf);
	pclose(pf);

	for(int i = 0;i < 4;i++){

	printf("\n===============%d================\n",buffer[i]);
	}
	//利用第一个字符是ascii编码是111来判断，111是o。mipi则显示，rgb不显示
	if(buffer[0] == 111){
		strncpy(screen_str,"MIPI-",5);
	}else{
		memset(screen_str,0,sizeof(screen_str));

	}
};

static void LAYOUT_ENETER_FUNC(set_system)
{
	//判断mipi版本和rgb版本
	screen_check();
	test_times = 0;
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	system_head_label_create();
	system_back_btn_create();
	
	//创建显示系统信息的容器
	system_page_create(lv_scr_act());
}


static void LAYOUT_QUIT_FUNC(set_system)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	test_times = 0;
}


CREATE_LAYOUT(set_system);

