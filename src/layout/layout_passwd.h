typedef enum{
    PASSWD_CH_NONE,
    PASSWD_CH_SETTING_FLOOR,
    PASSWD_CH_MODIFY_PASSWD,
    PASSWD_CH_CCTV_INFORMATION,

}PASSWD_CH;
void enter_layout_passwd_ch(PASSWD_CH ch);

PASSWD_CH get_layout_passwd_ch();
