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
#include "avilib.h"
#include "ak_tde.h"
#include <sys/time.h>
#include <netinet/in.h>
#include "queue.h"
#include "leo_api.h"
#include "stdlib.h"
/**
 * AVI帧缓冲区最大数量
 */
#define AVI_FRAME_BUFFER_MAX 16

/**
 * 内存使用统计变量
 */
static int mem_free = 0;      // 内存释放次数
static int mem_alloc = 0;     // 内存分配次数
static int video_insert_count = 0;  // 视频帧插入次数
static int audio_insert_count = 0;  // 音频帧插入次数
static int video_del_count = 0;     // 视频帧删除次数
static int video_release_count = 0; // 视频帧释放次数
static int audio_del_count = 0;     // 音频帧删除次数
static int audio_release_count = 0; // 音频帧释放次数

/**
 * AVI帧信息结构
 */
typedef struct
{
    unsigned long long pts;     // 时间戳(毫秒)
    unsigned char* data;        // 帧数据
    unsigned int len;           // 数据长度
}avi_frame_info;

/**
 * AVI帧节点结构(包含双向链表指针和帧信息)
 */
typedef struct
{
    void* prev;                 // 前驱节点指针
    void* next;                 // 后继节点指针
    avi_frame_info frame;       // 帧信息
}avi_frame;

/**
 * AVI帧管理全局变量
 */
static avi_frame avi_frame_buffer[AVI_FRAME_BUFFER_MAX];  // 帧缓冲区
static queue_s avi_frame_queue_free;                       // 空闲帧队列
static queue_s avi_video_queue_head;                       // 视频帧队列头
static queue_s avi_audio_queue_head;                       // 音频帧队列头
static ak_mutex_t avi_queue_head_mutex;                    // 队列头互斥锁
static ak_mutex_t avi_queue_free_mutex;                    // 空闲队列互斥锁

/**
 * 视频录制状态标志
 */
static bool video_record_task_run = false;     // 录制任务运行标志
static bool video_record_thread_run = false;   // 录制线程运行标志
static bool video_record_ready = false;        // 录制准备就绪标志

/**
 * 创建新的AVI帧节点并分配内存
 * @param type 帧类型(视频帧相关)
 * @param data 源数据指针
 * @param len 源数据长度
 * @param is_video 是否为视频帧
 * @return 成功返回帧节点指针，失败返回NULL
 */
static avi_frame* avi_record_queue_node_new(char type,unsigned char* data,int len,bool is_video)
{
    avi_frame* node = NULL;
    
    // 从空闲队列获取节点
    ak_thread_mutex_lock(&avi_queue_free_mutex);
    if(queue_empty(&avi_frame_queue_free) == 0)
    {
        node = (avi_frame*)queue_delete_next(&avi_frame_queue_free);
    }
    ak_thread_mutex_unlock(&avi_queue_free_mutex);
    
    if(node == NULL)
    {
        return NULL;
    }
    
    // 释放原有内存
    if(node->frame.data != NULL)
    {
        ak_mem_free(node->frame.data);
        mem_free ++;
        //////printf("============mem_free count is %d\n",mem_free);
    }

    // 计算数据偏移量(视频I帧前添加起始码)
    int offset = 0;
    if((is_video == true)&&(type == 0 ))
    {
        offset = 4;
    }
    
    // 分配内存并复制数据
    node->frame.len  = len+offset;
    node->frame.data = (unsigned char*)ak_mem_alloc(MODULE_ID_VENC, node->frame.len);
    // //printf("ode->frame.len is %d\n",node->frame.len);
    mem_alloc ++;
    //////printf("============mem_alloc count is %d\n",mem_alloc);
    
    // 添加H.264起始码(0x00000001)
    if(offset == 4)
    {
        node->frame.data[0] = node->frame.data[1] = node->frame.data[2] = 0;
        node->frame.data[3] = 1;
    }
    
    // 复制数据并设置时间戳
    memcpy(&node->frame.data[offset],data,len);
    node->frame.pts = get_sys_ms();
    return node;
}

/**
 * 释放AVI帧节点并放回空闲队列
 * @param node 待释放的帧节点指针
 */
