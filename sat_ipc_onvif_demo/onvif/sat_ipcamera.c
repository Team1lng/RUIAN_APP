#include "sat_ipcamera.h"
#include "onvif.h"
#include <pthread.h>
#include <stdbool.h>
#include "sat_user_common.h"
// #include "common/sat_main_event.h"
// #include "common/sat_user_time.h"
#include "libbase64.h"
// #include "common/sat_user_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:在线设备
*****************************************************************/
static struct ipcamera_info ipc_device[IPCAMERA_NUM_MAX] = {0};
static int sat_ipcamera_online_num = 0;
/****************************************************************
2022-09-21 author:leo.liu 说明:设备是否已经在搜索中
*****************************************************************/
static bool sat_ipcamera_search_enable = false;

/****************************************************************
2022-09-21 author:leo.liu 说明:搜索设备类型:
0:IPC
1:DOOR CAMERA
2:IN DOOR
*****************************************************************/
static char sat_ipcamera_serarch_device_type = 0;

static pthread_mutex_t sat_ipcamera_mutex = PTHREAD_MUTEX_INITIALIZER;


bool sat_ipcamera_status_get(void)
{
        pthread_mutex_lock(&sat_ipcamera_mutex);
        bool status = sat_ipcamera_search_enable;
        pthread_mutex_unlock(&sat_ipcamera_mutex);

        // LOG_WHITE("status=%d\n", status);
        return status;
}


