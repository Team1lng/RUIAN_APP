#include "layout_define.h"
#include "tuya_ipc_api.h"

extern void back_btn_create(void);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern int wifi_connection_status_sucess(void);

extern lv_obj_t* input_textarea_create(lv_obj_t* parent, int x,int y, int w, int h, bool pwd_mode, const char * txt);
// extern const char * get_addwifi_name(void);
// extern const char * get_addcctv_pwd(void);
extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);
extern void keyboard_layout(lv_obj_t* obj);
extern bool wpa_cli_wlan_status(bool *continue_flag);

extern const char * * kb_map[];
extern const lv_btnmatrix_ctrl_t * kb_ctrl[];

extern int connecting_cctv_ipaddr;
extern int added_num;
extern char ip_str[8][64];
static  char addcctv_name[24] = {0};
static  char addcctv_pwd[24] = {0};
static  char addcctv_index[24] = {0};
static  char write_cctv_brand[24] = {0};
static int stream_flag= 0;
static int sub_stream_id=0;

#define CONNECTING 			0
#define CONNECT_SUCCESS 	1
#define CONNECT_FAIL 		2
#define WIFINAME_EMPTY		3
#define WIFIPWD_SHOT 		4
#define NOT_FIND 			5

static lv_obj_t *name_label = NULL;
static lv_obj_t *pwd_label = NULL;
static lv_obj_t *brand_label = NULL;

static lv_task_t *msg_ui_ptask = NULL;
static lv_obj_t* stream_cont = NULL;

static int task_timer = 0;
static int format_falg = 0;


static void substream_change_cont_create();

//键盘的回调函数
static void lv_keyboard_event1_cb(lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

  //  if(event != LV_EVENT_VALUE_CHANGED) return;

    lv_keyboard_ext_t * ext = lv_obj_get_ext_attr(kb);
    uint16_t btn_id   = lv_btnmatrix_get_active_btn(kb);
    if(btn_id == LV_BTNMATRIX_BTN_NONE) return;
    if(lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_HIDDEN | LV_BTNMATRIX_CTRL_DISABLED)) return;
  //  if(lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT) return;

    const char * txt = lv_btnmatrix_get_active_btn_text(kb);//获得活跃的按钮的文本
    if(txt == NULL) return;

    /*Do the corresponding action according to the text of the button*/
    if(strcmp(txt, "abc") == 0) {
        ext->mode = LV_KEYBOARD_MODE_TEXT_LOWER;
        lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_LOWER]);
        lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_LOWER]);
		
		lv_keyboard_img_set(kb,10,NULL);
		
		static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
		lv_keyboard_img_set(kb,11,&info11);
		
		static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
		lv_keyboard_img_set(kb,22,&info22);
		
        return;
    }
#if LV_USE_ARABIC_PERSIAN_CHARS == 1
    else if(strcmp(txt, "أب") == 0) {
        ext->mode = LV_KEYBOARD_MODE_TEXT_ARABIC;
        lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_ARABIC]);
        lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_ARABIC]);
        return;
    }
#endif
    else if(strcmp(txt, "ABC") == 0) {
        ext->mode = LV_KEYBOARD_MODE_TEXT_UPPER;
        lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_UPPER]);
        lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_UPPER]);

//对图片进行调整、
		lv_keyboard_img_set(kb,10,NULL);
			
		static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
		lv_keyboard_img_set(kb,11,&info11);
		
		static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
		lv_keyboard_img_set(kb,22,&info22);

        return;
    }
    else if(strcmp(txt, "1#") == 0) {
        ext->mode = LV_KEYBOARD_MODE_SPECIAL;
        lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_SPECIAL]);
        lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_SPECIAL]);
		
		static rom_bin_info info10 = rom_bin_info_get(ROM_RES_KB_DELETL_PNG);
		lv_keyboard_img_set(kb,10,&info10);
		
		lv_keyboard_img_set(kb,11,NULL);

		lv_keyboard_img_set(kb,22,NULL);
        return;
    }

	
