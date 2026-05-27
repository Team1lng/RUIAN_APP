#include "layout_define.h"

void playback_sd_status_change_callback(unsigned long arg1, unsigned long arg2);

static media_type playback_pview_type = FILE_TYPE_NONE;
media_type playback_pview_type_get(void)
{
	return playback_pview_type;
}

static char* playback_pview_path = NULL;
const char* playback_pview_path_get(void)
{
	return playback_pview_path;
}

static int playback_pview_total = 0;
int* playback_pview_total_get(void)
{
	return &playback_pview_total;
}

static int playback_pview_item = 0;
int* playback_pview_item_get(void)
{
	return &playback_pview_item;
}

static int playback_pview_select = -1;
int* playback_pview_select_get(void)
{
	return &playback_pview_select;
}

static unsigned int quit_playback_mask = 0x00;
void layout_playback_quit_mask_set(unsigned int mask)
{
	quit_playback_mask = mask;
}

unsigned int layout_playback_quit_mask_get()
{
	return quit_playback_mask;
}

static bool lock_falg = false;
#define FILE_MAX_NUMS 1000
bool del_file_index[FILE_MAX_NUMS];

static lv_obj_t* playback_thumb_win_cont[6] = {0};

void playback_thumb_parameter_init(void)
{
	if(is_sdcard_insert() == true)//sd卡插入
	{
		playback_pview_type = FILE_TYPE_SD_MIXED;//查看照片的类型
		playback_pview_path = SD_MIXED_PATH;//照片路径
	}
	else
	{
		playback_pview_type = FILE_TYPE_FLASH_PHOTO;
		playback_pview_path = FLASH_PHOTO_PATH;
	}

	playback_pview_total = media_file_total_get(playback_pview_type, 0);//照片数量
	playback_pview_item = playback_pview_select = playback_pview_total - 1;
	playback_pview_select = -1;
}





const media_info* playback_file_path_get(int index,char* file_path)
{
	media_info* info = media_info_get(playback_pview_type, index);
	if(info == NULL)
	{
		printf("media_info_get is NULL ========%s========\n",__func__);
	}
	strcpy(file_path,playback_pview_path);
	strcat(file_path,info->file_name);
	return info;
}

static void playback_thumb_check_box_up(lv_obj_t* obj)
{
	for(int i = 0 ; i < 6 ; i++)
	{
		lv_obj_t* child = lv_obj_get_child_form_id(playback_thumb_win_cont[i],3);
		if((child != NULL)&&(child == obj))
		{
			printf("checkbox click\n");
			int index = playback_pview_item - i;
			bool checked = lv_checkbox_is_checked(obj);
			if(checked)
			{

				del_file_index[index] = true;

			}
			else if(checked == false)
			{
				del_file_index[index] = false;

			}
			//media_file_lock_set(playback_pview_type, index, lv_checkbox_is_checked(obj)?true:false);
			return ;
		}
	}
}

//显示照片
static void playback_thumb_win_cont_up(lv_obj_t* obj)
{
	for(int i = 0 ; i < 6 ; i++)
	{
		if(playback_thumb_win_cont[i] == obj)
		{
			int index = playback_pview_item - i;
			printf("playback_pview_item is %d\n",playback_pview_item);
			lv_obj_t* checkbox_chhild = lv_obj_get_child_form_id(obj, 3);
			if((checkbox_chhild != NULL)&&(lv_obj_get_hidden(checkbox_chhild) == false))
			{
				bool checked = lv_checkbox_is_checked(checkbox_chhild)?false:true;
				lv_checkbox_set_checked(checkbox_chhild, checked);
				if(checked)
				{

					del_file_index[index] = true;

				}
				else if(checked == false)
				{
					del_file_index[index] = false;

				}
			}
			else
			{
				playback_pview_select = index;
				media_info* info = media_info_get(playback_pview_type, playback_pview_select);
				layout_playback_quit_mask_set(0x00);
				if(info->type == FILE_TYPE_SD_MIXED_VIDEO)
				{
					goto_layout(pLAYOUT(video));
				}
				else
				{
					goto_layout(pLAYOUT(photo));
				}
				
			}
			return ;
		}
	}
}

