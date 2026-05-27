#ifndef __LOG_INFO_H__
#define __LOG_INFO_H__


#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/**************************************************************************
 * @brief  自定义打印输出
 * @date   2022-10-07 14:19
 * @author xiaoele
 **************************************************************************/
#define _LOG_COLOR_B_WHITE  "\033[0;1m"
#define _LOG_COLOR_B_RED    "\033[0;1;31m"
#define _LOG_COLOR_B_GREEN  "\033[0;1;32m"
#define _LOG_COLOR_B_YELLOW "\033[0;1;33m"
#define _LOG_COLOR_B_BLUE   "\033[0;1;34m"
#define _LOG_COLOR_B_PINK   "\033[0;1;35m"
#define _LOG_COLOR_B_CYAN   "\033[0;1;36m"

#define _LOG_COLOR_WHITE    "\033[0m"
#define _LOG_COLOR_RED      "\033[0;31m"
#define _LOG_COLOR_GREEN    "\033[0;32m"
#define _LOG_COLOR_YELLOW   "\033[0;33m"
#define _LOG_COLOR_BLUE     "\033[0;34m"
#define _LOG_COLOR_PINK     "\033[0;35m"
#define _LOG_COLOR_CYAN     "\033[0;36m"
#define _LOG_COLOR_NONE     "\033[0m"



/* 加粗 */
#define LOG_B_WHITE(format,...)     printf(_LOG_COLOR_B_WHITE  "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_B_RED(format,...)       printf(_LOG_COLOR_B_RED    "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_B_GREEN(format,...)     printf(_LOG_COLOR_B_GREEN  "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_B_YELLOW(format,...)    printf(_LOG_COLOR_B_YELLOW "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_B_BLUE(format,...)      printf(_LOG_COLOR_B_BLUE   "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_B_PINK(format,...)      printf(_LOG_COLOR_B_PINK   "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_B_CYAN(format,...)      printf(_LOG_COLOR_B_CYAN   "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* 常规 */
#define LOG_WHITE(format,...)       printf(_LOG_COLOR_WHITE    "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_RED(format,...)         printf(_LOG_COLOR_RED      "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_GREEN(format,...)       printf(_LOG_COLOR_GREEN    "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_YELLOW(format,...)      printf(_LOG_COLOR_YELLOW   "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_BLUE(format,...)        printf(_LOG_COLOR_BLUE     "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_PINK(format,...)        printf(_LOG_COLOR_PINK     "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_CYAN(format,...)        printf(_LOG_COLOR_CYAN     "\r[%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__)


/* 打印错误编号 */
#define LOG_ERROR(format,...)       printf(_LOG_COLOR_RED      "\r[%s:%04u][(%d)%s]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)



#endif /* __LOG_INFO_H__ */

/************************************** 
0	重置所有属性
1	高亮/加粗
2	暗淡
4	下划线
5	闪烁
7	反转
8	隐藏

前景色(字体颜色)
30	        黑色
31	        红色
32	        绿色
33	        黄色
34	        蓝色
35	        品红
36	        青色

背景色
40      黑色
41      红色
42      绿色
43      黄色
44      蓝色
45      品红
46      青色

 ***************************************/