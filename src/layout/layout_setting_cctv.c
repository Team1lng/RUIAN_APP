#include "layout_define.h"
#include "layout_setting_common.h"
//点击搜索到的cctv的ip索引
int connecting_cctv_ipaddr;
//点击已注册的cctv的索引
int current_cctv_index;
int added_num = 0;
int discovered_num = 0;
char ip_str[8][64] = {};
static lv_obj_t* cctv_page = NULL;
static lv_task_t* onvif_search_ptask = NULL;
static lv_task_t* cctv_status_fresh_ptask = NULL;
static void cctb_btn_create(int x,int y,lv_obj_t* parent,btn_data *btn_pdata, const void *img_src);
// void set_location(lv_obj_t *obj, int x, int y, int w, int h){
// 	lv_obj_set_pos(obj, x, y);
// 	lv_obj_set_size(obj, w, h);
// }

static void cancel_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}
//返回建
static void layout_bg_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(NULL, cancel_btn_up, NULL);
	printf("%x",ROM_RES_THUMB_EXIT_PNG);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
}

//页面标题创建
static void cctv_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	

	lv_label_set_text(obj, "CCTV");
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
}

static bool is_wifi_page_move = false;
static void cctv_page_touch_anything(lv_obj_t* obj,lv_event_t event)
{
	//printf("event：%d \n",event);
	if(LV_EVENT_DRAG_BEGIN  == event)
	{
		is_wifi_page_move = true;
	}
	else if(LV_EVENT_DRAG_END == event)
	{
		is_wifi_page_move = false;
	}

}

static void discovered_cctv_add(lv_obj_t* obj,lv_event_t event){
	connecting_cctv_ipaddr = obj->obj_id;
	connecting_cctv_ipaddr = connecting_cctv_ipaddr-200;
	printf("\n======================%d\n",connecting_cctv_ipaddr);
	goto_layout(pLAYOUT(add_cctv));
}
static void registerd_cctv_detaiil(lv_obj_t* obj,lv_event_t event){
	current_cctv_index = obj->obj_id;
	current_cctv_index = current_cctv_index-100;
	enter_layout_passwd_ch(PASSWD_CH_CCTV_INFORMATION);
	goto_layout(pLAYOUT(setting_password));
}
//给已搜索到的cctv创建按钮
static void cctv_discovered_create(lv_obj_t *parent, int x, int y, int w, int h,const char *cctv_str,int index){
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_id(btn, index+200);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	// lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	//cctv的名称
	lv_obj_t *label = lv_label_create(btn, NULL);

	lv_label_set_text(label,cctv_str);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);
	static btn_data btn_data = btn_data_up_create(discovered_cctv_add);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
}
//给保存到的cctv创建按钮
static void cctv_register_create(lv_obj_t *parent, int x, int y, int w, int h,char *cctv_str,int index){
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_id(btn, index+100);
	lv_obj_set_ext_click_area(btn,5,5,5,5);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	// lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	//cctv的状态
	lv_obj_t *cctv_mode = lv_obj_create(btn, NULL);
	lv_obj_set_id(cctv_mode,1);
	static rom_bin_info pwd_info_false = rom_bin_info_get(ROM_RES_ONLINE_TRUE_PNG);
	static rom_bin_info pwd_info_true = rom_bin_info_get(ROM_RES_ONLINE_FALSE_PNG);
	lv_obj_align(cctv_mode,btn,LV_LABEL_ALIGN_LEFT,0,0);
	lv_obj_align(cctv_mode, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);
	lv_obj_set_style_local_pattern_image(cctv_mode, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &pwd_info_false);
	lv_obj_set_style_local_pattern_image(cctv_mode, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, &pwd_info_true);
	//通过保存的标识来判断在线状态，以显示相对应的图标
	if(user_data_get()->onvif_dev[index].cannel == 1){
		lv_obj_set_state(cctv_mode,LV_STATE_CHECKED);
		//set_tuya_channel_state(index+19,true);
	}
	else{
		lv_obj_set_state(cctv_mode,LV_STATE_DEFAULT);
	}
	//cctv的名称
	lv_obj_t *label = lv_label_create(btn, NULL);
	char buffer[12];
	sprintf(buffer,"%s %d",cctv_str,index+1);
	lv_label_set_text(label,buffer);
	lv_obj_align(label,btn,LV_LABEL_ALIGN_LEFT,0,0);
	// lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 50, 0);
	static btn_data btn_data = btn_data_up_create(registerd_cctv_detaiil);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
}
// 刷新键回调
static void cctv_fresh_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_fresh_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(set_cctv));
	
}

