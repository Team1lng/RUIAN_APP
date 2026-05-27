#include "sat_user_common.h"
#include "onvif_user_token.h"
#include "mxml-private.h"
#include "mxml.h"
#include "onvif.h"
#include "sat_ipcamera.h"

#ifndef ONVIF_XML_PATH
#define ONVIF_XML_PATH  /* "/etc/config/onvif/" */   "/home/xiaole/smb/work/sourceCode/ovftst/ipc_onvif_demo/xml_res/"  
#endif
/****************************************************************
2022-09-19 author:leo.liu 说明:获取流的xml文件路径
*****************************************************************/
// #define ONVIF_XML_PATH  ONVIF_XML_PATH//"/tmp/nfs/ssd20x/project/layout/resource/onvif/"

/****************************************************************
2022-09-20 author:leo.liu 说明:discover devices
*****************************************************************/
#define GET_DISCONVER_DEVICE_TDS    ONVIF_XML_PATH "onvif_discover_devices_tds.xml"
#define GET_DISCONVER_DEVICE_DN     ONVIF_XML_PATH "onvif_discover_devices_dn.xml"          /* 2023/12/12 xiaole add */
#define GET_DISCONVER_DEVICE_TDS_DN ONVIF_XML_PATH "onvif_discover_devices_tds_dn.xml"  /* 2023/12/12 xiaole add */


/****************************************************************
2022-09-20 author:leo.liu 说明:获取设备信息
*****************************************************************/
#define GET_DEVICE_INFOMATION ONVIF_XML_PATH "device_infomation.xml"

/****************************************************************
2022-09-20 author:leo.liu 说明:获取rtsp url
*****************************************************************/
#define GET_STREAM_URL ONVIF_XML_PATH "rtsp_stream_url.xml"

/****************************************************************
2022-09-21 author:leo.liu 说明:mediaprofile
*****************************************************************/
#define GET_MEIDA_PROFILE ONVIF_XML_PATH "media_profile.xml"

/****************************************************************
2022-09-21 author:leo.liu 说明:mediaprofile
*****************************************************************/
#define DEVICE_EDIT_XML ONVIF_XML_PATH "device_edit.xml"

/****************************************************************
2022-09-21 author:leo.liu 说明:mediaprofile
*****************************************************************/


#define DEVICE_SET_SCOPES_XML ONVIF_XML_PATH "device_service_set_scopes.xml"
#define DEVICE_GET_SCOPES_XML ONVIF_XML_PATH "get_device_service_scopes.xml"
#define DEVICE_SET_PASSWORD_XML ONVIF_XML_PATH "indoor_set_device_password.xml"

/* 从技术层面来说，通过单播、多播、广播三种方式都能探测到IPC，但多播最具实用性*/
#define COMM_TYPE_UNICAST 1   // 单播
#define COMM_TYPE_MULTICAST 2 // 多播
#define COMM_TYPE_BROADCAST 3 // 广播
#define COMM_TYPE COMM_TYPE_MULTICAST

/* 发送探测消息（Probe）的目标地址、端口号 */
#if COMM_TYPE == COMM_TYPE_UNICAST
#define CAST_ADDR "100.100.100.15" // 单播地址，预先知道的IPC地址
#elif COMM_TYPE == COMM_TYPE_MULTICAST
#define CAST_ADDR "239.255.255.250" // 多播地址，固定的239.255.255.250
#elif COMM_TYPE == COMM_TYPE_BROADCAST
#define CAST_ADDR "100.100.100.255" // 广播地址
#endif
#define CAST_PORT 3702 // 端口号

/****************************************************************
**@日期: 2022-09-17
**@作者: leo.liu
**@功能:解析文本中的xadds
** ipc_type=0x01,CIP-D20YS
*****************************************************************/
static bool xml_parse_device_ipaddr(const char *xml, char *ip, int len)
{
        char *p = strstr(xml, ":XAddrs>http://");
        if (p == NULL)
        {
                LOG_WHITE("%s", xml);
                return false;
        }

        char *start = p + 15;
        if (start == NULL)
        {
                return false;
        }
        char *end = strchr(start, '/');
        if (end == NULL)
        {
                return false;
        }
        end[0] = 0;

        strncpy(ip, start, len);
        return true;
}