/****************************************设置模式************************************************************************/
/*Add the characters to the text area if set*/

	if(ext->ta == NULL) return;

	  if(ext->btnm.pattern_p[btn_id]){ 
	  	
		  	if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_CANCEL_PNG){ /*LV_SYMBOL_CLOSE*/
				    
		        lv_textarea_set_text(ext->ta, ""); /*De-assign the text area to hide it cursor if needed*/
		        return; 
			
			} else if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_APPLY_PNG) {/*LV_SYMBOL_OK*/
			
			lv_obj_t* child = lv_obj_get_child_form_id (kb->parent ,0);
			if(child == kb){//密码模式

				memset(addcctv_pwd,0,strlen(addcctv_pwd));
				sprintf(addcctv_pwd,"%s",lv_textarea_get_text(ext->ta));
				char a[25] = "*";
				char *b = "*";
				for(int i = strlen(addcctv_pwd); i>1; i--){
					strncat(a,b,1);
				}
				lv_label_set_text((lv_obj_t *)kb->parent->user_data,a);
				lv_obj_del(lv_obj_get_child(lv_scr_act(),NULL));
				lv_obj_del(kb->parent);
				return;
			}
			
			child = lv_obj_get_child_form_id (kb->parent ,1);
			if (child == kb) {
					
				memset(addcctv_name,0,strlen(addcctv_name));
				sprintf(addcctv_name,"%s",lv_textarea_get_text(ext->ta));
			
				// if(strlen(addcctv_name) < 1){
				// 	memset(addcctv_name,0,strlen(addcctv_name));
				// 	lv_label_set_text((lv_obj_t *)kb->parent->user_data,addcctv_name);
				
				// 	// lv_obj_t *msg = connect_wifi_cb();
				// 	// set_msg_text(msg,WIFINAME_EMPTY);
				// 	return ;
				// }

				lv_label_set_text((lv_obj_t *)kb->parent->user_data,addcctv_name);

				lv_obj_del(lv_obj_get_child(lv_scr_act(),NULL ));
				lv_obj_del(kb->parent);
				return;
			}
			
		}
		else if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_ENTER_PNG)/* LV_SYMBOL_NEW_LINE*/
			{}
		
		else if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_LEFT_PNG)/*LV_SYMBOL_LEFT*/
			{}
		
		else if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_RIGHT_PNG)/*LV_SYMBOL_RIGHT*/
			{}
		
		else if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_DELETL_PNG || ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_DELETS_PNG)/*LV_SYMBOL_BACKSPACE*/
		    lv_textarea_del_char(ext->ta);
		
		else if(ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_SPACE_PNG)/*LV_SYMBOL_SPACE*/
			 lv_textarea_add_char(ext->ta, ' ');
	 
	} else if(strcmp(txt, "+/-") == 0) {
        uint16_t cur        = lv_textarea_get_cursor_pos(ext->ta);
        const char * ta_txt = lv_textarea_get_text(ext->ta);
        if(ta_txt[0] == '-') {
            lv_textarea_set_cursor_pos(ext->ta, 1);
            lv_textarea_del_char(ext->ta);
            lv_textarea_add_char(ext->ta, '+');
            lv_textarea_set_cursor_pos(ext->ta, cur);
        }
        else if(ta_txt[0] == '+') {
            lv_textarea_set_cursor_pos(ext->ta, 1);
            lv_textarea_del_char(ext->ta);
            lv_textarea_add_char(ext->ta, '-');
            lv_textarea_set_cursor_pos(ext->ta, cur);
        }
        else {
            lv_textarea_set_cursor_pos(ext->ta, 0);
            lv_textarea_add_char(ext->ta, '-');
            lv_textarea_set_cursor_pos(ext->ta, cur + 1);
        }
    }
    else {
        lv_textarea_add_text(ext->ta, txt);
    }
}


//创建键盘
static lv_obj_t *keybord_create(lv_obj_t* parent, lv_obj_t *ta){
	lv_obj_t *kb = lv_keyboard_create(parent, NULL);
	
	set_location(kb, 0, 200, 1024, 400);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_keyboard_set_textarea(kb, ta);
	lv_keyboard_set_cursor_manage(kb,false);

	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,  lv_color_hex(0x191919));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,LV_OPA_COVER);

	lv_obj_set_style_local_pad_top(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,7);
	lv_obj_set_style_local_pad_bottom(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,5);
	lv_obj_set_style_local_pad_left(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,7);
	lv_obj_set_style_local_pad_right(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,7);
	lv_obj_set_style_local_pad_inner(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,7);

	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT,  lv_color_hex(0x242424));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED,  lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_text_font(kb,LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE(31));


	keyboard_layout(kb);
	
	
	static btn_data kb_data = btn_data_up_create(lv_keyboard_event1_cb);
	
	kb->user_data = &kb_data;
	btn_touch_event_listen(kb);


	// if (msg_ui_ptask != NULL)
	// 	lv_task_del(msg_ui_ptask);
	
	// msg_ui_ptask = lv_task_create(msg_task, 300, LV_TASK_PRIO_HIGH, kb);
	// lv_task_ready(msg_ui_ptask);
	// msg_task(msg_ui_ptask);

	return kb;
}





//页面标题创建
static void addcctv_head_label_create(void){
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	

	// char *src[language_total] = {"Add network", "添加网络", "Добавить сеть", "Ajouter un réseau", "Añadir red", "Netzwerk hinzufügen", "اضافة شبكة ", "Přidat síťové připojení", "הוספת רשת","Adicionar Rede Wi-Fi","Aggiungi rete"};
	lv_label_set_text(obj, ip_str[connecting_cctv_ipaddr]);
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	
}

void addcctv_back_btn_down(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

void addcctv_back_btn_up(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(set_cctv));
	
}

//多选框的回调
static void reset_btnm_up(lv_obj_t* obj,lv_event_t event){
	// printf("\n00000%d0000000000000\n",event);
	if(event == LV_EVENT_VALUE_CHANGED){
		uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
		stream_flag = ++btn_id;
		user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].stream = stream_flag -1;
		printf("\n00000stream_flag=%d0000000000000\n",stream_flag);
	}
}

//子码流分辨率选择框的回调
static void substream_reset_btnm_up(lv_obj_t* obj,lv_event_t event){
	if(event == LV_EVENT_VALUE_CHANGED){
		uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
		sub_stream_id = btn_id;
		printf("\n00000stream_flag=>%d=========\n",btn_id);
	}
}

