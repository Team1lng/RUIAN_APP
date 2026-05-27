#if 0
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

#ifndef likely
# define likely(x)        __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)        __builtin_expect(!!(x), 0)
#endif

struct block_desc
{
    uint32_t version;
    uint32_t offset_to_priv;
    struct tpacket_hdr_v1 h1;
};

struct ring
{
    struct iovec *rd;
    uint8_t *map;
    struct tpacket_req3 req;
};

static unsigned long packets_total = 0, bytes_total = 0;
static sig_atomic_t sigint = 0;

static void sighandler(int num)
{
    sigint = 1;
}

/* 初始化套接字，包括套接口创建、接收缓冲区的创建等 */
static int setup_socket(struct ring *ring, char *netdev)
{
    int err, i, fd, v = TPACKET_V3;
    struct sockaddr_ll ll;
    unsigned int blocksiz = 4096;//1 << 22,
    unsigned int framesiz = 2048;//1 << 11;
    unsigned int blocknum = 64;

    /* 创建套接口 */
    fd = socket(AF_PACKET, SOCK_RAW, htons(0xFFFF));
    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    /* 设置PACKET版本，有v1、v2和v3三个版本，默认是v1 */
    err = setsockopt(fd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
    if (err < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    memset(&ring->req, 0, sizeof(ring->req));
    ring->req.tp_block_size = blocksiz;
    ring->req.tp_frame_size = framesiz;
    ring->req.tp_block_nr = blocknum;
    ring->req.tp_frame_nr = (blocksiz * blocknum) / framesiz;
    ring->req.tp_retire_blk_tov = 60;
    ring->req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

    /* 创建ringBuf */
    err = setsockopt(fd, SOL_PACKET, PACKET_RX_RING, &ring->req, sizeof(ring->req));
    if (err < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    /* 将ringBuf映射到用户态 */
    ring->map = mmap(NULL, ring->req.tp_block_size * ring->req.tp_block_nr, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
    if (ring->map == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    /* 使用iovec向量的方式来访问缓冲区，为每个块创建一个向量，存储到ring->rd中 */
    ring->rd = malloc(ring->req.tp_block_nr * sizeof(*ring->rd));
    assert(ring->rd);
    /* 初始化向量，使用与每个块对应 */
    for (i = 0; i < ring->req.tp_block_nr; ++i)
    {
        ring->rd[i].iov_base = ring->map + (i * ring->req.tp_block_size);
        ring->rd[i].iov_len = ring->req.tp_block_size;
    }

    memset(&ll, 0, sizeof(ll));
    ll.sll_family = PF_PACKET;
    ll.sll_protocol = htons(ETH_P_ALL);
    ll.sll_ifindex = if_nametoindex(netdev);
    ll.sll_hatype = 0;
    ll.sll_pkttype = 0;
    ll.sll_halen = 0;

    /* 将这个原始套接字绑定到某个网口（netdev） */
    err = bind(fd, (struct sockaddr *) &ll, sizeof(ll));
    if (err < 0)
    {
        perror("bind");
        exit(1);
    }

    return fd;
}

/* 显示报文数据 */
static void display(struct tpacket3_hdr *ppd)
{
    uint8_t* buf = (uint8_t*)((uint8_t *)ppd + ppd->tp_mac);
    for(int i = 0 ; i < 68 ; i++)
    {
        printf("%02x ",buf[i]);
    }
    printf("\n");
    return ;


    /* 帧头部的地址加上MAC偏移量，就是以太网报文的地址 */
    struct ethhdr *eth = (struct ethhdr *) ((uint8_t *) ppd + ppd->tp_mac);
    struct iphdr *ip = (struct iphdr *) ((uint8_t *) eth + ETH_HLEN);

    if (eth->h_proto == htons(0xFFFF))
    {
        struct sockaddr_in ss, sd;
        char sbuff[NI_MAXHOST], dbuff[NI_MAXHOST];

        memset(&ss, 0, sizeof(ss));
        ss.sin_family = PF_INET;
        ss.sin_addr.s_addr = ip->saddr;
        /* 将源IP地址转换成主机名字 */
        getnameinfo((struct sockaddr *) &ss, sizeof(ss), sbuff, sizeof(sbuff), NULL, 0, NI_NUMERICHOST);

        memset(&sd, 0, sizeof(sd));
        sd.sin_family = PF_INET;
        sd.sin_addr.s_addr = ip->daddr;
        getnameinfo((struct sockaddr *) &sd, sizeof(sd), dbuff, sizeof(dbuff), NULL, 0, NI_NUMERICHOST);

        /* 打印出来地址信息 */
        printf("%s -> %s, ", sbuff, dbuff);
    }

    printf("rxhash: 0x%x\n", ppd->hv1.tp_rxhash);
}

static void walk_block(struct block_desc *pbd, const int block_num)
{
    int num_pkts = pbd->h1.num_pkts, i;
    unsigned long bytes = 0;
    struct tpacket3_hdr *ppd;

    /* 获取当前块中第一个帧 */
    ppd = (struct tpacket3_hdr *) ((uint8_t *) pbd + pbd->h1.offset_to_first_pkt);
    for (i = 0; i < num_pkts; ++i)
    {
        bytes += ppd->tp_snaplen;
        display(ppd);

        /* 获取下一个帧的位置 */
        ppd = (struct tpacket3_hdr *) ((uint8_t *) ppd + ppd->tp_next_offset);
    }

    packets_total += num_pkts;
    bytes_total += bytes;
}

static void flush_block(struct block_desc *pbd)
{
    pbd->h1.block_status = TP_STATUS_KERNEL;
}

static void teardown_socket(struct ring *ring, int fd)
{
    /* 销毁套接字 */
    munmap(ring->map, ring->req.tp_block_size * ring->req.tp_block_nr);
    free(ring->rd);
    close(fd);
}

int main(int argc, char **argp)
{
    int fd, err;
    socklen_t len;
    struct ring ring;
    struct pollfd pfd;
    unsigned int block_num = 0, blocks = 64;
    struct block_desc *pbd;
    struct tpacket_stats_v3 stats;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s INTERFACE\n", argp[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, sighandler);

    memset(&ring, 0, sizeof(ring));
    /* 初始化套接字 */
    fd = setup_socket(&ring, argp[argc - 1]);
    assert(fd > 0);

    /* 初始化poll参数 */
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    pfd.events = POLLIN | POLLERR;
    pfd.revents = 0;

    /* 进入poll的循环收包环节 */
    while (likely(!sigint))
    {
        pbd = (struct block_desc *) ring.rd[block_num].iov_base;

        /* 检查当前块头的状态，判断是否有数据，没有的话就进行poll */
        if ((pbd->h1.block_status & TP_STATUS_USER) == 0)
        {
            poll(&pfd, 1, -1);
            continue;
        }

        /* 有数据，遍历块里面的帧 */
        walk_block(pbd, block_num);
        /* 将块恢复为就绪状态 */
        flush_block(pbd);
        block_num = (block_num + 1) % blocks;
    }

    len = sizeof(stats);
    /* 获取报文统计信息，然后打印出来。 */
    err = getsockopt(fd, SOL_PACKET, PACKET_STATISTICS, &stats, &len);
    if (err < 0)
    {
        perror("getsockopt");
        exit(1);
    }

    fflush(stdout);
    printf("\nReceived %u packets, %lu bytes, %u dropped, freeze_q_cnt: %u\n", stats.tp_packets, bytes_total, stats.tp_drops, stats.tp_freeze_q_cnt);

    teardown_socket(&ring, fd);
    return 0;
}


#else

#include "lvgl.h"
#include <stdio.h>
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "ak_thread.h"
#include "ak_mem.h"

#include "layout_define.h"
#include "leo_api.h"
#include "user_data.h"
#include "layout_watch_dog.h"
#include "../api/xls/lang_xls.h"

/*TP_TYPE:0-->800*1280, 1-->1024*600*/
// #define TP_TYPE 1
//
/*
 * 功能：启用核心转储功能，允许程序崩溃时生成核心转储文件
 * 返回值：成功返回1，失败返回0
 */
// static int enableCoreDumpFunc()
// {
//     // 引入资源限制和错误处理所需的头文件
//     #include <sys/resource.h>
//     #include <errno.h>

//     // 设置资源类型为核心转储文件大小限制
//     int iRes = RLIMIT_CORE;
//     // 定义资源限制参数结构体
//     struct rlimit limitParam;

//     // 根据条件设置核心转储文件大小限制
//     // 当前逻辑：若条件为真（非零值），设置为无限制；否则禁止生成核心转储文件
//     // 注意：此处条件判断为硬编码的1，实际效果是始终启用无限制的核心转储
//     limitParam.rlim_cur = 1 ? RLIM_INFINITY : 0;
//     limitParam.rlim_max = 1 ? RLIM_INFINITY : 0;

//     // 应用核心转储文件大小限制设置
//     if (0 != setrlimit(iRes, &limitParam))
//     {
//         // 设置失败时输出错误信息并返回0
//         printf("Error: setrlimit failed, %s\n", strerror(errno));
//         return 0;
//     }
//     else
//     {
//         // 设置成功时配置核心转储文件命名格式和存储路径
//         // 生成的文件名将包含程序名(%e)、进程ID(%p)和时间戳(%t)
//         system("echo /tmp/coredump_%e_%p_%t > /proc/sys/kernel/core_pattern");
//         // 输出设置成功信息，显示当前核心转储文件大小限制
//         printf("Set coredump file size to %lu, path = /tmp\n", limitParam.rlim_cur);
//         return 1;
//     }

//     // 默认返回0（此代码行理论上不会被执行）
//     return 0;
// }

/**
 * 功能：打印LittlevGL图形库的内存使用状态信息
 * 描述：
 *   该函数通过调用lv_mem_monitor获取内存使用统计数据，
 *   并将关键指标格式化输出到标准输出，用于调试和性能分析
 * 参数：无
 * 返回值：无
 * 输出示例：
 *   used:  12345 ( 25 %), frag:  15 %, biggest free:  45678 used_cnt:123, max_used = 12345 free_size:37035 free_cnt:15 ,total_size:49380
 */
void memory_print(void)
{
    // 定义内存监控结构体，用于存储内存使用统计信息
    lv_mem_monitor_t mon;
    
    // 调用LittlevGL内存监控函数，填充内存使用数据到mon结构体
    lv_mem_monitor(&mon);
    
    // 格式化输出内存使用信息：
    // 1. 已使用内存大小（字节）
    // 2. 已使用内存百分比
    // 3. 内存碎片百分比
    // 4. 最大连续空闲内存块大小（字节）
    // 5. 已分配内存块数量
    // 6. 历史最高内存使用量（字节）
    // 7. 当前空闲内存大小（字节）
    // 8. 空闲内存块数量
    // 9. 总内存大小（字节）
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d used_cnt:%d, max_used = %d free_size:%d free_cnt:%d ,total_size:%d\n", 
           (int)mon.total_size - mon.free_size,  // 已使用内存 = 总内存 - 空闲内存
           mon.used_pct,                        // 已使用百分比
           mon.frag_pct,                        // 碎片百分比
           (int)mon.free_biggest_size,          // 最大空闲块大小
           mon.used_cnt,                        // 已分配块数量
           mon.max_used,                        // 历史峰值使用量
           mon.free_size,                       // 当前空闲大小
           mon.free_cnt,                        // 空闲块数量
           mon.total_size                       // 总内存大小
    );
}

/***********************************************
** 作者: leo.liu
** 日期: 2023-1-7 14:46:50
** 说明: 杀死指定的进程
***********************************************/
bool main_kill_task_process(const char *process_name)
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
        pclose(pipe);
        return result;
}

/**
 * LVGL 系统滴答定时器任务 - 维护 LVGL 图形库的时间基准
 * 此任务通过固定间隔更新系统时钟，确保 LVGL 能够正常处理动画、超时等时间相关功能
 * 
 * @param arg 线程参数（未使用）
 * @return 线程返回值（NULL）
 */
static void* lvgl_titck_task(void*arg)
{
    // 定义两个时间结构体用于计算时间差
    struct ak_timeval tv1,tv2;

    // 初始化时间基准
    ak_get_ostime(&tv1);
    ak_get_ostime(&tv2);
    while(1)
    {
         // 获取当前时间点 1
        ak_get_ostime(&tv1);
        
        // 计算两次采样之间的时间差（毫秒级）并更新到 LVGL 系统
        // 时间差计算：(tv1_sec*1000 + tv1_usec/1000) - (tv2_sec*1000 + tv2_usec/1000)
        lv_tick_inc(tv1.sec*1000 + tv1.usec/1000 - tv2.sec*1000 - tv2.usec/1000);
        
        // 更新时间基准为当前时间点
        ak_get_ostime(&tv2);
        
        // 任务休眠 5 毫秒，控制采样频率
        ak_sleep_ms(5);

        // 看门狗定时器喂狗操作
        // 注：实际循环周期约为 10ms（5ms 休眠 + 5ms 处理）
        // 当连续 1500 次（约 15s）未通过喂狗检查时触发系统重启
        if(watch_dog_wait() >= 1500)//平台限制，实际上循环一次的时间是10MS，超时10秒没喂狗，杀死进程，守护进程重新启动应用
		{
			// 输出看门狗超时错误信息
			printf("Watch dog timeout\n");
			
			// 终止主应用进程及备份进程，触发守护进程重启机制
            main_kill_task_process("ANYKA37E.BIN");
            main_kill_task_process("ANYKA37E_BACKUP.BIN");
		}
    }
    // 线程正常退出（理论上不会执行到此处）
    ak_thread_exit();
    return NULL;
}

/*************************************************************************
 * @description:  获取指定网卡的默认网关
 * @date   2023-07-08 10:21
 * @author xiaole
 * @return
    成功：1
    失败：0
 * @param
    interface:网卡接口
    gateway:网关地址暂存区
 **************************************************************************/
int GetDefaultGateway(const char *interface, char *gateway)
{
    char command[100];
    char line[100];
    FILE *fp;

    // 构建要执行的命令
    snprintf(command, sizeof(command), "ip route show dev %s | awk '/default/ {print $3}'", interface);

    // 执行命令并打开管道获取输出
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        return 0;
    }

    // 读取命令输出，获取网关地址
    if (fgets(line, sizeof(line) - 1, fp) != NULL)
    {
        // 去除可能的换行符
        line[strcspn(line, "\n")] = 0;
        strncpy(gateway, line, sizeof(line));
    }

    pclose(fp);
    return 1;
}

/**
 * 检测指定网卡的默认网关连通性
 * 通过执行系统ping命令测试与网关的连接，结果反映网络接口状态
 * 
 * @param interface 网卡名称，如"eth0"、"wlan0"
 * @return 执行结果：0表示ping成功，非0表示失败或超时
 */
int PingGateway(const char *interface)
{
#define PING_TIMEOUT 2 // Ping超时时间（秒），控制单次ping等待响应的最长时间

    int ret;                   // 存储系统命令执行结果
    char command[100];         // 存储构造的ping命令字符串
    char gateway[20];          // 存储获取的网关IP地址

    // 1. 获取指定网卡的默认网关IP
    //    通过系统API或配置文件解析获取网关地址
    GetDefaultGateway(interface, gateway);

    // 2. 构造ping命令
    //    -I：指定使用的网络接口
    //    -c 1：只发送1个ping包
    //    -W 2：设置超时时间为2秒
    //    > /dev/null 2>&1：将标准输出和错误输出重定向到空设备，静默执行
    snprintf(command, sizeof(command), "ping -I %s -c 1 -W %d %s > /dev/null 2>&1", interface, PING_TIMEOUT, gateway);

    // 3. 执行系统命令并返回结果
    //    返回值为shell的退出状态（0表示成功，非0表示失败）
    ret = system(command);
    return ret;
}

/**
 * WiFi健康检查任务 - 监控WiFi连接状态并执行恢复操作
 * 此任务周期性检查WiFi连接，在检测到异常时通过ping网关进一步确认，
 * 连续多次ping失败后执行网络重置脚本恢复连接
 * 
 * @param arg 线程参数（未使用）
 * @return 线程返回值（NULL）
 */
// static void* wifi_health_check_task(void*arg)
// {
//     int fail = 0;                 // 连续ping失败计数器
//     int fail_count = 5;           // 触发重置的失败阈值（5次）
    
//     // 无限循环，持续监控WiFi状态
//     while(1)
//     {
//         // 1. 快速检查WiFi连接状态
//         if(wifi_connection_status_sucess())
//         {
//             // 连接正常，休眠2秒后继续下一次检查
//             ak_sleep_ms(2000);
//             continue;
//         }

//         // WiFi连接状态异常，输出提示信息
//         printf("wifi connection status fail\n");
        
//         // 2. 通过ping网关进一步验证网络连通性
//         if (PingGateway("wlan0"))
//         {
//             // ping失败，增加失败计数
//             fail++;
//         }
//         else
//         {
//             // ping成功，重置失败计数器
//             fail = 0;
//         }

//         // 3. 检查是否达到重置阈值
//         if(fail >= fail_count)
//         {
//             // 连续失败达到阈值，输出提示并执行网络重置脚本
//             printf("ping_fail_count is 5\n");
//             system("/app/app/hi3881_reload.sh");
            
//             // 重置失败计数器
//             fail = 0;
//         }

//         // 每次循环结束休眠2秒，控制检查频率
//         ak_sleep_ms(2000);
//     }
    
//     // 线程正常退出（理论上不会执行到此处）
//     ak_thread_exit();
//     return NULL;
// }

#if 0
static void dma_mem_user_check(lv_task_t *task)
{
    unsigned long total_used = 0;

    ak_mem_get_used_total(MODULE_ID_VDEC, &total_used);
    printf("total_used is %llu\n",total_used);
}
#endif

/**
 * 升级文件检查与解压处理
 * 检测外部存储中的固件升级包并执行解压操作
 * 
 * @note 当前函数被空实现（直接返回），实际功能被注释掉
 * @note 代码中包含两组升级逻辑：注释部分为旧版逻辑，实际执行新版逻辑
 */
static void upgrade_file_check_decompression(void)
{
    // 函数直接返回，实际功能被禁用（可能用于开发调试阶段）
    return;
    
    // 检查TF卡根目录下是否存在固件升级包
    if(access("/mnt/tf/firmware", F_OK) == 0)
    {
        // 旧版升级逻辑（已禁用）
        #if 0
        // 1. 移除旧配置文件
        system("rm -f /etc/config/leo/ANYKA37EOS");
        
        // 2. 切换到临时目录并解压系统固件包
        chdir("/tmp/");
        printf("start pack\n");
        system("tar -xvf /etc/config/leo/ANYKA37EOS rom.bin sat_leo.ttf ANYKA37E.BIN");
        printf("finish pack\n");
        
        // 3. 修改文件权限并执行升级脚本
        system("chmod -R 777 /etc/config/leo/ANYKA37EOS");
        printf("start upgrade firmware\n");
        system("/tmp/update.nor.sh");
        #endif
        
        // 新版升级逻辑（当前启用）
        printf("find the firmware\n");
        
        // 1. 创建临时升级目录
        system("mkdir /tmp/upgrade");
        
        // 2. 将TF卡中的固件包解压到临时目录
        system("tar -xvf /mnt/tf/firmware -C /tmp/upgrade/");
    }
}

/**
 * 守护进程启动控制函数
 * 检查系统中守护进程的运行状态并按需启动/重启
 * 
 * @note 该函数确保系统中守护进程(daemon_client)以正确数量运行
 * @note 采用 ps aux + grep 组合方式检测进程，可能存在边缘情况误判
 */
static void start_daemon_process(void)
{
    // 1. 执行系统命令检测daemon_client进程数量
    //    - 使用ps aux获取所有进程信息
    //    - 通过grep过滤出daemon_client进程
    //    - 使用grep -v grep排除grep命令自身
    //    - wc -l统计最终结果行数
    FILE *fp = popen("ps aux | grep daemon_client | grep -v grep | wc -l", "r");  
    if (fp == NULL) {  
        perror("popen");  
        return;  
    }  
  
    // 2. 读取命令输出结果
    char buffer[128] = {0}; // 足够大的缓冲区存储命令输出  
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {  
        perror("fgets");  
        pclose(fp);  
        return;  
    }  
  
    // 3. 处理输出字符串（去除尾部换行符和空格）
    size_t len = strlen(buffer);  
    while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == ' ')) {  
        buffer[--len] = '\0';  
    }  
  
    // 4. 将字符串转换为整数（进程数量）
    int count = atoi(buffer);  
    pclose(fp);  
  
    // 5. 根据进程数量执行不同策略
    if (count > 1) {  
        // 情况1：存在多个守护进程实例（异常状态）
        // 处理方式：杀死所有实例并重新启动一个
        printf("start runing daemon_client processes.\n");  
        system("killall daemon_client");
        system("/app/app/daemon_client &");
    } 
    else if (count == 1) { 
        // 情况2：已有一个守护进程在运行（正常状态）
        // 处理方式：不做任何操作
        printf("daemon_client processes are running.\n");  
    } 
    else {
        // 情况3：没有守护进程在运行（异常状态）
        // 处理方式：启动一个新的守护进程
        printf("start runing daemon_client processes.\n");  
        system("/app/app/daemon_client &");
    }  
}

