#include "layout_define.h"

extern int* playback_pview_select_get(void);
extern media_type playback_pview_type_get(void);
extern const char* playback_pview_path_get(void);
extern int* playback_pview_total_get(void);
extern int* playback_pview_item_get(void);
extern void playback_sd_status_change_callback(unsigned long arg1, unsigned long arg2);

extern void layout_playback_quit_mask_set(unsigned int mask);
extern unsigned int layout_playback_quit_mask_get();
static void media_photo_info_label_display(void);
static void media_photo_lock_img_display(bool en);


static bool media_photo_display(void)
{
	int* select = playback_pview_select_get();
	media_info* info = media_info_get(playback_pview_type_get(), *select);
	char file_path[64] = {0};
	
	strcpy(file_path,playback_pview_path_get());
	strcat(file_path,info->file_name);
	
	if(media_thumb_load(0,0,1024,600,file_path) == false)
	{
		media_file_delete(info->type, *select);
		playback_thumb_parameter_init();
		goto_layout(pLAYOUT(playback));
		return false;				
	}

	media_file_new_clear(info->type, *select);

//	media_photo_lock_img_display(info->is_lock);

	media_photo_lock_img_display(false);

	lv_obj_invalidate(lv_scr_act());

	return true;
	
	
}



static void media_photo_btn_state_set(lv_obj_t* obj,lv_state_t state)
{
	btn_data* pdata = (btn_data*)obj->user_data;
	lv_obj_t* children = (lv_obj_t*)pdata->user_data;
	lv_obj_set_state( children, state);
}

static void media_photo_btn_img_transform_set(lv_obj_t* obj)
{
	lv_obj_set_style_local_transform_zoom(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,256);
	lv_obj_set_style_local_transform_zoom(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,300);
	
	lv_obj_set_style_local_transition_prop_1(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_STYLE_TRANSFORM_ZOOM);
	lv_obj_set_style_local_transition_prop_2(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_STYLE_TRANSFORM_ZOOM);

	lv_obj_set_style_local_transition_time(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,200);
	lv_obj_set_style_local_transition_time(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,200);

	static lv_anim_path_t path ;
	path.cb = lv_anim_path_overshoot,
	path.user_data = NULL;
	lv_obj_set_style_local_transition_path(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,&path);
	lv_obj_set_style_local_transition_path(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,&path);
}

static lv_obj_t* media_photo_btn_create(int x,int y,int w,int h,btn_data* btn_pdata,const void* img_src,bool bg_color)
{
	lv_obj_t* btn = lv_btn_create(lv_scr_act(),NULL);
	lv_obj_set_pos(btn, x,y);
	lv_obj_set_size(btn,w,h);

	if(bg_color == true)
	{
		lv_obj_set_style_local_bg_color(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
		lv_obj_set_style_local_bg_color(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(0x4d,0x7a,0xFF));
		
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_OPA_70);

		lv_obj_set_style_local_radius(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);
		lv_obj_set_style_local_radius(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,45);
	}
	else
	{
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_OPA_TRANSP);
	}
	
	lv_obj_t* img = lv_img_create(lv_scr_act(),NULL);
	lv_img_set_src(img, img_src);


	media_photo_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	
	return btn;
}


static void media_photo_cancel_btn_down(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_PRESSED);
}


static void media_photo_cancel_cancel_btn_up(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_DEFAULT);

	int* select_index = playback_pview_select_get();
	int* media_total = playback_pview_total_get();
	int* item_index = playback_pview_item_get();
	int count = (*media_total) - 1;
	(*item_index) = count - (count - (*select_index))/6*6;

	layout_playback_quit_mask_set(0x01);
	goto_layout(pLAYOUT(playback));
}
static void media_photo_cancel_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(media_photo_cancel_btn_down, media_photo_cancel_cancel_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t * btn = media_photo_btn_create(25,25,60,60,&btn_data,&info,true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
}



static void media_photo_delete_btn_down(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_PRESSED);
}




static void media_photo_msgbox_btn_up(lv_obj_t* obj)
{
	unsigned int btn_id = lv_msgbox_get_active_btn(obj);
	if(btn_id  == 1)
	{
		int* select_index = playback_pview_select_get();
		
		const media_type type = playback_pview_type_get();
		
		media_file_delete(type, *select_index);
		
		int total = media_file_total_get(type, false);
		if(total < 1)
		{
			goto_layout(pLAYOUT(home));
		}
		else
		{
			int* ptotal = playback_pview_total_get();
			*ptotal = total;
			if((*select_index) > (total - 1))
			{
				*select_index = total- 1;
			}
			media_info* info = media_info_get(type, *select_index);
			if(info->type != FILE_TYPE_SD_MIXED_VIDEO)
			{
			
				media_photo_info_label_display();
				media_photo_display();
				lv_obj_del(obj);
			}
			else
			{
				layout_playback_quit_mask_set(1);
				goto_layout(pLAYOUT(video));
			}
		}
	}
	else if(btn_id == 0)
	{
		lv_obj_del(obj);
	}
}


