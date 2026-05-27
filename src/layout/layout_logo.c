#include "layout_define.h"
#include "layout_setting_common.h"
// #define BACKGROUND_FILE_PATH "/app/data/background/"
extern lv_obj_t *setting_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);
static int cur_picture_index = 0;
static media_type picture_file_type = FILE_TYPE_FLASH_LOGO;
static int picture_total;
static bool bg_img_set_flg = false;
char *logo_picture_name = NULL;
#define PICTURE_NAM_LENGTH 128
#define layout_bg_info_adj_id 0xa0
#define layout_bg_setting_tips_cont_adi_id 0x0a1
static lv_task_t* home_begin_ptask = NULL;
static lv_obj_t *logo = NULL;

static int background_parameter_init(void)
{

	picture_file_type = FILE_TYPE_FLASH_LOGO;
	cur_picture_index = 0;
	picture_total = 0;
	picture_total = media_file_total_get(picture_file_type, cur_picture_index);
    printf("========picture_total %d\n",picture_total);
	logo = lv_img_create(lv_scr_act(),NULL);
    printf("========p\n");

	if(logo == NULL)
	{
		perror("logo obj is null\n");
	}
	set_location(logo, 0,0, 430,53);
	lv_obj_align(logo,lv_scr_act(),LV_ALIGN_CENTER,0,0);
	return picture_total;
	
}


extern bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);
static void layout_bg_thumb_dsiplay(void)
{
    unsigned char * picture_data = picture_data_get();
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    fb_video_mode_enable(false);
	
    // if(cur_picture_index >= picture_total)
	// {
	// 	cur_picture_index = 0;
	// }
	media_info *info = media_info_get(picture_file_type,cur_picture_index);
	// printf("custom_music_play index :%d       name:%s\n\r",cur_picture_index,info->file_name);
	if (picture_data == NULL)
	{
		picture_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, 1024 * 600 * 4);
		printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__, 1024 * 600 * 4);
	}
    static rom_bin_info img = rom_bin_raw_get();
	rom_bin_raw_init(img, picture_data, 430, 55);
	bzero(logo_picture_name,sizeof(PICTURE_NAM_LENGTH));
	sprintf(logo_picture_name,"%s%s",FLASH_LOGO_PATH,info->file_name);
	lv_jpg_decode_data(logo_picture_name, &img, 430, 55); //jpg图片解码
	
	lv_obj_set_style_local_bg_opa(lv_scr_act(),LV_OBJ_PART_MAIN,LV_STATE_DEFAULT, LV_OPA_COVER); //设置背景透明度
	
	lv_obj_set_style_local_bg_color(lv_scr_act(),LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x000000));
	// lv_disp_set_bg_image(lv_disp_get_default(), &img);  //图片显示
	if(logo == NULL)
	{
		// printf("1123123123");
		perror("logo obj is null\n");
	}
	lv_img_set_src(logo, &img);
}




static void home_begin(void){
	cur_picture_index++;
    if(cur_picture_index >45)
	{
		cur_picture_index = 0;
	}
	 if(cur_picture_index  == 45)
	{
		lv_task_del(home_begin_ptask);
		home_begin_ptask = NULL;
		usleep(500*1000);
		goto_layout(pLAYOUT(home));
		return;
	}
	layout_bg_thumb_dsiplay();
}


static void LAYOUT_ENETER_FUNC(logo)
{
	bg_img_set_flg = false;
	logo_picture_name = (char *)malloc(PICTURE_NAM_LENGTH);
    background_parameter_init();
	// layout_bg_left_page_btn_create();//上一页
	// layout_bg_right_page_btn_create();//下一页
	layout_bg_thumb_dsiplay();
	home_begin_ptask = lv_task_create(home_begin, 80, LV_TASK_PRIO_MID, &clock);  
}


static void LAYOUT_QUIT_FUNC(logo)
{
	// printf("123");
    unsigned char * picture_data = picture_data_get();
	free(logo_picture_name);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    fb_video_mode_enable(false);
    static rom_bin_info img = rom_bin_raw_get();
	rom_bin_raw_init(img, picture_data, 1024, 600);

	lv_jpg_decode_data(SYSTEM_BG_FILE_PATH, &img, 1024, 600);
	// lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	// lv_disp_set_bg_image(lv_disp_get_default(), &img);

}

CREATE_LAYOUT(logo);