static void avi_record_queue_node_del(avi_frame* node)
{
    if(node != NULL)
    {
        // 释放帧数据内存
        if(node->frame.data != NULL)
        {
            ak_mem_free(node->frame.data);
            mem_free ++;
            //////printf("============mem_free count is %d\n",mem_free);
            node->frame.data = NULL;
        }
        
        // 重置帧长度
        node->frame.len = 0;
        
        // 将节点放回空闲队列
        ak_thread_mutex_lock(&avi_queue_free_mutex);
        queue_insert((queue_s*)node, &avi_frame_queue_free);
        ak_thread_mutex_unlock(&avi_queue_free_mutex);
    }
}

/**
 * 判断H.264帧类型
 * @param buffer 帧数据缓冲区
 * @param len 数据长度
 * @return 0=非关键帧 1=I帧 2=SPS 3=PPS
 */
/*
*   0:no key frame
*   1:frame
*   2:sps
*   3:pps
*/
int h264_is_keyframe(const unsigned char* buffer, int len)
{
        #define SPS_FRAME1 0x67
        #define KEY_FRAME2 0x27
        #define PPS_FRAME 0x68
        #define I_FRAME1   0x65
    //#define I_FRAME2   0x21
    // printf("%02x \n",*buffer);
  
        // 通过NAL头判断帧类型
        if(*(buffer) == SPS_FRAME1)  //ox67为 0110 0111(nal_unit_type为低5位，u(5)= 0 0111 = 7)
        { 
                return 2;  // SPS帧
        }

        else if(*(buffer) == PPS_FRAME) //ox68为 0110 1000 （nal_unit_type为低5位，u(5)= 0 1000 = 8）
        {  
                return 3;  // PPS帧 
        }

        else if(*(buffer) == I_FRAME1) //ox65为 0110 0101 （nal_unit_type为低5位，u(5)= 0 0101 = 5）
        { 
                return 1;  // I帧
        }
        else if(*(buffer) == KEY_FRAME2)
        {
            return 0xFF;  // 特殊关键帧类型
        }
        return 0;  // 非关键帧
}

/**
 * 写入AVI文件头
 * @param handle AVI文件句柄
 * @param fps 帧率
 * @param width 视频宽度
 * @param height 视频高度
 * @param has_audio 是否包含音频
 * @param h264 是否为H.264编码(0=H.264 1=MJPG)
 */
static void avi_write_head(avi_t* handle,double fps,int width,int height,bool has_audio,char h264)
{
    // 设置视频流参数
    AVI_set_video(handle,  width , height , fps,h264 == 0?"H264":"MJPG");
    
    // 设置音频流参数(如果有音频)
    if(has_audio == true)
    {
        AVI_set_audio(handle, 1, 16000, 16, 1,128);
    }
    
    // 更新AVI文件头
    avi_update_header(handle);
}

/**
 * 写入一帧音视频数据到AVI文件
 * @param handle AVI文件句柄
 * @param info 录制信息结构体指针
 * @return 成功返回true，失败返回false
 */