// 添加键回调
static void cctv_add_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void cctv_add_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(write_cctv));
	
}

//创建切换连接方式按钮
static void cctv_connect_mode(lv_obj_t* parent,int btn_id,btn_data *btn_pdata, const void *img_src){
	lv_obj_t *btn_change = lv_btn_create(parent, NULL);
	lv_obj_set_id(btn_change,btn_id);
	set_location(btn_change,850, user_data_get()->onvif_dev_count*60+90, 60, 60);
	lv_obj_set_style_local_radius(btn_change, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 45);
	lv_obj_set_style_local_radius(btn_change, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 45);
	lv_obj_t *img_add = lv_img_create(btn_change, NULL);
	lv_img_set_src(img_add, img_src);
	setting_btn_img_transform_set(img_add);
	lv_obj_align(img_add, btn_change, LV_ALIGN_CENTER, 0, 0);
	(btn_pdata)->user_data = img_add;
	btn_change->user_data = btn_pdata;
	btn_touch_event_listen(btn_change);
	lv_obj_set_ext_click_area(btn_change, 10, 10, 10, 10);
	//默认隐藏wifi连接键
	printf("\n====================当前按钮的id=>%d\n",btn_id);
	printf("\n====================当前连接模式=>%d\n",user_data_get()->cctv_connect_mode);
	if(btn_id != user_data_get()->cctv_connect_mode){
		lv_obj_set_hidden(btn_change,true);
	}
}

//点击有线连接按钮
static void line_connect_event(lv_obj_t *obj ,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(obj,true);
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_obj_get_parent(obj),1),false);
		system("route del -net 224.0.0.0 netmask 224.0.0.0 br0");
		user_data_get()->cctv_connect_mode = 1;
		user_data_save();
		goto_layout(pLAYOUT(set_cctv));
	}
}
//点击无线连接按钮
static void wire_connect_event(lv_obj_t *obj ,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		lv_obj_set_hidden(obj,true);
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_obj_get_parent(obj),0),false);
		system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
		user_data_get()->cctv_connect_mode = 0;
		user_data_save();
		goto_layout(pLAYOUT(set_cctv));
	}
}

