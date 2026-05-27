

#include "sat_user_common.h"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define NETWORK_PACKAET_MAX (100 * 1024)

/*
 * @日期: 2022-08-06
 * @作者: leo.liu
 * @功能: 获取线程栈内存大小
 * @return:
 */
pthread_attr_t *sat_pthread_attr_get(void)
{
        static pthread_attr_t thread_attr;
        size_t stacksize = 200 * 1024;

        pthread_attr_setstacksize(&thread_attr, stacksize);
        return &thread_attr;
}

/***
** 日期: 2022-04-26 11:02
** 作者: leo.liu
** 函数作用：获取当前时间戳
** 单位：秒
***/
unsigned long long sat_timestamp_get(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec;
}

/****************************************************************
**@日期: 2022-09-28
**@作者: leo.liu
**@功能:获取文件大小
*****************************************************************/
size_t sat_file_size_get(const char *path)
{
        struct stat st;

        if (stat(path, &st) != 0)
        {
                LOG_ERROR("read file size failed,(%s) \n", path);
                return 0;
        }

        return st.st_size;
}

/****************************************************************
**@日期: 2022-09-28
**@作者: leo.liu
**@功能:读取文件
**@ data
*****************************************************************/
size_t sat_file_read(const char *path, char *data, size_t size)
{
//     LOG_WHITE("file=%s\n", path);
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
            LOG_ERROR("open %s fialed\n", path);
            return 0;
    }

    size_t read_total = 0;
    size_t ret = 0;
    do{
        ret = read(fd, &data[read_total], size);
        read_total += ret;
        // LOG_WHITE("total=%d, ret=%d\n", read_total, ret);
    }while(ret > 0 && read_total < size);

    close(fd);
    return read_total;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 9:49:0
