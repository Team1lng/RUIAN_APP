#include "layout_define.h"
#include "tuya_ipc_api.h"
int udhcpc_flag = 0;
extern void setting_btn_img_transform_set(lv_obj_t *obj);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);

extern lv_obj_t* input_textarea_create(lv_obj_t* parent, int x,int y, int w, int h, bool pwd_mode, const char * txt);


extern int wpa_cli_check_connect_status(void);//检查连接的状态-1: psk error 1: connect  0:other error
extern bool wpa_cli_connect_new_wifi(const char* ssid,const char *psk);//连接新的wifi 成功返回true 失败返回false
extern int wpa_cli_scan_wifi(bool *continue_flag);//搜索wifi 并且返回wifi数量 失败-1
extern bool get_scanned_wifi_info(int index ,wifi_info* info);//获取指定结点的搜索的wifi结点
extern void get_linked_wifi_info(linked_info* info);//获取连接的wifi的信息
extern void turn_on_wlan_connect(void);//打开wifi连接
extern void turn_off_wlan_connect(void);//关闭wifi连接
extern void update_wifi_list(void);//更新wifi列表
extern int get_max_wifi_list_index(void);//获取wifi列表内的wifi数量
extern bool wpa_cli_wlan_status(bool *continue_flag);

extern void turn_on_walan_reconnect(void);
extern void CloseConnectingWifi(void);

extern lv_obj_t* connect_wifi_cb(void);
extern void set_msg_text(lv_obj_t *msg,int state);


extern void StartConnectingWifi(void);

extern void turn_off_wlan_break(void);
static void tuya_qrcode_destroy(void);
extern linked_info link_info ;

int connected_wifi_max = 0;
int connectwifi_index = 0;
char connectwifi_name[24] = {0};
static lv_obj_t *qr = NULL;


static lv_task_t *msg_ui_ptask = NULL;
static bool connect_falge = false;

static lv_obj_t *msg = NULL;

#define CONNECTING 			0
#define CONNECT_SUCCESS 	1
#define CONNECT_FAIL 		2
#define WIFINAME_EMPTY		3
#define WIFIPWD_SHOT 		4
#define NOT_FIND 			5


static int task_timer = 0;


static lv_obj_t* wifi_page = NULL;


void set_location(lv_obj_t *obj, int x, int y, int w, int h){
	lv_obj_set_pos(obj, x, y);
	lv_obj_set_size(obj, w, h);
}



static void freemsg_task(struct _lv_task_t *task_t){
	if(connect_falge){

		if(task_timer <= 12){
			task_timer++;
			int connect_ret = wifi_connection_status_sucess();
			if(connect_ret == 1&&(task_timer > 6)){//连接成功

				system("rm -rf /etc/config/wpa_supplicant.conf &");
				system("cp -rf /tmp/wpa_supplicant.conf /etc/config/ &");
				system("sync");
				user_data_get()->wifi.wifi_connect_flag = true;
				user_data_save();
				
				goto_layout(pLAYOUT(set_wifi));
				lv_obj_t* msg1 = connect_wifi_cb();
				set_msg_text(msg1, CONNECT_SUCCESS);
				
				tuya_ipc_reconnect_wifi();
				task_timer = 0;
			}
		}else{//连接失败
			connect_falge = false;
			task_timer = 0;
			if(msg)
				lv_obj_del(msg);
   			system("killall wpa_supplicant &");
    		system("killall udhcpc &");
			system("rm -rf /tmp/wpa_supplicant.conf &");
			
			system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B &");
			system("udhcpc -i wlan0 -n 4 -R &");
	
			goto_layout(pLAYOUT(connect_wifi));
			//在键盘的确认按钮的回调上是将密码匹配wifi名称并且尝试连接 连接成功 在用户数据中保存 失败 直接清空密码 并且提示
			lv_obj_t* msg1 = connect_wifi_cb();
			set_msg_text(msg1, CONNECT_FAIL);
		}
	}else
		return ;
}


//页面标题创建
static void wifi_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	
	lv_label_set_text(obj, "Wifi");
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	
}



//返回设置页面按钮
static void wifi_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void wifi_back_btn_up(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(setting));
	
}