//确定键回调
static void stream_confirm_btn_up(lv_obj_t* obj){
		printf("\n============branf=》%d==========\n",user_data_get()->cctv_brand_Index);
		printf("\n============dev=》%d==========\n",user_data_get()->onvif_dev_count);

	if(strlen(addcctv_name) < 1 || strlen(addcctv_pwd) < 2)
		return ;

	if(user_data_get()->onvif_dev_count < 8){

		strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].ip,ip_str[connecting_cctv_ipaddr]);
		strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].user,addcctv_name);
		strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].pswd,addcctv_pwd);
		user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].cannel = 0;

		//判断如果选择的是其他品牌，那采用onvif库自动搜索url
		user_data_get()->cctv_brand_Index = brand_other;
		if(user_data_get()->cctv_brand_Index == brand_other){
			if((sat_ipcamera_user_password_set(connecting_cctv_ipaddr, addcctv_name,addcctv_pwd) == true)){
				printf("\n=============“注册索引成功”===========\n");
				usleep(1000*5000);
				if((sat_ipcamera_rtsp_url_get(connecting_cctv_ipaddr) == true)){
					printf("\n=============“获取url成功”===========\n");
					int num = 0;
					num = sat_ipcamera_profile_token_num_get (connecting_cctv_ipaddr);
					printf("\n============流=》%d==========\n",num);
					while(sat_ipcamera_status_get() == true){
						printf("\n======c=======“ipc正在工作”===========\n");
						usleep(1000*500);
					}
						printf("\n======c======*“ipc空闲”===========\n");
						num = sat_ipcamera_profile_token_num_get (connecting_cctv_ipaddr);
						printf("\n============流=》%d==========\n",num);
						int get_Url_num = 0;
						char *RTSPurl = NULL;
						RTSPurl = (char *)malloc(128);
						memset(RTSPurl,0,128);
						char *tmp_url = sat_ipcamera_rtsp_addr_get (connecting_cctv_ipaddr, stream_flag-1);
						if(tmp_url){
							strcpy(RTSPurl,tmp_url);
						}
						// printf("\n=============“RTSPurl=>%s”===========\n",RTSPurl);

						while(strlen(RTSPurl) == 0 && get_Url_num<5){
								// printf("\n=========1====“2222222222222”===========\n");
								num = sat_ipcamera_profile_token_num_get (connecting_cctv_ipaddr);
								// printf("\n=========2====“2222222222222”===========\n");
								char *tmp_url = sat_ipcamera_rtsp_addr_get (connecting_cctv_ipaddr, stream_flag-1);
								if(tmp_url != NULL){
								// printf("\n=========3====“2222222222222”===========\n");

									strcpy(RTSPurl,tmp_url);
								}
								// printf("\n=========4====“2222222222222”===========\n");
								usleep(1000*100);
								get_Url_num++;
								// printf("====get_Url_num=>%d======流=>%s============",get_Url_num,num);
								// printf("======get_Url_num=>%d====url=>%s============",get_Url_num,RTSPurl);

						}
						if(strlen(RTSPurl)){
						printf("\n=============“3333333333333”===========\n");
							// printf("\n=============“RTSPurl----->%s”===========\n",RTSPurl);
							char *pstr = strstr(RTSPurl, "//");
								pstr += 2;
								sprintf(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url, "rtsp://%s:%s@%s", addcctv_name, addcctv_pwd, pstr);  
								// printf("\n=============“RTSPurl=>%s”===========\n",RTSPurl);
								printf("\n======获取url成功！！！=======“RTSPurl=>%s”===========\n",user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url);
								user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].cannel = 1;
						}else{
							sprintf(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url, " "); 
						}
						free(RTSPurl);
						RTSPurl = NULL;
						// printf("\n=============num=>%d”===========\n",num);
				}else{
					printf("\n=============“获取url失败”===========\n");
				}
			}else{
				printf("\n=============“注册失败”===========\n");
			}
		}
		//如果手动选择品牌则采用字符串接成url
		else{
     	  	 char *write_url[64];
			switch (user_data_get()->cctv_brand_Index)
			{
			case brand_MAZi: //MAZi,实际上还是链接着海康
				sprintf(write_url,"rtsp://%s:%s@%s:554/Streaming/Channels/10%d",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr],stream_flag);
				break;
			case brand_Hikivision: //海康威视
				sprintf(write_url,"rtsp://%s:%s@%s:554/Streaming/Channels/10%d",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr],stream_flag);
				break;
			case brand_Dahua: //大华
				sprintf(write_url,"rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=%d",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr],stream_flag);
				break;
			case brand_XM: //雄迈
				sprintf(write_url,"rtsp://%s:554/user=%s_password=%s_channel=1_stream=%d&amp;amp;onvif=0.sdp?real_st",ip_str[connecting_cctv_ipaddr],addcctv_name,addcctv_pwd,stream_flag-1);
				break;
			case brand_urmet: //欧盟特
				sprintf(write_url,"rtsp://%s/live/%d/MAIN",ip_str[connecting_cctv_ipaddr],stream_flag-1);
				break;
			case brand_Onvif: //onvif
				sprintf(write_url,"rtsp://%s:%s@%s:554/Streaming/Channels/10%d",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr],stream_flag);
				break;
			default:
				break;
			}
			strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url,write_url);
			if(net_work_ping(ip_str[connecting_cctv_ipaddr]) == true){
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].cannel = 1;
			}
		}
		//添加一个已注册的cctv的数目，并保存到本地
		
		user_data_get()->onvif_dev_count++;
		user_data_save();
		printf("\n=============“user_data_get()->onvif_dev_count=%d”===========\n",user_data_get()->onvif_dev_count);

	}
		goto_layout(pLAYOUT(set_cctv));
}


