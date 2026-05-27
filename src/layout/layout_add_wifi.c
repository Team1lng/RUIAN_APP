#include "layout_define.h"
#include "tuya_ipc_api.h"

extern void back_btn_create(void);
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern int wifi_connection_status_sucess(void);

extern lv_obj_t* input_textarea_create(lv_obj_t* parent, int x,int y, int w, int h, bool pwd_mode, const char * txt);
extern const char * get_addwifi_name(void);
extern const char * get_addwifi_pwd(void);
extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);
extern void keyboard_layout(lv_obj_t* obj);
extern bool wpa_cli_wlan_status(bool *continue_flag);

//

extern const char * * kb_map[];
extern const lv_btnmatrix_ctrl_t * kb_ctrl[];

static  char addwifi_name[30] = {0};
static  char addwifi_pwd[30] = {0};

#define CONNECTING 			0
#define CONNECT_SUCCESS 	1
#define CONNECT_FAIL 		2
#define WIFINAME_EMPTY		3
#define WIFIPWD_SHOT 		4
#define NOT_FIND 			5

static lv_obj_t *name_label = NULL;
static lv_obj_t *pwd_label = NULL;

static lv_task_t *msg_ui_ptask = NULL;
static bool connect_falge = false;

static int task_timer = 0;


lv_obj_t* connect_wifi_cb(void){
	lv_obj_t* msg = lv_msgbox_create(lv_scr_act(), NULL);

	lv_obj_set_pos(msg, 240,145);
	lv_obj_set_size(msg, 550,300);
	lv_msgbox_set_anim_time(msg, 0);
	
	lv_obj_set_style_local_bg_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x1A1A1A));
	lv_obj_set_style_local_bg_opa(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);
	
	lv_obj_set_style_local_border_width(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_text_font(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(28));	
	lv_obj_set_style_local_text_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xffffff));	
	return msg;
}


void set_msg_text(lv_obj_t *msg,int state){
	if(state == CONNECTING){
		// char *str[language_total] = {"Connecting! Please wait...",
		// 							 "连接中!请等待...",
		// 							 "Подключение! Пожалуйста ждите...",
		// 							 "De liaison! S'il vous plaît, attendez...",
		// 							 "Conectando! Espere por favor...",
		// 							 "Verbinden! Warten Sie mal...",
		// 							 "جاري التوصيل يرجى الانتظار...",
		// 							 "Spojuji! Prosím čekéjte...",
		// 							 "מתחבר נא להמתין",
		// 							 "A Conectar! Por Favor, Aguarde",
		// 							 "In connessione, attendere!"};
		// lv_msgbox_set_text(msg, "111111");
		lv_msgbox_set_text(msg, str_get(LAYOUT_ADDWIFI_LANG_CONNECTING_ID));
		//lv_msgbox_start_auto_close(msg,1000);
	}else if(state == CONNECT_SUCCESS){
		
		// char *str[language_total] = {"Connection successful!",
		// 							 "连接成功!",
		// 							 "Подключение успешно!",
		// 							 "Connexion réussie!",
		// 							 "Conexión exitosa!",
		// 							 "Verbindung erfolgreich!",
		// 							 "!تم التوصيل بنجاح",
		// 							 "Spojeno úspěšně!",
		// 							 "חיבור בוצע בהצלחה",
		// 							 "Conecção Bem Sucedida",
		// 							 "Connessione OK"};
		lv_msgbox_set_text(msg, str_get(LAYOUT_ADDWIFI_LANG_CONNECTSUCCESSFUL_ID));
		lv_msgbox_start_auto_close(msg,1000);
	}else if(state == CONNECT_FAIL){
		// char *str[language_total] = {"Connection fail!",
		// 							 "连接失败!",
		// 							 "Ошибка подключения!",
		// 							 "Échec de la connexion!",
		// 							 "Conexión fallida!",
		// 							 "Verbindung fehlgeschlagen!",
		// 							 "!فشل الاتصال",
		// 							 "Chyba spojení!",
		// 							 "חיבור נכשל",
		// 							 "Conecção Falhada",
		// 							 "Connessione fallita"};
		lv_msgbox_set_text(msg, str_get(LAYOUT_ADDWIFI_LANG_CONNECTFAIL_ID));
		lv_msgbox_start_auto_close(msg,1000);
	}else if(state == WIFINAME_EMPTY){;
		// char *str[language_total] = {"Wifi name is empty!",
		// 							 "Wifi名为空!",
		// 							 "Имя Wi-Fi пусто!",
		// 							 "Le nom du Wi-Fi est vide!",
		// 							 "El nombre de wifi está vacío!",
		// 							 "WLAN-Name ist leer!",
		// 							 "!فشل الاتصال",
		// 							 "Název Wifi je prázdný!",
		// 							 "שם רשת WIFI ריק",
		// 							 "O Nome da Rede Wi-Fi Está Vazio",
		// 							 "Nome Wifi e vuoto"};
		lv_msgbox_set_text(msg, str_get(LAYOUT_ADDWIFI_LANG_WIFINAMEEMPTY_ID));
		lv_msgbox_start_auto_close(msg,1000);
	}else if(state == WIFIPWD_SHOT){
		// char *str[language_total] = {"Wifi password is too short!",
		// 							 "Wifi密码太短!",
		// 							 "Пароль Wi-Fi слишком расстрелян!",
		// 							 "Le mot de passe Wifi est trop tiré!",
		// 							 "La contraseña de wifi está demasiado disparada!",
		// 							 "WLAN-Passwort ist zu kurz!",
		// 							 "!كلمة السر قصيرة جدا",
		// 							 "Heslo Wifi je příliš krátké!",
		// 							 "סיסמת WIFI קצרה מידי",
		// 							 "A Senha da Rede Wi-Fi é Muito Curta",
		// 							 "La password Wi-Fi è breve"};
		lv_msgbox_set_text(msg, str_get(LAYOUT_ADDWIFI_LANG_WIFIPASSWORDSHORT_ID));
		lv_msgbox_start_auto_close(msg,1000);
	}else if(state == NOT_FIND){
		// char *str[language_total] = {"Not find!",
		// 							 "未找到!",
		// 							 "Не найти!",
		// 							 "Pas trouvé",
		// 							 "No encontrar!",
		// 							 "Nicht finden!",
		// 							 "!غير موجود",
		// 							 "Nenalezeno!",
		// 							 "לא נמצאה רשת",
		// 							 "Nenhuma Rede foi Encontrada",
		// 							 "Non trovato"};
		lv_msgbox_set_text(msg, str_get(LAYOUT_ADDWIFI_LANG_WIFIPASSWORDSHORT_ID));
		lv_msgbox_start_auto_close(msg,1000);
	}
}