static lv_obj_t* wifi_back_btn_create(void)
{
	static btn_data btn_data = btn_data_create(wifi_back_btn_down, wifi_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	// printf("%x",ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}

static void wifi_fresh_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void wifi_fresh_btn_up(lv_obj_t *obj)
{
	printf("\n=======udhcpc_flag=>%d==========\n",udhcpc_flag);
	if(udhcpc_flag>0 && udhcpc_flag<3){
		udhcpc_flag++;
	}else{
		if(udhcpc_flag == 3 || udhcpc_flag > 3){
			udhcpc_flag = 0;
		}else{
			udhcpc_flag++;
		}
		printf("\n\n=======udhcpc_begin==========\n\n");

		system("killall udhcpc");
		system("udhcpc -i wlan0 &");

	}
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B");
	tuya_ipc_reconnect_wifi();
	goto_layout(pLAYOUT(set_wifi));
	
}


static lv_obj_t* wifi_fresh_btn_create(void){
	static btn_data btn_data = btn_data_create(wifi_fresh_btn_down, wifi_fresh_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_WIFI_FRESH_PNG);
	lv_obj_t* btn = setting_btn_create(939, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);
	return btn;

}




//返回wifi界面的按钮的回调
static void add_wifi_back_btn_up(lv_obj_t *obj){
	//清除二维码
	tuya_qrcode_destroy();

	/*清空保存的wifi名称*/
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	lv_label_set_text(lv_obj_get_child_back(lv_scr_act(), NULL), "Wifi");//设置标题
	lv_obj_del(lv_obj_get_child(lv_scr_act(), NULL));
	lv_obj_del(obj);

}


//从输入wifi的界面返回
static void input_wifi_back_btn_up(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	lv_obj_del(lv_obj_get_child(lv_scr_act(), obj));
	lv_obj_del(obj);
}

void back_btn_create(void){
	static btn_data btn_data = btn_data_create(wifi_back_btn_down, input_wifi_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);//创建返回按钮
	lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);
}





static bool is_wifi_page_move = false;
static void wifibtn_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){

		if(is_wifi_page_move == true)
		{
			return ;
		}

		memset(connectwifi_name,0,strlen(connectwifi_name));
		sprintf(connectwifi_name,"%s",lv_label_get_text(lv_obj_get_child_back(obj, NULL)));
		connectwifi_index = obj->obj_id ;
		goto_layout(pLAYOUT(connect_wifi));
	}
}

static void freewifibtn_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){

		if(is_wifi_page_move == true)
		{
			return ;
		}
		memset(connectwifi_name,0,strlen(connectwifi_name));
		sprintf(connectwifi_name,"%s",lv_label_get_text(lv_obj_get_child_back(obj, NULL)));
		connectwifi_index = obj->obj_id ;


		connect_falge = true;
		wpa_cli_connect_new_wifi(lv_label_get_text(lv_obj_get_child_back(obj, NULL)),NULL);
		
		msg = connect_wifi_cb();
		set_msg_text(msg,CONNECTING);
	}
}



//添加wifi按钮的回调

static void add_wifi_btn_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		goto_layout(pLAYOUT(add_wifi));
	}
}


//创建添加wifi的按钮
static lv_obj_t *wifipage_addwifi_btn_create(lv_obj_t *parent, int x, int y, int w, int h)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);

	//lv_obj_set_parent_event(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_btn_set_layout(btn, LV_LAYOUT_OFF);


	//wifi的名称
	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	// char * src[language_total] = {"Add network","添加网络","Добавить сеть","Ajouter un réseau" ,"Añadir red","Netzwerk hinzufügen","اضافة شبكة ","Přidat síťové připojení","הוספת רשת","Adicionar Rede Wi-Fi","Aggiungi rete"};
	lv_label_set_text(label, str_get(LAYOUT_SETNETWORK_LANG_ADDNETWORK_ID));
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);


	//wifi的图片 根据信号强度选择对应的图片
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ADD_WIFI_PNG);
	lv_obj_t *img = lv_img_create(btn, NULL);

	lv_img_set_src(img, &info);
	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);

	static btn_data btn_data = btn_data_anything_create(add_wifi_btn_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}




