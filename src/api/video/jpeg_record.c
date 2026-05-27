#include "video_decode.h"
#include "ak_common.h"
#include "ak_venc.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "string.h"
#include "ak_common_graphics.h"
#include "../../include/tuya/tuya_ipc_api.h"
#include "../../layout/user_data.h"


#include "ak_tde.h"
#define JPEG_WIDTH 640                  // 定义JPEG图像的宽度为640像素
#define JPEG_HEIGHT 360                 // 定义JPEG图像的高度为360像素

extern bool video_auto_record_falg ;    // 自动录制视频标志
extern bool tuya_motion_record;         // 涂鸦移动检测录制标志

static bool jpeg_record_task_run = false;  // 静态变量：JPEG录制任务运行状态标志

// 获取原始视频数据函数，尝试100次获取YUV420P格式数据
static bool jpeg_get_raw_video_data(unsigned char* addres,int width,int height)
{
    int ms = 100;                       // 设置获取数据的最大尝试次数为100
    extern bool video_raw_get(unsigned char*,int ,int,int);  // 声明外部函数：获取原始视频数据
    while(video_raw_get(addres, width , height,GP_FORMAT_YUV420P) == false)  // 循环尝试获取数据
    {
        ak_sleep_ms(1);                 // 每次尝试间隔1毫秒
        ms--;                           // 剩余尝试次数减1
        if(ms == 0)                     // 尝试次数用尽
        {
            return false;               // 获取数据失败，返回false
        }
    }
    return true;                        // 成功获取数据，返回true
}

// 初始化并打开JPEG编码器，配置编码器参数
static int jpeg_encode_open(int width,int heigh)
{
    struct venc_param ve_param;         // 定义视频编码参数结构体变量
    ve_param.width  = width;            // 设置编码视频宽度
    ve_param.height = heigh;            // 设置编码视频高度
    ve_param.fps    = 25;               // 设置编码帧率为25fps
    ve_param.goplen = 50;               // 设置I帧间隔为50帧
    ve_param.target_kbps = 800;         // 设置目标码率为800kbps
    ve_param.max_kbps     = 1024;       // 设置最大码率为1024kbps
    ve_param.br_mode      = BR_MODE_VBR;// 设置码率控制模式为可变比特率(VBR)
    ve_param.minqp        = 25;         // 设置最小量化参数为25
    ve_param.maxqp        = 50;         // 设置最大量化参数为50
    ve_param.initqp       = (ve_param.minqp + ve_param.maxqp)/2;  // 初始量化参数取中间值
    ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;  // 设置JPEG质量等级为默认值
    ve_param.chroma_mode = CHROMA_4_2_0;         // 设置色度格式为4:2:0
    ve_param.max_picture_size = 0;               // 设置最大图片尺寸为默认值
    ve_param.enc_level     = 50;                 // 设置编码级别为50
    ve_param.smart_mode    = 0;                  // 禁用智能编码模式
    ve_param.smart_goplen  = 100;                // 智能模式下的GOP长度
    ve_param.smart_quality = 50;                 // 智能模式下的质量参数
    ve_param.smart_static_value = 0;             // 智能模式下的静态值
    ve_param.enc_out_type = MJPEG_ENC_TYPE;      // 设置编码输出类型为MJPEG
    ve_param.profile = PROFILE_JPEG;             // 设置编码配置文件为JPEG

    int jpeg_handle_id = -1;         // 初始化JPEG编码器句柄为-1(无效值)
    ak_venc_open(&ve_param, &jpeg_handle_id);  // 打开编码器并获取句柄

    return jpeg_handle_id;           // 返回编码器句柄ID
}

// 将JPEG数据流写入指定文件，确保数据同步到磁盘
static bool jpeg_stream_write_file(unsigned char* data,int size,const char* file_path)
{
    int fd = open(file_path,O_WRONLY|O_CREAT);  // 打开文件，若不存在则创建
    if(fd < 0)                        // 检查文件打开是否成功
    {
        return false;                 // 文件打开失败，返回false
    }

    write(fd,data,size);              // 写入JPEG数据流到文件
    fsync(fd);                        // 强制将数据同步到磁盘
    close(fd);                        // 关闭文件

    return true;                      // 写入成功，返回true
}

/**
 * 使用指定编码器将原始视频帧编码为JPEG并写入文件
 * 
 * @param handle_id 编码器句柄ID
 * @param addres    原始视频数据地址(YUV420P格式)
 * @param frame_size 视频帧大小(字节)
 * @param file_path 输出JPEG文件路径
 * @return 成功返回true，失败返回false
 */