//创建整个cctv页面
static void cctv_page_create(lv_obj_t* parent){
	//创建页面
	cctv_page = lv_page_create(parent,NULL);//在当前活跃的屏幕上创建页面

	static btn_data page_data = btn_data_anything_create(cctv_page_touch_anything);
	cctv_page->user_data = &page_data;
	btn_touch_event_listen(cctv_page);

	set_location(cctv_page, 0,100, 1024,500);
	lv_page_set_scrollbar_mode(cctv_page, LV_SCROLLBAR_MODE_DRAG);
	lv_obj_set_style_local_bg_color(cctv_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(cctv_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cctv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(cctv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT,LV_OPA_COVER);

	//已注册的cctv
	//标题
	lv_obj_t *discovered_label = lv_label_create(cctv_page, NULL);
	lv_label_set_text(discovered_label, str_get(LAYOUT_CCTV_LANG_ADDDEVICE_ID));//wifi名称
	set_location(discovered_label, 49,10, 200,30);
		printf("\n======已注册的cctv_num==>%d=======\n",user_data_get()->onvif_dev_count);

	//已注册的cctv的列表//
	for(int i = 0;i<user_data_get()->onvif_dev_count;i++){
		printf("\n======re_num==>%s=======\n",user_data_get()->onvif_dev[i].ip);
		// printf("\n======re_num==>%d=======\n",user_data_get()->onvif_dev_count);
		cctv_register_create(cctv_page,70,i*70+40,842,70,str_get(LAYOUT_CCTV_LANG_CCTV_ID),i);
	}
	//切换搜索cctv的模式按钮
	//有线
	static btn_data btn_line_data = btn_data_anything_create(line_connect_event);
	static rom_bin_info line_info = rom_bin_info_get(ROM_RES_CCTV_LINE_CONNECT_PNG);
	cctv_connect_mode(cctv_page,0,&btn_line_data,&line_info);
	//wifi
	static btn_data btn_wire_data = btn_data_anything_create(wire_connect_event);
	static rom_bin_info wire_info = rom_bin_info_get(ROM_RES_CCTV_WIRE_CONNECT_PNG);
	cctv_connect_mode(cctv_page,1,&btn_wire_data,&wire_info);

	//搜索到的cctv
	//标题
	lv_obj_t *searched_label = lv_label_create(cctv_page, NULL);
	lv_label_set_text(searched_label, str_get(LAYOUT_CCTV_LANG_DISCOVERDDEVICE_ID));
	set_location(searched_label, 49,user_data_get()->onvif_dev_count*60+110, 220,30);
	//刷新键
	static btn_data btn_data_fresh = btn_data_create(cctv_fresh_btn_down, cctv_fresh_btn_up, NULL);
	static rom_bin_info info_fresh = rom_bin_info_get(ROM_RES_WIFI_FRESH_PNG);
	cctb_btn_create(850,0,cctv_page,&btn_data_fresh,&info_fresh);
	//添加键
	static btn_data btn_data_add = btn_data_create(cctv_add_btn_down, cctv_add_btn_up, NULL);
	static rom_bin_info info_add = rom_bin_info_get(ROM_RES_CCTV_ADD_PNG);
	cctb_btn_create(780,0,cctv_page,&btn_data_add,&info_add);
	#if 0
	lv_obj_t *btn = lv_btn_create(cctv_page, NULL);
	// set_location(btn,880, 110+user_data_get()->onvif_dev_count*70+40+117, 60, 60);
	set_location(btn,850, 0, 60, 60);
	// set_location(btn,850, user_data_get()->onvif_dev_count*60+80, 60, 60);
	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

	lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 45);
	lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 45);
	lv_obj_t *img = lv_img_create(btn, NULL);
	lv_img_set_src(img, &info);
	setting_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);
	(&btn_data)->user_data = img;
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
	lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);
	#endif
}

//创建圆型按钮
static void cctb_btn_create(int x,int y,lv_obj_t* parent,btn_data *btn_pdata, const void *img_src){
		lv_obj_t *btn = lv_btn_create(parent, NULL);
	// set_location(btn,880, 110+user_data_get()->onvif_dev_count*70+40+117, 60, 60);
	set_location(btn,x, y, 60, 60);
	// set_location(btn,850, user_data_get()->onvif_dev_count*60+80, 60, 60);
	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
	lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

	lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 45);
	lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 45);
	lv_obj_t *img = lv_img_create(btn, NULL);
	lv_img_set_src(img, img_src);
	setting_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);
	(btn_pdata)->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);
}



