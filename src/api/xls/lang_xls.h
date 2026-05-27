#ifndef _LANGUAGE_XLS_H
#define _LANGUAGE_XLS_H

#include <stdbool.h>
// #include "version.h"
// #include "../../src/_wasApp/wasApp.h"

#define XLS_TMP_PATH    "/app/app/language.xls"          // xls文件的路径

#define CODE "UTF-8"                    // xls文件的编码格式


char ***lang_xls_init(int sheet_num);

bool lang_xls_file_state_get(void);

int lang_xls_null_str_num_get(void);

int lang_xls_language_num_get(void);

int lang_xls_str_num_get(void);

char *lang_xls_str_get(int str_num, int lang_type);

bool init_language_xls_info(void);
















#endif