static bool jpeg_encode_write_frame(int handle_id,unsigned char*addres,int frame_size,const char* file_path)
{
    bool result = false;                // 操作结果标志
    struct video_stream stream = {0};   // 初始化视频流结构体
    printf("[%s:%d] file:%s \n",__func__,__LINE__,file_path);  // 打印函数调用信息
    
    // 调用编码器进行帧编码
    if(ak_venc_encode_frame(handle_id, addres, frame_size, NULL,&stream) == 0)
    {
        printf("[%s:%d] file:%s \n",__func__,__LINE__,file_path);  // 编码成功提示
        
        // 将编码后的JPEG数据流写入文件
        result = jpeg_stream_write_file(stream.data,stream.len,file_path);
        
        // 释放编码器分配的视频流资源
        ak_venc_release_stream(handle_id,&stream);
    }
    else
    {
        // 打印编码失败信息
        printf("[%s:%d] video encode frame mjpeg fail :%s \n",__func__,__LINE__,file_path);
    }
    
    printf("[%s:%d] file:%s \n",__func__,__LINE__,file_path);  // 函数结束提示
    return result;                  // 返回操作结果
}

bool tuya_sent_flag = false;        // 涂鸦云推送标志，控制是否向涂鸦云发送门铃事件
extern bool is_online_tuya_cloud(void);  // 声明外部函数：检查是否连接到涂鸦云

/**
 * JPEG录制任务线程函数
 * 
 * @param arg 文件路径参数
 * @return 始终返回NULL
 * 
 * 该线程负责获取原始视频数据，编码为JPEG并保存到文件，支持触发涂鸦云推送
 */
static void* jpeg_record_task(void* arg)
{
    const char*file_path = (char*)arg;  // 获取文件路径参数
    
    // 计算YUV420P格式帧大小(width*height*3/2)
    int frame_size = JPEG_WIDTH*JPEG_HEIGHT*3/2;
    
    // 分配DMA内存用于存储原始视频数据
    unsigned char* addres = ak_mem_dma_alloc(MODULE_ID_VENC, frame_size);
    // printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__,frame_size);
    
    bool result = false;                // 操作结果标志
    
    // 使用do-while(0)结构实现一次性执行的代码块，便于错误处理
    do
    {
        // 获取原始视频数据
        if(jpeg_get_raw_video_data(addres,JPEG_WIDTH,JPEG_HEIGHT) == false)
        {
            printf("[%s:%d] get raw video fail \n",__func__,__LINE__);
            break;                      // 获取失败则退出处理
        }
        
        // 打开JPEG编码器
        int hande_id = jpeg_encode_open(JPEG_WIDTH,JPEG_HEIGHT);
        if(hande_id == -1)
        {
            printf("[%s:%d] encode frame open fail \n",__func__,__LINE__);
            break;                      // 编码器打开失败则退出
        }
        
        // 编码并写入JPEG文件
        if(jpeg_encode_write_frame(hande_id,addres,frame_size,file_path) == false)
        {
            printf("[%s:%d] encode frame open fail \n",__func__,__LINE__);
            break;                      // 写入失败则退出
        }
        
        // 检查是否需要向涂鸦云推送门铃事件
        if((user_data_get()->motion.record == 1) && (tuya_sent_flag == true)){
            if(is_online_tuya_cloud() == true){  // 检查是否在线
                tuya_sent_flag = false;           // 重置推送标志
                
                // 再次编码帧数据用于推送
                struct video_stream stream;
                if (ak_venc_encode_frame(hande_id, addres, frame_size, NULL, &stream) == 0) 
                    // 向涂鸦云推送门铃按下事件及JPEG图像
                    tuya_ipc_notify_door_bell_press((char *) stream.data, stream.len,NOTIFICATION_CONTENT_JPEG);
            }
        }
        
        // 关闭编码器
        ak_venc_close(hande_id);
        result = true;                  // 设置操作成功标志
    }while(0);                          // 确保代码块只执行一次
    
    // 释放DMA分配的内存
    if(addres != NULL)
    {
        ak_mem_dma_free(addres);
    }
    
    // 打印录制结果
    printf("\n jpeg %s record %s \n",file_path,result?"success":"fail");
    
    // 推送JPEG录制事件
    extern bool record_jpeg_event_push(bool is_finish);
    record_jpeg_event_push(result);
    
    // 标记录制任务已停止
    jpeg_record_task_run = false;
    
    // 退出线程
    ak_thread_exit();
    return NULL;
}

/**
 * 启动JPEG录制任务
 * 
 * @param file_path 输出JPEG文件路径
 * @return 成功启动返回true，已有任务运行返回false
 * 
 * 该函数创建一个新线程执行JPEG录制任务
 */