//创建wifi按钮
static lv_obj_t *wifipage_btn_create(lv_obj_t *parent, int x, int y, int w, int h, char *string)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

	//wifi的名称
	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_text(label, string);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	static btn_data btn_data = btn_data_up_create(NULL);
	btn->user_data = &btn_data;

	btn_touch_event_listen(btn);

	return btn;
}

//给已经搜索到的wifi创建显示按钮
static lv_obj_t *wifi_btn_create(lv_obj_t *parent, int x, int y, int w, int h,wifi_info *w_info,int wifi_index)
{
	
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	
	lv_obj_set_id(btn, wifi_index);
	
	lv_obj_set_drag_parent(btn, true);
	
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

	//wifi的名称
	lv_obj_t *label = lv_label_create(btn, NULL);
	 
	lv_label_set_text(label,w_info->ssid);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	//根据搜索到的wifi的信号强度 和上锁情况 选择对应的图片显示(-100, -88] (-88, -77] (-77, -55] rssi>=-55)
	lv_obj_t *img = lv_img_create(btn, NULL);
	

	if(w_info->signal_level > -55 && w_info->signal_level < 0){//信号满格
	
		if(w_info->free){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_C_PNG);
			lv_img_set_src(img, &info);
		}
		
	}else if(w_info->signal_level <= -55 && w_info->signal_level > -77){//2格信号
	
		if(w_info->free){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_C_PNG);
			lv_img_set_src(img, &info);
		}
	}else if(w_info->signal_level <= -77 && w_info->signal_level){//一格信号
	
		if(w_info->free){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_C_PNG);
			lv_img_set_src(img, &info);
		}
	}else if(w_info->signal_level <= -88){
		if(w_info->free){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	
	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, 0, 0);
	
	//单独的回调函数
	if (w_info->free == true){
		static btn_data btn_data = btn_data_anything_create(freewifibtn_up);
		btn->user_data = &btn_data;
	}
	else {
		static btn_data btn_data = btn_data_anything_create(wifibtn_up);
		btn->user_data = &btn_data;
	}
	btn_touch_event_listen(btn);
	
	connect_falge = false;

	if (msg_ui_ptask != NULL)
	{
	lv_task_del(msg_ui_ptask);
	}
	msg_ui_ptask = lv_task_create(freemsg_task, 300, LV_TASK_PRIO_HIGH, NULL);
	lv_task_ready(msg_ui_ptask);
	freemsg_task(msg_ui_ptask);

	return btn;

}

extern linked_info link_info ;


static void tuya_qrcode_destroy(void)
{
	if (qr != NULL)
	{
		ak_mem_free((void *)(lv_canvas_get_img(qr)->data));
		lv_qrcode_delete(qr);
		qr = NULL;
	}
}