static void playback_thumb_win_cont_down(lv_obj_t* obj)
{
	for(int i = 0 ; i < 6 ; i++)
	{
		if((obj != playback_thumb_win_cont[i])&&(lv_obj_get_state(playback_thumb_win_cont[i], LV_CONT_PART_MAIN) == LV_STATE_FOCUSED))
		{
			lv_obj_set_state(playback_thumb_win_cont[i], LV_STATE_DEFAULT);
		}
	}
}




static void playback_thumb_win_child_create(int i)
{
	//index 0
	lv_obj_t* obj_bg = lv_cont_create(playback_thumb_win_cont[i], NULL);
	lv_obj_set_click(obj_bg, false);
	lv_obj_set_pos(obj_bg, 0, 140);
	lv_obj_set_size(obj_bg, 300, 40);
	lv_obj_set_style_local_outline_color(obj_bg,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0,0,0));
	lv_obj_set_style_local_bg_opa(obj_bg,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);
	lv_obj_set_id(obj_bg,0);
	

	//index 1
	lv_obj_t* obj_new = lv_img_create(playback_thumb_win_cont[i], NULL);
	lv_obj_align(obj_new, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_set_id(obj_new,1);
	
	
	rom_bin_info obj_new_info = rom_bin_info_get(ROM_RES_THUMB_NEW_PNG);	
	lv_img_set_src(obj_new, &obj_new_info);

	//index 2
	lv_obj_t* obj_play = lv_cont_create(playback_thumb_win_cont[i], NULL);
	lv_obj_set_click(obj_play, false);
	lv_obj_align(obj_play, NULL, LV_ALIGN_CENTER, 0, -40);
	lv_obj_set_size(obj_play, 72, 72);
	lv_obj_set_style_local_bg_opa(obj_play,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);
	lv_obj_set_style_local_radius(obj_play,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,45);	
	lv_obj_set_style_local_bg_color(obj_play,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0,0,0));
	lv_obj_set_id(obj_play,2);
	
	
	lv_obj_t* play_icon_img = lv_img_create(obj_play, NULL);
	rom_bin_info play_icon_img_info = rom_bin_info_get(ROM_RES_THUMB_PLAY_PNG);	
	lv_img_set_src(play_icon_img, &play_icon_img_info);
	lv_obj_align(play_icon_img, NULL, LV_ALIGN_CENTER, 0, 0);


	//index 3
	lv_obj_t* obj_check = lv_checkbox_create(playback_thumb_win_cont[i], NULL);
	lv_checkbox_set_text(obj_check,"");
	lv_obj_set_size(obj_check, 60, 60);
	lv_obj_align(obj_check, NULL, LV_ALIGN_IN_TOP_RIGHT, 10, 10);

	static rom_bin_info img_info_on = rom_bin_info_get(ROM_RES_THUMB_CHECK_ON_PNG);
	lv_obj_set_style_local_pattern_image(obj_check,LV_OBJ_PART_MAIN,LV_STATE_CHECKED,&img_info_on);
	static rom_bin_info img_info_off = rom_bin_info_get(ROM_RES_THUMB_CHECK_PNG);
	lv_obj_set_style_local_pattern_image(obj_check,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,&img_info_off);
	lv_obj_set_id(obj_check,3);
	
	
	static btn_data btn_data = btn_data_up_create(playback_thumb_check_box_up);
	obj_check->user_data = &btn_data;

	btn_touch_event_listen(obj_check);
	if(obj_check != NULL)
		lv_obj_set_hidden(obj_check, true);
#if 1
	lv_obj_t* obj_lock = lv_img_create(playback_thumb_win_cont[i], NULL);
	lv_obj_align(obj_lock, NULL, LV_ALIGN_IN_TOP_RIGHT, 40, 10);
	rom_bin_info lock_info = rom_bin_info_get(ROM_RES_THUMB_LOCK_PNG);
	lv_img_set_src(obj_lock,&lock_info);
	lv_obj_set_hidden(obj_lock, true);
	lv_obj_set_id(obj_lock,4);
