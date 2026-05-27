#include "tuya_uuid_and_key.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include"iniparser.h"
// #include "user_common.h"

#include "xls.h"

/*****  文件名格式为:xxxx-xx-xx.conf *****/
#define TUYA_UUID_CONF_ETC_PATH "/etc/config/tuya_key/"
/*****  文件名格式为”xxxx-xx-xx.xls“ *****/
#define TUYA_UUID_AND_KEY_XLS_PATH "/mnt/tf/tuya_key/"
/*****  tuya配置文档的文件名 *****/
static char tuya_uuid_and_key_mnt_xls[128] = {0};
static char tuya_uuid_and_key_mnt_ini[128] = {0};
static char tuya_uuid_and_key_etc_conf[128] = {0};
/*****  标志tuya id已经注册 *****/
static bool is_tuya_uuid_and_kay_register = false;

/***
**   日期:2022-06-30 08:19:56
**   修改者: link.wu
**   函数作用：判断flash中涂鸦.conf文件是否存在
**   参数说明:
***/
bool tuya_uuid_etc_exist_check(void)
{
	printf("%s ==============================>>>\n\r",__func__);
	struct dirent *pdirent;
	struct tm tm;
	int find_count = 0;
	is_tuya_uuid_and_kay_register = false;
	
	DIR *d_info = opendir(TUYA_UUID_CONF_ETC_PATH);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".conf")) && (strlen(pdirent->d_name) == 15))
			{
				printf("FIND ===========>>%s\n", pdirent->d_name);
				sscanf(pdirent->d_name, "%04d-%02d-%02d.conf", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
				if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
				{
					find_count++;
				}
			}
		}
	}
	else   
	{
		printf("open dir %s fail\n\r",TUYA_UUID_CONF_ETC_PATH);
	}
	bool result = false;
	
	if (find_count > 0)
	{
		is_tuya_uuid_and_kay_register = true;
		result = true;
		if (find_count == 1)
		{
			memset(tuya_uuid_and_key_etc_conf, 0, sizeof(tuya_uuid_and_key_etc_conf));
			sprintf(tuya_uuid_and_key_etc_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
			printf("read tuya config:%s \n", tuya_uuid_and_key_etc_conf);
		}
	}
	if (d_info)
	{
		closedir(d_info);
	}
	return result;
}
/***
**   日期:2022-06-30 08:19:56
**   修改者: link.wu
**   函数作用：判断SD卡中涂鸦.ini文件是否存在
**   参数说明:
***/
static bool tuya_uuid_mnt_exist_check(const char * dir)
{
	printf("%s ==============================>>>\n\r",__func__);
	struct dirent *pdirent;
	struct tm tm;
	int find_count = 0;
	is_tuya_uuid_and_kay_register = false;
	
	DIR *d_info = opendir(dir);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".ini")) && (strlen(pdirent->d_name) == 14))
			{
				printf("FIND ===========>>%s\n", pdirent->d_name);
				sscanf(pdirent->d_name, "%04d-%02d-%02d.ini", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
				if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
				{
					find_count++;
				}
			}
		}
	}
	else   
	{
		printf("open dir %s fail\n\r",dir);
	}
	bool result = false;
	
	if (find_count > 0)
	{
		is_tuya_uuid_and_kay_register = true;
		result = true;
		if (find_count == 1)
		{
			memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
			sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);
			printf("read tuya config:%s \n", tuya_uuid_and_key_mnt_ini);
		}
	}
	if (d_info)
	{
		closedir(d_info);
	}
	return result;
}

/***
**   日期:2022-06-30 08:13:17
**   修改者: link.wu
**   函数作用：遍历SD卡中涂鸦.xls文件是否存在
**   参数说明:
***/
static bool tuya_key_xls_exist_check(void)
{
	printf("%s ==============================>>>\n\r",__func__);
	// if (is_tuya_uuid_and_kay_register == true)
	// {
	// 	return true;
	// }
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
	}
	else
	{
		printf("open dir %s fail\n\r",TUYA_UUID_AND_KEY_XLS_PATH);
	}
	bool result = false;
	if (find_count == 1)
	{
		result = true;
		memset(tuya_uuid_and_key_mnt_xls, 0, sizeof(tuya_uuid_and_key_mnt_xls));
		sprintf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", tm.tm_year, tm.tm_mon, tm.tm_mday);
		printf("find tuya xls%s%s \n", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	}
	if (d_info)
	{
		closedir(d_info);
	}
	return result;
}