//确定按钮创建
static void stream_confirm_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_pos(obj, 45, 275);
	lv_obj_set_size(obj, 228, 72);
	lv_obj_set_id(obj,0);
	
	// char *src[language_total] = {"Confirm", "确认", "Подтверждать", "Confirmer", "Confirmar", "Bestätigen Sie", "تاكيد", "Potvrdit", "אישור","Confirmar","Conferma"};
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CONFIRM_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(stream_confirm_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}

//
static void substream_num_choice_btn_up(lv_obj_t* obj){
		substream_change_cont_create();
}


//子码流设置分辨率按钮创建
static void substream_btn_create(lv_obj_t* parent)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_pos(btn, 440, 150);
	lv_obj_set_size(btn, 60, 60);
	
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_SET_PNG);
	lv_obj_t *img = lv_img_create(btn, NULL);
	lv_img_set_src(img, &info);

	setting_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);
	

	static btn_data btn_data = btn_data_up_create(substream_num_choice_btn_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
}



//取消键回调
static void stream_cancel_btn_up(lv_obj_t* obj){
		lv_obj_del(stream_cont->parent);
}


//取消按钮创建
static void stream_cancel_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_pos(obj, 311, 275);
	lv_obj_set_size(obj, 228, 72);
	lv_obj_set_id(obj,1);
	// char *src[language_total] = {"Cancel", "取消", "Отмена", "Annuler", "Cancelar", "Absagen", "الغاء", "Zrušit", "ביטול","Cancelar","Annulla"};
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CANCEL_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(stream_cancel_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}

//子码流分辨率确定键回调
static void substream_confirm_btn_up(lv_obj_t* obj){
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(),361);
	lv_obj_t *my_stream_cont = lv_obj_get_child_form_id(cont,362);
	lv_obj_t *sub_num_label = lv_obj_get_child_form_id(my_stream_cont,363);
	switch (sub_stream_id)
		{
		case 0:
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight = 1920;
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low = 1080;
			lv_label_set_text_fmt(sub_num_label,"1920,1080");
			break;

		case 1:
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight = 1280;
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low = 760;
			lv_label_set_text_fmt(sub_num_label,"1280,760");

		break;
		case 2:
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight = 704;
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low = 576;
			lv_label_set_text_fmt(sub_num_label,"704,576");

		break;
			case 3:
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight = 640;
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low = 480;
			lv_label_set_text_fmt(sub_num_label,"640,480");

		break;
			case 4:
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight = 640;
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low = 360;
			lv_label_set_text_fmt(sub_num_label,"640,360");
		break;
		
		default:
			break;
		}
	// printf("\n====sub_stream_id=>%d==\n",sub_stream_id);
	// printf("\n====substream_hight=>%d==\n",user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight);
	// printf("\n====substream_low=>%d==\n",user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low);

		lv_obj_del(lv_obj_get_parent(lv_obj_get_parent(obj)));

}

//子码流分辨率确定按钮创建
static void substream_confirm_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_pos(obj, 45, 365);
	lv_obj_set_size(obj, 228, 62);
	lv_obj_set_id(obj,0);
	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CONFIRM_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(substream_confirm_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}



//子码流分辨率取消键回调
static void substream_cancel_btn_up(lv_obj_t* obj){
	lv_obj_del(lv_obj_get_parent(lv_obj_get_parent(obj)));

}


//子码流分辨率取消按钮创建
static void substream_cancel_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_pos(obj, 311, 365);
	lv_obj_set_size(obj, 228, 62);
	lv_obj_set_id(obj,1);
	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CANCEL_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(substream_cancel_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}

//子码流分辨率显示
static void substream_num_create(lv_obj_t* parent){
	char substream_num[32];
	user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight = 1280;
	user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low = 720;
	lv_obj_t* substream_num_label = lv_label_create(parent,NULL);
	lv_obj_set_pos(substream_num_label, 335, 195);
	lv_obj_set_size(substream_num_label, 30, 30);
	lv_obj_set_id(substream_num_label,363);	
	lv_obj_set_style_local_text_font(substream_num_label,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_FONT_SIZE_20);
	lv_obj_set_style_local_text_color(substream_num_label,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,lv_color_hex(0x3a3a3a));
	sprintf(substream_num,"%d,%d",user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_hight,user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].substream_low);
	lv_label_set_text(substream_num_label, substream_num);
	user_data_save();
}