static bool xml_parse_device_name(const char *xml, char *name, int len)
{
        char *p = strstr(xml, "/name/");
        if (p == NULL)
        {
                return true;
        }

        char *start = p + 6;
        if (start == NULL)
        {
                return false;
        }
        char *end = strstr(start, "</");
        if (end == NULL)
        {
                end = strstr(start, "/");
                if (end == NULL)
                {
                        end = strstr(start, " ");
                        if (end == NULL)
                        {
                                return false;
                        }
                }
        }
        end[0] = 0;

        strncpy(name, start, len);
        return true;
}

static bool xml_parse_device_type(const char *xml, int *type)
{
        *type = 0x00;
        char *p = strstr(xml, "hardware/CIP-D20YS");
        if (p != NULL)
        {
                *type = 0x01;
                return true;
        }
        p = strstr(xml, "hardware/CIP-70QPT");
        if (p != NULL)
        {
                *type = 0x02;
                return true;
        }
        return true;
}

static bool xml_to_200_ok_parse(const char *xml, char *data)
{
        const char *p = strstr(xml, "HTTP/1.1 200 OK");
        if (p == NULL)
        {
                return false;
        }

        bool result = true;
        if (data == NULL)
        {
                return result;
        }

        char *pxml = strstr(xml, "<?xml version=\"1.0\"");
        if (pxml == NULL)
        {
                printf("%s\n", xml);
                return false;
        }
        mxml_node_t *root = mxmlLoadString(NULL, pxml, MXML_NO_CALLBACK);
        if (root != NULL)
        {
                // printf("%s\n", pxml);
                mxml_node_t *url = mxmlFindElementSub(root, root, "Body", NULL, NULL, MXML_DESCEND);
                while (url != NULL)
                {
                        if ((url->child != NULL) && (url->child->value.text.string != NULL))
                        {
                                LOG_WHITE("[%s:%d]%s\n", __func__, __LINE__, url->child->value.text.string);
                                strcpy(data, url->child->value.text.string);
                                break;
                        }
                        url = mxmlFindElementSub(url, url, "Body", NULL, NULL, MXML_DESCEND);
                }
        }
        mxmlDelete(root);
        return result;
}
static bool xml_parse_device_infomation(const char *xml, char *ip, char *name, size_t len, int *type)
{
        if (xml_parse_device_ipaddr(xml, ip, len) == false)
        {
                LOG_WHITE("%s", xml);
                return false;
        }

        if (xml_parse_device_name(xml, name, len) == false)
        {
                strncpy(name, "unknow", len);
                LOG_WHITE("%s", xml);
        }

        xml_parse_device_type(xml, type);
        return true;
}
/****************************************************************
**@日期: 2022-09-17
**@作者: leo.liu
**@功能:解析文本中的xadds
*****************************************************************/
static bool xml_to_profile_token_parse(const char *file, char profile[8][64], int *profile_num)
{
        /*返回会话没有 200 OK，获取失败*/
        char *p = strstr(file, "200 OK");
        if (p == NULL)
        {
                return false;
        }

        bool result = false;
        char *pxml = strstr(file, "<?xml version=\"1.0\"");
        if (pxml == NULL)
        {
                printf("%s\n", file);
                return false;
        }
        mxml_node_t *root = mxmlLoadString(NULL, pxml, MXML_NO_CALLBACK);
        if (root != NULL)
        {
                // printf("%s\n", pxml);
                mxml_node_t *profile_token = mxmlFindElementSub(root, root, ":Profiles", NULL, NULL, MXML_DESCEND);
                int count = 0;
                while (profile_token != NULL)
                {
                        const char *attr = mxmlElementGetAttr(profile_token, "token");
                        if ((attr != NULL) && (count < 8))
                        {
                                strcpy(profile[count++], attr);
                                *profile_num = count;
                                LOG_WHITE("parse token%d:%s\n", count, attr);
                        }
                        profile_token = mxmlFindElementSub(profile_token, root, ":Profiles", NULL, NULL, MXML_NO_DESCEND);
                        result = true;
                }
        }
        mxmlDelete(root);
        return result;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:解析url
*****************************************************************/
static bool xml_to_stream_url_parse(const char *file, char *rtsp_url, char *sip_uri, int len)
{
        bool result = false;
        char *pxml = strstr(file, "<?xml version=\"1.0\"");
        if (pxml == NULL)
        {
                LOG_WHITE("%s\n", file);
                return false;
        }
        mxml_node_t *root = mxmlLoadString(NULL, pxml, MXML_NO_CALLBACK);
        if (root != NULL)
        {
                // printf("%s\n", pxml);
                mxml_node_t *url = mxmlFindElementSub(root, root, "Uri", NULL, NULL, MXML_DESCEND);
                while (url != NULL)
                {
                        if ((url->child != NULL) && (url->child->value.text.string != NULL) && (strstr(url->child->value.text.string, "rtsp:")))
                        {
                                LOG_WHITE("%s\n", url->child->value.text.string);
                                memset(rtsp_url, 0, len);
                                strncpy(rtsp_url, url->child->value.text.string, len);
                                result = true;
                                break;
                        }
                        url = mxmlFindElementSub(url, url, "Uri", NULL, NULL, MXML_DESCEND);
                }

                url = mxmlFindElementSub(root, root, "SIP", NULL, NULL, MXML_DESCEND);
                while (url != NULL)
                {
                        if ((url->child != NULL) && (url->child->value.text.string != NULL) && (strstr(url->child->value.text.string, "sip:")))
                        {
                                LOG_WHITE("%s\n", url->child->value.text.string);
                                memset(sip_uri, 0, len);
                                strncpy(sip_uri, url->child->value.text.string, len);
                                result = true;
                                break;
                        }
                        url = mxmlFindElementSub(url, url, "SIP", NULL, NULL, MXML_DESCEND);
                }
        }
        mxmlDelete(root);
        return result;
}
/****************************************************************
**@日期: 2022-09-16
**@作者: leo.liu
**@功能: 设备发现
**@参数：
*****************************************************************/
bool ipc_camera_search(char ipc_addr[8][32], char device_name[8][32], int *num, char device_type)
{
    bool result = true;
    char *xml_buffer = malloc(XML_FILE_MAX);
    size_t xml_len = 0;
    memset(xml_buffer, 0, XML_FILE_MAX);

    


    /****************************************************************
    2022-09-20 author:leo.liu 说明:打开onvif组播套接字
    *****************************************************************/
    int sokcet_fd = -1;
    if (sat_socket_udp_open(&sokcet_fd, 0, false) == false)
    {
        printf("%s:open socket failed:%d \n", __func__, __LINE__);
        result = false;
        goto finish;
    }
    if (sat_socket_multicast_join(sokcet_fd, CAST_ADDR) == false)
    {
        printf("%s:jion socket failed:%d \n", __func__, __LINE__);
        result = false;
        goto finish;
    }

    {
        /****************************************************************
        2022-09-20 author:leo.liu 说明:读取discovery xml
        *****************************************************************/
        if ((device_type == 0x00) || (device_type == 0x01))
        {
            xml_len = sat_file_read(GET_DISCONVER_DEVICE_TDS, xml_buffer, XML_FILE_MAX);
            if (xml_len <= 0)
            {
                LOG_ERROR("read %s failed \n", GET_DISCONVER_DEVICE_TDS);
                result = false;
                goto finish;
            }
        }
        // LOG_WHITE("%d, %s\n", xml_len,xml_buffer);


        /****************************************************************
        2022-09-20 author:leo.liu 说明:发送探测消息
        *****************************************************************/
        for (int i = 0; i < 3; i++)
        {
            // LOG_CYAN("\n%s\n", xml_buffer);
            if (sat_socket_udp_send(sokcet_fd, xml_buffer, xml_len, CAST_ADDR, CAST_PORT, 500) == false)
            {
                sat_socket_close(sokcet_fd);
                LOG_ERROR("sned msg failed:\n%s \n", xml_buffer);
                result = false;
                goto finish;
            }
            usleep(10 * 1000);
        }

    }

#if 1
    /****************************************************************
    2023-12-12 author:xiaole 说明:通过抓包发现, 探测消息有两种, 
    有的摄像头在上面TDS的探测消息不会返回,而在这个DN方式的才能返回
    *****************************************************************/

    /****************************************************************
    2022-09-20 author:leo.liu 说明:读取discovery xml
    *****************************************************************/
    if ((device_type == 0x00) || (device_type == 0x01))
    {
        xml_len = sat_file_read(GET_DISCONVER_DEVICE_DN, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
            LOG_ERROR("read %s failed \n", GET_DISCONVER_DEVICE_DN);
            result = false;
            goto finish;
        }
    }
    // LOG_WHITE("%d, %s\n", xml_len,xml_buffer);

    
    /****************************************************************
    2022-09-20 author:leo.liu 说明:发送探测消息
    *****************************************************************/
    for (int i = 0; i < 3; i++)
    {
        // LOG_CYAN("\n%s\n", xml_buffer);
        if (sat_socket_udp_send(sokcet_fd, xml_buffer, xml_len, CAST_ADDR, CAST_PORT, 500) == false)
        {
            sat_socket_close(sokcet_fd);
            LOG_ERROR("sned msg failed:\n%s \n", xml_buffer);
            result = false;
            goto finish;
        }
        usleep(10 * 1000);
    }

#endif

    /****************************************************************
    2022-09-17 author:leo.liu 说明:接受探测的消息
    *****************************************************************/
    memset(xml_buffer, 0, XML_FILE_MAX);
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    while (sat_socket_udp_receive(sokcet_fd, xml_buffer, XML_FILE_MAX, &client_addr, 2000) > 0)
    {
        // LOG_WHITE("buffer='%s'\n", xml_buffer);

        char xaddres[32] = {0};
        char name[32] = {0};
        int ipc_type = 0x00;
        if (xml_parse_device_infomation(xml_buffer, xaddres, name, sizeof(xaddres)-2, &ipc_type) == true)
        {
            if (device_type != ipc_type)
            {
                continue;
            }

            bool same = false;
            for (int i = 0; i < (*num); i++)
            {
                if (strcmp(xaddres, ipc_addr[i]) == 0)
                {
                    same = true;
                }
            }

            if (same == false)
            {
                if((*num) < IPCAMERA_NUM_MAX){
                    LOG_WHITE("serch onvif %s:%s\n", name, xaddres);
                    strncpy(ipc_addr[*num], xaddres, 31);
                    strncpy(device_name[*num], name, 31);   
                    (*num)++;
                }else{
                    LOG_RED("value(%d) will too large after++ \n", (*num));
                    break;
                }

            }
        }
    }
    sat_socket_close(sokcet_fd);

finish:
    if (xml_buffer != NULL)
    {
            free(xml_buffer);
    }
    return result;
}

/****************************************************************
**@日期: 2022-09-17
**@作者: leo.liu
**@功能: 获取认证失败后的摘要信息
*****************************************************************/
static bool ipc_onvif_digest_parse(const char *file, char *qop, char *realm, char *nonce)
{
        /*返回会话没有 200 OK，获取失败*/
        char *p = strstr(file, "401 Unauthorized");
        if (p == NULL)
        {
                // LOG_WHITE("xml format failed\n(%s)\n", file);
                return false;
        }
        /*获取qop 字符串*/
        {
                char *p_qop = strstr(file, "qop=\"");

                if ((p_qop == NULL) || ((p_qop + 5) == NULL))
                {
                        printf("xml not find qop= \n(%s)\n", file);
                        return false;
                }
                p_qop += 5;
                char *e_qop = strstr(p_qop, "\"");
                if (e_qop == NULL)
                {
                        printf("xml not find qop= \n(%s)\n", file);
                        return false;
                }
                *e_qop = '\0';
                if (strcmp(p_qop, "auth"))
                {
                        printf("qop not support:%s\n", p_qop);
                        return false;
                }
                strcpy(qop, p_qop);
                p = e_qop + 1;
        }

        /*获取realm*/
        {
                if (p == NULL)
                {
                        LOG_WHITE("xml format failed\n(%s)\n", file);
                        return false;
                }
                char *p_realm = strstr(p, "realm=\"");
                if ((p_realm == NULL) || ((p_realm + 7) == NULL))
                {
                        printf("xml not find realm= \n(%s)\n", p);
                        return false;
                }
                p_realm += 7;
                char *e_realm = strstr(p_realm, "\"");
                if (e_realm == NULL)
                {
                        printf("xml not find realm= \n(%s)\n", p);
                        return false;
                }
                *e_realm = '\0';
                strcpy(realm, p_realm);
                p = e_realm + 1;
        }
        /*获取nonce*/
        {
                if (p == NULL)
                {
                        LOG_WHITE("xml format failed\n(%s)\n", file);
                        return false;
                }

                char *p_nonce = strstr(p, "nonce=\"");
                if ((p_nonce == NULL) || ((p_nonce + 7) == NULL))
                {
                        printf("xml not find nonce= \n(%s)\n", p);
                        return false;
                }
                p_nonce += 7;
                char *e_nocne = strstr(p_nonce, "\"");
                if (e_nocne == NULL)
                {
                        printf("xml not find nonce= \n(%s)\n", p);
                        return false;
                }
                *e_nocne = '\0';
                strcpy(nonce, p_nonce);
        }

        return true;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能: onvif发送http 口令模式
*****************************************************************/
static bool ipc_onvif_tcp_send(char *data, int size, const char *ip, int port, int timeout)
{
        bool result = true;
        /****************************************************************
        2022-09-20 author:leo.liu 说明:连接tcp ipc
        *****************************************************************/
        int sokcet_fd = -1;
        if (sat_socket_tcp_open(&sokcet_fd, 0, -1) == false)
        {
                printf("%s:open socket failed:%d \n", __func__, __LINE__);
                result = false;
                goto finish;
        }

        if (sat_socket_tcp_connect(sokcet_fd, ip, port < 0 ? 80 : port, timeout) == false)
        {
                printf("%s:open socket failed:%d \n", __func__, __LINE__);
                result = false;
                goto finish;
        }
        /****************************************************************
         2022-09-20 author:leo.liu 说明:发送探测消息
        *****************************************************************/
        if (sat_socket_tcp_send(sokcet_fd, (unsigned char *)data, size, timeout) == false)
        {
                sat_socket_close(sokcet_fd);
                printf("sned msg failed:\n%s \n", data);
                result = false;
                goto finish;
        }

        /****************************************************************
         2022-09-17 author:leo.liu 说明:接受探测的消息
        *****************************************************************/
        int read_size = 0;
        int remain_size = XML_FILE_MAX;
        int relut = 0;
        memset(data, 0, XML_FILE_MAX);
        while ((relut = sat_socket_tcp_receive(sokcet_fd, (unsigned char *)&data[read_size], remain_size, timeout)) > 0)
        {
                remain_size -= relut;
                read_size += relut;
                if (remain_size <= 0)
                {
                        break;
                }
        }
        sat_socket_close(sokcet_fd);

        char *p = strstr(data, "200 OK");
        if (p == NULL)
        {
                result = false;
        }
finish:
        return result;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能: onvif发送http 摘要模式
*****************************************************************/
static bool ipc_onvif_tcp_send_by_digest(char *xml_path, char *data, int size, const char *ip, int port, const char *user, const char *password, const char *math, const char *uri, int argv, char **argc, int timeout)
{
        bool result = true;
        size_t xml_len = sat_file_read(xml_path, data, size);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", xml_path);
                result = false;
                goto finish;
        }
        // LOG_WHITE("read \n%s\n", data);


        /*先发送无鉴权的http*/
        if ((xml_len = onvif_xml_fomrat_by_digest(data, ip, user, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL)) <= 0)
        {
                LOG_ERROR("read format failed %s \n", xml_path);
                result = false;
                goto finish;
        }

        /*发送无鉴权数据*/
        // LOG_WHITE("send \n%s\n", data);
        result = ipc_onvif_tcp_send(data, xml_len, ip, port, timeout);
        // LOG_WHITE("recv \n%s\n", data);
        if (result == false)
        {

                /*鉴权失败后，解析摘要数据*/
                char realm[128] = {0};
                char qop[128] = {0};
                char nonce[256] = {0};
                result = ipc_onvif_digest_parse(data, qop, realm, nonce);
                if (result == false)
                {
                        // LOG_WHITE("digest info parse failed\n(%s)\n", data);
                        result = false;
                        goto finish;
                }
                /*摘要鉴权后发送*/
                memset(data, 0, XML_FILE_MAX);
                size_t xml_len = sat_file_read(xml_path, data, XML_FILE_MAX);
                if (xml_len <= 0)
                {
                        LOG_ERROR("read failed  %s \n", xml_path);
                        result = false;
                        goto finish;
                }

                if ((xml_len = onvif_xml_fomrat_by_digest(data, ip, user, password, math, uri, qop, realm, nonce, argv, argc)) <= 0)
                {
                        LOG_ERROR("read format failed %s \n", xml_path);
                        result = false;
                        goto finish;
                }

                // LOG_CYAN("\n%s\n", data);
                result = ipc_onvif_tcp_send(data, xml_len, ip, port, timeout);
                // LOG_YELLOW("\n%s\n", data);

        }
finish:
        return result;
}
/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取media profile token
*****************************************************************/
bool ipc_profile_token_get(const char *ip, int port, const char *user, const char *password, char profile[8][64], int *profile_num, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);
        size_t xml_len = sat_file_read(GET_MEIDA_PROFILE, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", GET_MEIDA_PROFILE);
                result = false;
                goto finish;
        }

        /*格式化token字符串*/
        if ((xml_len = onvif_xml_fomrat_by_token(xml_buffer, ip, user, password, 0, NULL)) <= 0)
        {
                LOG_ERROR("\n format profile token failed (%s)\n", xml_buffer);
                result = false;
                goto finish;
        }

        // LOG_CYAN("send +++\n%s\n", xml_buffer);
        result = ipc_onvif_tcp_send(xml_buffer, xml_len, ip, port, timeout);
        // LOG_YELLOW("recv +++\n%s\n", xml_buffer);

        if (result == true)
        {
                result = xml_to_profile_token_parse(xml_buffer, profile, profile_num);
        }
finish:
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }

        LOG_WHITE("result=%d\n", result);
        return result;
}

