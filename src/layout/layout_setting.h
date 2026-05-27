/***
 * 设置按键的状态
***/
void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
/***
 * 设置进入设置界面应该进入哪一个页面
***/
void set_enter_setting_page_which(int which);

/***
 * 获取进入设置界面应该进入哪一个页面
***/
int get_enter_setting_page_which();

/***
 * 获取当前室内机与二次确认机交互的状态
***/
void device_confirm_status_set(int status);

/***
 * 设置当前室内机与二次确认机交互的状态
***/
int device_confirm_status_get();

/*******
设置系统时间
********/
void setting_rtc_time_set(struct tm* date);

/*******
获取系统时间
********/
void setting_rtc_time_get(struct tm* date);