/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:设置ipcamera参数
*****************************************************************/
bool sat_ipcamera_initialization_parameters(struct ipcamera_info *device, int total)
{
        pthread_mutex_lock(&sat_ipcamera_mutex);
        if (sat_ipcamera_search_enable == true)
        {
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return false;
        }
        sat_ipcamera_search_enable = true;
        if (total > IPCAMERA_NUM_MAX)
        {
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return false;
        }

        memcpy(ipc_device, device, sizeof(struct ipcamera_info) * total);
        sat_ipcamera_online_num = total;
        sat_ipcamera_search_enable = false;
        pthread_mutex_unlock(&sat_ipcamera_mutex);
        return true;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:设置账号和密码
*****************************************************************/
bool sat_ipcamera_user_password_set(int index, const char *username, const char *password)
{
        pthread_mutex_lock(&sat_ipcamera_mutex);
        if ((username == NULL) || (password == NULL))
        {
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return false;
        }
        if (index == 0xFF)
        {
                for (int i = 0; i < IPCAMERA_NUM_MAX; i++)
                {
                        strncpy(ipc_device[i].username, username, sizeof(ipc_device[i].username));
                        strncpy(ipc_device[i].password, password, sizeof(ipc_device[i].password));
                }
        }
        else if (index < IPCAMERA_NUM_MAX)
        {
                strncpy(ipc_device[index].username, username, sizeof(ipc_device[index].username));
                strncpy(ipc_device[index].password, password, sizeof(ipc_device[index].password));
        }
        pthread_mutex_unlock(&sat_ipcamera_mutex);
        LOG_WHITE("[%d] (%s) user:%s, password:%s\n", index, ipc_device[index].ipaddr, ipc_device[index].username, ipc_device[index].password);
        return true;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:初始化参数
*****************************************************************/
static void ipc_device_param_init(void)
{
        sat_ipcamera_online_num = 0;
        for (int i = 0; i < IPCAMERA_NUM_MAX; i++)
        {
                memset(ipc_device[i].ipaddr, 0, sizeof(ipc_device[i].ipaddr));
                ipc_device[i].profile_token_num = 0;
                ipc_device[i].port = 80;
                ipc_device[i].auther_flag = 0x00;
                for (int j = 0; j < IPCAMERA_PROFILE_MAX; j++)
                {
                        memset(ipc_device[i].rtsp[j].profile_token, 0, sizeof(ipc_device[i].rtsp[j].profile_token));
                        memset(ipc_device[i].rtsp[j].rtsp_url, 0, sizeof(ipc_device[i].rtsp[j].rtsp_url));
                }
                memset(ipc_device[i].door_name, 0, sizeof(ipc_device[i].door_name));
                memset(ipc_device[i].sip_url, 0, sizeof(ipc_device[i].sip_url));
        }
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:IPC搜索
*****************************************************************/
static void *sat_ipcamera_online_search_task(void *arg)
{
        ipc_device_param_init();
        /****************************************************************
        2022-09-21 author:leo.liu 说明:搜索在线设备
        *****************************************************************/
        char ipaddr[IPCAMERA_NUM_MAX][32];
        char name[IPCAMERA_NUM_MAX][32];
        for (int i = 0; i < IPCAMERA_NUM_MAX; i++)
        {
                memset(ipaddr[i], 0, sizeof(ipaddr[i]));
                memset(name[i], 0, sizeof(name[i]));
        }

        ipc_camera_search(ipaddr, name, &sat_ipcamera_online_num, sat_ipcamera_serarch_device_type);
        for (int i = 0; i < sat_ipcamera_online_num; i++)
        {
                char *p = strchr(ipaddr[i], ':');
                if ((p != NULL) && ((p + 1) != NULL))
                {
                        *p = '\0';
                        sscanf(p + 1, "%d", &ipc_device[i].port);
                }
                strncpy(ipc_device[i].ipaddr, ipaddr[i],20);
                strncpy(ipc_device[i].door_name, name[i], 31);
        }
        // sat_msg_send_cmd(MSG_EVENT_CMD_IPCAMERA, 1, sat_ipcamera_online_num);


        pthread_mutex_lock(&sat_ipcamera_mutex);
        sat_ipcamera_search_enable = false;
        pthread_mutex_unlock(&sat_ipcamera_mutex);
        // pthread_exit(0);
        return NULL;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:搜索onvif所有的rtsp流
0:IPC
1:DOOR CAMERA
2:IN DOOR
*****************************************************************/
bool sat_ipcamera_device_discover_search(char type)
{
        pthread_t pthread_id;
        pthread_mutex_lock(&sat_ipcamera_mutex);
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamer device search ing");
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return false;
        }
        sat_ipcamera_search_enable = true;
        pthread_mutex_unlock(&sat_ipcamera_mutex);

        sat_ipcamera_serarch_device_type = type;
        pthread_create(&pthread_id, sat_pthread_attr_get(), sat_ipcamera_online_search_task, NULL);
        // pthread_detach(pthread_id);
        return true;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:搜索流
*****************************************************************/
static void *sat_ipcamera_rtsp_url_task(void *arg)
{
        int rtsp_url_get_index = *((int *)arg);
        ipc_device[rtsp_url_get_index].profile_token_num = 0;       /* 搜索前进行参数重置 */

        char profile[IPCAMERA_PROFILE_MAX][64];
        int profile_num[IPCAMERA_NUM_MAX] = {0};
        if (rtsp_url_get_index < 0)
        {
                for (int i = 0; i < sat_ipcamera_online_num; i++)
                {
                        profile_num[i] = 0;
                        ipc_device[i].auther_flag = 0x00;
                        if (ipc_profile_token_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, profile, &(profile_num[i]), 1500) == false)
                        {
                                if (ipc_profile_digest_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, profile, &(profile_num[i]), 1500) == true)
                                {
                                        ipc_device[i].auther_flag = 0x01;
                                }
                        }

                        if (profile_num[i] > 0)
                        {
                                ipc_device[i].profile_token_num = profile_num[i];
                                for (int j = 0; j < profile_num[i]; j++)
                                {
                                        strcpy(ipc_device[i].rtsp[j].profile_token, profile[j]);
                                }
                        }
                }

                for (int i = 0; i < sat_ipcamera_online_num; i++)
                {
                        for (int j = 0; j < ipc_device[i].profile_token_num; j++)
                        {
                                if (ipc_device[i].auther_flag == 0)
                                {
                                        ipc_rtsp_token_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, ipc_device[i].rtsp[j].profile_token,
                                                           ipc_device[i].rtsp[j].rtsp_url, ipc_device[i].sip_url, sizeof(ipc_device[i].rtsp[j].rtsp_url), 1500);
                                }
                                else if (ipc_device[i].auther_flag == 1)
                                {
                                        ipc_rtsp_digest_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, ipc_device[i].rtsp[j].profile_token,
                                                            ipc_device[i].rtsp[j].rtsp_url, ipc_device[i].sip_url, sizeof(ipc_device[i].rtsp[j].rtsp_url), 1500);
                                }
                        }
                }
        }
        else
        {
                int i = rtsp_url_get_index;
                profile_num[i] = 0;
                ipc_device[i].auther_flag = 0x00;
                LOG_WHITE("[%d] user:%s, password:%s\n", i, ipc_device[i].username, ipc_device[i].password);
                if (ipc_profile_token_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, profile, &(profile_num[i]), 5000) == false)
                {
                        LOG_WHITE("profile digest\n");
                        if (ipc_profile_digest_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, profile, &(profile_num[i]), 5000) == true)
                        {
                                LOG_WHITE("\n\n +++++++ profile number :%d ++++++++++\n\n", profile_num[i]);
                                ipc_device[i].auther_flag = 0x01;
                        }
                }


                if (profile_num[i] > 0)
                {

                        ipc_device[i].profile_token_num = profile_num[i];
                        LOG_YELLOW("profile number :%d\n\n", profile_num[i]);
                        for (int j = 0; j < ipc_device[i].profile_token_num; j++)
                        {

                                strcpy(ipc_device[i].rtsp[j].profile_token, profile[j]);
                                if (ipc_device[i].auther_flag == 0x00)
                                {
                                        LOG_WHITE("token\n");
                                        ipc_rtsp_token_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, ipc_device[i].rtsp[j].profile_token, ipc_device[i].rtsp[j].rtsp_url,
                                                           ipc_device[i].sip_url, sizeof(ipc_device[i].rtsp[j].rtsp_url), 1500);
                                }
                                else if (ipc_device[i].auther_flag == 1)
                                {
                                        LOG_WHITE("digest\n");
                                        ipc_rtsp_digest_get(ipc_device[i].ipaddr, ipc_device[i].port, ipc_device[i].username, ipc_device[i].password, ipc_device[i].rtsp[j].profile_token,
                                                            ipc_device[i].rtsp[j].rtsp_url, ipc_device[i].sip_url, sizeof(ipc_device[i].rtsp[j].rtsp_url), 1500);
                                }
                                // LOG_WHITE("rtsp token:%s, url:%s\n",ipc_device[i].rtsp[j].profile_token, ipc_device[i].rtsp[j].rtsp_url);
                        }
                }
        }
        // sat_msg_send_cmd(MSG_EVENT_CMD_IPCAMERA, 2, sat_ipcamera_online_num);
        pthread_mutex_lock(&sat_ipcamera_mutex);
        sat_ipcamera_search_enable = false;
        pthread_mutex_unlock(&sat_ipcamera_mutex);
        return NULL;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取onvif所有的rtsp流