/****************************************************************
**@日期: 2022-09-21
**@作者: leo.liu
**@功能:获取media profile digest
*****************************************************************/
bool ipc_profile_digest_get(const char *ip, int port, const char *user, const char *password, char profile[8][64], int *profile_num, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);

        result = ipc_onvif_tcp_send_by_digest(GET_MEIDA_PROFILE, xml_buffer, XML_FILE_MAX, ip, port, user, password, "POST", "/onvif/Media", 0, NULL, timeout);
        if (result == true)
        {
                result = xml_to_profile_token_parse(xml_buffer, profile, profile_num);
        }


        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }

        return result;
}

/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:获取设备的rtsp
*****************************************************************/
bool ipc_rtsp_token_get(const char *ip, int port, const char *user, const char *password, char *profile_token, char *rtsp, char *sip, int len, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);
        /****************************************************************
        2022-09-20 author:leo.liu 说明:获取xml
        *****************************************************************/
        size_t xml_len = sat_file_read(GET_STREAM_URL, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", GET_STREAM_URL);
                result = false;
                goto finish;
        }
        char *argc[1];
        argc[0] = profile_token;
        if ((xml_len = onvif_xml_fomrat_by_token(xml_buffer, ip, user, password, 1, argc)) <= 0)
        {
                LOG_ERROR("read format failed %s \n", GET_STREAM_URL);
                result = false;
                goto finish;
        }
        result = ipc_onvif_tcp_send(xml_buffer, xml_len, ip, port, timeout);
        if (result == true)
        {
                result = xml_to_stream_url_parse(xml_buffer, rtsp, sip, len);
        }