static bool avi_write_one_frame(avi_t* handle,const record_info* info)
{
    avi_frame* video_node = NULL;  // 视频帧节点
    avi_frame* audio_node = NULL;  // 音频帧节点

    bool i_frame = false;          // 是否找到I帧
    bool sps_frame = false;        // 是否找到SPS帧
    bool pps_frame = false;        // 是否找到PPS帧
    int key_frame = 0;             // 帧类型
    
    // 主循环：处理音视频帧直到任务停止
    while(video_record_task_run == true)
    {
        // 从队列获取视频帧
        if(video_node == NULL)
        {
            ak_thread_mutex_lock(&avi_queue_head_mutex);
            if(queue_empty(&avi_video_queue_head) == 0)
            {
                video_node = (avi_frame*)queue_delete_next(&avi_video_queue_head);
                video_del_count++;
                // printf("delete frame_count is %d\n",video_del_count);
            }
            ak_thread_mutex_unlock(&avi_queue_head_mutex);
        }

        // 从队列获取音频帧(如果需要)
        if((info->has_audio == true)&&(audio_node == NULL))
        {
            ak_thread_mutex_lock(&avi_queue_head_mutex);
            if(queue_empty(&avi_audio_queue_head) == 0)
            {
                audio_node = (avi_frame*)queue_delete_next(&avi_audio_queue_head);
                audio_del_count++;
                // printf("delete audio_count is %d\n",audio_del_count);
            }
            ak_thread_mutex_unlock(&avi_queue_head_mutex);
        }
        
        // 处理无效视频帧(非关键帧且需要关键帧)
        if((video_node == NULL)||((info->frame_type == 0)&&((key_frame = h264_is_keyframe(video_node->frame.data + 4,video_node->frame.len - 4)) == 0x00)))
        {
            if(video_node != NULL)
            {
                // 释放无效视频帧
                avi_record_queue_node_del(video_node);
                video_release_count++;
                // printf("===========%d==========release frame_count is %d\n",__LINE__,video_release_count);
                video_node = NULL;
            }
            
            // 释放音频帧(如果有)
            if(audio_node != NULL)
            {
                avi_record_queue_node_del(audio_node);
                audio_release_count++;
                // printf("release audio_count is %d\n",audio_release_count);
                audio_node = NULL;
            }
            
            // 短暂休眠后继续
            ak_sleep_ms(10);
            // printf("===========%d===========quit %s\n",__LINE__,__func__);
            continue;
        }

        // 等待音频帧(如果需要)
        if((info->has_audio == true)&&(audio_node == NULL))
        {
            ak_sleep_ms(10);
            continue;
        }

        // 音视频同步处理
        if((info->has_audio == false)||(abs(video_node->frame.pts - audio_node->frame.pts) < 40))
        {
            // 写入视频帧到文件
            AVI_write_frame(handle, (char*)video_node->frame.data ,video_node->frame.len, 1);
            video_release_count++;
            printf("===========%d==========release frame_count is %d\n",__LINE__,video_release_count);
            
            // 释放视频帧节点
            avi_record_queue_node_del(video_node);
            
            // 释放音频帧节点(如果有)
            if(audio_node != NULL)
            {
                avi_record_queue_node_del(audio_node);
                audio_release_count++;
                printf("release audio_count is %d\n",audio_release_count);
                audio_node = NULL;
            }
            
            video_node = NULL;
            
            // 记录关键帧状态(H.264)
            if(info->frame_type == 0)
            {
                if((key_frame == 1)&&(i_frame == false))
                {
                    i_frame = true;  // 标记找到I帧
                }
                else if((key_frame == 2)&&(sps_frame == false))
                {
                    sps_frame = true;  // 标记找到SPS帧
                }
                else if((key_frame == 3)&&(pps_frame == false))
                {
                    pps_frame = true;  // 标记找到PPS帧
                }
                
                // 所有关键帧类型都找到时返回成功
                if((i_frame == true)&&(sps_frame == true)&&(pps_frame == true))
                {
                    return true;
                }
                else if(key_frame == 0xFF)  // 特殊关键帧类型
                {
                    return true;
                }
            }
            else  // 非H.264编码直接返回成功
            {
                return true;
            }
        }
        else if(video_node->frame.pts < audio_node->frame.pts)
        {
            // 视频帧时间戳早于音频帧，丢弃视频帧
            avi_record_queue_node_del(video_node);
            video_release_count++;
            printf("===========%d==========release frame_count is %d\n",__LINE__,video_release_count);
            video_node = NULL;
            audio_node  =NULL;
        }
        else
        {
            // 视频帧时间戳晚于音频帧，丢弃音频帧
            avi_record_queue_node_del(audio_node);
            audio_release_count++;
            //printf("release audio_count is %d\n",audio_release_count);
            audio_node  =NULL;
            video_node = NULL;
        }
    }

    // 任务停止时释放剩余资源
    if (video_node != NULL)
    {
        avi_record_queue_node_del(video_node);
        video_node = NULL;
        video_release_count++;
        //printf("===========%d==========release frame_count is %d\n",__LINE__,video_release_count);
    }

    if (audio_node != NULL)
    {
        avi_record_queue_node_del(audio_node);
        audio_release_count++;
        //printf("release audio_count is %d\n",audio_release_count);
        audio_node = NULL;
    }
    // printf("===========%d===========quit %s\n",__LINE__,__func__);
    return false;
}

/**
 * 循环写入AVI帧到文件(支持批量处理)
 * @param handle AVI文件句柄
 * @param info 录制信息结构体指针
 * @param frame_total 输出总帧数计数指针
 */