static void refresh_wifi_diaplay_task(lv_task_t * task)
{
	goto_layout(pLAYOUT(set_wifi));
	lv_obj_t* msg1 = connect_wifi_cb();
	set_msg_text(msg1, CONNECT_FAIL);
	lv_task_del(task);
}



static void msg_task(struct _lv_task_t *task_t){
	if(connect_falge){
		if(task_timer <= 12){
			task_timer++;
			int connect_ret = wifi_connection_status_sucess();
			if(connect_ret == 1 && (task_timer > 6)){//连接成功
				task_timer = 0;
				system("rm -rf /etc/config/wpa_supplicant.conf &");
				system("cp -rf /tmp/wpa_supplicant.conf /etc/config/ &");
				system("sync");
				user_data_get()->wifi.wifi_connect_flag = true;
				user_data_save();
				goto_layout(pLAYOUT(set_wifi));
				lv_obj_t* msg1 = connect_wifi_cb();
				set_msg_text(msg1, CONNECT_SUCCESS);
				connect_falge = false;
				tuya_ipc_reconnect_wifi();
			}
		}else{//连接失败
			connect_falge = false;
			task_timer = 0;
   			system("killall wpa_supplicant &");
    		system("killall udhcpc &");
			system("rm -rf /tmp/wpa_supplicant.conf &");
			
			system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B");
			system("udhcpc -i wlan0 -n 4 -R");

			lv_task_create(refresh_wifi_diaplay_task, 2000, LV_TASK_PRIO_HIGH, NULL);
		}
		
	}else
		return ;
}



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

				memset(addwifi_pwd,0,strlen(addwifi_pwd));
				sprintf(addwifi_pwd,"%s",lv_textarea_get_text(ext->ta));
			
				if(strlen(addwifi_pwd) < 8){
					
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg,WIFIPWD_SHOT);
				
					memset(addwifi_pwd,0,strlen(addwifi_pwd));
					lv_textarea_set_text(ext->ta,"");
					lv_label_set_text((lv_obj_t *)kb->parent->user_data,"");
					return ;
				}
				
				char a[31] = "*";
				char *b = "*";
				for(int i = strlen(addwifi_pwd); i>1; i--){
					strncat(a,b,1);
				}
				lv_label_set_text((lv_obj_t *)kb->parent->user_data,a);
				lv_obj_del(lv_obj_get_child(lv_scr_act(),NULL));
				lv_obj_del(kb->parent);
				return;
			}
			
			child = lv_obj_get_child_form_id (kb->parent ,1);
			if (child == kb) {
					
				memset(addwifi_name,0,strlen(addwifi_name));
				sprintf(addwifi_name,"%s",lv_textarea_get_text(ext->ta));
			
				if(strlen(addwifi_name) < 1){
					memset(addwifi_name,0,strlen(addwifi_name));
					lv_label_set_text((lv_obj_t *)kb->parent->user_data,addwifi_name);
				
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg,WIFINAME_EMPTY);
					return ;
				}

				lv_label_set_text((lv_obj_t *)kb->parent->user_data,addwifi_name);

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

	connect_falge = false;

	keyboard_layout(kb);
	
	
	static btn_data kb_data = btn_data_up_create(lv_keyboard_event1_cb);
	
	kb->user_data = &kb_data;
	btn_touch_event_listen(kb);


	if (msg_ui_ptask != NULL)
		lv_task_del(msg_ui_ptask);
	
	msg_ui_ptask = lv_task_create(msg_task, 300, LV_TASK_PRIO_HIGH, kb);
	lv_task_ready(msg_ui_ptask);
	msg_task(msg_ui_ptask);

	return kb;
}