static void cctv_searching_task(){
	// printf("\n-----------11111111111-  status=%d---------\n", sat_ipcamera_status_get());

	if (sat_ipcamera_status_get() == true)
	{
		return;
	}
	if (sat_ipcamera_status_get() == false)
		{
			discovered_num = sat_ipcamera_online_num_get();
			printf("\n-----------总的cctv数量%d----------\n",discovered_num);
			
			int vaild_ip = 0;
			int vaild_height = 0;
			int same_flag = 0;
			for (int i = 0; i < discovered_num; i++)
			{	

				const char *ip_addr = sat_ipcamera_ipaddr_get(i);
				printf("ip地址=============%d:%s\n", i, ip_addr);
				for(int i = 0;i<user_data_get()->onvif_dev_count;i++){
					if(user_data_get()->onvif_dev_count != 0){
						if(strcmp(ip_addr,user_data_get()->onvif_dev[i].ip) == 0){
							same_flag = 1;
						}
					}
					// printf("次数=============%d,ip========>%s,是否隐藏======>%d\n", i,user_data_get()->onvif_dev[i].ip,same_flag);
				}
				if(same_flag == 0){
					printf("添加ip地址=============%d:%s\n", i, ip_addr);
					strcpy(ip_str[vaild_ip],ip_addr);
					//创建搜索到的cctv列表
					cctv_discovered_create(cctv_page,70,vaild_height*70+(user_data_get()->onvif_dev_count*60)+135,842,70,ip_addr,vaild_ip);
					vaild_height++;
				}
					vaild_ip++;
					same_flag = 0;
		
			}
			lv_task_del(onvif_search_ptask);
			onvif_search_ptask = NULL;
		}
	
}

// 将在线的cctv通道上传给tuya
static void online_tuya_cctv_check(){
	// for(int i = 0;i < 8;i++){
	// 	set_tuya_channel_state(19+i,false);
	// }
}

static void cctv_searching(){
	sat_ipcamera_device_discover_search(0);
    onvif_search_ptask = lv_task_create(cctv_searching_task, 500, LV_TASK_PRIO_MID, &clock);  // 1.5秒任务
}

//刷新cctv状态任务
static void cctv_status_fresh_task(){
	for(int i = 0;i<user_data_get()->onvif_dev_count;i++){
		//先将cctv在线状态置零
		if((strlen(user_data_get()->onvif_dev[i].url) > 1) && net_work_ping(user_data_get()->onvif_dev[i].ip)){
			user_data_get()->onvif_dev[i].cannel = 1;		
			lv_obj_set_state(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(cctv_page,0xFFFFFF),100+i),1),LV_STATE_CHECKED);

		}else{
			user_data_get()->onvif_dev[i].cannel = 0;
			lv_obj_set_state(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_obj_get_child_form_id(cctv_page,0xFFFFFF),100+i),1),LV_STATE_DEFAULT);
		}
		// printf("\n\n====================刷新完成=========================\n\n");
	}
	user_data_save();
	if(cctv_status_fresh_ptask != NULL){
		lv_task_del(cctv_status_fresh_ptask);
		cctv_status_fresh_ptask = NULL;
	}
}

//刷新已注册的cctv的状态
static void fresh_cctv_status(){
	cctv_status_fresh_ptask = lv_task_create(cctv_status_fresh_task, 100, LV_TASK_PRIO_MID, &clock);
}

//初始化当前搜索的状态
static void set_cctv_connect_mode(){
	if(user_data_get()->cctv_connect_mode == 0){
		system("route add -net 224.0.0.0 netmask 224.0.0.0 br0");
	}else{
		system("route del -net 224.0.0.0 netmask 224.0.0.0 br0");
	}
}
static void LAYOUT_ENETER_FUNC(set_cctv)
{
	set_cctv_connect_mode();
	fresh_cctv_status();
	online_tuya_cctv_check();    //将在线的cctv通道上传给tuya
	cctv_page_create(lv_scr_act());
	memset(ip_str,0,sizeof(ip_str));
	cctv_searching();
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置
	layout_bg_cancel_btn_create();
	cctv_head_label_create();
}
static void LAYOUT_QUIT_FUNC(set_cctv)
{
if(onvif_search_ptask != NULL){
	lv_task_del(onvif_search_ptask);
	onvif_search_ptask = NULL;

}
if(cctv_status_fresh_ptask != NULL){
	lv_task_del(cctv_status_fresh_ptask);
	cctv_status_fresh_ptask = NULL;

}
cctv_page = NULL;

}

CREATE_LAYOUT(set_cctv);