//子码流分辨率选择框创建
static void substream_change_cont_create(){
	//背景的阴影
		lv_obj_t *cont_bg_substream = lv_cont_create(lv_scr_act(), NULL);
		lv_obj_set_size(cont_bg_substream,1024,600);
		lv_obj_set_pos(cont_bg_substream,0,0);
		lv_obj_set_style_local_bg_color(cont_bg_substream, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(cont_bg_substream, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
		
		//按钮的容器
		lv_obj_t *substream_cont = lv_cont_create(cont_bg_substream, NULL);
		lv_obj_set_size(substream_cont,584,450);
		lv_obj_set_pos(substream_cont,220,54);
		lv_obj_set_style_local_bg_color(substream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x212121));
		lv_obj_set_style_local_bg_opa(substream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
		lv_obj_set_style_local_border_color(substream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(substream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
		lv_obj_set_style_local_border_color(substream_cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(substream_cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
		lv_obj_set_style_local_border_color(substream_cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(substream_cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);


		lv_obj_t *obj = lv_btnmatrix_create(substream_cont, NULL);
		lv_obj_set_id(obj,202);
		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));

		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);

		static rom_bin_info info_off1 = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off1);
		

		static rom_bin_info info_on1 = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on1);
		static const char *resolution_arr[] = {"1080p","\n","720p","\n","576p","\n","480p","\n","360p","\n",""};
		lv_btnmatrix_set_map(obj, resolution_arr);
		lv_btnmatrix_set_align(obj, LV_LABEL_ALIGN_LEFT, 130, 0);
		lv_obj_set_x(obj, 50);
		lv_obj_set_y(obj, 20);
		lv_obj_set_size(obj, 384,410);
		

		static btn_data btn_data = btn_data_anything_create(substream_reset_btnm_up);
		obj->user_data = &btn_data;
		btn_touch_event_listen(obj);
		

		lv_btnmatrix_set_one_check(obj, true);
		lv_btnmatrix_set_btn_ctrl_all( obj, LV_BTNMATRIX_CTRL_CHECKABLE);

		lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
		lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_BORDER_SIDE_BOTTOM);
		
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_hex(0x000000));
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x000000));
		
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 1);
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED,1);

	
		lv_obj_set_style_local_text_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x666666));



		substream_confirm_btn_create(substream_cont);
		substream_cancel_btn_create(substream_cont);
}



void substream_btn_down(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

void substream_btn_up(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(set_cctv));
	
}

