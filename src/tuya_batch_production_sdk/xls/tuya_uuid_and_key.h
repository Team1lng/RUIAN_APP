#ifndef TUYA_UUID_AND_KEY_H_
#define TUYA_UUID_AND_KEY_H_
#include <stdbool.h>

typedef struct{
	char tuya_uuid[25];
	char tuya_key[32];
	int index;
	bool used;
	bool lock_id;
}tuya_conf_info;


typedef enum{
    TUYA_FILE_NOT,
    TUYA_ID_NO_FIND,
	TUYA_FILE_FORMAT_ERROR,
    TUYA_ACTION_OK,
}TUYA_FILE_ERROR;

bool tuya_uuid_etc_exist_check(void);
/***
**   日期:2022-06-28 16:25:56
**   修改者: link.wu
**   函数作用：判断是否有涂鸦uuid文档并获取信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_key_and_uuid_check(char *uuid,char *key);
/***
**   日期:2022-05-27 09:48:37
**   作者: leo.liu
**   函数作用：从xls中读取读取指定的文档
**   参数说明:
***/
bool tuya_uuid_and_key_swtch(char *uuid,char *key);

bool tuya_conf_uuid_etc_read(tuya_conf_info * info_etc);

bool tuya_uuid_and_key_copy_create(char *uuid,char *key);
#endif