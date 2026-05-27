#include "stdbool.h"
#include "layout_define.h"
static MON_QUIT_FLG layout_monitor_quit_flag = MON_QUIT_NONE;
/**
 * 获取退出监控的标志位
 * 
 * @return 当前的退出监控标志位状态
 * 
 * 功能说明：
 * - 返回layout_monitor_quit_flag的当前值
 * - 用于外部模块查询是否应该退出监控状态
 */
MON_QUIT_FLG monitor_quit_mask_get(void)
{
    return layout_monitor_quit_flag;
}

/**
 * 设置退出监控的标志位
 * 
 * @param flg 要设置的退出监控标志位值
 * 
 * 功能说明：
 * - 设置layout_monitor_quit_flag为指定值
 * - 用于外部模块通知监控系统退出监控状态
 */
void monitor_quit_mask_set(MON_QUIT_FLG flg)
{
    layout_monitor_quit_flag = flg;
}

/** 布局监控进入标志（静态全局变量） */
static MON_ENTER_FLG layout_monitor_enter_flag = MON_ENTER_NONE;

/**
 * 设置监控进入标志位
 * 
 * @param flg 要设置的进入标志位值
 * 
 * 功能说明：
 * - 设置layout_monitor_enter_flag为指定值
 * - 用于控制监控系统的进入状态
 */
void monitor_enter_flag_set(MON_ENTER_FLG flg)
{
    layout_monitor_enter_flag = flg;
}

/**
 * 获取监控进入标志位
 * 
 * @return 当前的进入标志位状态
 * 
 * 功能说明：
 * - 返回layout_monitor_enter_flag的当前值
 * - 用于查询监控系统的进入状态
 */
MON_ENTER_FLG monitor_enter_flag_get(void)
{
    return layout_monitor_enter_flag;
}

/** 底部导航点动画对象（静态全局变量） */
static lv_anim_t buttom_dot_anmation;

/** 当前选中点索引指针（静态全局变量） */
static int *dot_cur_index;

/** 导航点总数指针（静态全局变量） */
static int *dot_total;

/**
 * 初始化导航点动画参数
 * 
 * @param cur_index 当前选中点索引的指针
 * @param total 导航点总数的指针
 * 
 * 功能说明：
 * - 保存当前选中点索引和导航点总数的指针
 * - 用于后续动画计算中动态获取当前状态
 */
void dot_animation_param_init(int *cur_index, int *total)
{
    dot_cur_index = cur_index;
    dot_total = total;
}

/**
 * 更新底部导航点的显示位置
 * 
 * 功能说明：
 * - 根据当前选中点索引和导航点总数动态调整导航点位置
 * - 支持1-3个导航点的显示布局
 * - 自动隐藏不需要显示的导航点
 * 
 * 布局逻辑：
 * - 当导航点总数为1时：只显示第一个点，居中显示
 * - 当导航点总数为2时：显示两个点，根据当前选中点调整位置
 * - 当导航点总数为3时：显示三个点，根据当前选中点调整位置
 */