extern char *tuya_qrcode_str_get(void);
extern bool is_online_tuya_cloud(void);
static void tuya_qrcode_display(lv_obj_t *parent)
{

	if (!is_online_tuya_cloud() || !wifi_connection_status_sucess() || tuya_qrcode_str_get() == NULL){
		printf("\n\n=======================>>>> tuya qrcode info not get \n\n");

		return;
	}
	
	if (qr == NULL)
	{
		char *info = tuya_qrcode_str_get();
		printf("=======================>>>> tuya qrcode info get : [%s]\n", info);
		if (info == NULL)
		{
			printf("tuya qrcode info get failed ......\n");
			return;
		}
		
		rom_bin_info img = rom_bin_raw_get();
		/*****  data:手动分配，不需要的时候需要手动释放 *****/
		unsigned char *data = ak_mem_alloc(MODULE_ID_APP, 200 * 200 * LV_COLOR_SIZE / 8);
		rom_bin_raw_init(img, data, 200, 200);
		lv_obj_t *temp_par =  lv_obj_create(parent,NULL);
		lv_obj_set_id(temp_par, 31);
		lv_obj_set_pos(temp_par, 425, 245);
		lv_obj_set_size(temp_par, 210, 210);
		lv_obj_set_style_local_radius(temp_par,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,10);
		lv_obj_set_style_local_bg_color(temp_par,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_bg_opa(temp_par,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

		qr = lv_qrcode_create(parent, &img, 200, lv_color_hex(0x1e1e1e), lv_color_hex(0xececec));
		lv_obj_set_id(qr, 32);
		lv_obj_set_pos(qr, 430, 250);
		lv_obj_set_size(qr, 200, 200);
		lv_qrcode_update(qr, info, strlen(info));
	}
	else
	{
		lv_obj_set_hidden(qr, false);
	}
}










//点击已经连接的wifi按钮 显示已经连接的wifi信息

static void wifi_info_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		wpa_cli_wlan_status(&user_data_get()->wifi.wifi_connect_flag);
		get_linked_wifi_info(&link_info);

		// char *src[language_total] = {"Wifi info","Wifi信息","Информация Wi-Fi","Infos Wifi","Información wifi","WLAN-Info","معلومات شبكة الواي فاي","Informace o wifi", "מידע WIFI","Informações de Sistema","Informazioni Wi-Fi"};
		lv_label_set_text(lv_obj_get_child_back(lv_scr_act(), NULL), str_get(LAYOUT_SETNETWORK_LANG_WIFIINFO_ID));//设置标题
		lv_obj_align(lv_obj_get_child_back(lv_scr_act(), NULL), lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);


		static btn_data btn_data = btn_data_create(wifi_back_btn_down, add_wifi_back_btn_up, NULL);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
		lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);//创建返回按钮
		lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);

		lv_obj_t* info_cont = lv_cont_create(lv_scr_act(),NULL);//在当前活跃的屏幕上创建容器
		set_location(info_cont, 0, 100, 1024, 500);
		lv_cont_set_layout(info_cont, LV_LAYOUT_OFF);
		lv_obj_set_style_local_bg_color(info_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
		lv_obj_set_style_local_bg_opa(info_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

		// char *src1[language_total] = {"IP address:","",,"Adresse IP:","Dirección IP:","IP-Adresse:","عنوان IP:",	 "IP adresa:", "כתובת IP:","Endereço IP:","Indirizzo IP:"};
		// char *src2[language_total] = {"Security:","安全性:","Безопасность:","  Sécurité:","Seguridad:","Sicherheit:","الامان:", 	 "Zapezpečení:", "אבטחה:","Segurança:","Sicurezza"};
		
		//===============================cyy："Айпи адрес:"=======================
		lv_obj_t *ip_btn = wifipage_btn_create(info_cont, 70, 0, 842, 99, str_get(LAYOUT_SETNETWORK_LANG_IPADDRESS_ID));
		lv_btn_set_layout(ip_btn, LV_LAYOUT_OFF);

		
		lv_obj_t *security_btn = wifipage_btn_create(info_cont, 70, 100, 842, 99, str_get(LAYOUT_SETNETWORK_LANG_SECURITY_ID));								  
		lv_btn_set_layout(security_btn, LV_LAYOUT_OFF);


		lv_obj_t *ip_label = lv_label_create(ip_btn, NULL);
		//获取已经连接的wifi的ip并且显示 
		lv_label_set_text(ip_label,link_info.ip);
		lv_obj_set_style_local_text_color(ip_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x393939));
		lv_obj_align(ip_label, ip_btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);

		lv_obj_t *security_label = lv_label_create(security_btn, NULL);
		//获取已经连接的wifi的安全性并且显示 
		lv_label_set_text(security_label,link_info.key_mgmt);
		lv_obj_set_style_local_text_color(security_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x393939));
		lv_obj_align(security_label, security_btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);
		//获取涂鸦二维码
		tuya_qrcode_display(info_cont);
	}
}