finish:
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:获取设备的rtsp 来自摘要
*****************************************************************/
bool ipc_rtsp_digest_get(const char *ip, int port, const char *user, const char *password, char *profile_token, char *rtsp, char *sip, int len, int tiemout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);

        char *argc[1];
        argc[0] = profile_token;
        result = ipc_onvif_tcp_send_by_digest(GET_STREAM_URL, xml_buffer, XML_FILE_MAX, ip, port, user, password, "POST", "/onvif/Media", 1, argc, tiemout);
        if (result == true)
        {
                result = xml_to_stream_url_parse(xml_buffer, rtsp, sip, len);
        }
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}

/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:编辑注册或者删除设备
*****************************************************************/
static bool ipc_device_token_edit(char *edit_type, char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);
        /****************************************************************
        2022-09-20 author:leo.liu 说明:获取xml
        *****************************************************************/
        size_t xml_len = sat_file_read(DEVICE_EDIT_XML, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", DEVICE_EDIT_XML);
                result = false;
                goto finish;
        }
        char *argc[3];
        argc[0] = edit_type;
        argc[1] = data;
        argc[2] = edit_type;
        if ((xml_len = onvif_xml_fomrat_by_token(xml_buffer, ip, user, password, 3, argc)) <= 0)
        {
                LOG_ERROR("read format failed %s \n", DEVICE_EDIT_XML);
                result = false;
                goto finish;
        }

        result = ipc_onvif_tcp_send(xml_buffer, xml_len, ip, port, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
        }