static void dot_annimation_display(void)
{
    // 获取父容器对象（ID=100）
    lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), 100);
    if (parent != NULL)
    {
        // 获取三个导航点对象（ID=0,1,2）
        lv_obj_t* obj_child1 = lv_obj_get_child_form_id(parent, 0);
        lv_obj_t* obj_child2 = lv_obj_get_child_form_id(parent, 1);
        lv_obj_t* obj_child3 = lv_obj_get_child_form_id(parent, 2);
        
        // 安全检查：确保三个导航点对象都存在
        if ((obj_child1 == NULL) || (obj_child2 == NULL) || (obj_child3 == NULL))
        {
            return;
        }

        // 根据导航点总数和当前选中点索引调整布局
        if (*dot_total <= 1)
        {
            // 只有一个导航点时：隐藏其他点，显示第一个点并居中
            if (obj_child2 != NULL)
                lv_obj_set_hidden(obj_child2, true);
            if (obj_child3 != NULL)
                lv_obj_set_hidden(obj_child3, true);
            if (obj_child1 != NULL)
                lv_obj_set_hidden(obj_child1, false);
            lv_obj_set_pos(obj_child1, 24, 1);
        }
        else if (*dot_total <= 2)
        {
            // 有两个导航点时：隐藏第三个点，根据当前选中点调整前两个点的位置
            if (obj_child3 != NULL)
                lv_obj_set_hidden(obj_child3, true);
                
            if (*dot_cur_index < 6)
            {
                // 当前选中第一个点附近：第一个点在右，第二个点在左
                lv_obj_set_pos(obj_child2, 0, 0);
                lv_obj_set_pos(obj_child1, 24, 1);
            }
            else
            {
                // 当前选中第二个点附近：第一个点在左，第二个点在右
                lv_obj_set_pos(obj_child2, 36, 0);
                lv_obj_set_pos(obj_child1, 0, 1);
            }
        }
        else if (*dot_cur_index < 1)
        {
            // 当前选中第一个点：三个点依次排列，第一个点在最左
            lv_obj_set_pos(obj_child1, 0, 1);
            lv_obj_set_pos(obj_child2, 36, 0);
            lv_obj_set_pos(obj_child3, 60, 0);
        }
        else if (*dot_cur_index == (*dot_total - 1))
        {
            // 当前选中最后一个点：三个点依次排列，第一个点在最右
            lv_obj_set_pos(obj_child2, 0, 0);
            lv_obj_set_pos(obj_child3, 24, 0);
            lv_obj_set_pos(obj_child1, 48, 0);
        }
        else
        {
            // 当前选中中间点：三个点依次排列，第一个点在中间
            lv_obj_set_pos(obj_child1, 24, 1);
            lv_obj_set_pos(obj_child2, 0, 0);
            lv_obj_set_pos(obj_child3, 60, 0);
        }
    }
}

/**
 * 播放底部导航点动画
 * 
 * @param is_add 动画方向：true为向右移动，false为向左移动
 * 
 * 功能说明：
 * - 根据指定方向播放底部导航点的动画效果
 * - 动画基于当前位置进行偏移（±24像素）
 * - 启动动画后会触发buttom_aimmation_exec_callback回调
 */
void buttom_aimmation_play(bool is_add)
{	
    // 获取动画关联的对象
    lv_obj_t* obj = (lv_obj_t*)buttom_dot_anmation.var;
    
    // 获取当前对象的X坐标作为动画起始值
    int vol_base = lv_obj_get_x(obj);

    // 设置动画的起始值和结束值（向右+24像素，向左-24像素）
    lv_anim_set_values(&buttom_dot_anmation, vol_base, vol_base + (is_add ? 24 : (-24)));	
	
    // 启动动画
    lv_anim_start(&buttom_dot_anmation);
}

static void buttom_aimation_exec_callback(void * obj, lv_anim_value_t val)
{
	lv_obj_set_x((lv_obj_t *) obj, val);
	if(val == buttom_dot_anmation.end)
	{
		if((val == 0)&&( *dot_cur_index < (*dot_total - 6)))
		{
			buttom_aimmation_play(true);
		}
		else if((val == 48)&&( *dot_cur_index > 5))
		{
			buttom_aimmation_play(false);
		}
		else
		{
			dot_annimation_display();
		}
	}
}

/**
 * 底部导航点动画执行回调函数
 * 
 * @param obj 动画关联的对象
 * @param val 当前动画值（X坐标）
 * 
 * 功能说明：
 * - 更新导航点的X坐标位置
 * - 当动画到达终点时，根据条件决定：
 *   1. 继续向同一方向播放动画（级联动画）
 *   2. 调用dot_annimation_display更新导航点布局
 */