//给已经连接的wifi创建显示按钮
static lv_obj_t *connectwifi_btn_create(lv_obj_t *parent, int x, int y, int w, int h){
	
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));
	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

	//wifi的名称

	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_text(label, link_info.wlan_ssid);//wifi名称
	
	
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	//根据搜索到的wifi的信号强度 和上锁情况 选择对应的图片显示(-100, -88] (-88, -77] (-77, -55] rssi>=-55)
	

	lv_obj_t *img = lv_img_create(btn, NULL);
	
	if(link_info.level > -55 && link_info.level < 0){//信号满格
	
		if(strcmp(link_info.key_mgmt, "NONE") == 0){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_C_PNG);
			lv_img_set_src(img, &info);
		}
		
	}else if(link_info.level <= -55 && link_info.level > -77){//2格信号
	
		if(strcmp(link_info.key_mgmt, "NONE") == 0){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_C_PNG);
			lv_img_set_src(img, &info);
		}
	}else if(link_info.level <= -77 && link_info.level > -88){//一格信号
	
		if(strcmp(link_info.key_mgmt, "NONE") == 0){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_C_PNG);
			lv_img_set_src(img, &info);
		}
	}else if(link_info.level <= -88){
		if(strcmp(link_info.key_mgmt, "NONE") == 0){
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_PNG);
			lv_img_set_src(img, &info);
		}else{
			static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	
	
	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, 0, 0);
	
	//单独的回调函数
	static btn_data btn_data = btn_data_anything_create(wifi_info_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}


/*	
	搜索wifi 并且创建wifi按钮和添加wifi按钮
	添加wifi按钮 搜索到的wifi数量 添加wifi按钮在最后面 坐标是 搜索到的wifi的数量+1 *100
	将搜索wifi到的创建按钮并且显示wifi名称 根据网络信号强度选择显示的图标
	搜索到的wifi数量统计 
	线的数量应该是wifi数量+1
*/


static void findwifi_wifibtn_create(lv_obj_t* parent){
	int8_t sum = 0;//划线的数量
	connectwifi_index = 0;

	//打开后 
	bool a = true;
	wpa_cli_scan_wifi(&a);
	wpa_cli_wlan_status(&a);

	memset(&link_info,0,sizeof(linked_info));
	get_linked_wifi_info(&link_info);


	if(link_info.completed){
		user_data_get()->wifi.wifi_connect_flag = true;
	
		//wifi已经连接 创建按钮显示当前wifi的按钮
		connectwifi_btn_create(parent, 70, 100, 842, 100);
				
		//创建添加wifi的按钮
		wifipage_addwifi_btn_create(parent, 70, 200, 842, 100);//修改回调 点击按钮之后显示wifi名称 ip 密码 和 加密方式
		sum = 3;
	}else {
		wifipage_addwifi_btn_create(parent, 70, 100, 842, 100);
		sum	= 2;
	}
	
#if 0
	wpa_cli_scan_wifi(&a);
	wpa_cli_wlan_status(&a);
#endif

	connected_wifi_max = get_max_wifi_list_index();//获取到搜索到的wifi的总数
	printf("@@@@@@@@@@@@@@@@@@@@@@@%d\n",connected_wifi_max);

	static wifi_info info;
	for(int i = 0; i < connected_wifi_max; i++){
		memset(&info,0,sizeof(wifi_info));
		get_scanned_wifi_info(i ,&info);//获取指定结点的搜索的wifi结点
		
		if(strcmp(info.ssid,link_info.wlan_ssid)  == 0){
			sum--;
			continue;
		}
		if(info.hidden == true){
			sum--;
			continue;
		}
		wifi_btn_create(parent, 70, (i+sum) * 100 , 842, 100, &info , i);
	}
	
}




//wifi界面打开或者关闭wifi的回调
static void wifi_page_sw_cb(lv_obj_t* obj){
	if(lv_switch_get_state(obj)){

		StartConnectingWifi();
		
		user_data_get()->wifi.wifi_open_flag = true;
		
		ak_sleep_ms(1000);
		
		findwifi_wifibtn_create(wifi_page);
		tuya_ipc_reconnect_wifi();
		goto_layout(pLAYOUT(set_wifi));
		
	}else if(!lv_switch_get_state(obj)){
		
		/*wifi按钮 删除*/
		lv_obj_t *child = lv_obj_get_child_back(wifi_page, obj);
		
		while(child){
			lv_obj_del(child);
			child = lv_obj_get_child_back(wifi_page, child);
		}
		
		user_data_get()->wifi.wifi_open_flag = false;
		user_data_get()->wifi.wifi_connect_flag = false;
		CloseConnectingWifi();//将wifi标志设置为false 关闭了wifi连接
	}
	user_data_save();

}