#endif	
	
	//bg child
	//index 0 file name
	lv_obj_t* obj_ch = lv_label_create(obj_bg, NULL);
	lv_label_set_align(obj_ch, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(obj_ch, obj_bg, LV_ALIGN_IN_LEFT_MID, 10, 0);
	lv_obj_set_id(obj_ch,0);

	//index 1 channel
	lv_obj_t* obj_date = lv_label_create(obj_bg, NULL);
	lv_label_set_align(obj_date, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(obj_date, obj_bg, LV_ALIGN_IN_RIGHT_MID, -20, 0);
	lv_obj_set_id(obj_date,1);
}


static void playback_thumb_head_label_create(void)
{

	lv_obj_t* label = lv_label_create(lv_scr_act(), NULL);
	
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	

		// char *language[language_total] = {"Playback","浏览","Проигрывание","Playback","Reproducción","Wiedergabe","عرض التسجيلات","Přehrávání", "הקלטות","Reprodução","Riproduzione"};

	lv_label_set_text(label, str_get(LAYOUT_HOME_LANG_PLAYBACK_ID));
	lv_label_set_long_mode(label,LV_LABEL_LONG_CROP);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_size(label,1024,60);
	lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 36);
	lv_label_set_align(label,LV_LAYOUT_CENTER);
}


static void playback_thumb_win_cont_create(void)
{
	lv_area_t  pos[6] =
	{
		{48  ,148},
		{362, 148},
		{674, 148},
		{48  ,340},
		{362, 340},
		{674, 340}
	};

	playback_thumb_win_cont[0] = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(playback_thumb_win_cont[0], pos[0].x1, pos[0].y1);
	lv_obj_set_size(playback_thumb_win_cont[0], 300,180);

	lv_obj_set_style_local_outline_color(playback_thumb_win_cont[0],LV_CONT_PART_MAIN,LV_STATE_FOCUSED,lv_color_make(0xB8,0x8d,0x56));
	lv_obj_set_style_local_outline_width(playback_thumb_win_cont[0],LV_CONT_PART_MAIN,LV_STATE_FOCUSED,3);
	lv_obj_set_style_local_outline_pad(playback_thumb_win_cont[0],LV_CONT_PART_MAIN,LV_STATE_FOCUSED,1);

	playback_thumb_win_child_create(0);
	
	static btn_data btn_data  = btn_data_create(playback_thumb_win_cont_down,playback_thumb_win_cont_up,NULL);
	playback_thumb_win_cont[0]->user_data = &btn_data;
	btn_touch_event_listen(playback_thumb_win_cont[0]);
	
	for(int i = 1 ; i < 6 ; i++)
	{
		playback_thumb_win_cont[i] = lv_cont_create(lv_scr_act(), playback_thumb_win_cont[0]);
		lv_obj_set_pos(playback_thumb_win_cont[i], pos[i].x1, pos[i].y1);
		
		playback_thumb_win_child_create(i);//对照片进行信息的补充和创建
	}

}