static void buttom_aimmation_exec_callback(void * obj, lv_anim_value_t val)
{
    // 更新对象的X坐标
    lv_obj_set_x((lv_obj_t *) obj, val);
    
    // 检查动画是否到达终点
    if (val == buttom_dot_anmation.end)
    {
        // 当动画到达最左位置(0)且当前索引较小时，继续向右播放动画
        if ((val == 0) && (*dot_cur_index < (*dot_total - 6)))
        {
            buttom_aimmation_play(true);
        }
        // 当动画到达最右位置(48)且当前索引较大时，继续向左播放动画
        else if ((val == 48) && (*dot_cur_index > 5))
        {
            buttom_aimmation_play(false);
        }
        // 其他情况：动画结束，更新导航点布局
        else
        {
            dot_annimation_display();
        }
    }
}

static void buttom_animation_create(lv_obj_t* obj)
{
	 
     lv_anim_init(&buttom_dot_anmation);
 
     /* 必选设置
      *------------------*/
 
    /* 设置“动画制作”功能 */
     lv_anim_set_exec_cb(&buttom_dot_anmation, (lv_anim_exec_xcb_t) buttom_aimation_exec_callback);

	
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
   //lv_anim_set_ready_cb(&buttom_dot_anmation, buttom_aimation_ready_callback);

    /* 设置在动画开始时（延迟后）调用的回调。 */
  //  lv_anim_set_start_cb(&a, start_cb);

    /* 在此持续时间内，也向后播放动画。默认值为0（禁用）[ms] */
  //  lv_anim_set_time(&buttom_dot_anmation, 100);

    /* 播放前延迟。默认值为0（禁用）[ms] */
  //  lv_anim_set_delay(&a, wait_time);

    /* 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复 */
   // lv_anim_set_repeat_count(&a, wait_time);

    /* 重复之前要延迟。默认值为0（禁用）[ms] */
  //  lv_anim_set_repeat_delay(&a, wait_time);

    /* true（默认）：立即应用开始值，false：延迟设置动画后再应用开始值。真正开始。 */
  //  lv_anim_set_early_apply(&a, true);
}


void buttom_annimation_dot_create(void)
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

	dot_annimation_display();

	buttom_animation_create(obj1);
}

/************************************************************
** 函数说明: 自定义文件保存设置显示
** 作者: xiaoxiao
** 日期: 2023-03-28 11:49:16
** 参数说明: 
** 注意事项: 
************************************************************/
void layout_customiz_file_set_display_create(int bg_id,layout_lang_id langid,bool preload,bool normal)
{
	lv_obj_t *cont_bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont_bg,bg_id);
	lv_obj_set_size(cont_bg,1024,600);
	lv_obj_set_pos(cont_bg,0,0);
	lv_obj_set_style_local_bg_opa(cont_bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	
	lv_obj_t *cont = lv_cont_create(cont_bg, NULL);
	lv_obj_set_size(cont,200,200);
	lv_obj_set_size(cont,350,200);
	lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, normal ? lv_color_hex(0xffffff) : LV_COLOR_RED);
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(cont,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);


	lv_obj_t * label = lv_label_create(cont,NULL);	
	lv_label_set_text(label,str_get(langid));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_auto_realign(label,true);
	lv_obj_set_style_local_text_color(label, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
	lv_obj_set_style_local_text_font(label, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, normal?-40:0);
	// lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	// lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
	// lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	// lv_obj_set_style_local_value
	// lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, src[user_data_get()->user_language]);
	// lv_obj_set_auto_realign(cont,true);
	// lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -50);

    if(normal)
    {
        lv_obj_t *preload = lv_spinner_create(cont, NULL);
        lv_obj_set_size(preload, 70, 70);
        lv_obj_align(preload, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0XFF, 0XFF, 0XFF));
        lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0X00, 0X00, 0X00));
        lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, 4);
        lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, 4);
    }

}