** 说明: 获取网卡的Ip
***********************************************/
bool sat_ip_mac_addres_get(const char *eth, char *ip, char *mac, char *mask)
{
        int sock;
        bool result = false;
        struct sockaddr_in sin;
        struct ifreq ifr;
        sock = socket(AF_INET, SOCK_DGRAM, 0);

        if (sock == -1)
        {
                close(sock);
                printf("Error:get local IP socket fail!\n");
                return false;
        }

        strncpy(ifr.ifr_name, eth, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = 0;

        if (ip != NULL)
        {
                if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
                {
                        close(sock);
                        //    printf("Error:get local IP ioctl fail! \n");
                        return false;
                }

                memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
                sprintf(ip, "%s", inet_ntoa(sin.sin_addr));
                result = true;
                printf("ip:%s\n", ip);
        }

        if (mask != NULL)
        {
                if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0)
                {
                        close(sock);
                        //    printf("Error:get local IP ioctl fail! \n");
                        return false;
                }

                memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
                sprintf(mask, "%s", inet_ntoa(sin.sin_addr));
                result = true;
        }

        if (mac != NULL)
        {

                if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
                {
                        close(sock);
                        //  perror("Error:get local mac ioctl fail! \n");
                        return false;
                }

                struct sockaddr sa;
                memcpy(&sa, &ifr.ifr_addr, sizeof(struct sockaddr_in));
                sprintf((char *)mac, "%02X:%02X:%02X:%02X:%02X:%02X", sa.sa_data[0], sa.sa_data[1], sa.sa_data[2], sa.sa_data[3], sa.sa_data[4], sa.sa_data[5]);
                result = true;
        }

        close(sock);
        return result;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 10:45:38
** 说明: 获取linphone sip账号
***********************************************/
bool sat_sip_local_user_get(char *user)
{
        char ip[64] = {0};
        memset(ip, 0, sizeof(ip));

        if (sat_ip_mac_addres_get(NETWORK_ETH_NAME, ip, NULL, NULL) == true)
        {
                const char *username = getenv("SIP");
                if ((username == NULL) || (strlen(username) < 12))
                {
                        printf("getenv sip failed \n");
                        return false;
                }
                sprintf(user, "sip:%s@%s", username, ip);
                return true;
        }

        return false;
}
/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 10:45:38
** 说明: 获取指定door camera 的 sip账号
local 010-001-001-011@192.168.31.25
***********************************************/
bool sat_sip_local_doorcamera_number_get(int camera_index, char *dst_number)
{
        char door_username[13] = {0};
        const char *username = getenv("SIP");
        if ((username == NULL) || (strlen(username) != 12))
        {
                printf("getenv sip failed \n");
                return false;
        }
        strcpy(door_username, username);

        int value = (door_username[3] - 48) * 100 + (door_username[4] - 48) * 10 + (door_username[5] - 48);
        value = (value & 0x1F) | 0xC0;

        char value_str[4] = {0};
        sprintf(value_str, "%03d", value);
        strcpy(&door_username[3], value_str);
        door_username[6] = username[6];
        door_username[11] = camera_index + 48 + 1;

        sprintf(dst_number, "%s", door_username);
        return true;
}
/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 10:45:38
** 说明: 获取指定室内分机 的 sip账号
local 010-001-001-011@192.168.31.25
***********************************************/
bool sat_sip_local_indoor_number_get(int id, char *dst_number)
{
        char indoor_username[13] = {0};
        int buffer[4] = {0};
        char ip[32] = {0};

        const char *username = getenv("SIP");
        if ((username == NULL) || (strlen(username) != 12))
        {
                printf("getenv sip failed \n");
                return false;
        }
        strcpy(indoor_username, username);
        indoor_username[11] = id + 48;

        buffer[0] = (indoor_username[0] - 48) * 100 + (indoor_username[1] - 48) * 10 + (indoor_username[2] - 48);
        buffer[1] = ((indoor_username[3] - 48) * 100 + (indoor_username[4] - 48) * 10 + (indoor_username[5] - 48));
        buffer[2] = (indoor_username[6] - 48) * 100 + (indoor_username[7] - 48) * 10 + (indoor_username[8] - 48);
        buffer[3] = (indoor_username[9] - 48) * 100 + (indoor_username[10] - 48) * 10 + (indoor_username[11] - 48);
        sprintf(ip, "%d.%d.%d.%d", buffer[0], buffer[1], buffer[2], buffer[3]);

        sprintf(dst_number, "%s@%s", indoor_username, ip);
        return true;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 11:25:54
** 说明: udhchc 获取IP
***********************************************/
void sat_network_udhcpc_ip(const char *eth)
{
        char cmd[256] = {
            0};
        //      sprintf(cmd, "killall udhcpc");
        sprintf(cmd, "udhcpc -i %s -s /etc/init.d/udhcpc.script", eth);
        sat_kill_task_process(cmd);
        // system(cmd);
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "udhcpc  -i %s -s /etc/init.d/udhcpc.script &", eth);
        system(cmd);
}

#if 0

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 11:26:51
** 说明: eth0写入mac
***********************************************/
bool sat_network_eth0_mac_write(const char *mac)
{

        if (mac == NULL)
        {
                printf("setting %s mac failed:mac is null\n", "eth0");
                return false;
        }

        if (access(MAC_ETH0_PATH, F_OK) == 0)
        {
                remove(MAC_ETH0_PATH);
                system("sync");
        }

        int fd = open(MAC_ETH0_PATH, O_CREAT | O_WRONLY);

        if (fd < 0)
        {
                printf("open %s failed \n", MAC_ETH0_PATH);
                return false;
        }

        write(fd, mac, strlen(mac));
        close(fd);
        return true;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-26 11:26:51
** 说明: eth1写入mac
***********************************************/
bool sat_network_eth1_mac_write(const char *mac)
{

        if (mac == NULL)
        {
                printf("setting %s mac failed:mac is null\n", "eth1");
                return false;
        }

        if (access(MAC_ETH1_PATH, F_OK) == 0)
        {

                remove(MAC_ETH1_PATH);
                system("sync");
        }

        int fd = open(MAC_ETH1_PATH, O_CREAT | O_WRONLY);

        if (fd < 0)
        {
                printf("open %s failed \n", MAC_ETH1_PATH);
                return false;
        }

        write(fd, mac, strlen(mac));
        close(fd);
        return true;
}
#endif

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-28 10:6:44
** 说明: 开启和关闭网络
***********************************************/
void sat_network_enable(const char *eth, bool en)
{
        char cmd[256] = {0};
        sprintf(cmd, "ifconfig %s %s", eth, en ? "up" : "down");
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-9 11:16:23
** 说明: 设置参数非阻塞模式
***********************************************/
static bool sat_socket_block_enable(int fd, bool en)
{
        int ret;
        int flags = fcntl(fd, F_GETFL);

        if (flags == -1)
        {

                printf("[%s:%d] fcntl error\n", __func__, __LINE__);
                return false;
        }

        if (en == true)
        {

                flags &= ~O_NONBLOCK;
        }
        else
        {

                flags |= O_NONBLOCK;
        }

        ret = fcntl(fd, F_SETFL, flags);

        if (ret == -1)
        {

                printf("[%s:%d] fcntl error\n", __func__, __LINE__);
                return false;
        }

        return true;
}

static int sat_socket_open(int socket_type, int port)
{
        struct sockaddr_in addr;
        int err = 0;
        int optval = 1;
        unsigned char loop = 0;
        int sock = -1;

        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-5 16:47:3
        ** 说明: 创建套接字
        ***********************************************/
        {
                sock = socket(AF_INET, socket_type, 0);

                if (sock < 0)
                {

                        printf("#############socket error############# \n");
                        goto finish;
                }
        }
        // char bind_addr[] = NETWORK_ETH_NAME;
        // if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, bind_addr, sizeof(bind_addr)) < 0)
        // {
        //         SAT_DEBUG("==========\n");
        // }
        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-5 16:47:15
        ** 说明: 设置地址复用
        ***********************************************/
        {
                err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(optval));
                if (err < 0)
                {

                        printf("Fail to set rtp address reusable \n");
                        close(sock);
                        sock = -1;
                        goto finish;
                }

                err = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void *)&optval, sizeof(optval));
                if (err < 0)
                {
                        printf("Fail to set rtp address reusable \n");
                        close(sock);
                        sock = -1;
                        goto finish;
                }
        }

        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-5 16:47:42
        ** 说明: 绑定端口
        ***********************************************/
        {
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);
                addr.sin_addr.s_addr = htonl(INADDR_ANY);
                err = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
                if (err < 0)
                {
                        printf("[%s:%d]bing fail \n", __func__, __LINE__);
                        perror("bind");
                        close(sock);
                        sock = -1;
                        goto finish;
                }
        }

        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-5 16:49:23
        ** 说明: 设置本机无需接收
        ***********************************************/
        {
                err = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

                if (err < 0)
                {

                        printf("[%s:%d]Fail to join set loop \n", __func__, __LINE__);
                        close(sock);
                        sock = -1;
                        goto finish;
                }
        }