static lv_obj_t* playback_total_page_label;
static void playback_total_page_label_display(void)
{
	char str[64] = {0};
	sprintf(str,"%03d/%03d",(playback_pview_total - playback_pview_item)/6 + 1,(playback_pview_total-1)/6+1);

	lv_label_set_text(playback_total_page_label, str);
}
static void playback_total_page_label_create(void)
{
	lv_obj_t* obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(obj,774 , 520);
	lv_obj_set_size(obj, 200,60);

	playback_total_page_label = lv_label_create(obj, NULL);
	lv_obj_set_style_local_text_font(playback_total_page_label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,FONT_SIZE(31));	
	playback_total_page_label_display();
	lv_label_set_align(playback_total_page_label, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(playback_total_page_label, obj, LV_ALIGN_IN_RIGHT_MID , 0, 0);
}


static void playback_thumb_info_display(const media_info* info,int index)
{
	lv_obj_t* parent = lv_obj_get_child_form_id(playback_thumb_win_cont[index],0);
	
	lv_obj_t* child = NULL;
	if(parent !=  NULL)
	{
		child = lv_obj_get_child_form_id(parent,0);
		if(child != NULL)
		{
			lv_label_set_text(child, info->file_name);
		}

		child = lv_obj_get_child_form_id(parent,1); 
		if(child != NULL)
		{
            // char * src[language_total] = {"Door", "大门", "Дверь", "Porte", "Puerta", "Tür","باب","Dveře","שער","Porta","Porta"};
			char str_num[1] = {0};
            if(info->ch<=2)
            {
                sprintf(str_num,"%d",info->ch); 
            }
            else
            {
                sprintf(str_num,"%d",info->ch-3);
            }
			if(info->ch>16){
                sprintf(str_num,"%d",info->ch-18);
				char *door_str = (char *) malloc(strlen(str_num) + 8);
				strcpy(door_str, "CCTV");
				strcat(door_str, str_num);
				lv_label_set_text(child, door_str);
				lv_obj_align(child, parent, LV_ALIGN_IN_RIGHT_MID, 0, 0);
				free(door_str);	
			}
			else{
				char *door_str = (char *) malloc(strlen(str_num) + strlen(str_get(LAYOUT_HOME_LANG_DOOR_ID)));
				strcpy(door_str, str_get(LAYOUT_HOME_LANG_DOOR_ID));
				strcat(door_str, str_num);
				lv_label_set_text(child, door_str);
				lv_obj_align(child, parent, LV_ALIGN_IN_RIGHT_MID, 0, 0);
				free(door_str);	
			}
		}
	}

	parent = lv_obj_get_child_form_id(playback_thumb_win_cont[index],1);
	if(parent != NULL)
	{
		lv_obj_set_hidden(parent, info->is_new?false:true);
	}


	parent = lv_obj_get_child_form_id(playback_thumb_win_cont[index],2);
	if(parent != NULL)
	{
		lv_obj_set_hidden(parent, info->type == FILE_TYPE_SD_MIXED_VIDEO?false:true);
	}

	
	parent = lv_obj_get_child_form_id(playback_thumb_win_cont[index],3);
	lv_obj_set_hidden(parent,lock_falg?false:true);
	if(parent != NULL)
	{
		int i = playback_pview_item - index;
		// media_info* info = media_info_get(playback_pview_type,i);
		lv_checkbox_set_checked(parent, del_file_index[i] == true?true:false);
		
#if 0

		lv_checkbox_set_checked(parent, info->is_lock?true:false);

		lv_obj_t* child = lv_obj_get_child_form_id(playback_thumb_win_cont[index],4);
		if(child != NULL)
		{
			if(lv_obj_get_hidden(parent) == false)
			{
				lv_obj_set_hidden(child,true);
			}
			else
			{
				lv_obj_set_hidden(child,info->is_lock?false:true);
			}
		}
#endif
	}

}


static void playback_thumb_dsiplay(void)
{
	char file_path[64] = {0};
	const media_info* pmedia_info = NULL;
	for(int i = 0 ; i < 6 ; i++)
	{
		int index = playback_pview_item - i;
		if(index < 0)
		{
			if(playback_thumb_win_cont[i] != NULL)
				lv_obj_set_hidden(playback_thumb_win_cont[i], true);
			system_bg_fill_color(0x00,lv_obj_get_x(playback_thumb_win_cont[i]), lv_obj_get_y(playback_thumb_win_cont[i]), lv_obj_get_width(playback_thumb_win_cont[i]) , lv_obj_get_height(playback_thumb_win_cont[i]));
		}
		else
		{
			if(playback_thumb_win_cont[i] != NULL)
			lv_obj_set_hidden(playback_thumb_win_cont[i], false);
			pmedia_info = playback_file_path_get(index,file_path);
			printf("meida path is %s\n",file_path);			
			if(media_thumb_load(lv_obj_get_x(playback_thumb_win_cont[i]), lv_obj_get_y(playback_thumb_win_cont[i]), lv_obj_get_width(playback_thumb_win_cont[i]) , lv_obj_get_height(playback_thumb_win_cont[i]) , file_path) == false)
			{
				media_file_delete(pmedia_info->type, index);
				playback_thumb_parameter_init();
				goto_layout(pLAYOUT(playback));
				return ;
				
			}
			playback_thumb_info_display(pmedia_info,i);
		}
	}
}




static void playback_btn_state_set(lv_obj_t* obj,lv_state_t state)
{
	btn_data* pdata = (btn_data*)obj->user_data;
	lv_obj_t* children = (lv_obj_t*)pdata->user_data;
	lv_obj_set_state( children, state);
}

static void playback_btn_img_transform_set(lv_obj_t* obj)
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

static lv_obj_t*  playback_btn_create(int x,int y,int w,int h,btn_data* btn_pdata,const void* img_src,bool bg_color)
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


	playback_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	
	return btn;
}



static void playback_cancel_btn_down(lv_obj_t* obj)
{	
	playback_btn_state_set(obj,LV_STATE_PRESSED);
}


static void playback_cancel_btn_up(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_DEFAULT);

	media_thumb_device_close();//关闭

	goto_layout(pLAYOUT(home));
}