finish:
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
static bool ipc_device_digest_edit(char *edit_type, char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);

        char *argc[3];
        argc[0] = edit_type;
        argc[1] = data;
        argc[2] = edit_type;
        result = ipc_onvif_tcp_send_by_digest(DEVICE_EDIT_XML, xml_buffer, XML_FILE_MAX, ip, port, user, password, "POST", "/onvif/device_service", 1, argc, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
        }

        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
static bool ipc_camera_device_edit(char *edit_type, char *data, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        if (auther_flag == 0x00)
        {
                return ipc_device_token_edit(edit_type, data, ip, port, user, password, timeout);
        }

        return ipc_device_digest_edit(edit_type, data, ip, port, user, password, timeout);
}

/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:set scopes
*****************************************************************/
static bool ipc_camera_device_token_setting_scopes(char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);
        size_t xml_len = sat_file_read(DEVICE_SET_SCOPES_XML, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", DEVICE_SET_SCOPES_XML);
                result = false;
                goto finish;
        }
        char *argc[1];
        argc[0] = data;
        /*格式化token字符串*/
        if ((xml_len = onvif_xml_fomrat_by_token(xml_buffer, ip, user, password, 1, argc)) <= 0)
        {
                printf("\n format profile token failed (%s)\n", xml_buffer);
                result = false;
                goto finish;
        }

        result = ipc_onvif_tcp_send(xml_buffer, xml_len, ip, port, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
        }