finish:
        return sock;
}

/****************************************************************
**@日期: 2022-09-16
**@作者: leo.liu
**@功能:关闭
*****************************************************************/
bool sat_socket_close(int socket_fd)
{
        if (socket_fd < 0)
        {

                return false;
        }

        close(socket_fd);
        return true;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-5 16:15:12
** 说明: 创建套接字，并且接入组播ip
***********************************************/
bool sat_socket_multicast_join(int socket_fd, const char *multicast_ip)
{
        int err = 0;
        /***********************************************
         ** 作者: leo.liu
         ** 日期: 2023-1-5 16:49:56
         ** 说明: 添加组播接口
         ***********************************************/
        /*
        {
                struct in_addr addr;
                addr.s_addr = htonl(INADDR_ANY);
                err = setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr));
                if(err < 0)
                {
                        perror("JIOIN FAILED:");
                        LOG_WHITE("Fail to join address group (%s)", multicast_ip);
                        //     return false;
                }
        }
        */
        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-5 16:49:56
        ** 说明: 加入组播
        ***********************************************/
        {
                struct ip_mreq mreq;
                mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                err = setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq, sizeof(mreq));

                if (err < 0)
                {
                        perror("JIOIN FAILED:");
                        LOG_WHITE("Fail to join address group (%s)", multicast_ip);
                        return false;
                }
        }
        return true;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-9 10:22:45
** 说明: read 超时
***********************************************/
static bool sat_socket_select(int fd, unsigned int wait_ms, bool read_flg, bool write_flg)
{
        bool ret = true;

        if (wait_ms <= 0)
        {

                return true;
        }

        fd_set read_fdset;
        fd_set write_fdset;

        struct timeval timeout;
        FD_ZERO(&read_fdset);
        FD_ZERO(&write_fdset);
        FD_SET(fd, &read_fdset);
        FD_SET(fd, &write_fdset);

        timeout.tv_sec = wait_ms / 1000;
        timeout.tv_usec = wait_ms % 1000 * 1000;

        do
        {

                ret = select(fd + 1, read_flg ? &read_fdset : NULL, write_flg ? &write_fdset : NULL, NULL, &timeout);

                // select会阻塞直到检测到事件或者超时
                //  如果select检测到可读事件发送，则此时调用read不会阻塞
        } while ((ret < 0) && errno == EINTR);

        if (ret == 0)
        {

                ret = false;

                // errno				= ETIMEDOUT;
        }
        else if (ret == 1)
        {

                ret = true;
        }

        return ret;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-6 8:37:33
** 说明: tcp server open
***********************************************/
bool sat_socket_tcp_open(int *fd, int port, int max_client)
{
        int tcp_fd = -1;
        int err = 0;

        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-6 8:57:32
        ** 说明: 创建tcp套接字
        ***********************************************/
        {

                tcp_fd = sat_socket_open(SOCK_STREAM, port);

                if (tcp_fd < 0)
                {

                        printf("socket open failed\n");
                        return false;
                }
        }

        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-6 9:5:54
        ** 说明: 监听套接字
        ***********************************************/
        if (max_client > 0)
        {

                err = listen(tcp_fd, max_client);

                if (err < 0)
                {
                        close(tcp_fd);
                        printf("listen socket error \n");
                        return false;
                }
        }

        *fd = tcp_fd;
        return true;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-6 9:8:19
** 说明: server fd 同意连接
***********************************************/
int sat_socket_tcp_accept(int server_fd, struct sockaddr_in *client_addr, unsigned int timeout_ms)
{
        int client_fd = -1;
        unsigned int len = sizeof(struct sockaddr_in);
        int ret = 0;

        if (timeout_ms > 0)
        {

                fd_set accept_fdset;

                struct timeval timeout;
                FD_ZERO(&accept_fdset);
                FD_SET(server_fd, &accept_fdset);

                timeout.tv_sec = timeout_ms / 1000;
                timeout.tv_usec = timeout_ms % 1000 * 1000;

                do
                {
                        ret = select(server_fd + 1, &accept_fdset, NULL, NULL, &timeout);
                } while (ret < 0 && errno == EINTR);

                if (ret == -1)
                {

                        printf("accept wait failed \n");
                        return -1;
                }

                else if (ret == 0)
                {

                        // errno				= ETIMEDOUT;
                        // printf("accept wait timemout \n");
                        return -1;
                }
        }

        /***********************************************
        ** 作者: leo.liu
        ** 日期: 2023-1-7 8:18:50
        ** 说明: 同意连接
        ***********************************************/
        {
                client_fd = accept(server_fd, (struct sockaddr *)client_addr, &len);

                if (client_fd < 0)
                {

                        printf("client socket accept failed\n");
                        return -1;
                }

                printf("one client succes(%s:%d) \n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
        }
        return client_fd;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-7 10:1:24
** 说明: 连接服务器
***********************************************/
bool sat_socket_tcp_connect(int socket_fd, const char *server_ip, int port, unsigned int wait_ms)
{
        int result = 0;
        bool res = false;

        if (wait_ms > 0)
        {

                sat_socket_block_enable(socket_fd, false);
        }

        struct sockaddr_in ser_addr;
        memset(&ser_addr, 0, sizeof(ser_addr));

        ser_addr.sin_family = AF_INET;
        ser_addr.sin_port = htons(port);
        ser_addr.sin_addr.s_addr = inet_addr(server_ip);
        result = connect(socket_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));

        if (result < 0 && errno == EINPROGRESS)
        {

                fd_set connect_fdset;

                struct timeval timeout;
                FD_ZERO(&connect_fdset);
                FD_SET(socket_fd, &connect_fdset);

                timeout.tv_sec = wait_ms / 1000;
                timeout.tv_usec = wait_ms % 1000 * 1000;

                do
                {
                        /* 一旦连接建立，套接字就可写 */
                        result = select(socket_fd + 1, NULL, &connect_fdset, NULL, &timeout);
                } while (result < 0 && errno == EINTR);

                if (result == 0)
                {
                        // errno				= ETIMEDOUT;
                        printf("[%s:%d] socket connect select timeout\n", __func__, __LINE__);
                        // res = true;
                        goto finish;
                }
                else if (result < 0)
                {

                        printf("[%s:%d] socket connect select timeout\n", __func__, __LINE__);
                        goto finish;
                }
                else if (result == 1)
                {

                        /* ret返回为1，可能有两种情况，一种是连接建立成功，一种是套接字产生错误
                         * 此时错误信息不会保存至errno变量中（select没出错）,因此，需要调用
                         * getsockopt来获取 */
                        int err;
                        socklen_t socklen = sizeof(err);
                        int sockoptret = getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &err, &socklen);

                        if (sockoptret == -1)
                        {

                                printf("[%s:%d] socket connect select timeout\n", __func__, __LINE__);
                                goto finish;
                        }

                        if (err == 0)
                        {

                                // printf("[%s:%d] socket connect select ssuccess\n", __func__, __LINE__);
                                res = true;
                        }
                        else
                        {

                                // errno				= err;
                                LOG_ERROR("socket connect select timeout\n");
                                res = false;
                        }
                }
        }
        else
        {

                printf("[%s:%d] socket connect [%s:%d] failed\n", __func__, __LINE__, server_ip, port);
        }
finish:
        if (wait_ms > 0)
        {

                sat_socket_block_enable(socket_fd, true);
        }

        return res;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-6 9:8:19
** 说明: server fd 接收数据
***********************************************/
int sat_socket_tcp_receive(int client_fd, unsigned char *data, int data_len, int timeout_ms)
{
        int len = 0;

        if (sat_socket_select(client_fd, timeout_ms, true, false) == false)
        {

                printf("[%s:%d]read tiemout \n", __func__, __LINE__);
                return 0;
        }
        len = recv(client_fd, data, data_len, 0);
        if (len == 0)
        {

                // LOG_WHITE("tcp client socket closed \n");
                return 0;
        }

        if (len < 0)
        {

                printf("tcp client socket recv error :%d - buffer error:%d\n", len, errno);
                return -1;
        }

        return len;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-7 9:46:26
** 说明: tcp 发送数据
***********************************************/
bool sat_socket_tcp_send(int socket_fd, unsigned char *data, int data_len, unsigned int timeout_ms)
{
        int result = 0;
        int send_len = 0;
        int packet_size = 0;

        while (1)
        {
                if (sat_socket_select(socket_fd, timeout_ms, false, true) == false)
                {

                        printf("[%s:%d]  tcp send tiemout \n", __func__, __LINE__);
                        return false;
                }

                packet_size = data_len > NETWORK_PACKAET_MAX ? NETWORK_PACKAET_MAX : data_len;

                result = send(socket_fd, &data[send_len], packet_size, MSG_NOSIGNAL);

                if (result <= 0)
                {

                        printf("[%s:%d] send failed\n", __func__, __LINE__);
                        return false;
                }

                data_len -= result;
                send_len += result;

                if (data_len <= 0)
                {

                        return true;
                }

                // usleep(1000);
        }

        return false;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-9 13:39:14
** 说明: UDP打开套接字
***********************************************/
bool sat_socket_udp_open(int *socket_fd, int port, bool broadcast)
{
        /****************************************************************
        2022-09-16 author:leo.liu 说明:建立数据报套接字
        *****************************************************************/
        int fd = sat_socket_open(SOCK_DGRAM, port);
        if (fd < 0)
        {

                printf("#############socket error############# \n");
                return false;
        }

        if (broadcast == true)
        {
                int optval = 1;
                setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char *)&optval, sizeof(int));
        }

        *socket_fd = fd;
        return true;
}

/****************************************************************
**@日期: 2022-09-16
**@作者: leo.liu
**@功能:发送探测消息
*****************************************************************/
bool sat_socket_udp_send(int socket_fd, const char *data, size_t data_len, const char *ip, int port, int timeout_ms)
{
        int ret = -1;

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);

        int result = 0;
        int send_len = 0;
        int packet_size = 0;

        while (1)
        {

                if (sat_socket_select(socket_fd, timeout_ms, false, true) == false)
                {

                        printf("[%s:%d]read tiemout \n", __func__, __LINE__);
                        return false;
                }

                packet_size = data_len > NETWORK_PACKAET_MAX ? NETWORK_PACKAET_MAX : data_len;

                result = sendto(socket_fd, &data[send_len], packet_size, 0, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in));

                data_len -= result;
                send_len += result;

                if (data_len <= 0)
                {

                        return true;
                }

                usleep(1000);
        }

        return ret < 0 ? false : true;
}

/****************************************************************
**@日期: 2022-09-20
**@作者: leo.liu
**@功能:接受一个消息
*****************************************************************/
int sat_socket_udp_receive(int socket_fd, char *data, size_t data_len, struct sockaddr_in *client_addr, int timeout_ms)
{
        int len = 0;
        socklen_t adder_len = sizeof(struct sockaddr_in);

        if (sat_socket_select(socket_fd, timeout_ms, true, false) == false)
        {

                //	printf("[%s:%d]read tiemout \n", __func__, __LINE__);
                return false;
        }

        len = recvfrom(socket_fd, data, data_len, 0, (struct sockaddr *)client_addr, (socklen_t *)&adder_len);

        if (len == 0)
        {

                printf("udp client socket closed \n");
                return 0;
        }

        if (len < 0)
        {

                printf("tcp client socket error \n");
                return -1;
        }

        return len;
}
/***********************************************
** 作者: leo.liu
** 日期: 2023-1-7 14:46:50
** 说明: 发现IP是否存在
***********************************************/
bool sat_network_ip_pings_check(const char *ip, int pings_count, int one_wait_sec)
{
        bool online = false;
        char cmd[128] = {0};
        sprintf(cmd, "ping -W %d -c %d %s", pings_count, one_wait_sec, ip);
        FILE *pfd = popen(cmd, "r");
        if (pfd == NULL)
        {
                return online;
        }

        char buffer[128] = {0};
        while (fgets(buffer, sizeof(buffer), pfd))
        {
                char *pstr = strstr(buffer, "ttl=");
                if (pstr != NULL)
                {
                        online = true;
                        break;
                }
                memset(buffer, 0, sizeof(buffer));
        }
        pclose(pfd);
        return online;
}
/***********************************************
** 作者: leo.liu
** 日期: 2023-1-7 14:46:50
** 说明: 杀死指定的进程
***********************************************/
bool sat_kill_task_process(const char *process_name)
{
#define MAX_BUFF_SIZE 1024
        bool result = false;
        char buffer[MAX_BUFF_SIZE];
        char cmd[128] = {0};
        sprintf(cmd, "ps aux | grep -v grep | grep -F \"%s\"", process_name);
        //    printf("\n[%s]\n", cmd);
        FILE *pipe = popen(cmd, "r");
        if (pipe == NULL)
        {
                return false;
        }

        printf("empty %s :\n", process_name);
        while (fgets(buffer, MAX_BUFF_SIZE, pipe) != NULL)
        {
                if (strstr(buffer, process_name) != NULL)
                {
                        int pid = 0;
                        char temp[256] = {0};
                        sscanf(buffer, "%d %s", &pid, temp);

                        memset(cmd, 0, sizeof(cmd));
                        sprintf(cmd, "kill -s 9 %d", pid);
                        system(cmd);
                        printf("kill %s pid is %d\n", process_name, pid);
                        result = true;
                }
        }

        return result;
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-5 15:21:6
** 说明: 通过连接asterisk 获取注册设备信息
***********************************************/
#define ASTERISK_REGISTER_NUM_MAX 20
#define ASTERISK_SHMEM_KEY 0x99999999
asterisk_register_info *asterisk_register_info_get(void)
{
        static asterisk_register_info *p_register_info = NULL;
        if (p_register_info == NULL)
        {
                int shmid = shmget(ASTERISK_SHMEM_KEY, sizeof(asterisk_register_info) * ASTERISK_REGISTER_NUM_MAX, IPC_CREAT | 0664);
                if (shmid >= 0)
                {
                        p_register_info = (asterisk_register_info *)shmat(shmid, NULL, 0);
                        memset(p_register_info, 0, sizeof(asterisk_register_info) * ASTERISK_REGISTER_NUM_MAX);
                }
        }

        return p_register_info;
}
