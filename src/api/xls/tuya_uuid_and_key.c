#include "tuya_uuid_and_key.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
//#include "user_common.h"

#include <unistd.h>
  
#include <stdlib.h>  



#include<fcntl.h>

#include "xls.h"

/*****  文件名格式为:xxxx-xx-xx.conf *****/
#define TUYA_UUID_AND_KEY_CONF_PATH "/app/tuya/tuya_key/"
/*****  文件名格式为”xxxx-xx-xx.xls“ *****/
#define TUYA_UUID_AND_KEY_XLS_PATH "/mnt/tf/"
/*****  tuya配置文档的文件名 *****/
static char tuya_uuid_and_key_find_file[128] = {0};
/*****  标志tuya id已经注册 *****/
static bool is_tuya_uuid_and_kay_register = false;

/***
**   日期:2022-05-27 08:19:56
**   作者: leo.liu
**   函数作用：判断tuya密钥是否存在
**   参数说明:
***/
static bool tuya_uuid_and_key_exist_check(void)
{
	struct dirent *pdirent;
	struct tm tm;
	int find_count = 0;
	int index = 0;
	bool tuya_czech_flag = false; //涂鸦定制版命名和通用版命名。true代表定制版 false代表通用版命名
	
	is_tuya_uuid_and_kay_register = false;
	memset(tuya_uuid_and_key_find_file, 0, sizeof(tuya_uuid_and_key_find_file));
	DIR *d_info = opendir(TUYA_UUID_AND_KEY_CONF_PATH);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			// if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".conf")) && (strlen(pdirent->d_name) == 15))
			// {
			// 	printf("find %s\n", pdirent->d_name);
			// 	sscanf(pdirent->d_name, "%04d-%02d-%02d.conf", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
			// 	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
			// 	{
			// 		find_count++;
			// 	}
			// }
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".conf")))
			{
				if(strlen(pdirent->d_name) == 15){
					printf("find %s\n", pdirent->d_name);
					sscanf(pdirent->d_name, "%04d-%02d-%02d.conf", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
					if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
					{
						find_count++;
						tuya_czech_flag = false;
					}
				}else if ((strlen(pdirent->d_name) == 21))
				{
					printf("find %s\n", pdirent->d_name);
					sscanf(pdirent->d_name, "%04d-%02d-%02d-%05d.conf", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &index);
					if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
					{
						find_count++;
						tuya_czech_flag = true;
					}
				}
	
			}
		}
		closedir(d_info);
	}
	bool result = false;
		printf("\n=======find_count=>%d========\n",find_count);
	if (find_count > 0)
	{
		is_tuya_uuid_and_kay_register = true;
		result = true;
		if (find_count == 1)
		{
			if(tuya_czech_flag){
				sprintf(tuya_uuid_and_key_find_file, "%04d-%02d-%02d-%05d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday,index);
			}else{
				sprintf(tuya_uuid_and_key_find_file, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
			}
			printf("read tuya config:%s \n", tuya_uuid_and_key_find_file);
		}
	}
	return result;
}

/***
**   日期:2022-05-27 08:13:17
**   作者: leo.liu
**   函数作用：遍历文件是否存在
**   参数说明:
***/
static bool tuya_key_xls_exist_check(void)
{
	if (is_tuya_uuid_and_kay_register == true)
	{
		return true;
	}
	struct tm tm;
	struct dirent *pdirent;
	int find_count = 0;
	DIR *d_info = opendir(TUYA_UUID_AND_KEY_XLS_PATH);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".xls")) && (strlen(pdirent->d_name) == 14))
			{
				sscanf(pdirent->d_name, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
				if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
				{
					find_count++;
				}
			}
		}
		closedir(d_info);
	}
	bool result = false;
	if (find_count == 1)
	{
		result = true;
		memset(tuya_uuid_and_key_find_file, 0, sizeof(tuya_uuid_and_key_find_file));
		sprintf(tuya_uuid_and_key_find_file, "%04d-%02d-%02d.xls", tm.tm_year, tm.tm_mon, tm.tm_mday);
		printf("find tuya xls%s%s \n", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_find_file);
	}
	return result;
}

/***
**   日期:2022-05-27 08:11:56
**   作者: leo.liu
**   函数作用：判断是否有tuya文档
**   参数说明:1:可以进入系统，0:需要重新注册tuya key和uuid -1:直接
***/
int tuya_key_and_uuid_init(bool production)
{
	if(production)
	{
		if (tuya_uuid_and_key_exist_check() == true)
		{
			return 1;
		}
		return tuya_key_xls_exist_check() == true ? 0 : -1;	
	}
	else if(tuya_key_xls_exist_check())
	{
		return 0;
	}
	return tuya_uuid_and_key_exist_check() == true ? 1 : -1;	
}

/***
**   日期:2022-05-27 16:10:13
**   作者: leo.liu
**   函数作用：判断头文件是否满足格式
**   参数说明:
***/
static bool tuya_xls_head_valid_check(const xlsWorkSheet *sheet)
{
	printf("lastcol = %d \n", sheet->rows.lastcol);
	if (sheet->rows.lastcol < 1)
	{
		return false;
	}

	struct st_row_data *row = &sheet->rows.row[0];
	unsigned char *row_str = row->cells.cell[0].str;
	if ((row_str == NULL) || (strcmp((char *)row_str, "uuid") != 0))
	{
		printf("uuid:[%s]\n",row_str);
		return false;
	}
	row_str = row->cells.cell[1].str;
	if ((row_str == NULL) || (strcmp((char *)row_str, "key") != 0))
	{
		printf("key:[%s]\n",row_str);
		return false;
	}
	return true;
}