static void playback_cancel_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(playback_cancel_btn_down, playback_cancel_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t *btn = playback_btn_create(25,25,60,60,&btn_data,&info,true);
	lv_obj_set_ext_click_area(btn, 15, 15, 15, 15);
}



static void playback_lock_btn_down(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_PRESSED);
	lock_falg = !lock_falg;
}


static void playback_lock_btn_up(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_DEFAULT);
	
	if(lock_falg){
		if(lv_obj_get_hidden(playback_thumb_win_cont[0]) == false)
		lv_obj_set_style_local_bg_color(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x4d,0x7a,0xFF));
	}else
		lv_obj_set_style_local_bg_color(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
	
	for(int i = 0 ; i < 6 ; i++)
	{
		if(lv_obj_get_hidden(playback_thumb_win_cont[i]) == true)
		{
			continue;
		}
		
		
		lv_obj_t* child = lv_obj_get_child_form_id(playback_thumb_win_cont[i],3);
		if(child != NULL)
		{
			lv_obj_set_hidden(child, lv_obj_get_hidden(child)?false:true);
#if 0
	lv_obj_t* child_lock = lv_obj_get_child_form_id(playback_thumb_win_cont[i],4);
			int index = playback_pview_item - i;
			if((index >= 0)&&(index < playback_pview_total))
			{
				 media_info* pinfo = media_info_get(playback_pview_type, index);
				 if(pinfo != NULL)
				 {
					if(lv_obj_get_hidden(child) == false)
					{
						lv_obj_set_state(child, pinfo->is_lock?LV_STATE_CHECKED:LV_STATE_DEFAULT);
						
						if(child_lock != NULL)
						{
							lv_obj_set_hidden(child_lock, true);
						}
					}
					else 
					{
						lv_obj_set_hidden(child_lock, pinfo->is_lock?false:true);
					}

				 }
			}
#endif		
		}	
		
	}
	
}

static void playback_lock_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(playback_lock_btn_down, playback_lock_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_LOCK_PNG);
	playback_btn_create(841,25,60,60,&btn_data,&info,true);
}





static void playback_delete_btn_down(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_PRESSED);
	
}



static void playback_thumb_msgbox_btn_up(lv_obj_t* obj)
{
	unsigned int btn_id = lv_msgbox_get_active_btn(obj);
	if(btn_id  == 1)
	{
		if(lock_falg)
		{
			printf("==playback_pview_type=== is %d\n",playback_pview_type);
			for(int i = FILE_MAX_NUMS - 1;i >= 0;i--)
			{
				if(del_file_index[i] == true)
				{
					media_file_delete(playback_pview_type,i);
				}
			}
		}
		else
		{
			start_delete_media(playback_pview_type == FILE_TYPE_FLASH_PHOTO?DELETE_ALL_FLASH_PHOTO:DELETE_ALL_MIXED);
			while(delete_media_status())
			{
				ak_sleep_ms(10);
			}
		}
		playback_thumb_parameter_init();
		goto_layout(pLAYOUT(playback));
	}
	else if(btn_id == 0)
	{
		lv_obj_del(obj);
	}
}