finish:
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }

        return result;
}
static bool ipc_camera_device_digest_setting_scopes(char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);

        char *argc[1];
        argc[0] = data;
        result = ipc_onvif_tcp_send_by_digest(DEVICE_SET_SCOPES_XML, xml_buffer, XML_FILE_MAX, ip, port, user, password, "POST", "/onvif/device_service", 1, argc, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
        }

        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
static bool ipc_camera_device_setting_scopes(const char *edit_type, char *data, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        if (auther_flag == 0x00)
        {
                return ipc_camera_device_token_setting_scopes(data, ip, port, user, password, timeout);
        }
        return ipc_camera_device_digest_setting_scopes(data, ip, port, user, password, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:set scopes
*****************************************************************/
static bool ipc_camera_device_token_setting_password(char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);
        /****************************************************************
        2022-09-20 author:leo.liu 说明:获取xml
        *****************************************************************/
        size_t xml_len = sat_file_read(DEVICE_SET_PASSWORD_XML, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", DEVICE_SET_PASSWORD_XML);
                result = false;
                goto finish;
        }

        char *argc[1];
        argc[0] = data;
        if ((xml_len = onvif_xml_fomrat_by_token(xml_buffer, ip, user, password, 1, argc)) <= 0)
        {
                LOG_ERROR("read format failed %s \n", DEVICE_SET_PASSWORD_XML);
                result = false;
                goto finish;
        }
        result = ipc_onvif_tcp_send(xml_buffer, xml_len, ip, port, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
        }
finish:
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
static bool ipc_camera_device_digest_setting_password(char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);

        char *argc[1];
        argc[0] = data;
        result = ipc_onvif_tcp_send_by_digest(DEVICE_SET_PASSWORD_XML, xml_buffer, XML_FILE_MAX, ip, port, user, password, "POST", "/onvif/device_service", 0, argc, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
        }

        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}

static bool ipc_camera_device_setting_password(const char *edit_type, char *data, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        if (auther_flag == 0x00)
        {
                return ipc_camera_device_token_setting_password(data, ip, port, user, password, timeout);
        }
        return ipc_camera_device_digest_setting_password(data, ip, port, user, password, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:get scopes
*****************************************************************/
static bool ipc_camera_device_token_get_scopes(char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);
        /****************************************************************
        2022-09-20 author:leo.liu 说明:获取xml
        *****************************************************************/
        size_t xml_len = sat_file_read(DEVICE_GET_SCOPES_XML, xml_buffer, XML_FILE_MAX);
        if (xml_len <= 0)
        {
                LOG_ERROR("read failed  %s \n", DEVICE_GET_SCOPES_XML);
                result = false;
                goto finish;
        }

        if ((xml_len = onvif_xml_fomrat_by_token(xml_buffer, ip, user, password, 0, NULL)) <= 0)
        {
                LOG_ERROR("read format failed %s \n", DEVICE_GET_SCOPES_XML);
                result = false;
                goto finish;
        }
        result = ipc_onvif_tcp_send(xml_buffer, xml_len, ip, port, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
                if (result == true)
                {
                        char *p = strstr(xml_buffer, "/name/");
                        if (p != NULL)
                        {
                                char *end = strstr(p, "</");
                                if (end != NULL)
                                {
                                        *end = '\0';
                                        strncpy(data, p + 6, 63);
                                        printf("[%s:%d] get name:%s\n", __func__, __LINE__, data);
                                }
                        }
                }
        }
finish:
        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:get scopes
*****************************************************************/
static bool ipc_camera_device_digest_get_scopes(char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        bool result = true;
        char *xml_buffer = malloc(XML_FILE_MAX);
        memset(xml_buffer, 0, XML_FILE_MAX);

        result = ipc_onvif_tcp_send_by_digest(DEVICE_GET_SCOPES_XML, xml_buffer, XML_FILE_MAX, ip, port, user, password, "POST", "/onvif/device_service", 0, NULL, timeout);
        if (result == true)
        {
                result = xml_to_200_ok_parse(xml_buffer, data);
                if (result == true)
                {
                        char *p = strstr(xml_buffer, "/name/");
                        if (p != NULL)
                        {
                                char *end = strstr(p, "</");
                                if (end != NULL)
                                {
                                        *end = '\0';
                                        strncpy(data, p + 6, 63);
                                        printf("[%s:%d] get name:%s\n", __func__, __LINE__, data);
                                }
                        }
                }
        }

        if (xml_buffer != NULL)
        {
                free(xml_buffer);
        }
        return result;
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:get scopes
*****************************************************************/
static bool ipc_camera_device_get_scopes(const char *edit_type, char *data, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        if (auther_flag == 0x00)
        {
                return ipc_camera_device_token_get_scopes(data, ip, port, user, password, timeout);
        }

        return ipc_camera_device_digest_get_scopes(data, ip, port, user, password, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:注册
*****************************************************************/
bool ipc_camera_device_register(char *loc_sip_uri, const char *ip, int port, const char *user, const char *password, int timeout)
{
        return ipc_camera_device_edit("Register", loc_sip_uri, ip, port, user, password, 0x00, timeout);
}

/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能: 查询是否在线
*****************************************************************/
bool ipc_camera_device_name_get(char *name, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        return ipc_camera_device_get_scopes("GetName", name, ip, port, user, password, auther_flag, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能: 设置用户名
*****************************************************************/
bool ipc_camera_device_name_set(char *name, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        return ipc_camera_device_setting_scopes("SetName", name, ip, port, user, password, auther_flag, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能: 查询版本号
*****************************************************************/
bool ipc_camera_device_version_get(char *version, const char *ip, int port, const char *user, const char *password, int timeout)
{
        return ipc_camera_device_edit("GetVersion", version, ip, port, user, password, 0x00, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能: 密码修改
*****************************************************************/
bool ipc_camera_device_password_change(char *pwd, const char *ip, int port, const char *user, const char *password, char auther_flag, int timeout)
{
        return ipc_camera_device_setting_password("ChangePassword", pwd, ip, port, user, password, auther_flag, timeout);
}
/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能: 数据同步
*****************************************************************/
bool ipc_camera_device_send_data(char *data_type, char *data, const char *ip, int port, const char *user, const char *password, int timeout)
{
        return ipc_camera_device_edit(data_type, data, ip, port, user, password, 0x00, timeout);
}