/***
**   日期:2022-06-28 16:25:56
**   修改者: link.wu
**   函数作用：判断是否有涂鸦.xls文档并获取信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_key_and_uuid_check(char *uuid,char *key)
{
	#if 0
	if (tuya_uuid_mnt_exist_check(TUYA_UUID_AND_KEY_XLS_PATH) == true)
	{
		return true;
	}
	return tuya_key_xls_exist_check() == true ? false : true;
	#else
	if(tuya_key_xls_exist_check() == true)
	{
		if(tuya_uuid_mnt_exist_check(TUYA_UUID_AND_KEY_XLS_PATH) == false)
		{
			if(tuya_uuid_and_key_copy_create(uuid,key) == false)
				return TUYA_FILE_FORMAT_ERROR;
		}
		else
		{
			if(tuya_uuid_and_key_swtch(uuid,key) == false)
			{
				printf("no find unused uuid....\n\n");
				return TUYA_ID_NO_FIND;
			}
		}
		return TUYA_ACTION_OK;
	}
	return TUYA_FILE_NOT;
	#endif
}


/***
**   日期:2022-05-27 16:10:13
**   作者: leo.liu
**   函数作用：判断.xls头文件是否满足格式
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
		fflush(stdout);
		return false;
	}
	row_str = row->cells.cell[1].str;
	if ((row_str == NULL) || (strcmp((char *)row_str, "key") != 0))
	{
		fflush(stdout);
		return false;
	}
	
	return true;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：获取.conf文件中涂鸦uuid信息
**   参数说明:
**                       info_etc 	.conf文件涂鸦数据
***/
bool tuya_conf_uuid_etc_read(tuya_conf_info * info_etc)
{
	char conf_path[128] = {0};
	sprintf(conf_path, "%s%s", TUYA_UUID_CONF_ETC_PATH, tuya_uuid_and_key_etc_conf);
	
	FILE *fp = NULL;
	fp = fopen(conf_path, "r");
	if (fp == NULL)
	{
		printf("open %s fail\n\r",conf_path);
		return false;
	}		
	fscanf(fp,"%d%s%s",&info_etc->index,info_etc->tuya_uuid,info_etc->tuya_key);
	printf("read %s index :%d uuid :%s	kye :%s\n\r",conf_path,info_etc->index,info_etc->tuya_uuid,info_etc->tuya_key);
	fclose(fp);

	if(strlen(info_etc->tuya_uuid) == 0 || strlen(info_etc->tuya_key) == 0)
	{
		remove(conf_path);
		return false;
	}
	return true;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：创建涂鸦.conf副本或添加涂鸦.conf文件信息
**   参数说明:
							info_mnt  .ini文件涂鸦数据
							info_etc	.conf文件涂鸦数据
***/
static void tuya_conf_uuid_etc_create(tuya_conf_info * info_mnt,tuya_conf_info * info_etc)
{
	struct tm tm;
	char conf_path[128] = {0};
	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	// printf("======================================>%d\n",__LINE__);
    if(tuya_uuid_etc_exist_check() == true)
	{
		// printf("======================================>%d\n",__LINE__);
		char  temp_conf[128] = {0}; 
		memset(temp_conf, 0, sizeof(temp_conf));
		sprintf(temp_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
		if(strcmp(tuya_uuid_and_key_etc_conf,temp_conf) != 0)
		{
			printf("rm -rf /etc/config/tuya_key/*.conf\n");
			system("rm -rf /etc/config/tuya_key/*.conf");
		}
	}
	memset(tuya_uuid_and_key_etc_conf, 0, sizeof(tuya_uuid_and_key_etc_conf));
	sprintf(tuya_uuid_and_key_etc_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
	sprintf(conf_path, "%s%s", TUYA_UUID_CONF_ETC_PATH, tuya_uuid_and_key_etc_conf);

	FILE *fp = NULL;
	fp = fopen(conf_path, "w");
	if (fp == NULL)
	{
		printf("tuya_conf_uuid_etc fp NULL\n\r");
		return;
	}		
	fscanf(fp,"%d%s%s",&info_etc->index,info_etc->tuya_uuid,info_etc->tuya_key);//获取当前使用的uuid信息

	fprintf(fp,"%d %s %s\n",info_mnt->index,info_mnt->tuya_uuid,info_mnt->tuya_key);
	fclose(fp);

}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：创建涂鸦.ini副本或添加SD涂鸦信息
**   参数说明:
							fp					涂鸦文档.ini文件句柄
							info_mnt   涂鸦数据
***/
static bool tuya_conf_uuid_mnt_create(FILE *fp,tuya_conf_info * info_mnt)
{
		if (fp == NULL)
		{
			printf("tuya_conf_uuid_mnt fp NULL\n\r");
			return false;
		}		

		fprintf(fp,"[%d]\n uuid = %s\n key = %s\n used = %s\n\n",info_mnt->index,info_mnt->tuya_uuid,info_mnt->tuya_key,info_mnt->used ? "true" : "false");

		return true;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：修改SD卡中涂鸦.ini副本数据参数
**   参数说明:
							ini_name  SD卡涂鸦.ini副本名
							info_mnt  涂鸦数据
***/
int tuya_conf_uuid_mnt_set(char * ini_name,tuya_conf_info *info)
{
		printf("%s===============================>>>%d\n\r",__func__,info->index);
		if(info->index != 0)
		{
			dictionary  *   ini ;
			char section[10]={0};
			char entry[10]={0};
			char val[10] = {0};

			sprintf(val, "%s",info->used ? "true" : "false");
			sprintf(section, "%d",info->index);
			sprintf(entry, "%s:used",section);

		/* Some temporary variables to hold query results */

			ini = iniparser_load(ini_name);
			if (ini==NULL) {
				fprintf(stderr, "cannot parse file: %s\n", ini_name);
				return -1 ;
			}
			iniparser_dump(ini, stderr);
			iniparser_set(ini, entry, val);

			FILE  *fp = NULL  ;
			fp = fopen(ini_name, "w");
			if( fp == NULL ) {
				printf("stone:fopen error!\n");
				exit(-1);
			}

    		iniparser_dump_ini(ini,fp);
			fclose(fp);
			iniparser_freedict(ini);
		}
    return 0 ;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：读取SD卡中涂鸦.ini文件数据参数
**   参数说明:
							ini  SD卡涂鸦.ini副本对象
							info_mnt  涂鸦数据
***/
int tuya_conf_uuid_mnt_read(dictionary  *   ini,tuya_conf_info *info_mnt )
{
		if (ini==NULL) {
			printf("cannot parse file:uuid_mnt\n");
			return -1 ;
		}

		if(info_mnt->index != 0)
		{
			char entry[10]={0};
			sprintf(entry, "%d:used",info_mnt->index);
			info_mnt->used = iniparser_getboolean(ini, entry, 0);
			if(info_mnt->used == false)
			{
				bzero(entry,sizeof(entry));
				sprintf(entry, "%d:uuid",info_mnt->index);

				const char *uuid =  iniparser_getstring(ini, entry, NULL);
				if(uuid == NULL)
				{
					printf("tuya_conf_uuid_mnt_read uuid fail ....\n\r");
					return -1;
				}
				else
				{
					sprintf(info_mnt->tuya_uuid, "%s",iniparser_getstring(ini, entry, NULL));
				}
				bzero(entry,sizeof(entry));
				sprintf(entry, "%d:key",info_mnt->index);
				const char *key =  iniparser_getstring(ini, entry, NULL);
				if(key == NULL)
				{
					printf("tuya_conf_uuid_mnt_read key fail ....\n\r");
					return -1;
				}
				else
				{
					sprintf(info_mnt->tuya_key, "%s",iniparser_getstring(ini, entry, NULL));
				}
			}
		}

    	return info_mnt->used ;
}

/***
**   日期:2022-06-30 16:35:28
**   作者: link.wu
**   函数作用：读取SD卡中.ini文件中未使用的uuid
**   参数说明:
							ini  SD卡涂鸦文档副本对象
							info_mnt  涂鸦数据
***/
static bool tuya_uuid_ini_null_find(dictionary  *   ini,tuya_conf_info * info_mnt)
{
	if (tuya_conf_uuid_mnt_read(ini,info_mnt) == 0)
	{
		printf(" SD save uuid sd to flash faild 1\n");
		return true;
	}
	return false;
}

/***
**   日期:2022-06-30 16:35:28
**   修改者: link.wu 
**   函数作用：读取涂鸦xls文件信息保存至ini即conf副本中
**   参数说明:
							sheet    .xls文件数据接口
							fp           .ini副本句柄
							info_mnt	.ini涂鸦数据
							info_etc	  .conf 涂鸦数据
***/
static bool tuya_xls_uuid_key_read(const xlsWorkSheet *sheet,FILE *fp,tuya_conf_info * info_mnt,tuya_conf_info * info_etc)
{
	struct st_row_data *row = &sheet->rows.row[info_mnt->index];

	sprintf(info_mnt->tuya_uuid,"%s",row->cells.cell[0].str);
	if (row->cells.cell[0].str == NULL)
	{
		printf("read uuid is null \n");
		return false;
	}
	sprintf(info_mnt->tuya_key,"%s",row->cells.cell[1].str);
	if (row->cells.cell[1].str == NULL)
	{
		printf("read uuid is null \n");
		return false;
	}


	if(info_mnt->used == true)
	{
		tuya_conf_uuid_etc_create(info_mnt,info_etc);
	}
	if (tuya_conf_uuid_mnt_create(fp,info_mnt) == false)
	{
		printf("SD save uuid sd to flash faild 2\n");
	}
	return true;
}


#if 1
/***
**   日期:2022-06-30 15:00:00
**   修改者: link.wu
**   函数作用：从xls创建ini副本
**   参数说明:
***/
bool tuya_uuid_and_key_copy_create(char *uuid,char *key)
{
	printf("%s+========================\n\r",__func__);
	bool result = true;
	/*****  已经有注册的uuid文档 *****/
	// if (is_tuya_uuid_and_kay_register == true)
	// {
	// 	return false;
	// }
	char xls_path[128] = {0};
	
	char conf_path[128] = {0};

	sprintf(xls_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	xlsWorkBook *pxls_xls = xls_open(xls_path, "UTF-8");
	if (pxls_xls == NULL)
	{
		printf("unable to open %s \n", xls_path);
		return false;
	}
	xlsWorkSheet *pxls_sheet = xls_getWorkSheet(pxls_xls, 0);
	xls_parseWorkSheet(pxls_sheet);//解析XLS表

	/*****  判断头文件 *****/
	if (tuya_xls_head_valid_check(pxls_sheet) == false)
	{
		printf("xls head valid failed \n");
		result = false;
		goto finish;
	}
	
	/*****  保存指定的tuyaid *****/
	FILE *fp = NULL;
	struct tm tm;

	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
	{
		if (access(TUYA_UUID_AND_KEY_XLS_PATH, F_OK))
		{
			return false;
		}


		memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
		sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);
		sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_ini);
		printf("open %s\n", conf_path);
		fp = fopen(conf_path, "a+");
		

		tuya_conf_info info_mnt = {{0},{0},0,false};
		tuya_conf_info info_etc = {{0},{0},0,false};
		for(info_mnt.index = 1 ; info_mnt.index <=  pxls_sheet->rows.lastrow; info_mnt.index ++)
		{

			info_mnt.used = info_mnt.index == 1 ? true : false;

			if (tuya_xls_uuid_key_read(pxls_sheet,fp,&info_mnt,&info_etc) == false)
			{
				printf("xls save failed index:%d \n", info_mnt.index);
				result = false;
				goto finish;
			}
		}
		sprintf(uuid,info_mnt.tuya_uuid,sizeof(info_mnt.tuya_uuid));
		sprintf(key,info_mnt.tuya_key,sizeof(info_mnt.tuya_key));
		printf("info_mnt.tuya_uuid :%s         uuid:%s\n\r",info_mnt.tuya_uuid,uuid);
		printf("info_mnt.tuya_key :%s           key:%s\n\r",info_mnt.tuya_key,key);

		fclose(fp);

		tuya_conf_uuid_mnt_set(conf_path,&info_etc);
	}
finish:
	
	xls_close_WS(pxls_sheet);
	xls_close_WB(pxls_xls);
	return result;
}

// #else

/***
**   日期:2022-06-30 09:48:37
**   修改者: link.wu
**   函数作用：从.conf中读取信息并修改.ini文件，获取新涂鸦信息
**   参数说明:
***/
bool tuya_uuid_and_key_swtch(char *uuid,char *key)
{
	printf("%s+=======================11=\n\r",__func__);
	bool result = false;
	/*****  已经有注册的uuid文档 *****/
	char xls_path[128] = {0};
	
	char conf_path[128] = {0};

	sprintf(xls_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	printf("open %s+========================\n\r",xls_path);
	xlsWorkBook *pxls_xls = xls_open(xls_path, "UTF-8");
	if (pxls_xls == NULL)
	{
		printf("unable to open %s \n", xls_path);
		return false;
	}
	xlsWorkSheet *pxls_sheet = xls_getWorkSheet(pxls_xls, 0);
	xls_parseWorkSheet(pxls_sheet);//解析XLS表
	
	
	/*****  保存指定的tuyaid *****/
	struct tm tm;
	tuya_conf_info info_mnt = {{0},{0},0,false};
	tuya_conf_info info_etc = {{0},{0},0,false};
	dictionary  *   ini ;

	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
	{
		if (access(TUYA_UUID_AND_KEY_XLS_PATH, F_OK))
		{
			return false;
		}


		memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
		sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);

		memset(tuya_uuid_and_key_etc_conf, 0, sizeof(tuya_uuid_and_key_etc_conf));
		sprintf(tuya_uuid_and_key_etc_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
		
		sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_ini);
		printf("open %s\n", conf_path);

		/* 读出原有UUID信息 */
		if(tuya_conf_uuid_etc_read(&info_etc) == false)
		{
			info_mnt.index = 1;
		}
		else
		{
			info_mnt.index =  info_etc.index;
		}
		ini = iniparser_load(conf_path);

	/* Some temporary variables to hold query results */
		iniparser_dump(ini, stderr);
		printf("%s===============================>>>%s\n\r",__func__,conf_path);
		for( ; info_mnt.index <=  pxls_sheet->rows.lastrow; info_mnt.index ++)//当前UUID索引查找至文件尾
		{

			if (tuya_uuid_ini_null_find(ini,&info_mnt) == true)
			{
				printf("tuya_uuid_ini_null_find succeed:%d \n", info_mnt.index);
				result = true;
				goto find_finish;
			}
		}
		
		for(info_mnt.index = 1;info_mnt.index <= info_etc.index;info_mnt.index ++ )//文件头查找至当前UUID索引
		{
			if (tuya_uuid_ini_null_find(ini,&info_mnt) == true)
			{
				printf("tuya_uuid_ini_null_find succeed:%d \n", info_mnt.index);
				result = true;
				goto find_finish;
			}
		}
		printf("%s===============================>>>%d\n\r",__func__,__LINE__);

find_finish:

		iniparser_freedict(ini);
		if(result == true)
		{
			info_mnt.used = true;
			tuya_conf_uuid_mnt_set(conf_path,&info_mnt);
			
			info_etc.used = false;
			tuya_conf_uuid_mnt_set(conf_path,&info_etc);

			tuya_conf_uuid_etc_create(&info_mnt,&info_mnt);

			sprintf(uuid,info_mnt.tuya_uuid,sizeof(info_mnt.tuya_uuid));
			sprintf(key,info_mnt.tuya_key,sizeof(info_mnt.tuya_key));
			
		}
	}
	
	xls_close_WS(pxls_sheet);
	xls_close_WB(pxls_xls);
	return result;
}
#endif