static bool tuya_conf_uuid_key_save(const unsigned char *uuid, const unsigned char *key, int index)
{
	struct tm tm;
	char conf_path[128] = {0};
	sscanf(tuya_uuid_and_key_find_file, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
	{
		if (access(TUYA_UUID_AND_KEY_CONF_PATH, F_OK))
		{
			system("mkdir " TUYA_UUID_AND_KEY_CONF_PATH);
		}
		else
		{
			system("rm -rf " TUYA_UUID_AND_KEY_CONF_PATH"*");//删除旧的涂鸦uid文件
		}
		memset(tuya_uuid_and_key_find_file, 0, sizeof(tuya_uuid_and_key_find_file));
		sprintf(tuya_uuid_and_key_find_file, "%04d-%02d-%02d-%05d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday,index);
		sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_CONF_PATH, tuya_uuid_and_key_find_file);
		int fd = open(conf_path, O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			printf("open %s failed \n", conf_path);
			return false;
		}

		char buffer[1024] = {0};
		memset(buffer, 0, sizeof(buffer));
		int len = sprintf(buffer, "%s %s", uuid, key);
		write(fd, buffer, len);
		printf("write uuid and key:%s \n",buffer);
		close(fd);
		return true;
	}
	return false;
}

/***
**   日期:2022-05-27 16:35:28
**   作者: leo.liu
**   函数作用：保存uuid 与 key
**   参数说明:
***/
static bool tuya_xls_uuid_key_read(const xlsWorkSheet *sheet, int index)
{
	struct st_row_data *row = &sheet->rows.row[index];
	unsigned char *uuid = row->cells.cell[0].str;
	if (uuid == NULL)
	{
		printf("read uuid is null \n");
		return false;
	}
	unsigned char *key = row->cells.cell[1].str;
	if (uuid == NULL)
	{
		printf("read uuid is null \n");
		return false;
	}
	if (tuya_conf_uuid_key_save(uuid, key,index) == false)
	{
		printf("save uuid and key to flash faild \n");
	}
	return true;
}

/***
**   日期:2022-05-27 09:48:37
**   作者: leo.liu
**   函数作用：从xls中读取读取指定的文档
**   参数说明:
***/
bool tuya_key_and_key_xls_register(int index)
{
	bool result = true;
	/*****  已经有注册的uuid文档 *****/
	if (is_tuya_uuid_and_kay_register == true)
	{
		return false;
	}
	char xls_path[128] = {0};
	sprintf(xls_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_find_file);
	xlsWorkBook *pxls_xls = xls_open(xls_path, "UTF-8");
	if (pxls_xls == NULL)
	{
		printf("unable to open %s \n", xls_path);
		return false;
	}
	xlsWorkSheet *pxls_sheet = xls_getWorkSheet(pxls_xls, 0);
	xls_parseWorkSheet(pxls_sheet);

	/*****  索引从1开始 *****/
	printf("lastrow = %d \n", pxls_sheet->rows.lastrow);
	if ((index > pxls_sheet->rows.lastrow) || (index < 1))
	{
		result = false;
		printf("The entered index is greater than the maximum %d > %d\n", index, pxls_sheet->rows.lastrow);
		goto finish;
	}

	/*****  判断头文件 *****/
	if (tuya_xls_head_valid_check(pxls_sheet) == false)
	{
		printf("xls head valid failed \n");
		result = false;
		goto finish;
	}

	/*****  保存指定的tuyaid *****/
	if (tuya_xls_uuid_key_read(pxls_sheet, index) == false)
	{
		printf("xls save failed index:%d \n", index);
		result = false;
		goto finish;
	}
	is_tuya_uuid_and_kay_register = true;
finish:
	xls_close_WS(pxls_sheet);
	xls_close_WB(pxls_xls);
	return result;
}

/***
**   日期:2022-05-27 16:51:54
**   作者: leo.liu
**   函数作用：读取tuya的uuid和key
**   参数说明:
***/
bool tuya_uuid_and_key_read( char *uuid,  char *key)
{
	if ((uuid == NULL) || (key == NULL) || (is_tuya_uuid_and_kay_register == false))
	{
		return false;
	}
	char conf_path[128] = {0};
	sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_CONF_PATH, tuya_uuid_and_key_find_file);
	int fd = open(conf_path, O_RDONLY);
	if (fd < 0)
	{
		printf("open %s failed \n", conf_path);
		return false;
	}
	char buffer[1024] = {0};
	memset(buffer, 0, sizeof(buffer));
	read(fd, buffer, sizeof(buffer));
	close(fd);
	sscanf(buffer, "%s %s", uuid, key);
	printf("uuid:%s \n key:%s \n", uuid, key);
	return true;
}



/***
**   日期:2022-06-16 15:41:19
**   作者: leo.liu
**   函数作用：读取tuya序列号
**   参数说明:
***/
bool tuya_serial_number_get(char *serial)
{
	if (is_tuya_uuid_and_kay_register == false)
	{
		return false;
	}
	strcpy(serial, tuya_uuid_and_key_find_file);
	char *p = strchr(serial, '.');
	if (p == NULL)
	{
		return false;
	}
	p[0] = '\0';
	return true;
}