static void playback_thumb_delete_msgbox_create(void)
{
	lv_obj_t* msg_box = lv_msgbox_create(lv_scr_act(), NULL);

	lv_obj_set_id(msg_box, 200);
	lv_obj_set_style_local_bg_color(msg_box,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
	lv_obj_set_style_local_bg_opa(msg_box,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);	
	
	// static const char * btns[language_total][3] ={{"Cancle", "Confirm", ""},{"取消","确认",""},{"Отмена","Подтверждать",""},{"Annuler","Confirmer",""},{"Cancelar","Confirmar",""},{"Absagen","Bestätigen Sie",""},{"الغاء","تاكيد",""},{"Zrušit","Potvrdit",""}, {"ביטול", "אישור",""},{"Cancelar","Confirmar"},{"Annulla","Conferma",""}};

	if(lock_falg)
	{
		// char *src[language_total] = {"\n\nDelete Selected ?\n\n","\n\n删除已选中的 ?\n\n","\n\nУдалить выбранное ?\n\n","\n\nSupprimer ce qui est sélectionné ?\n\n","\n\n¿Eliminar los seleccionados ?\n\n","\n\nAusgewählte löschen ?\n\n","\n\nحذف مختارة ؟\n\n","\n\nSmazat vybrané ?\n\n", "\n\nמחק את הנבחר ?\n\n","\n\nApagar os Seleccionados?\n\n","\n\nElimina l' elemento selezionato?\n\n"};
		lv_msgbox_set_text(msg_box, str_get(LAYOUT_PHOTO_LANG_DELETESELECTED_ID));
	}
	else
	{
		// char *src[language_total] = {"\n\nDelete All ?\n\n","\n\n全部删除 ?\n\n","\n\nУдалить все ?\n\n","\n\nSupprimer tout ?\n\n","\n\nBorrar todo ?\n\n","\n\nAlle löschen ?\n\n","\n\n? حذف الكل\n\n","\n\nSmazat všechny ?\n\n", "\n\nמחיקת הכל?\n\n","\n\nApagar Todas\n\n","\n\nElimina tutto\n\n"};

		lv_msgbox_set_text(msg_box, str_get(LAYOUT_PHOTO_LANG_DELETEALL_ID));

	}

	lv_msgbox_add_btns(msg_box, btns_str_get());
	lv_obj_set_size(msg_box, 400,300);

	static btn_data btn_data = btn_data_up_create(playback_thumb_msgbox_btn_up);	
	msg_box->user_data = &btn_data;
	btn_touch_event_listen(msg_box);
//	lv_obj_set_event_cb(msg_box, event_handler);
	lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/

	lv_obj_t* btnmatri_btn = lv_msgbox_get_btnmatrix(msg_box);

	lv_obj_set_style_local_bg_color(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_PRESSED,LV_COLOR_MAKE(0xFF,0,0));
	lv_obj_set_style_local_radius(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_DEFAULT,45);
	//lv_btnmatrix_set_btn_ctrl(btnmatri_btn, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
	//lv_btnmatrix_set_btn_ctrl(btnmatri_btn, 1, LV_BTNMATRIX_CTRL_CHECK_STATE);

}

static void playback_delete_btn_up(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_DEFAULT);

	if(playback_pview_total <= 0)
	{
		return ;
	}

	lv_obj_t* msgbox = lv_obj_get_child_form_id(lv_scr_act(), 200);
	if(msgbox != NULL)
	{
		lv_obj_del(msgbox);
	}
	else
	{
		playback_thumb_delete_msgbox_create();
	}
}

static void playback_delete_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(playback_delete_btn_down, playback_delete_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_DEL_ALL_PNG);
	playback_btn_create(917,25,60,60,&btn_data,&info,true);
}






static void playback_left_page_btn_down(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_PRESSED);
}

static void playback_buttom_aimmation_play(bool);
static void playback_left_page_btn_up(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_DEFAULT);

	if(playback_pview_total < 7)
	{
		return ;
	}
	playback_pview_item += 6;
	if(playback_pview_item > playback_pview_total)
	{
		playback_pview_item = playback_pview_total%6 -1;
		if(playback_pview_item == -1)
		{
			playback_pview_item = 5;
		}
	}
	playback_total_page_label_display();

	playback_thumb_dsiplay();

	playback_buttom_aimmation_play(false);
}

static void playback_left_page_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(playback_left_page_btn_down, playback_left_page_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t* obj = playback_btn_create(0,307,54,54,&btn_data,&info,true);
	lv_obj_set_style_local_bg_opa(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(57,57,57));
}



static void playback_right_page_btn_down(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_PRESSED);
}

static  lv_anim_t buttom_dot_anmation;

static void playback_right_page_btn_up(lv_obj_t* obj)
{
	playback_btn_state_set(obj,LV_STATE_DEFAULT);

	if(playback_pview_total < 7)
	{
		return ;
	}

	playback_pview_item -= 6;
	if(playback_pview_item < 0)
	{
		playback_pview_item = playback_pview_total -1;
	}

	playback_total_page_label_display();

	playback_thumb_dsiplay();
	
	playback_buttom_aimmation_play(true);
}

static void playback_right_page_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(playback_right_page_btn_down, playback_right_page_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_RIGHT_PNG);
	lv_obj_t* obj = playback_btn_create(970,307,54,54,&btn_data,&info,true);
	lv_obj_set_style_local_bg_opa(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);	
	lv_obj_set_style_local_bg_color(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(57,57,57));
}