//页面标题创建
static void addwifi_head_label_create(void){
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
	


	// char *src[language_total] = {"Add network", "添加网络", "Добавить сеть", "Ajouter un réseau", "Añadir red", "Netzwerk hinzufügen", "اضافة شبكة ", "Přidat síťové připojení", "הוספת רשת","Adicionar Rede Wi-Fi","Aggiungi rete"};
	lv_label_set_text(obj, str_get(LAYOUT_SETNETWORK_LANG_ADDNETWORK_ID));
	lv_label_set_long_mode(obj,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(obj,1024,60);
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(obj,LV_LAYOUT_CENTER);
	
}

void addwifi_back_btn_down(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

void addwifi_back_btn_up(lv_obj_t *obj){
	setting_btn_state_set(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(set_wifi));
	
}




static void cancel_wifi_btn_up(lv_obj_t *obj){
	lv_obj_set_state(obj, LV_STATE_DEFAULT);

	goto_layout(pLAYOUT(set_wifi));	
}



static void confirm_wifi_btn_up(lv_obj_t *obj){
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));

	if(strlen(addwifi_name) < 1 || strlen(addwifi_pwd) < 8)
		return ;
	
	connect_falge = true;
	wpa_cli_connect_new_wifi(addwifi_name, addwifi_pwd);

	lv_obj_t* obj1 = connect_wifi_cb();
	set_msg_text(obj1,CONNECTING);	 
}
	




//确认连接wifi
static void addwifi_confirm_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	set_location(obj, 245, 343, 220, 72);
	lv_obj_set_id(obj,0);
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	
	// char *src[language_total] = {"Confirm", "确认", "Подтверждать", "Confirmer", "Confirmar", "Bestätigen Sie", "تاكيد ", "Potvrdit", "אישור","Confirmar","Conferma"};
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CONFIRM_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(confirm_wifi_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}

static void addwifi_cancel_btn_create(lv_obj_t* parent)
{
	lv_obj_t* obj = lv_btn_create(parent,NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x26,0x26,0x26));
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xCF, 0xA0, 0x75));
	set_location(obj, 563, 343, 220, 72);
	lv_obj_set_id(obj,1);
	
	lv_obj_t* label = lv_label_create(parent,NULL);
	// char *src[language_total] = {"Cancel", "取消", "Отмена", "Annuler", "Cancelar", "Absagen", "الغاء", "Zrušit", "ביטול","Cancelar","Annulla"};
	lv_label_set_text(label, str_get(LAYOUT_SETTING_LANG_CANCEL_ID));
	lv_obj_align(label, obj, LV_ALIGN_CENTER, 0,0);


	static btn_data btn_data = btn_data_up_create(cancel_wifi_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}