bool jpg_record(const char*file_path)
{
    // 检查是否已有录制任务在运行
    if(jpeg_record_task_run == true)
    {
        printf("jpeg record thread runing \n");
        return false;
    }
    
    // 复制文件路径到静态缓冲区
    static char jpeg_file_path[128] = {0};
    strcpy(jpeg_file_path,file_path);
    
    // 标记录制任务正在运行
    jpeg_record_task_run = true;
    
    // 创建新线程执行录制任务
    ak_pthread_t thread_id;
    printf("file_path is %s\n",file_path);
    ak_thread_create(&thread_id, jpeg_record_task, jpeg_file_path, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    
    // 设置线程为分离状态，允许系统自动回收资源
    ak_thread_detach(thread_id);
    return true;
}

/**
 * 关闭JPEG录制任务
 * 
 * @return 成功关闭返回true，无任务运行返回false
 * 
 * 该函数通过标志位控制录制任务停止
 */
bool close_jpg_record(void){
    // 检查是否有录制任务在运行
    if(jpeg_record_task_run == false){
        return false;
    }
    
    // 标记录制任务停止
    jpeg_record_task_run = false;
    return true;
}

/**
 * 检查JPEG录制任务状态
 * 
 * @return 正在录制返回true，否则返回false
 */
bool is_jpg_record_ing(void)
{
    return jpeg_record_task_run;
}

static bool sent_tuya_task_run = false;    // 静态变量：涂鸦推送任务运行状态标志

/**
 * 涂鸦云推送录制任务线程函数
 * 
 * @param arg 文件路径参数
 * @return 始终返回NULL
 * 
 * 该线程负责获取视频数据、编码为JPEG并根据条件向涂鸦云推送事件
 */
static void* sent_tuya_record_task(void* arg)
{
    const char*file_path = (char*)arg;  // 获取文件路径参数
    printf("@@@@@@@@>>>>>>>>>:%s %d:\n",__func__,__LINE__);  // 打印函数调用信息

    // 计算YUV420P格式帧大小(width*height*3/2)
    int frame_size = JPEG_WIDTH*JPEG_HEIGHT*3/2;
    
    // 分配DMA内存用于存储原始视频数据
    unsigned char* addres = ak_mem_dma_alloc(MODULE_ID_VENC, frame_size);
    printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__,frame_size);  // 打印内存分配信息
    
    bool result = false;                // 操作结果标志
    
    // 使用do-while(0)结构实现一次性执行的代码块，便于错误处理
    do
    {
        // 检查自动录制标志，若未开启则退出处理
        if((video_auto_record_falg == false))
            break;

        // 获取原始视频数据
        if(jpeg_get_raw_video_data(addres,JPEG_WIDTH,JPEG_HEIGHT) == false)
        {
            break;  // 获取失败则退出处理
        }

        // 打开JPEG编码器
        int hande_id = jpeg_encode_open(JPEG_WIDTH,JPEG_HEIGHT);
        if(hande_id == -1)
        {
            break;  // 编码器打开失败则退出
        }

        // 编码并写入JPEG文件
        if(jpeg_encode_write_frame(hande_id,addres,frame_size,file_path) == false)
        {
            break;  // 写入失败则退出
        }

        // 定义视频流结构体用于存储编码后数据
        struct video_stream stream;
        
        // 调用编码器进行帧编码
        int a = ak_venc_encode_frame(hande_id, addres, frame_size, NULL, &stream);
        if (a == 0) {  // 编码成功
            if(tuya_motion_record)  // 检测到移动事件
            {
                // 向涂鸦云推送移动检测事件及JPEG图像
                tuya_ipc_notify_with_event((char *) stream.data, stream.len, NOTIFICATION_CONTENT_JPEG,NOTIFICATION_NAME_MOTION);
                tuya_motion_record = false;  // 重置移动检测标志
            }
            else  // 非移动检测事件(门铃事件)
            {
                // 向涂鸦云推送门铃按下事件及JPEG图像
                tuya_ipc_notify_door_bell_press((char *) stream.data, stream.len,NOTIFICATION_CONTENT_JPEG);
            }
        }

        // 释放编码器分配的视频流资源
        ak_venc_release_stream(hande_id,&stream);
        
        // 关闭编码器
        ak_venc_close(hande_id);

        result = true;  // 设置操作成功标志
    }while(0);  // 确保代码块只执行一次

    // 释放DMA分配的内存
    if(addres != NULL)
    {
        ak_mem_dma_free(addres);
    }
    
    // 打印录制结果
    printf("\n jpeg %s record %s \n",file_path,result?"success":"fail");
    
    // 推送JPEG录制事件
    extern bool record_jpeg_event_push(bool is_finish);
    record_jpeg_event_push(result);
    
    // 标记涂鸦推送任务已停止
    sent_tuya_task_run = false;
    
    // 退出线程
    ak_thread_exit();
    return NULL;
}

/**
 * 启动向涂鸦云推送的JPEG录制任务
 * 
 * @param file_path 输出JPEG文件路径
 * @return 成功启动返回true，已有任务运行返回false
 * 
 * 该函数创建一个新线程执行涂鸦推送录制任务
 */
bool sent_tuya_record(const char*file_path)
{
    // 检查是否已有涂鸦推送任务在运行
    if(sent_tuya_task_run == true)
    {
        printf("sent tuya record thread runing \n");
        return false;
    }

    // 复制文件路径到静态缓冲区
    static char jpeg_file_path[128] = {0};
    strcpy(jpeg_file_path,file_path);
    
    // 标记涂鸦推送任务正在运行
    sent_tuya_task_run = true;

    // 创建新线程执行涂鸦推送录制任务
    ak_pthread_t thread_id;
    ak_thread_create(&thread_id, sent_tuya_record_task, jpeg_file_path, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    
    // 设置线程为分离状态，允许系统自动回收资源
    ak_thread_detach(thread_id);
    return true;
}