static void wifi_page_touch_anything(lv_obj_t* obj,lv_event_t event)
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


//创建整个关于wifi的页面 可以上下滑动
static void wifi_page_create(lv_obj_t* parent){
	//创建页面
	wifi_page = lv_page_create(parent,NULL);//在当前活跃的屏幕上创建页面

	static btn_data page_data = btn_data_anything_create(wifi_page_touch_anything);
	wifi_page->user_data = &page_data;
	btn_touch_event_listen(wifi_page);

	
	set_location(wifi_page, 0,100, 1024,500);
	lv_page_set_scrollbar_mode(wifi_page, LV_SCROLLBAR_MODE_DRAG);
	lv_obj_set_style_local_bg_color(wifi_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(wifi_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(wifi_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(wifi_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT,LV_OPA_COVER);


	//创建一个按钮 但是设置为不可点击 作用是提示 右侧摆放wifi的开关
	lv_obj_t *btn = wifipage_btn_create(wifi_page, 70, 0, 842, 100, "Wifi");
	lv_obj_set_click(btn, false);
	lv_btn_set_layout(btn, LV_LAYOUT_OFF);
	

	lv_obj_t *sw = lv_switch_create(wifi_page, NULL);//创建sw
	lv_obj_set_size(sw, 53, 26);
	lv_obj_align(sw, btn, LV_ALIGN_IN_RIGHT_MID, 0, 0);
	
	lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xCE9F74));
	lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x6D6D6D));
	lv_obj_set_style_local_border_width(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_outline_width(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, 0);
	lv_switch_set_anim_time(sw,40);
	lv_obj_set_ext_click_area(sw, 10, 10, 15, 15);
	

	static btn_data sw_data = btn_data_up_create(wifi_page_sw_cb);
	sw->user_data = &sw_data;
	btn_touch_event_listen(sw);

	if(user_data_get()->wifi.wifi_open_flag){//如果进来wifi是打开的 那么需要搜索和创建一些按钮

		findwifi_wifibtn_create(wifi_page);//搜索wifi并且创建按钮
		
		lv_switch_on(sw, LV_ANIM_OFF);
		
	}else
		lv_switch_off(sw, LV_ANIM_OFF);//如果wifi是关闭状态 无需搜索

}

static lv_task_t * set_scan_wifi_ptask = NULL;


static void scan_wifi_task(struct _lv_task_t *task_t){
	bool a = true;
	wpa_cli_scan_wifi(&a);
	wpa_cli_wlan_status(&a);
	
}

static void scan_wifi(void){
	
	if (set_scan_wifi_ptask != NULL) {
			lv_task_del(set_scan_wifi_ptask);
	}
	
	set_scan_wifi_ptask = lv_task_create(scan_wifi_task, 2000, LV_TASK_PRIO_MID, NULL);
	lv_task_ready(set_scan_wifi_ptask);
	
	scan_wifi_task(set_scan_wifi_ptask);

}

static void LAYOUT_ENETER_FUNC(set_wifi)
{
	is_wifi_page_move = false;
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	wifi_head_label_create();//创建标题

	wifi_back_btn_create();//仿照取消按钮 创建一个返回按钮

	wifi_page_create(lv_scr_act());

	wifi_fresh_btn_create();//
	
	scan_wifi();
}

static void LAYOUT_QUIT_FUNC(set_wifi)
{
	tuya_qrcode_destroy();
	is_wifi_page_move = false;
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	if (msg_ui_ptask != NULL)//计时任务未退出
	{
		lv_task_del(msg_ui_ptask);//退出计时任务
		msg_ui_ptask = NULL;//置空指针
	}
	
	if (set_scan_wifi_ptask != NULL) //计时任务未退出
	{
		lv_task_del(set_scan_wifi_ptask);			//退出计时任务
		set_scan_wifi_ptask	= NULL; 				//置空指针
	}

}


CREATE_LAYOUT(set_wifi);

