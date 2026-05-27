#include "user_data.h"
#include "file_api.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "stdlib.h"
#include "ak_common.h"
#include "layout_define.h"
#include "layout_background.h"

// #define USER_DATA_PATH "/etc/config/user_data.cfg"
#define USER_DATA_PATH "/app/data/user_data.cfg"
extern void CloseConnectingWifi(void);

static user_data_info user_data = {};

static user_data_info user_data_default = {

	.super_password = {"8025"},
	.system_mute = false,

	.user_language = language_english, // language_english

	.motion.enable = true,
	.motion.record_flag = 0,
	.motion.record = 0,
	.motion.motion = false,
	.auto_record_mode = 0,

	.audio.key_sound = true,
	.audio.door1_ring = 0,
	.audio.door2_ring = 1,

	.audio.outdoor_ring_val = 25,
	.audio.door_ring_val = 50,
	.audio.intercom_ring_val = 50,

	.audio.outdoor_talk_val = 100,

	.audio.door_talk_val = 50,

	.audio.intercom_talk_val = 50,

	.alarm.auto_record = false,
	.alarm.alarm_1_enable = false,
	.alarm.alarm_1_trigger = false,
	.alarm.alarm_2_enable = false,
	.alarm.alarm_2_trigger = false,

	.other.network_device = 1, // 1-4表示室内机ID1-ID4  5-6表示DOOR1-DOOR2, 7-8表示CCTV1-CCTV2
	.other.family_id = 1,
	.other.screen_saver = 0, // 0 off,1:clock
	.other.mode = 1,		 // 0:休眠模式 1:居家模式 2:离家模式

	.other.password = {"1234"},

	.wifi.wifi_open_flag = true,
	.wifi.wifi_connect_flag = false,

	.door1_delay = 2,
	.door2_delay = 2,
	.ring_time = 10,
	.onvif_dev_count = 0,
	.cctv_brand_Index = 0,
	.cctv_connect_mode = 0,
	.tuya_ch_name = {},
	.floor = {0},
	.nobody_message = true,
};