static void avi_write_frame_loop(avi_t* handle,const record_info* info,unsigned int* frame_total)
{
    // 帧缓冲区(最多缓存10个视频帧和10个音频帧)
    avi_frame* video_node[10] = {NULL};
    avi_frame* audio_node[10] = {NULL};
    int audio_node_total = 0;  // 当前音频帧数量
    int video_node_total = 0;  // 当前视频帧数量
    
    // 获取当前时间
    struct timeval tv;
    gettimeofday(&tv,NULL);
    //bool falg = info->has_audio ;

    // unsigned long long cur_ms = tv.tv_sec*1000 + tv.tv_usec/1000;
    
    // 主循环：持续处理帧数据直到录制任务停止
    while(video_record_task_run == true)
    {
        // 从队列获取视频帧(最多10个)
        ak_thread_mutex_lock(&avi_queue_head_mutex);
        if(queue_empty(&avi_video_queue_head) == 0)
        {
            if (video_node[video_node_total] == NULL)
            {
                if (video_node_total < 9)
                {
                    video_node[video_node_total] = (avi_frame*)queue_delete_next(&avi_video_queue_head);
                    video_node_total++;
                    //printf("video_node_total %d\n",video_node_total);
                }
                else
                {
                    //printf("record video data full \n");
                }
            }
            video_del_count++;
            //printf("delete frame_count is %d\n",video_del_count);
        }
        ak_thread_mutex_unlock(&avi_queue_head_mutex);

        // 从队列获取音频帧(最多10个)
        ak_thread_mutex_lock(&avi_queue_head_mutex);
        if(queue_empty(&avi_audio_queue_head) == 0)
        {
            if (audio_node[audio_node_total] == NULL)
            {
                if (audio_node_total < 9)
                {
                    audio_node[audio_node_total] = (avi_frame*)queue_delete_next(&avi_audio_queue_head);
                    audio_node_total++;
                    //printf("audio_node_total %d\n",audio_node_total);
                }
                else
                {
                    //printf("record video data full \n");
                }
            }
            audio_del_count++;
            //printf("delete audio_count is %d\n",audio_del_count);
        }
        ak_thread_mutex_unlock(&avi_queue_head_mutex);

        // 注释掉的代码：用于丢弃过时帧(时间戳早于当前时间的帧)
        #if 0
        if((video_node[video_node_total -1] != NULL) &&(video_node_total > 0))
        {
            if((video_node[video_node_total -1]->frame.pts < cur_ms))
            {
                //printf("========skip frame============\n");
                for(int i = 0;i < video_node_total; i ++)
                {
                    avi_record_queue_node_del(video_node[i]);
                    video_node[i] = NULL;
                }
                video_node_total = 0;
                for(int i  =  0;i < audio_node_total; i ++)
                {
                    avi_record_queue_node_del(audio_node[i]);
                    audio_node[i] = NULL;
                }
                audio_node_total = 0;
                ak_sleep_ms(10);
                continue;
            }
        }
        #endif

        // 如果没有视频帧，短暂休眠后继续
        if((video_node[0] == NULL))
        {
            ak_sleep_ms(1);
            continue;
        }

        /*
        * 音视频同步逻辑：
        * 如果video_pts > audio_pts :音频在后，视频在前。则需要直接写入音频帧。
        * 如果video_pts < audio_pts :音频在前，视频在后，则直接将过滤掉视频帧，直到获取到大于音频帧的数据
        */
        if((audio_node[0] != NULL) && (video_node[0] != NULL)&& (video_node[0]->frame.pts > audio_node[0]->frame.pts))
        {
            // 视频时间戳大于音频时间戳，先写入音频帧
            AVI_write_audio(handle, (char*)audio_node[0]->frame.data, audio_node[0]->frame.len);
            avi_record_queue_node_del(audio_node[0]);
            audio_release_count++;
            //printf("release audio_count is %d\n",audio_release_count);
            
            // 调整音频帧缓冲区
            if (audio_node_total > 1)
            {
                memmove(&audio_node[0], &audio_node[1], (audio_node_total - 1)* sizeof(avi_frame*));
            }
            audio_node_total--;
            audio_node[audio_node_total] = NULL;
        }
        else if(video_node[0] != NULL)
        {
            // 写入视频帧
            AVI_write_frame(handle, (char*)video_node[0]->frame.data, video_node[0]->frame.len,
                           h264_is_keyframe(video_node[0]->frame.data + 4,video_node[0]->frame.len - 1));

            // 释放视频帧并调整缓冲区
            avi_record_queue_node_del(video_node[0]);
            if (video_node_total > 1)
            {
                memmove(&video_node[0], &video_node[1], (video_node_total - 1) * sizeof(avi_frame*));
            }
            video_node_total--;
            video_node[video_node_total] = NULL;
            
            // 更新总帧数计数
            (*frame_total)++;
        }
        
        // 短暂休眠避免CPU占用过高
        ak_sleep_ms(1);
    }

    // 任务停止后释放所有缓存的帧
    for(int i = 0; i<10;i++)
    {
        if(video_node[i] != NULL)
        {
            avi_record_queue_node_del(video_node[i]);
            video_node[i] = NULL;
        }
        if(audio_node[i] != NULL)
        {
            avi_record_queue_node_del(audio_node[i]);
            audio_node[i] = NULL;
        }
    }
}