static void confirm_cctv_btn_down(lv_obj_t *obj){
	//  lv_obj_t* msg = connect_wifi_cb();
	// lv_msgbox_set_text(msg,str_get(LAYOUT_ADDWIFI_LANG_CONNECTING_ID));
	lv_obj_set_state(obj, LV_STATE_PRESSED);
}
static void confirm_cctv_btn_up(lv_obj_t *O_obj){
	lv_obj_set_state(O_obj, LV_STATE_DEFAULT);

		//背景的阴影
		lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
		lv_obj_set_size(cont_bg,1024,600);
		lv_obj_set_pos(cont_bg,0,0);
		lv_obj_set_style_local_bg_color(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
		lv_obj_set_id(cont_bg,361);
		
		//按钮的容器
		stream_cont = lv_cont_create(cont_bg, NULL);
		lv_obj_set_id(stream_cont,362);
		lv_obj_set_size(stream_cont,584,362);
		lv_obj_set_pos(stream_cont,220,120);
		lv_obj_set_style_local_bg_color(stream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x212121));
		lv_obj_set_style_local_bg_opa(stream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
		lv_obj_set_style_local_border_color(stream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(stream_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
		lv_obj_set_style_local_border_color(stream_cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(stream_cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, 3);
		lv_obj_set_style_local_border_color(stream_cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0xEFCC8C));
		lv_obj_set_style_local_border_width(stream_cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, 3);


		lv_obj_t *obj = lv_btnmatrix_create(stream_cont, NULL);
		lv_obj_set_id(obj,66);
		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_make(0x30, 0x30, 0x30));

		lv_obj_set_style_local_bg_opa(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_TRANSP);

		static rom_bin_info info_off1 = rom_bin_info_get(ROM_RES_SETTING_CHECKBOX_OFF_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &info_off1);
		

		static rom_bin_info info_on1 = rom_bin_info_get(ROM_RES_SETTING_CHECBOX_ON_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &info_on1);

		lv_btnmatrix_set_map(obj, stream_str_get());
		lv_btnmatrix_set_align(obj, LV_LABEL_ALIGN_LEFT, 130, 0);
		lv_obj_set_x(obj, 50);
		lv_obj_set_y(obj, 20);
		lv_obj_set_size(obj, 384,220);
		

		static btn_data btn_data = btn_data_anything_create(reset_btnm_up);
		obj->user_data = &btn_data;
		btn_touch_event_listen(obj);
		

		lv_btnmatrix_set_one_check(obj, true);
		lv_btnmatrix_set_btn_ctrl(obj, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
		lv_btnmatrix_set_btn_ctrl(obj, 1, LV_BTNMATRIX_CTRL_CHECKABLE);

		lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
		lv_obj_set_style_local_border_side(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_BORDER_SIDE_BOTTOM);
		
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, lv_color_hex(0x000000));
		lv_obj_set_style_local_border_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x000000));
		
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 1);
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 1);
		lv_obj_set_style_local_border_width(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED,1);

	
		lv_obj_set_style_local_text_color(obj, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x666666));
		
		stream_confirm_btn_create(stream_cont);
		stream_cancel_btn_create(stream_cont);
		substream_num_create(stream_cont);
		substream_btn_create(stream_cont);
		







	#if 0
	if(strlen(addcctv_name) < 1 || strlen(addcctv_pwd) < 2)
		return ;

	if(user_data_get()->onvif_dev_count < 8){

		strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].ip,ip_str[connecting_cctv_ipaddr]);
		strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].user,addcctv_name);
		strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].pswd,addcctv_pwd);
		user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].cannel = 0;
		//判断如果选择的是其他品牌，那采用onvif库自动搜索url
		if(user_data_get()->cctv_brand_Index == brand_other){
			if((sat_ipcamera_user_password_set(connecting_cctv_ipaddr, addcctv_name,addcctv_pwd) == true)){
				printf("\n=============“注册索引成功”===========\n");
				usleep(1000*5000);
				if((sat_ipcamera_rtsp_url_get(connecting_cctv_ipaddr) == true)){
					printf("\n=============“获取url成功”===========\n");
					int num = 0;
					num = sat_ipcamera_profile_token_num_get (connecting_cctv_ipaddr);
					printf("\n============流=》%d==========\n",num);
					while(sat_ipcamera_status_get() == true){
						printf("\n======c=======“ipc正在工作”===========\n");
						usleep(1000*500);
					}
						printf("\n======c======*“ipc空闲”===========\n");
						num = sat_ipcamera_profile_token_num_get (connecting_cctv_ipaddr);
						printf("\n============流=》%d==========\n",num);
						int get_Url_num = 0;
						char *RTSPurl = NULL;
						RTSPurl = (char *)malloc(128);
						memset(RTSPurl,0,128);
						char *tmp_url = sat_ipcamera_rtsp_addr_get (connecting_cctv_ipaddr, 0);
						if(tmp_url){
							strcpy(RTSPurl,tmp_url);
						}
						// printf("\n=============“RTSPurl=>%s”===========\n",RTSPurl);

						while(strlen(RTSPurl) == 0 && get_Url_num<5){
								// printf("\n=========1====“2222222222222”===========\n");
								num = sat_ipcamera_profile_token_num_get (connecting_cctv_ipaddr);
								// printf("\n=========2====“2222222222222”===========\n");
								char *tmp_url = sat_ipcamera_rtsp_addr_get (connecting_cctv_ipaddr, 0);
								if(tmp_url != NULL){
								// printf("\n=========3====“2222222222222”===========\n");

									strcpy(RTSPurl,tmp_url);
								}
								// printf("\n=========4====“2222222222222”===========\n");
								usleep(1000*100);
								get_Url_num++;
								// printf("====get_Url_num=>%d======流=>%s============",get_Url_num,num);
								// printf("======get_Url_num=>%d====url=>%s============",get_Url_num,RTSPurl);

						}
						if(strlen(RTSPurl)){
						printf("\n=============“3333333333333”===========\n");
							// printf("\n=============“RTSPurl----->%s”===========\n",RTSPurl);
							char *pstr = strstr(RTSPurl, "//");
								pstr += 2;
								sprintf(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url, "rtsp://%s:%s@%s", addcctv_name, addcctv_pwd, pstr);  
								// printf("\n=============“RTSPurl=>%s”===========\n",RTSPurl);
								printf("\n======获取url成功！！！=======“RTSPurl=>%s”===========\n",user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url);
								user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].cannel = 1;
						}else{
							sprintf(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url, " "); 
						}
						free(RTSPurl);
						RTSPurl = NULL;
						// printf("\n=============num=>%d”===========\n",num);
				}else{
					printf("\n=============“获取url失败”===========\n");
				}
			}else{
				printf("\n=============“注册失败”===========\n");
			}
		}
		//如果手动选择品牌则采用字符串接成url
		else{
     	  	 char *write_url[64];
			switch (user_data_get()->cctv_brand_Index)
			{
			case brand_MAZi: //MAZi,实际上还是链接着海康
				sprintf(write_url,"rtsp://%s:%s@%s:554/Streaming/Channels/101",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr]);
				break;
			case brand_Hikivision: //海康威视
				sprintf(write_url,"rtsp://%s:%s@%s:554/Streaming/Channels/101",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr]);
				break;
			case brand_Dahua: //大华
				sprintf(write_url,"rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1",addcctv_name,addcctv_pwd,ip_str[connecting_cctv_ipaddr]);
				break;
			case brand_XM: //雄迈
				sprintf(write_url,"rtsp://%s:554/user=%s_password=%s_channel=1_stream=0&amp;amp;onvif=0.sdp?real_st",ip_str[connecting_cctv_ipaddr],addcctv_name,addcctv_pwd);
				break;
			default:
				break;
			}
			strcpy(user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].url,write_url);
			if(net_work_ping(ip_str[connecting_cctv_ipaddr]) == true){
			user_data_get()->onvif_dev[user_data_get()->onvif_dev_count].cannel = 1;
			}
		}
		//添加一个已注册的cctv的数目，并保存到本地
		user_data_get()->onvif_dev_count++;
		user_data_save();
	}
		goto_layout(pLAYOUT(set_cctv));
#endif
}
	