static void media_photo_delete_msgbox_create(void)
{
	lv_obj_t* msg_box = lv_msgbox_create(lv_scr_act(), NULL);
	lv_obj_set_id(msg_box, 200);
	lv_obj_set_style_local_bg_color(msg_box,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
	lv_obj_set_style_local_bg_opa(msg_box,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);	
	
	// static const char * btns[language_total][3] ={{"Cancle", "Confirm", ""},{"取消","确认",""},{"Отмена","Подтверждать",""},{"Annuler","Confirmer",""},{"Cancelar","Confirmar",""},{"Absagen","Bestätigen Sie",""},{"الغاء","تاكيد",""},{"Zrušit","Potvrdit",""}, {"ביטול", "אישור",""},{"Cancelar","Confirmar"},{"Annulla","Conferma",""}};
	// char *src[language_total] = {"\n\nDelete ?\n\n","\n\n删除 ?\n\n","\n\nУдалить ?\n\n","\n\nSupprimez ?\n\n","\n\nEliminar ?\n\n","\n\nLöschen ?\n\n","\n\n? حذف\n\n","\n\nVymazat ?\n\n", "\n\nמחיקה ?\n\n","\n\nApagar\n\n","\n\nElimina ?\n\n"};
	lv_msgbox_set_text(msg_box, str_get(LAYOUT_PHOTO_LANG_DELETE_ID));
	
	lv_msgbox_add_btns(msg_box, btns_str_get());
	lv_obj_set_size(msg_box, 470,300);

	static btn_data btn_data = btn_data_up_create(media_photo_msgbox_btn_up);	
	msg_box->user_data = &btn_data;
	btn_touch_event_listen(msg_box);
	lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/

	lv_obj_t* btnmatri_btn = lv_msgbox_get_btnmatrix(msg_box);

	lv_obj_set_style_local_bg_color(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_PRESSED,LV_COLOR_MAKE(0xFF,0,0));
	lv_obj_set_style_local_radius(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_DEFAULT,45);
}

static void media_photo_delete_btn_up(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_DEFAULT);

	lv_obj_t* msgbox = lv_obj_get_child_form_id(lv_scr_act(), 200);
	if(msgbox != NULL)
	{
		lv_obj_del(msgbox);
	}
	else
	{
		media_photo_delete_msgbox_create();
		screen_force_refresh();
	}
}

static void media_photo_delete_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(media_photo_delete_btn_down, media_photo_delete_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_DELETE_PNG);
	media_photo_btn_create(917,25,60,60,&btn_data,&info,true);
}



static void media_photo_left_btn_down(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_PRESSED);
}


static void media_photo_left_btn_up(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_DEFAULT);

	int* select_index = playback_pview_select_get();
	int* media_total = playback_pview_total_get();
	(*select_index) += 1;
	if((*select_index ) == (*media_total))
	{
		(*select_index) = 0;
	}
	media_info* info = media_info_get(playback_pview_type_get(), *select_index);
	if(info->type != FILE_TYPE_SD_MIXED_VIDEO)
	{
		// media_photo_info_label_display();
		// media_photo_display();

		goto_layout(pLAYOUT(photo));
	}
	else
	{
		layout_playback_quit_mask_set(0x01);
		goto_layout(pLAYOUT(video));
	}

}

static void media_photo_left_btn_create(void)
{
	int* total = playback_pview_total_get();
	if(*total < 2)
	{
		return ;
	}

	static btn_data btn_data  = btn_data_create(media_photo_left_btn_down, media_photo_left_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_LEFT_PNG);
	media_photo_btn_create(0,260,80,80,&btn_data,&info,false);
}



static void media_photo_right_btn_down(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_PRESSED);
}


static void media_photo_right_btn_up(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_DEFAULT);

	int* select_index = playback_pview_select_get();
	int* media_total = playback_pview_total_get();
	(*select_index) -= 1;
	if((*select_index ) <0)
	{
		(*select_index) = (*media_total) - 1;
	}
	media_info* info = media_info_get(playback_pview_type_get(), *select_index);
	if(info->type != FILE_TYPE_SD_MIXED_VIDEO)
	{
		media_photo_info_label_display();
		media_photo_display();
	}
	else
	{
		layout_playback_quit_mask_set(0x01);
		goto_layout(pLAYOUT(video));
	}
}