/**
 * 释放所有AVI帧资源
 * 清空视频和音频队列并释放所有帧内存
 */
static void avi_frame_release_all(void)
{
    // 释放所有视频帧
    while(!queue_empty (&avi_video_queue_head))
    {
        avi_frame *node = (avi_frame *)queue_delete_next(&avi_video_queue_head);
        video_del_count++;
        //printf("delete frame_count is %d\n",video_del_count);
        avi_record_queue_node_del(node);
        video_release_count++;
        //printf("===========%d==========release frame_count is %d\n",__LINE__,video_release_count);
    }

    // 释放所有音频帧
    while(!queue_empty (&avi_audio_queue_head))
    {
        avi_frame *node = (avi_frame *)queue_delete_next(&avi_audio_queue_head);
        audio_del_count++;
        //printf("delete audio_count is %d\n",audio_del_count);
        avi_record_queue_node_del(node);
        audio_release_count++;
        //printf("release audio_count is %d\n",audio_release_count);
    }
}
/**
 * 视频录制任务线程函数
 * @param arg 录制参数结构体指针
 * @return 无
 */
static void* video_record_task(void* arg)
{
    ////printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
    video_record_thread_run = true;  // 标记录制线程已启动

    record_info* info = (record_info*)arg;  // 获取录制参数

    // 创建临时文件路径
    char temp_file[128] = {0};
    sprintf(temp_file,"%stemp",info->file_path);
    
    // 打开AVI文件
    avi_t* avi_handle = AVI_open_output_file(temp_file);
    
    // 写入AVI文件头
    avi_write_head(avi_handle,30.0,info->width,info->height,info->has_audio,info->frame_type);

    // 记录写入帧数和耗时
    unsigned long avi_write_frame_duration = 0;
    unsigned int avi_write_frame_count = 0;
    
    // 标记录制准备就绪
    video_record_ready = true;
    
    // 写入第一帧(确保包含关键帧信息)
    if(avi_write_one_frame(avi_handle,info) == true)
    {
        // 开始计时
        struct timeval start_tv;
        gettimeofday(&start_tv,NULL);
        printf("@@@@@@encode avi start... \n");
        
        // 循环写入后续帧
        avi_write_frame_loop(avi_handle,info,&avi_write_frame_count);
        
        // 结束计时
        struct timeval end_tv;
        gettimeofday(&end_tv,NULL);
        
        // 计算总耗时
        avi_write_frame_duration = end_tv.tv_sec*1000 + end_tv.tv_usec/1000 - start_tv.tv_sec*1000 - start_tv.tv_usec/1000;    
    }
    
    // 关闭文件前更新帧率信息
    ak_thread_mutex_lock(&avi_queue_head_mutex);
    video_record_ready = false;
    
    // 计算实际帧率
    avi_handle->fps = 1000*avi_write_frame_count/(avi_write_frame_duration+0.1);
    
    ////printf("\n\n\nEncode to AVI Finish. video frame:%lffps drution:%lums \n\n\n\n",avi_handle->fps,avi_write_frame_duration);
    
    // 关闭AVI文件
    AVI_close(avi_handle);

    // 释放所有帧资源
    avi_frame_release_all();
    ak_thread_mutex_unlock(&avi_queue_head_mutex);

    // 推送录制完成事件
    extern void record_video_event_push(bool);
    record_video_event_push(true);

    // 如果帧数足够，将临时文件重命名为正式文件名
    if(avi_write_frame_count > 40)
    {
        rename(temp_file,info->file_path);
    }
    
    // 校验视频文件完整性
    extern int media_bad_path_check(const char* file);
    media_bad_path_check(info->file_path);
    
    // 强制文件系统同步
    system("sync");
    
    // 标记录制线程已停止
    video_record_thread_run = false;
    ak_thread_exit();
    return NULL;
}