//创建返回按钮 返回wifi页面
lv_obj_t* addwifi_back_btn_create(void){
	static btn_data btn_data = btn_data_create(addwifi_back_btn_down, addwifi_back_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* btn = setting_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
	return btn;
}



#if 1

//点击wifiname输入框 回调
static void input_wifi_name_up(struct _lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_PRESSED){
		extern void touch_sound_play(void);
		touch_sound_play();
	}

	if(event == LV_EVENT_CLICKED){
		/*创建新的容器 在容器上操作*/
		lv_obj_t *wifiname_cont = lv_cont_create(lv_scr_act(),NULL);
		wifiname_cont->user_data = obj->user_data;
		set_location(wifiname_cont, 0, 0, 1024,600);
		lv_cont_set_layout(wifiname_cont, LV_LAYOUT_OFF);
		lv_obj_set_style_local_bg_color(wifiname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(wifiname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

		back_btn_create();//创建放回按钮

		//创建一个输入框
		// char *src[language_total] = {"Please input wifi name", "请输入wifi名称", "Пожалуйста, введите имя Wi-Fi",
		// 							 "Veuillez saisir le nom du Wi-Fi", "Por favor ingrese el nombre wifi", "Bitte geben Sie den WLAN-Namen ein",
		// 							 "الرجاء ادخال اسم واي فاي", "Zadejte název wifi", "נא הכנס שם רשת WIFI","Introduza Nome da Rede Wi-Fi","Inserire il nome Wi-Fi"};
		lv_obj_t* ta = input_textarea_create(wifiname_cont, 40, 87, 942, 77, false, str_get(LAYOUT_ADDWIFI_LANG_INPUTWIFINAME_ID));
		if(strlen(addwifi_name))
			lv_textarea_set_text(ta, addwifi_name);

		
		//创建一个键盘 并且绑定这个输入框
		lv_obj_t* kb = keybord_create(wifiname_cont, ta);
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



static void input_wifi_pwd_up(struct _lv_obj_t * obj, lv_event_t event){
	if(event == LV_EVENT_PRESSED){
		extern void touch_sound_play(void);
		touch_sound_play();
	}

	if(event == LV_EVENT_CLICKED){
		
		/*创建新的容器 在容器上操作*/
		lv_obj_t *wifipwd_cont = lv_cont_create(lv_scr_act(),NULL);
		wifipwd_cont->user_data = obj->user_data;
	
		set_location(wifipwd_cont, 0, 0, 1024,600);
		lv_cont_set_layout(wifipwd_cont, LV_LAYOUT_OFF);
		lv_obj_set_style_local_bg_color(wifipwd_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
		lv_obj_set_style_local_bg_opa(wifipwd_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

		back_btn_create();//创建返回按钮

		//创建一个输入框
		// char *src[language_total] = {"Please input wifi password", "请输入wifi密码", "Пожалуйста, введите пароль Wi-Fi",
		// 							 "Veuillez saisir le mot de passe Wi-Fi", "Por favor ingrese la contraseña wifi", "Bitte geben Sie das WLAN-Passwort ein",
		// 							  "يرجى ادخال كلمة مرور واي فاي", "Zadejte heslo pro wifi", "נא הכנס סיסמת WIFI","Introduza Senha Wi-Fi Por Favor","Inserire la password Wi-Fi"};
		pwd_ta = input_textarea_create(wifipwd_cont, 40, 87, 942, 77, true, str_get(LAYOUT_ADDWIFI_LANG_INPUTWIFIPASSWORD_ID));
		if(strlen(addwifi_pwd))
			lv_textarea_set_text(pwd_ta, addwifi_pwd);

		//创建密码模式切换 密码模式进去一定是不可见的 可见只是当时可见 退出再进任然不可见
		
		lv_obj_t *pwd_mode = lv_img_create(wifipwd_cont, NULL);
		
		set_location(pwd_mode, 932, 102, 48, 48);
		static rom_bin_info pwd_info = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
		lv_img_set_src(pwd_mode, &pwd_info);
		
		lv_obj_set_click(pwd_mode, true);
		lv_obj_set_ext_click_area(pwd_mode, 15, 15, 15, 15);

		static btn_data img_data = btn_data_up_create(change_pwd_mode);
		pwd_mode->user_data = &img_data;
		btn_touch_event_listen(pwd_mode);
		
		
		//创建一个键盘 并且绑定这个输入框
		lv_obj_t* kb = keybord_create(wifipwd_cont, pwd_ta);
		lv_obj_set_id(kb, 0);

		//在键盘的确认按钮的回调上是保存要连接的wifi输入的密码
	}
}

#endif


static void addwifi_page_craete(lv_obj_t *parent){
	
	lv_obj_t* addwifi_cont = lv_cont_create(lv_scr_act(),NULL);//在当前活跃的屏幕上创建容器
	set_location(addwifi_cont, 0, 100, 1024, 500);
	lv_cont_set_layout(addwifi_cont, LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_color(addwifi_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(addwifi_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);

	//不再创建输入框 直接改为容器
	lv_obj_t* wifiname_cont = lv_cont_create(addwifi_cont,NULL);
	lv_obj_set_style_local_bg_color(wifiname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(wifiname_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  LV_OPA_COVER);
	set_location(wifiname_cont, 245,102,538,80);
	lv_cont_set_layout(wifiname_cont, LV_FIT_NONE);
	lv_obj_set_click(wifiname_cont, true);
	
	lv_obj_set_event_cb(wifiname_cont, input_wifi_name_up);

	
	lv_obj_t* wifipwd_cont = lv_cont_create(addwifi_cont,wifiname_cont);
	set_location(wifipwd_cont, 245,213,538,80);
	
	
	lv_obj_set_event_cb(wifipwd_cont, input_wifi_pwd_up);


	//创建提示符
	lv_obj_t *wifiname_label = lv_label_create(addwifi_cont, NULL);
	lv_obj_set_style_local_text_font(wifiname_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	// char *src[language_total] = {"Wifi name:", "Wifi名称:", "Имя Wi-Fi:", "   Nom Wi-Fi:", "Nombre wifi:", "WLAN-Name:", ":اسم شبكة واي فاي", "Název WiFi:", "שם רשת WIFI ","Nome da Rede Wi-Fi","Nome Wi-Fi"};
	// char *src1[language_total] = {" Password:", "Wifi密码:", "   Пароль:", "Mot de passe:", "      Clave:", " Passwort:", ":كلمة السر        ", "Heslo WiFi:", "סיסמה","Senha","Password"};
	lv_label_set_text(wifiname_label, str_get(LAYOUT_ADDWIFI_LANG_WIFINAME_ID));
	lv_obj_align(wifiname_label, wifiname_cont, LV_ALIGN_IN_LEFT_MID, 10, 0);
	lv_obj_set_parent_event(wifiname_label, true);

	lv_obj_t *wifipwd_label = lv_label_create(addwifi_cont, wifiname_label);
	
	
	lv_label_set_text(wifipwd_label, str_get(LAYOUT_ADDWIFI_LANG_WIFIPASSWORD_ID));
	lv_obj_align(wifipwd_label, wifipwd_cont, LV_ALIGN_IN_LEFT_MID, 10, 0);
	lv_obj_set_parent_event(wifipwd_label, true);

	//创建输入的内容
	name_label = lv_label_create(wifiname_cont, NULL);
	lv_obj_set_style_local_text_font(name_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_label_set_text(name_label, "");
	lv_obj_align(name_label, wifiname_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_set_parent_event(name_label, true);
	wifiname_cont->user_data = name_label;

	
	pwd_label = lv_label_create(wifipwd_cont, name_label);
	lv_label_set_text(pwd_label, "");
	lv_obj_align(pwd_label, wifipwd_label, LV_ALIGN_OUT_RIGHT_MID, 5, 7);
	lv_obj_set_parent_event(pwd_label, true);
	wifipwd_cont->user_data = pwd_label;
	
	addwifi_confirm_btn_create(addwifi_cont);
	addwifi_cancel_btn_create(addwifi_cont);
	
}


static lv_task_t * set_scan_wifi_ptask = NULL;


static void scan_wifi_task(struct _lv_task_t *task_t){
	
		wpa_cli_scan_wifi(&user_data_get()->wifi.wifi_open_flag);
		wpa_cli_wlan_status(&user_data_get()->wifi.wifi_open_flag);
	
}


static void scan_wifi(void){
	if(user_data_get()->wifi.wifi_open_flag == false)
		return ;
	
	if (set_scan_wifi_ptask != NULL) {
			lv_task_del(set_scan_wifi_ptask);
	}
	
	set_scan_wifi_ptask = lv_task_create(scan_wifi_task, 2000, LV_TASK_PRIO_MID, NULL);
	lv_task_ready(set_scan_wifi_ptask);
	
	scan_wifi_task(set_scan_wifi_ptask);

}



extern void memory_print(void);

static void LAYOUT_ENETER_FUNC(add_wifi)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);//系统背景使能设置

	addwifi_head_label_create();//创建头
	addwifi_back_btn_create();//创建返回按钮

	addwifi_page_craete(lv_scr_act());//创建添加wifi的页面
	scan_wifi();

}

static void LAYOUT_QUIT_FUNC(add_wifi)
{
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	memset(addwifi_name,0,strlen(addwifi_name));
	memset(addwifi_pwd,0,strlen(addwifi_pwd));
	lv_label_set_text(name_label,"");
	lv_label_set_text(pwd_label,"");

	
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
	
	if(connect_falge)
	{
		system("killall wpa_supplicant ");
		system("killall udhcpc ");
		system("rm -rf /tmp/wpa_supplicant.conf ");
		
		system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B &");
		system("udhcpc -i wlan0 -n 4 -R &");
	}

}

CREATE_LAYOUT(add_wifi);