extern void upgrade_check_firmware(void);







//线程有系统注册任务 时间任务 定时返回屏保页面任务

int main(int argv,char** argc)
{
   
/**
 * 系统初始化与启动流程
 * 负责设备初始化、网络配置、图形界面启动及后台服务管理
 */

    // WiFi初始化代码（当前被禁用）
#if 0
    // 启动WiFi supplicant守护进程（使用nl80211驱动，配置文件路径为/etc/config/wpa_supplicant.conf）
    system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B &");
    // 为WiFi接口分配动态IP地址
    system("udhcpc -i wlan0 &");
#endif
    // 打印系统启动信息（日期、时间、语言设置）
    printf("\n\n&&&&&&&&&&&&日期=>%s,时间=>%s&&&&&&&123&&&&&&1&&&&&\n\n",__DATE__,__TIME__);
    printf("\n\n&&&&&&&&&&&&语言=>%d&&&&&&&123&&&&&&1&&&&&\n\n",user_data_get()->user_language);

    
    start_daemon_process();// 启动系统守护进程（确保关键服务正常运行）    
    signal(SIGPIPE, SIG_IGN);// 忽略SIGPIPE信号（避免因向已关闭的socket写入数据导致程序崩溃）
    upgrade_file_check_decompression();// 检查升级文件并执行解压操作（如果存在）lv_obj_set_id
    upgrade_check_firmware(); // 检查固件升级需求并执行升级流程
    system("echo 0 > /proc/sys/vm/oom_dump_tasks");// 禁用OOM（Out of Memory）时的任务转储（减少系统开销）
    system("insmod /usr/modules/ak_eth.ko yt8510_rate_model=0x01");// 加载以太网驱动模块（yt8510速率模式设置为0x01）
    
    // 网络缓冲区大小配置
    // system("echo 10485760 > /proc/sys/net/core/rmem_default ");
    // system("echo 10485760 > /proc/sys/net/core/rmem_max");
    // system("echo 25971520 > /proc/sys/net/core/wmem_default ");
    // system("echo 25971520 > /proc/sys/net/core/wmem_max");
	
    // 启用以太网接口
    system("ifconfig eth0 up");
	system("ifconfig eth1 up");
    
    // 根据不同平台选择加载触摸屏驱动
    #ifndef _PLATFORM_800_1280
        // 非800x1280平台加载ts_gsl驱动
        system("insmod /usr/modules/ts_gsl.ko");
    #else
        #ifndef _PLATFORM_8inch
            // 800x1280且非8英寸平台加载goodix驱动（指定配置文件路径）
            system("modprobe goodix.ko config_file=/usr/sbin/goodix.cfg");
        #else
            // 8英寸平台加载ts_gsl驱动
            system("insmod /usr/modules/ts_gsl.ko");
        #endif
    #endif

	backlight_open(1);              
    user_data_init();               
	init_language_xls_info();       
    lv_init();                      
    lv_port_disp_init();            
    lv_port_indev_init();           
    //ak_ats_start(8192);
    //调试工具，发行时要关闭
	// enableCoreDumpFunc();
    
	det_key_pin_init();   //按键初始化
    leo_api_init();       //板级初始化



    tuya_wifi_sdk_init(IPC_APP_PID,user_data_get()->wifi.wifi_open_flag);  //涂鸦的wifi初始化


 //   lv_task_create(dma_mem_user_check, 100, LV_TASK_PRIO_HIGH, NULL);
    
    // 创建LVGL时钟更新线程（维持图形界面的时间基准）
    ak_pthread_t pthread_id;
    ak_thread_create(&pthread_id, lvgl_titck_task, NULL,ANYKA_THREAD_MIN_STACK_SIZE, -1);

    #ifdef WS73V100_WIFI
        // ak_pthread_t pthread_id2;
        // ak_thread_create(&pthread_id2, wifi_health_check_task, NULL,ANYKA_THREAD_MIN_STACK_SIZE, -1);
    #endif

	//int count = 0;
    while(1)
    {

        lv_task_handler();//lvgl
        
      
        static key_device key_last = 0;         // 记录上一次检测到的按键值
        key_device key = det_key_pin_call();    // 读取当前按键状态
        
       
        if(key != false && key_last != key) {
            key_last = key;                     // 更新按键状态记录
        }
        
        
        else if(key == false && key_last != false)
            {  
                // 输出按键值  
                printf("key value = %d\n",key_last);
                
                // 发送按键事件到事件处理系统
                extern bool key_call_event_push(char arg1,char arg2);
                key_call_event_push(key_last,0);
                
                // 播放触摸反馈音效
                touch_sound_play();
                
                // 重置按键状态记录
                key_last = key;
            }

          
                ak_sleep_ms(5);//心跳
                      
            // count++;
            // if(count > 100)
            // {
            // 	count = 0;
                    //system("sync");  // 同步文件系统缓存
                    //system("echo 3 > /proc/sys/vm/drop_caches");
                    //system("free");
                    //memory_print();
            //  }
        }
	

}

#endif