/**
 * 全局录制参数结构体
 */
record_info record_info1 = {{0},0};

/**
 * 启动视频录制
 * @param path 录制文件路径
 * @param has_audio 是否包含音频
 * @param width 视频宽度
 * @param height 视频高度
 * @param type 视频编码类型
 * @return 成功返回true，失败返回false
 */
bool video_record_start(const char* path,bool has_audio,int width,int height,char type)
{
    // 检查是否已有录制任务在运行
    if((video_record_task_run == true)||(video_record_thread_run == true))
    {
        ////printf("video record thread runing \n");
        return false;
    }

    ////printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
    printf("\n================width=>%d=======height=>%d====\n",width,height);
    
    // 设置录制参数
    strcpy(record_info1.file_path,path);
    record_info1.has_audio = has_audio;
    record_info1.frame_type = type;
    record_info1.width = width;
    record_info1.height = height;
    
    // 标记录制任务已启动
    video_record_task_run = true;
    
    // 创建并启动录制线程
    ak_pthread_t thread_id;
    ak_thread_create(&thread_id, video_record_task, &record_info1, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    ak_thread_detach(thread_id);
    
    return true;
}

/**
 * 停止视频录制
 * @return 成功返回true，失败返回false
 */
bool video_record_stop(void)
{
    // 如果录制任务未运行，直接返回成功
    if(video_record_task_run == false)
    {
        return true;
    }
    
    // 标记录制任务停止
    video_record_task_run = false;
    return true;
}

/**
 * 推送视频/音频数据到录制队列
 * @param type 帧类型(用于视频帧)
 * @param data 数据指针
 * @param len 数据长度
 * @param is_video 是否为视频数据
 * @return 成功返回true，失败返回false
 */
bool video_record_data_push(char type,unsigned char* data,int len,bool is_video)
{
    // 检查录制是否就绪
    ak_thread_mutex_lock(&avi_queue_head_mutex);
    if(video_record_ready == false)
    {
        ak_thread_mutex_unlock(&avi_queue_head_mutex);
        return false;
    }

    if(is_video == true)
    {
        // 创建视频帧节点并加入队列
        avi_frame* node = avi_record_queue_node_new(type,data,len,is_video);
        if(node == NULL)
        {
            //printf("record video full\n\r");
            ak_thread_mutex_unlock(&avi_queue_head_mutex);
            return false;
        }
        queue_insert((queue_s*)node, &avi_video_queue_head);
        video_insert_count ++;
        //printf("queue_insert frame_count is %d\n",video_insert_count);
    }
    else
    {
        // 如果需要录制音频，创建音频帧节点并加入队列
        if(record_info1.has_audio == true)
        {
            avi_frame* node = avi_record_queue_node_new(type,data,len,is_video);
            if(node == NULL)
            {
                //printf("record video full\n\r");
                ak_thread_mutex_unlock(&avi_queue_head_mutex);
                return false;
            }
            queue_insert((queue_s*)node, &avi_audio_queue_head);
            audio_insert_count ++;
            //printf("queue_insert audio count is %d\n",audio_insert_count);
        }
    }
    ak_thread_mutex_unlock(&avi_queue_head_mutex);
    return true;
}

/**
 * 初始化视频录制系统
 */
void video_record_init(void)
{
    // 初始化互斥锁
    ak_thread_mutex_init(&avi_queue_head_mutex, NULL);
    ak_thread_mutex_init(&avi_queue_free_mutex, NULL);

    // 初始化队列
    queue_initialize(&avi_video_queue_head);
    queue_initialize(&avi_audio_queue_head);
    queue_initialize(&avi_frame_queue_free);

    // 将所有帧节点加入空闲队列
    for(int i = 0 ; i < AVI_FRAME_BUFFER_MAX ; i++)
    {
        queue_insert((queue_s *)&avi_frame_buffer[i], &avi_frame_queue_free);
    }
}

/**
 * 检查是否正在录制
 * @return 正在录制返回true，否则返回false
 */
bool is_video_recording(void)
{
    return video_record_task_run;
}