#define user_data_check_range_out(cur, min, max)                                   \
	if ((user_##data.cur < min) || (user_##data.cur > max))                        \
	{                                                                              \
		printf("########user data error %d(%d,%d) \n", user_##data.cur, min, max); \
		user_##data.cur = user_##data##_default.cur;                               \
	}

#define user_data_check_language_range_out(cur, min, max, error1, error2)                                                     \
	if (((user_##data.cur < min) || (user_##data.cur > max)) || ((user_##data.cur == error1) && (user_##data.cur == error2))) \
	{                                                                                                                         \
		printf("++++++++++++++=user data error %d(%d,%d,%d,%d) \n", user_##data.cur, min, max, error1, error2);               \
		user_##data.cur = user_##data##_default.cur;                                                                          \
	}

static void user_data_check_valid(void)
{
	user_data_check_range_out(user_language, 0, 14);
	user_data_check_range_out(motion.enable, 0, 1);
	user_data_check_range_out(motion.record_flag, 0, 1);
	user_data_check_range_out(motion.record, 0, 2);
	user_data_check_range_out(motion.motion, 0, 1);
	user_data_check_range_out(motion.enable, 0, 1);
	user_data_check_range_out(auto_record_mode, 0, 1);
	user_data_check_range_out(audio.door1_ring, 0, 6);
	user_data_check_range_out(audio.door2_ring, 0, 6);
	user_data_check_range_out(audio.key_sound, 0, 1);
	user_data_check_range_out(audio.outdoor_ring_val, 5, 35);
	user_data_check_range_out(audio.intercom_ring_val, 0, 100);
	user_data_check_range_out(audio.outdoor_talk_val, 0, 100);
	user_data_check_range_out(audio.door_talk_val, 0, 100);
	user_data_check_range_out(audio.intercom_talk_val, 0, 100);
	user_data_check_range_out(alarm.auto_record, 0, 1);
	user_data_check_range_out(alarm.alarm_1_enable, 0, 1);
	user_data_check_range_out(alarm.alarm_2_enable, 0, 1);
	user_data_check_range_out(alarm.alarm_1_trigger, 0, 1);
	user_data_check_range_out(alarm.alarm_2_trigger, 0, 1);

	user_data_check_range_out(other.network_device, 1, 6);
	user_data_check_range_out(other.family_id, 1, 9999);
	user_data_check_range_out(other.screen_saver, 0, 1);
	user_data_check_range_out(other.mode, 0, 2);

	user_data_check_range_out(other.password[0], '0', '9');
	user_data_check_range_out(other.password[1], '0', '9');
	user_data_check_range_out(other.password[2], '0', '9');
	user_data_check_range_out(other.password[3], '0', '9');

	user_data_check_range_out(super_password[0], '8', '8');
	user_data_check_range_out(super_password[1], '0', '0');
	user_data_check_range_out(super_password[2], '2', '2');
	user_data_check_range_out(super_password[3], '5', '5');

	user_data_check_range_out(wifi.wifi_open_flag, 0, 1);
	user_data_check_range_out(wifi.wifi_connect_flag, 0, 1);
	user_data_check_range_out(door1_delay, 1, 25);
	user_data_check_range_out(door2_delay, 1, 25);
	user_data_check_range_out(ring_time, 10, 60);
	user_data_check_range_out(nobody_message, 0, 1);
}

bool user_data_save(void)
{
	int fd = open(USER_DATA_PATH, O_WRONLY | O_CREAT);
	if (fd < 0)
	{
		printf("write open %s fail \n", USER_DATA_PATH);
		return false;
	}

	write(fd, &user_data, sizeof(user_data_info));

	close(fd);
	system("sync");
	return true;
}

bool user_data_init(void)
{
	int fd = open(USER_DATA_PATH, O_RDONLY);
	if (fd < 0)
	{
		printf("read open %s fail \n", USER_DATA_PATH);
		user_data = user_data_default;
		return false;
	}

	read(fd, &user_data, sizeof(user_data_info));
	close(fd);
	user_data_check_valid();
	if ((user_data.other.network_device < 1) || (user_data.other.network_device > 6))
	{
		user_data.other.network_device = 1;
	}
	return true;
}

user_data_info *user_data_get(void)
{
	return &user_data;
}

// 恢复默认壁纸
static void reset_background()
{
	char picture_name[256] = {0};
	//sprintf(picture_name, BACKGROUND_FILE_PATH "system_bg.jpg");
	snprintf(picture_name, sizeof(picture_name), "%ssystem_bg.jpg", BACKGROUND_FILE_PATH);
	char cmd[128] = {0};
	if (access(BACKGROUND_FILE_PATH, F_OK) != 0)
	{
		system("mkdir " BACKGROUND_FILE_PATH);
	}
	system("rm -rf " BACKGROUND_FILE_PATH "frame.jpg");
	snprintf(cmd, sizeof(cmd), "cp -v %s %sframe.jpg", picture_name, BACKGROUND_FILE_PATH);
	printf("cmd is %s\n", cmd);
	system(cmd);
}

void user_data_reset(void)
{
	uint8_t language = user_data_get()->user_language;
	system("rm -rf " USER_DATA_PATH);
	system("sync");
	user_data = user_data_default;
	user_data.user_language = language;
	start_delete_media(DELETE_ALL_FLASH_PHOTO);
	// 删除自定义铃声
	if (access(LOCAL_RING7_FILE_PATH, F_OK) == 0)
	{
		system("rm " LOCAL_RING7_FILE_PATH);
	}
	// 恢复默认壁纸
	reset_background();

	while (delete_media_status())
	{
		ak_sleep_ms(10);
	}

	// 忘记wifi密码
	reset_wifi_password();

	user_data_save();

	//	system("rm -rf /etc/config/tuya_key/");
	system("rm -rf /app/data/tuya_user.db");
	system("rm -rf /app/data/tuya_user.db_bak");
	system("rm -rf /app/data/tuya_enckey.db");
	system("rm -rf /app/data/log_seq_stat");
	system("sync");

	usleep(1000 * 1000);
	// exit(-1);
	system("reboot");
}