static void playback_dot_annimation_display(void)
{
	lv_obj_t*parent = lv_obj_get_child_form_id(lv_scr_act(), 100);
	if(parent != NULL)
	{
		lv_obj_t* obj_child1 = lv_obj_get_child_form_id(parent, 0);
		lv_obj_t* obj_child2 = lv_obj_get_child_form_id(parent, 1);
		lv_obj_t* obj_child3 = lv_obj_get_child_form_id(parent, 2);
		if((obj_child1 == NULL)||(obj_child2 == NULL)||(obj_child3 == NULL))
		{
			return ;
		}

		if(playback_pview_total <= 6)
		{
			if(obj_child2 != NULL)
				lv_obj_set_hidden(obj_child2, true);
			if(obj_child3 != NULL)
				lv_obj_set_hidden(obj_child3, true);
			if(obj_child1 != NULL)
				lv_obj_set_hidden(obj_child1, false);
			lv_obj_set_pos(obj_child1, 24, 1);
		}
		else if(playback_pview_total <= 12)
		{
			if(obj_child3 != NULL)
				lv_obj_set_hidden(obj_child3, true);
			if(playback_pview_item < 6)
			{
				lv_obj_set_pos(obj_child2, 0, 0);
				lv_obj_set_pos(obj_child1, 24, 1);
			}
			else
			{
				lv_obj_set_pos(obj_child2, 36, 0);
				lv_obj_set_pos(obj_child1, 0, 1);
			}
		}
		else if(playback_pview_item < 6)
		{
			lv_obj_set_pos(obj_child2, 0, 0);
			lv_obj_set_pos(obj_child3, 24,  0);
			lv_obj_set_pos(obj_child1, 48, 0);
			
		}
		else if(playback_pview_item == (playback_pview_total-1))
		{
			lv_obj_set_pos(obj_child1, 0, 1);
			lv_obj_set_pos(obj_child2, 36,  0);
			lv_obj_set_pos(obj_child3, 60, 0);
		}
		else
		{
			lv_obj_set_pos(obj_child1, 24, 1);
			lv_obj_set_pos(obj_child2, 0,  0);
			lv_obj_set_pos(obj_child3, 60, 0);
		}
	}
	
}

static void playback_buttom_aimmation_play(bool is_add)
{	
	lv_obj_t* obj = (lv_obj_t*)buttom_dot_anmation.var;
	int vol_base = lv_obj_get_x(obj);

    lv_anim_set_values(&buttom_dot_anmation, vol_base , vol_base  + (is_add?24:(-24)));	
	
	lv_anim_start(&buttom_dot_anmation);
}

static void playback_buttom_aimation_exec_callback(void * obj, lv_anim_value_t val)
{
	lv_obj_set_x((lv_obj_t *) obj, val);
	if(val == buttom_dot_anmation.end)
	{
		if((val == 0)&&(playback_pview_item < (playback_pview_total - 6)))
		{
			playback_buttom_aimmation_play(true);
		}
		else if((val == 48)&&(playback_pview_item > 5))
		{
			playback_buttom_aimmation_play(false);
		}
		else
		{
			playback_dot_annimation_display();
		}
	}
}


static void playback_buttom_animation_create(lv_obj_t* obj)
{
	 
     lv_anim_init(&buttom_dot_anmation);
 
     /* 必选设置
      *------------------*/
 
    /* 设置“动画制作”功能 */
    lv_anim_set_exec_cb(&buttom_dot_anmation, (lv_anim_exec_xcb_t) playback_buttom_aimation_exec_callback);

	
    /* 设置“动画制作”功能 */
    lv_anim_set_var(&buttom_dot_anmation, obj);

    /* 动画时长[ms] */
    lv_anim_set_time(&buttom_dot_anmation, 400);

    /* 设置开始和结束值。例如。 0、150 */
   // lv_anim_set_values(&buttom_dot_anmation, 0, 24);


	/* 可选设置
    *------------------*/

   /* 开始动画之前的等待时间[ms] */
    //lv_anim_set_delay(&a, delay);

   /* 设置路径（曲线）。默认为线性 */
   // lv_anim_set_path(&a, &path);

   /* 设置一个回调以在动画准备好时调用。 */
   //lv_anim_set_ready_cb(&buttom_dot_anmation, playback_buttom_aimation_ready_callback);

    /* 设置在动画开始时（延迟后）调用的回调。 */
  //  lv_anim_set_start_cb(&a, start_cb);

    /* 在此持续时间内，也向后播放动画。默认值为0（禁用）[ms] */
  //  lv_anim_set_playback_time(&buttom_dot_anmation, 100);

    /* 播放前延迟。默认值为0（禁用）[ms] */
  //  lv_anim_set_playback_delay(&a, wait_time);

    /* 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复 */
   // lv_anim_set_repeat_count(&a, wait_time);

    /* 重复之前要延迟。默认值为0（禁用）[ms] */
  //  lv_anim_set_repeat_delay(&a, wait_time);

    /* true（默认）：立即应用开始值，false：延迟设置动画后再应用开始值。真正开始。 */
  //  lv_anim_set_early_apply(&a, true);
}




