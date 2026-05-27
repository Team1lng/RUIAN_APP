#include "layout_define.h"
int setting_password_edit_index_get(lv_obj_t *parent);

bool setting_password_input_reset(lv_obj_t *parent, int edit_max);

lv_obj_t *setting_passowrd_num_keyboard_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *click_data);


bool setting_password_get_string(lv_obj_t *parent, char *buffer);


bool setting_password_del_string(lv_obj_t *parent, int max_edit);

bool setting_password_input_string(lv_obj_t *parent, const char *string, int max_edit, bool passwd_mode);

bool setting_password_input_label_create(lv_obj_t *parent, int x, int y, int row);


lv_obj_t * dialog_msg_cont_creat();

lv_obj_t * network_init_reset_func(void);