static void addcctv_confirm_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	// lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xCF, 0xA0, 0x75));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x26,0x26,0x26));
	set_location(obj, 0, 0, 1024, 72);
	lv_obj_align(obj,parent,LV_ALIGN_IN_BOTTOM_MID,0,0);
	// lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_MID);
	lv_obj_set_id(obj,0);
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	
	// char *src[language_total] = {"Confirm", "确认", "Подтверждать", "Confirmer", "Confirmar", "Bestätigen Sie", "تاكيد ", "Potvrdit", "אישור","Confirmar","Conferma"};
	lv_label_set_text(label, str_get(LAYOUT_CCTV_LANG_REGISTER_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_create(confirm_cctv_btn_down,confirm_cctv_btn_up,NULL);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}


//创建返回按钮 
lv_obj_t* addcctv_back_btn_create(void){
	static btn_data btn_data = btn_data_create(addcctv_back_btn_down, addcctv_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}



#if 1

//点击cctvname输入框 回调
static void input_cctv_name_up(struct _lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_PRESSED){
		extern void touch_sound_play(void);
		touch_sound_play();
	}

	if(event == LV_EVENT_CLICKED){
		/*创建新的容器 在容器上操作*/
		lv_obj_t *cctvname_cont = lv_cont_create(lv_scr_act(),NULL);
		cctvname_cont->user_data = obj->user_data;
		set_location(cctvname_cont, 0, 0, 1024,600);
		lv_cont_set_layout(cctvname_cont, LV_LAYOUT_OFF);
		lv_obj_set_style_local_bg_color(cctvname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(cctvname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

		back_btn_create();//创建放回按钮

		//创建一个输入框
		lv_obj_t* ta = input_textarea_create(cctvname_cont, 40, 87, 942, 77, false, str_get(LAYOUT_CCTV_LANG_INPUTCCTVNAME_ID));
		if(strlen(addcctv_name))
			lv_textarea_set_text(ta, addcctv_name);

		
		//创建一个键盘 并且绑定这个输入框
		lv_obj_t* kb = keybord_create(cctvname_cont, ta);
		lv_obj_set_id(kb, 1);

		//键盘的保存回调 保存wifi名称
	}
}



static lv_obj_t * pwd_ta = NULL;
static void change_pwd_mode(lv_obj_t * obj){
		static rom_bin_info info_pwd = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
		static rom_bin_info info_inv = rom_bin_info_get(ROM_RES_SETWIFI_VISIBLE_PNG);
		
		if(lv_textarea_get_pwd_mode(pwd_ta)){
			lv_img_set_src(obj, &info_inv);
			lv_textarea_set_pwd_mode(pwd_ta, false);
		}else{
			lv_img_set_src(obj, &info_pwd);
			lv_textarea_set_pwd_mode(pwd_ta, true);
		}
}


//点击密码输入框 回调
static void input_cctv_pwd_up(struct _lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_PRESSED){
		extern void touch_sound_play(void);
		touch_sound_play();
	}

	if(event == LV_EVENT_CLICKED){
		/*创建新的容器 在容器上操作*/
		lv_obj_t *cctvpwd_cont = lv_cont_create(lv_scr_act(),NULL);
		cctvpwd_cont->user_data = obj->user_data;
	
		set_location(cctvpwd_cont, 0, 0, 1024,600);
		lv_cont_set_layout(cctvpwd_cont, LV_LAYOUT_OFF);
		lv_obj_set_style_local_bg_color(cctvpwd_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(cctvpwd_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

		back_btn_create();//创建返回按钮

		//创建一个输入框
		pwd_ta = input_textarea_create(cctvpwd_cont, 40, 87, 942, 77, true, str_get(LAYOUT_SETPASSWORD_LANG_PLEASEINPUTPASSWORD_ID));
		if(strlen(addcctv_pwd))
			lv_textarea_set_text(pwd_ta, addcctv_pwd);

		//创建密码模式切换 密码模式进去一定是不可见的 可见只是当时可见 退出再进任然不可见
		
		lv_obj_t *pwd_mode = lv_img_create(cctvpwd_cont, NULL);
		
		set_location(pwd_mode, 932, 102, 48, 48);
		static rom_bin_info pwd_info = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
		lv_img_set_src(pwd_mode, &pwd_info);
		
		lv_obj_set_click(pwd_mode, true);
		lv_obj_set_ext_click_area(pwd_mode, 15, 15, 15, 15);
		
		static btn_data img_data = btn_data_up_create(change_pwd_mode);
		pwd_mode->user_data = &img_data;
		btn_touch_event_listen(pwd_mode);
		
		
		//创建一个键盘 并且绑定这个输入框
		lv_obj_t* kb = keybord_create(cctvpwd_cont, pwd_ta);
		lv_obj_set_id(kb, 0);

		//在键盘的确认按钮的回调上是保存要连接的wifi输入的密码
	}
}

#endif

//点击brand输入框 回调
static void input_cctv_brand_up(struct _lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_CLICKED){
		#if (Urmet == 0)
			set_layout_cctv_brand_ch(CCTV_BRAND_CH_ADD);
			goto_layout(pLAYOUT(cctv_brand));
		#endif
	}
}

static void addcctv_page_craete(lv_obj_t *parent){
	
	lv_obj_t* addcctv_cont = lv_cont_create(lv_scr_act(),NULL);//在当前活跃的屏幕上创建容器
	set_location(addcctv_cont, 0, 100, 1024, 500);
	lv_cont_set_layout(addcctv_cont, LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_color(addcctv_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(addcctv_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

	//不再创建输入框 直接改为容器
	lv_obj_t* cctvname_cont = lv_cont_create(addcctv_cont,NULL);
	lv_obj_set_style_local_bg_color(cctvname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(cctvname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  LV_OPA_COVER);
	set_location(cctvname_cont, 245,161,538,80);
	lv_cont_set_layout(cctvname_cont, LV_FIT_NONE);
	lv_obj_set_click(cctvname_cont, true);
	
	lv_obj_set_event_cb(cctvname_cont, input_cctv_name_up);

	
	lv_obj_t* cctvpwd_cont = lv_cont_create(addcctv_cont,cctvname_cont);
	set_location(cctvpwd_cont, 245,263,538,80);
	lv_obj_set_event_cb(cctvpwd_cont, input_cctv_pwd_up);

	lv_obj_t* cctvbrand_cont = lv_cont_create(addcctv_cont,cctvname_cont);
	set_location(cctvbrand_cont, 245,62,538,80);
	lv_obj_set_event_cb(cctvbrand_cont, input_cctv_brand_up);

	//创建提示符
	lv_obj_t *cctvname_label = lv_label_create(addcctv_cont, NULL);
	lv_obj_set_style_local_text_font(cctvname_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_text(cctvname_label, str_get(LAYOUT_CCTV_LANG_ACCOUNT_ID));
	lv_obj_align(cctvname_label, cctvname_cont, LV_ALIGN_IN_LEFT_MID, 10, 0);
	lv_obj_set_parent_event(cctvname_label, true);

	lv_obj_t *cctvpwd_label = lv_label_create(addcctv_cont, cctvname_label);
	lv_label_set_text(cctvpwd_label, str_get(LAYOUT_ADDWIFI_LANG_WIFIPASSWORD_ID));
	lv_obj_align(cctvpwd_label, cctvpwd_cont, LV_ALIGN_IN_LEFT_MID, 10, 0);
	lv_obj_set_parent_event(cctvpwd_label, true);

	lv_obj_t *cctvbrand_label = lv_label_create(addcctv_cont, cctvname_label);
	lv_label_set_text(cctvbrand_label, str_get(LAYOUT_CCTV_LANG_BRAND_ID));
	lv_obj_align(cctvbrand_label, cctvbrand_cont, LV_ALIGN_IN_LEFT_MID, 10, 0);
	lv_obj_set_parent_event(cctvbrand_label, true);

	//创建输入的内容
	name_label = lv_label_create(cctvname_cont, NULL);
	lv_obj_set_style_local_text_font(name_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_text(name_label, "");
	lv_obj_align(name_label, cctvname_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_set_parent_event(name_label, true);
	cctvname_cont->user_data = name_label;

	
	pwd_label = lv_label_create(cctvpwd_cont, name_label);
	lv_label_set_text(pwd_label, "");
	lv_obj_align(pwd_label, cctvpwd_label, LV_ALIGN_OUT_RIGHT_MID, 5, 7);
	lv_obj_set_parent_event(pwd_label, true);
	cctvpwd_cont->user_data = pwd_label;
	
	// char *brand_str[5]={"MAZi","Hikivision","Dahua","XM","other"};
	// brand_str[5] = str_get(LAYOUT_SETTING_LANG_OTHER_ID);
	// strcpy(brand_str[4],str_get(LAYOUT_SETTING_LANG_OTHER_ID));
	brand_label = lv_label_create(cctvbrand_cont,name_label);
	// lv_label_set_text(brand_label, brand_str[user_data_get()->cctv_brand_Index]);
	lv_label_set_text(brand_label, brand_str_value_get(user_data_get()->cctv_brand_Index));
	lv_obj_align(brand_label, cctvbrand_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_set_parent_event(brand_label, true);
	cctvbrand_cont->user_data = brand_label;

	addcctv_confirm_btn_create(addcctv_cont);
	// addcctv_cancel_btn_create(addcctv_cont);
	
}




extern void memory_print(void);

static void LAYOUT_ENETER_FUNC(add_cctv)
{
	#if Urmet
		user_data_get()->cctv_brand_Index = 4;
	#endif 
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置
	addcctv_head_label_create();//创建头
	addcctv_back_btn_create();//创建返回按钮
	addcctv_page_craete(lv_scr_act());


}

static void LAYOUT_QUIT_FUNC(add_cctv)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	memset(addcctv_name,0,strlen(addcctv_name));
	memset(addcctv_pwd,0,strlen(addcctv_pwd));
	// memset(addcctv_index,0,strlen(addcctv_index));
	lv_label_set_text(name_label,"");
	lv_label_set_text(pwd_label,"");

	
	if (msg_ui_ptask != NULL)//计时任务未退出
	{
		lv_task_del(msg_ui_ptask);//退出计时任务
		msg_ui_ptask = NULL;//置空指针
	}
	

}

CREATE_LAYOUT(add_cctv);