static void playback_buttom_annimation_dot_create(void)
{
	lv_obj_t* cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, 72, 12);
	lv_obj_set_pos(cont,474,544);
	lv_obj_set_id(cont, 100);
	lv_obj_t* obj2= lv_obj_create(cont, NULL);
	
	lv_obj_set_id(obj2, 1);
	lv_obj_set_size(obj2, 12, 12);
	lv_obj_set_style_local_bg_opa(obj2,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);	
	lv_obj_set_style_local_radius(obj2,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);	
	lv_obj_set_style_local_bg_color(obj2,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x42,0x45,0x42));

	
	lv_obj_t* obj3= lv_obj_create(cont, NULL);
	
	lv_obj_set_id(obj3, 2);
	lv_obj_set_size(obj3, 12, 12);
	lv_obj_set_style_local_bg_opa(obj3,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);	
	lv_obj_set_style_local_radius(obj3,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);	
	lv_obj_set_style_local_bg_color(obj3,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x42,0x45,0x42));


	lv_obj_t* obj1= lv_obj_create(cont, NULL);
	lv_obj_set_id(obj1, 0);
	lv_obj_set_size(obj1, 24, 10);
	lv_obj_set_style_local_bg_opa(obj1,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);	
	lv_obj_set_style_local_radius(obj1,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);	
	lv_obj_set_style_local_bg_color(obj1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xC4,0xC4,0xC4));

	playback_dot_annimation_display();


	playback_buttom_animation_create(obj1);

	
}





static void playback_thumb_select_display(void)
{
	if((playback_pview_select != -1)&&(playback_pview_select < playback_pview_total)&&((playback_pview_item-playback_pview_select) < 6))
	{
		int index = playback_pview_item - playback_pview_select;
		lv_obj_set_state(playback_thumb_win_cont[index], LV_STATE_FOCUSED);//设置容器为聚焦状态
	}
}


static void LAYOUT_ENETER_FUNC(playback)
{
	lock_falg = 0;
	memset(del_file_index,0,sizeof(del_file_index));
	layout_playback_quit_mask_set(0x01);

	system_bg_fill_color(0x00,0,0,1024,600);

	playback_buttom_annimation_dot_create();//底部动态点创建

	

	playback_thumb_head_label_create();//标题初始化
	playback_thumb_win_cont_create();//创建显示照片的容器窗体
	playback_total_page_label_create();//创建标题数量
	
	playback_cancel_btn_create();//返回按钮

	playback_lock_btn_create();//上锁按钮
	playback_delete_btn_create();//删除全部按钮
	if( playback_pview_total > 0)//有照片
	{	
		extern bool video_decode_wait_thread_quit(void);
		video_decode_wait_thread_quit();
		media_thumb_device_open(1920, 1088);//照片数据解码
	}

	playback_thumb_dsiplay();//对照片容器的显示 


	if(playback_pview_total > 6)
	{
		playback_left_page_btn_create();//上一页
		playback_right_page_btn_create();//下一页
	}

	

	playback_thumb_select_display();//选中


	sdcard_event_register(playback_sd_status_change_callback);

}

static void LAYOUT_QUIT_FUNC(playback)
{
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);	
	sdcard_event_register(NULL);
	if(layout_playback_quit_mask_get())
	{
		media_thumb_device_close();//关闭
	}

}

CREATE_LAYOUT(playback);


/*
* arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
* arg2:arg1= 1,arg2参数无意义
*	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
*	   arg1=3,arg2=1:内存正常，2，内存已满
*/
 void playback_sd_status_change_callback(unsigned long arg1, unsigned long arg2)
{
    if (arg1 == 1)
    {	
        goto_layout(pLAYOUT(home));
    }
}