static void media_photo_right_btn_create(void)
{
	int* total = playback_pview_total_get();
	if(*total < 2)
	{
		return ;
	}

	static btn_data btn_data  = btn_data_create(media_photo_right_btn_down, media_photo_right_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_RIGHT_PNG);
	media_photo_btn_create(944,260,80,80,&btn_data,&info,false);
}



static void media_photo_info_label_display(void)
{
	lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), 0);
	if(parent != NULL)
	{		
		int* select = playback_pview_select_get();
		media_info* info = media_info_get(playback_pview_type_get(), *select);

	
		lv_obj_t* label_channel = lv_obj_get_child_form_id(parent, 0);
		if(label_channel != NULL)
		{
            // char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür","باب","Dveře","שער","Porta","Porta"};
            char str_num[1] = {0};
            if(info->ch<=2)
            {
                sprintf(str_num,"%d",info->ch);
            }
            else
            {
                sprintf(str_num,"%d",info->ch-2);
            }
			if(info->ch>16){
				sprintf(str_num,"%d",info->ch-18);
				char *door_str = (char *) malloc(strlen(str_num) + 8);
				strcpy(door_str, "CCTV");
				strcat(door_str, str_num);
				lv_label_set_text(label_channel, door_str);
				free(door_str);
			}
			else{
				char *door_str = (char *) malloc(strlen(str_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
				strcpy(door_str, str_get(LAYOUT_HOME_LANG_DOOR_ID));
				strcat(door_str, str_num);
				lv_label_set_text(label_channel, door_str);
				free(door_str);
			}
		}

		lv_obj_t* label_time = lv_obj_get_child_form_id(parent, 1);
		if(label_time != NULL)
		{
			char str[128] = {"0"};
			strncpy(&str[0],&info->file_name[0],4);
			str[4] = '-';
			strncat(&str[5],&info->file_name[4],2);
			str[7] = '-';
			strncat(&str[8],&info->file_name[6],2);
			str[10] = ' ';
			str[11] = ' ';
			
			strncat(&str[12],&info->file_name[9],2);
			str[14] = ':';
			strncat(&str[15],&info->file_name[11],2);
			str[17] = ':';
			strncat(&str[18],&info->file_name[13],2);
			lv_label_set_text(label_time, str);
		}
	}
}

static void media_photo_info_label_create(void)
{
	lv_obj_t* obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 0);
	lv_obj_set_pos(obj,38,500);
	lv_obj_set_size(obj, 400, 70);

	lv_obj_t* label_channel = lv_label_create(obj, NULL);
	lv_obj_set_id(label_channel,0);
	lv_obj_t* label_time = lv_label_create(obj, NULL);
	lv_obj_set_id(label_time,1);	
	media_photo_info_label_display();
	
	lv_obj_align(label_channel, obj, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_align(label_time, obj, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
}



static void media_photo_lock_img_display(bool en)
{	
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(),1);
	if(obj != NULL)
	{
		lv_obj_set_hidden(obj, en?false:true);
	}
	
}
static void media_photo_lock_img_create(void)
{
	lv_obj_t* obj = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj,1);
	lv_obj_set_pos(obj,499,30);
	lv_obj_set_size(obj, 27 , 32);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_LOCK_PNG);
	lv_img_set_src(obj,&info);
	if(obj != NULL)
		lv_obj_set_hidden(obj, true);
}	

static void photo_sd_status_change_callback(unsigned long arg1, unsigned long arg2);

static void LAYOUT_ENETER_FUNC(photo)
{
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);

	sdcard_event_register(photo_sd_status_change_callback);

	media_photo_cancel_btn_create();

	media_photo_lock_img_create();
	
	media_photo_delete_btn_create();

	media_photo_left_btn_create();

	media_photo_right_btn_create();

	media_photo_info_label_create();
	
	media_photo_display();
}


static void LAYOUT_QUIT_FUNC(photo)
{
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);	
	sdcard_event_register(NULL);
	if(!layout_playback_quit_mask_get())
	{
		media_thumb_device_close();//关闭
	}

}

CREATE_LAYOUT(photo);

/*
* arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
* arg2:arg1= 1,arg2参数无意义
*	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
*	   arg1=3,arg2=1:内存正常，2，内存已满
*/
static void photo_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
    if (arg1 == 1)
    {	
    	goto_layout(pLAYOUT(home));
    }
}