*****************************************************************/
bool sat_ipcamera_rtsp_url_get(int index)
{
        pthread_t pthread_id;
        pthread_mutex_lock(&sat_ipcamera_mutex);
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamera rsp stream error \n");
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return false;
        }
        if (sat_ipcamera_online_num < 1)
        {
                printf("not online device \n");
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return false;
        }
        sat_ipcamera_search_enable = true;
        pthread_mutex_unlock(&sat_ipcamera_mutex);
        static int rtsp_url_get_index = 0;
        rtsp_url_get_index = index;
        pthread_create(&pthread_id, sat_pthread_attr_get(), sat_ipcamera_rtsp_url_task, &rtsp_url_get_index);
        pthread_detach(pthread_id);
        return true;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取在线的IP
*****************************************************************/
int sat_ipcamera_online_num_get(void)
{
        return sat_ipcamera_online_num;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取ip
*****************************************************************/
const char *sat_ipcamera_ipaddr_get(int index)
{
        if (index >= sat_ipcamera_online_num)
        {
                return NULL;
        }
        return ipc_device[index].ipaddr;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取端口
*****************************************************************/
int sat_ipcamera_port_get(int index)
{
        if (index >= sat_ipcamera_online_num)
        {
                return -1;
        }
        return ipc_device[index].port;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取账号
*****************************************************************/
const char *sat_ipcamera_username_get(int index)
{
        if (index > IPCAMERA_NUM_MAX)
        {
                return NULL;
        }
        return ipc_device[index].username;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取密码
*****************************************************************/
const char *sat_ipcamera_password_get(int index)
{
        if (index > IPCAMERA_NUM_MAX)
        {
                return NULL;
        }
        return ipc_device[index].password;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取别名
*****************************************************************/
const char *sat_ipcamera_door_name_get(int index)
{
        if (index > IPCAMERA_NUM_MAX)
        {
                return NULL;
        }
        return ipc_device[index].door_name;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取rtsp url
*****************************************************************/
const char *sat_ipcamera_rtsp_addr_get(int index, int ch)
{
        if ((index > sat_ipcamera_online_num) || (ch > ipc_device[index].profile_token_num))
        {
                return NULL;
        }
        return ipc_device[index].rtsp[ch].rtsp_url;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取rtsp url
*****************************************************************/
const char *sat_ipcamera_sip_addr_get(int index)
{
        if (index > sat_ipcamera_online_num)
        {
                return NULL;
        }
        return ipc_device[index].sip_url;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取token 数目
*****************************************************************/
int sat_ipcamera_profile_token_num_get(int index)
{
        if (index > sat_ipcamera_online_num)
        {
                return 0;
        }
        return ipc_device[index].profile_token_num;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:向doorcamera 注册一个设备
*****************************************************************/
bool sat_ipcamera_device_register(char *loc_sip_uri, int index, int timeout)
{
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamera rsp stream error \n");
                return false;
        }
        if (sat_ipcamera_online_num < 1)
        {
                printf("not online device \n");
                return false;
        }
        sat_ipcamera_search_enable = true;
        /*私有指令，auther_flag = 0x00*/
        bool result = ipc_camera_device_register(loc_sip_uri, ipc_device[index].ipaddr, ipc_device[index].port, ipc_device[index].username, ipc_device[index].password, timeout);
        sat_ipcamera_search_enable = false;
        return result;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:向doorcamera 查询是否在线
*****************************************************************/
bool sat_ipcamera_device_name_get(int index, int timeout)
{
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamera rsp stream error \n");
                return false;
        }
        if (sat_ipcamera_online_num < 1)
        {
                printf("not online device \n");
                return false;
        }
        sat_ipcamera_search_enable = true;
        char name[64] = {0};
        bool result = ipc_camera_device_name_get(name, ipc_device[index].ipaddr, ipc_device[index].port, ipc_device[index].username, ipc_device[index].password, ipc_device[index].auther_flag, timeout);
        // printf("get door name %s\n%s\n", ipc_device[index].door_name, name);
        if ((result == true) && (strcmp(ipc_device[index].door_name, name)))
        {
                strncpy(ipc_device[index].door_name, name, sizeof(ipc_device[index].door_name));
        }
        sat_ipcamera_search_enable = false;
        return result;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:向doorcamera 用户名设置
*****************************************************************/
bool sat_ipcamera_device_name_set(char *name, int index, int timeout)
{
        bool result = false;
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamera rsp stream error \n");
                return false;
        }
        if (sat_ipcamera_online_num < 1)
        {
                printf("not online device \n");
                return false;
        }
        sat_ipcamera_search_enable = true;

        result = ipc_camera_device_name_set(name, ipc_device[index].ipaddr, ipc_device[index].port, ipc_device[index].username, ipc_device[index].password, ipc_device[index].auther_flag, timeout);
        if ((result == true) && (strcmp(ipc_device[index].door_name, name)))
        {
                strncpy(ipc_device[index].door_name, name, sizeof(ipc_device[index].door_name));
        }
        sat_ipcamera_search_enable = false;
        return result;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:向doorcamera 修改密码
*****************************************************************/
bool sat_ipcamera_device_password_set(char *new_pwd, int index, int timeout)
{
        bool result = false;
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamera rsp stream error \n");
                return false;
        }

        if (sat_ipcamera_online_num < 1)
        {
                printf("not online device \n");
                return false;
        }

        sat_ipcamera_search_enable = true;
        result = ipc_camera_device_password_change(new_pwd, ipc_device[index].ipaddr, ipc_device[index].port, ipc_device[index].username, ipc_device[index].password, ipc_device[index].auther_flag, timeout);
        if ((result == true) && (strcmp(ipc_device[index].password, new_pwd)))
        {
                strncpy(ipc_device[index].password, new_pwd, sizeof(ipc_device[index].password));
        }
        sat_ipcamera_search_enable = false;
        return result;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:向doorcamera 修改密码
*****************************************************************/
bool sat_ipcamera_device_version_get(char *version, int index, int timeout)
{
        if (sat_ipcamera_search_enable == true)
        {
                printf("ipcamera rsp stream error \n");
                return false;
        }
        if (sat_ipcamera_online_num < 1)
        {
                printf("not online device \n");
                return false;
        }

        if (ipc_device[index].sip_url[0] == '\0')
        {
                return false;
        }

        sat_ipcamera_search_enable = true;
        bool result = ipc_camera_device_version_get(version, ipc_device[index].ipaddr, ipc_device[index].port, ipc_device[index].username, ipc_device[index].password, timeout);

        sat_ipcamera_search_enable = false;
        return result;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取节点信息
*****************************************************************/
struct ipcamera_info *sat_ipcamera_node_data_get(int index)
{
        pthread_mutex_lock(&sat_ipcamera_mutex);
        if ((sat_ipcamera_search_enable == true) || (index > sat_ipcamera_online_num))
        {
                printf("ipcamera rsp stream error \n");
                pthread_mutex_unlock(&sat_ipcamera_mutex);
                return NULL;
        }
        pthread_mutex_unlock(&sat_ipcamera_mutex);

        return &ipc_device[index];
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-5 15:21:6
** 说明: 数据同步处理.注意：此接口只有ID1处理
** type: 0:user_data,1:network_data,2:asterisk data
** flag: bit0:1发送到室内分机，bit1:1发送到门口机,bit2:1发送给主机
** data:需要同步的数据
** size:同步数据的大小
** inline_t:最后刷新注册的时间戳到现在的时间差判定是否在线
** timeout：发送超时
** param: 预留参数，如果需要发送给门口机，则需要传送绑定的设备信息
***********************************************/
bool sat_ipcamera_data_sync(char type, char flag, const char *data, int size, int inline_t, int timeout, void *param)
{
        bool result = false;
        int sub_timestamp = 0;
        char *base64_bffer = NULL;
        size_t base64_size = 0;
        if ((type != 0) && (type != 1) && (type != 2))
        {
                return false;
        }
        char *data_type = type == 0 ? "SyncUserData" : type == 1 ? "SyncNetworkData"
                                                                 : "SyncAsteriskData";

        // printf("[%s:%d] data sync start\n", __func__, __LINE__);
        sat_ipcamera_search_enable = true;
        const asterisk_register_info *p_info = asterisk_register_info_get();
        if (p_info != NULL)
        {
                for (int i = 0; i < ASTERISK_REIGSTER_DEVICE_MAX; i++)
                {
                        if (p_info[i].name[0] != 0)
                        {
                                /*如果flag bit[0:1] & 0x3不为零，则是主机发送，判断实际的时间戳。如果falg bit[2] 不为零，则是分机发送，判断时间戳是否为0即可*/
                                if (flag & 0x03)
                                {
                                        sub_timestamp = (int)((sat_timestamp_get() - p_info[i].timestamp) / 1000);
                                        if (sub_timestamp > inline_t)
                                        {
                                                printf("device:%s(%s) offline,time:%d\n", p_info[i].name, p_info[i].ip, sub_timestamp);
                                                continue;
                                        }
                                }
                                else if ((flag & 0x04) && (p_info[i].timestamp == 0))
                                {
                                        continue;
                                }

                                char *str = strchr(p_info[i].ip, ':');
                                if (str)
                                {
                                        *str = '\0';
                                }
                                if (base64_bffer == NULL)
                                {
                                        base64_bffer = (char *)malloc(size * 2);
                                }
                                /*注意：base64_bffer数据会被“ipc_camera_device_sync_data”接口修改，所以每次传输都需要重新进行base64编码*/
                                memset(base64_bffer, 0, size * 2);
                                base64_encode(data, size, base64_bffer, &base64_size, 0);

                                if (((flag & 0x01) && (strncmp(p_info[i].name, "50", 2) == 0) && (strncmp(p_info[i].name, "501", 3) != 0)))
                                {
                                        /*室内机，密码账号为默认的*/
                                        result = ipc_camera_device_send_data(data_type, base64_bffer, p_info[i].ip, 80, "admin", "123456789", timeout);
                                }
                                if ((flag & 0x02) && (strncmp(p_info[i].name, "20", 2) == 0))
                                { /*门口机，需要对比绑定的设备，进行传输密码和账号*/
                                        struct ipcamera_info *p_ipc_info = (struct ipcamera_info *)param;
                                        for (int i = 0; i < 8; i++)
                                        {
                                                if (strcmp(p_ipc_info[i].ipaddr, p_info[i].ip) == 0)
                                                {
                                                        result = ipc_camera_device_send_data(data_type, base64_bffer, p_info[i].ip, 80, "admin", p_ipc_info[i].password, timeout);
                                                        break;
                                                }
                                        }
                                }
                                if (((flag & 0x04) && (strncmp(p_info[i].name, "501", 3) == 0)))
                                {
                                        /*室内机，密码账号为默认的*/
                                        result = ipc_camera_device_send_data(data_type, base64_bffer, p_info[i].ip, 80, "admin", "123456789", timeout);
                                }
                        }
                }
        }

        if (base64_bffer != NULL)
        {
                free(base64_bffer);
        }
        sat_ipcamera_search_enable = false;
        // printf("[%s:%d] data sync done\n", __func__, __LINE__);
        return result;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-5 15:21:6
** 说明: 发送shell cmd 执行命名
** ip: 对象IP，
** port: 对象端口
** user： 对象用户名
** password: 对象密码
** cmd: shell命令
** timeout : 超时时间
***********************************************/
bool sat_ipcamera_report_shellcmd(char *ip, int port, const char *user, const char *password, char *cmd, int timeout)
{
        size_t base64_size = strlen(cmd) * 2;
        char *base64_bffer = (char *)malloc(base64_size);
        sat_ipcamera_search_enable = true;
        memset(base64_bffer, 0, base64_size);
        base64_encode(cmd, strlen(cmd), base64_bffer, &base64_size, 0);
        // printf("[%s] base64 encode to [%s] \n", cmd, base64_bffer);
        bool ret = ipc_camera_device_send_data("ShellCmd", base64_bffer, ip, port, user, password, timeout);
        free(base64_bffer);
        sat_ipcamera_search_enable = false;
        return ret;
}
