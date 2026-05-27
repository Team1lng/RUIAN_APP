#include "language.h"
typedef enum
{
	MON_QUIT_NONE = 0,
	MON_QUIT_MANUAL_DOOR,
	MON_QUIT_DOOR_CALL,
	MON_QUIT_DOOR_TALK,
} MON_QUIT_FLG;

typedef enum
{
	MON_ENTER_NONE,
	MON_ENTER_MANUAL_DOOR,
	MON_ENTER_CALL,
	MON_ENTER_TALK,
	MON_ENTER_TUYA,
	MON_ENTER_DISPLAY,
} MON_ENTER_FLG;

MON_QUIT_FLG monitor_quit_mask_get(void);

void monitor_quit_mask_set(MON_QUIT_FLG flg);

void monitor_enter_flag_set(MON_ENTER_FLG flg);

MON_ENTER_FLG monitor_enter_flag_get(void);

void dot_animation_param_init(int *cur_index,int *total);

void buttom_annimation_dot_create(void);

void buttom_aimmation_play(bool is_add);

/************************************************************
** 函数说明: 自定义文件保存设置显示
** 作者: xiaoxiao
** 日期: 2023-03-28 11:49:16
** 参数说明: 
** 注意事项: 
************************************************************/
void layout_customiz_file_set_display_create(int bg_id,layout_lang_id langid,bool preload,bool normal);