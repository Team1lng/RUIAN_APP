#include "avilib.h"
#include "ak_thread.h"
#include "stdio.h"
#include "ak_mem.h"
#include "ak_ao.h"
#include "queue.h"
#include "stdbool.h"
#include "ak_vdec.h"
#include "video_decode.h"
#include "leo_api.h"
#include "audio_output.h"
#include "../lv_drivers/lv_port_disp.h"
// #define AVI_BUFFER_MAX 1024*1024

#define AVI_AUDIO_FRAME_SIZE 1024          // AVI音频帧大小定义
#define VIDEO_PLAY_QUEUE_MAX 5             // 视频播放队列最大长度
extern ak_mutex_t media_thumb_mutex;      // 外部声明的媒体缩略图互斥锁

static bool video_play_task_run = false;          // 视频播放任务运行标志
static bool video_play_thread_run = false;       // 视频播放线程运行标志
static char video_play_file_path[64] = {0};       // 视频播放文件路径缓冲区

/**
 * 视频播放队列节点结构体
 * 用于存储视频/音频数据及队列链接信息
 */
typedef struct
{
    void *prev;         // 前驱节点指针
    void *next;         // 后继节点指针
    unsigned char *data; // 数据缓冲区指针
    unsigned int len;   // 数据长度
} video_play_queue;

/**
 * 视频播放信息结构体
 * 用于管理播放线程及句柄状态
 */
typedef struct
{
    bool run;               // 运行标志
    int handle_id;          // 解码器句柄
    ak_pthread_t thread_id; // 线程ID
} video_play_info;

static ak_mutex_t video_play_video_head_mutex;    // 视频队列头部互斥锁
static ak_mutex_t video_play_video_free_mutex;    // 视频空闲队列互斥锁
static ak_mutex_t video_play_audio_head_mutex;    // 音频队列头部互斥锁
static ak_mutex_t video_play_audio_free_mutex;    // 音频空闲队列互斥锁

static queue_s video_play_video_queue_head;       // 视频播放队列头部
static queue_s video_play_audio_queue_head;       // 音频播放队列头部
static queue_s video_play_video_queue_free;       // 视频空闲队列
static queue_s video_play_audio_queue_free;       // 音频空闲队列

static video_play_queue video_play_video_queue_buffer[VIDEO_PLAY_QUEUE_MAX]; // 视频队列缓冲区
static video_play_queue video_play_audio_queue_buffer[VIDEO_PLAY_QUEUE_MAX]; // 音频队列缓冲区

static int video_play_video_frame_index = 0;      // 视频帧索引
static int video_play_audio_frame_index = 0;      // 音频帧索引
static int video_play_video_frame_total = 0;      // 视频总帧数
static int video_play_video_frame_duration = 0;   // 视频帧持续时间(毫秒)
static bool video_play_video_frame_eof = false;   // 视频帧EOF标志

static unsigned long long video_play_clock_base = 0; // 播放时钟基准
static avi_t *avi_handle = NULL;

#define PLAY_VIDEO_STATE_IDLE 0X00              // 播放状态：空闲
#define PLAY_VIDEO_STATE_PLAY 0X02              // 播放状态：播放中
#define PLAY_VIDEO_STATE_PAUSE 0X03             // 播放状态：暂停
static unsigned char video_play_state = PLAY_VIDEO_STATE_IDLE; // 当前播放状态
static bool video_pause_play_first = false;     // 暂停后首次播放标志
static int video_first_play = 0;                // 首次播放标志


/**
 * 创建视频/音频播放队列节点
 * 
 * @param type 队列类型(0:视频 1:音频)
 * @param size 数据大小
 * @return 新创建的队列节点，失败返回NULL
 */
static video_play_queue *video_play_queue_node_new(char type, int size)
{
    // 获取对应类型的空闲队列
    queue_s *queue_free = type == 0 ? (&video_play_video_queue_free) : (&video_play_audio_queue_free);
    // 获取对应类型的互斥锁
    ak_mutex_t *pmutex = type == 0 ? (&video_play_video_free_mutex) : (&video_play_audio_free_mutex);
    
    video_play_queue *node = NULL;
    ak_thread_mutex_lock(pmutex);
    // 从空闲队列中获取节点
    if (queue_empty(queue_free) == 0)
    {
        node = (video_play_queue *)queue_delete_next(queue_free);
    }
    ak_thread_mutex_unlock(pmutex);
    
    if (node == NULL)
    {
        return NULL;
    }
    
    // 释放旧数据并分配新内存
    if (node->data != NULL)
    {
        ak_mem_free(node->data);
    }
    node->data = ak_mem_alloc(MODULE_ID_VDEC, size);
    node->len = size;
    return node;
}

/**
 * 释放视频/音频播放队列节点
 * 
 * @param type 队列类型(0:视频 1:音频)
 * @param node 要释放的节点
 */
static void video_play_queue_node_del(char type, video_play_queue *node)
{
    if (node != NULL)
    {
        // 释放节点数据内存
        if (node->data != NULL)
        {
            ak_mem_free(node->data);
            node->data = NULL;
        }
        
        // 将节点插入对应类型的空闲队列
        queue_s *queue_free = type == 0 ? (&video_play_video_queue_free) : (&video_play_audio_queue_free);
        ak_mutex_t *pmutex = type == 0 ? (&video_play_video_free_mutex) : (&video_play_audio_free_mutex);
        
        ak_thread_mutex_lock(pmutex);
        queue_insert((queue_s *)node, queue_free);
        ak_thread_mutex_unlock(pmutex);
    }
}

/**
 * 等待视频播放线程退出
 * 
 * @return 成功等待返回true，超时返回false
 */
static bool video_play_wait_thread_quit(void)
{
    int timeout = 300;
    // 循环等待线程退出，超时300ms
    while (timeout--)
    {
        if (video_play_thread_run == false)
        {
            return true;
        }
        usleep(10 * 1000); // 休眠10微秒
    }
    return false;
}

/**
 * 获取视频帧持续时间
 * 
 * @param handle AVI文件句柄
 * @return 视频帧持续时间(毫秒)
 */
static int video_frame_duration_get(avi_t *handle)
{
    // 根据帧率计算帧持续时间
    return (int)1000 / (AVI_frame_rate(handle) + 0.1);
}

/**
 * 获取音频帧持续时间
 * 
 * @param sample 采样率
 * @param ch 声道数
 * @param byte 每个样本字节数
 * @return 音频帧持续时间(毫秒)
 */
static int audio_frame_duration_get(long sample, int ch, int byte)
{
    // 根据音频参数计算帧持续时间
    return (AVI_AUDIO_FRAME_SIZE * 8 * 1000.0) / (sample * byte * ch + 0.0001);
}

extern void screen_force_refresh(void); // 强制刷新屏幕
extern bool video_delete;          // 视频删除标志

/**
 * 视频播放解码帧任务线程函数
 * 负责处理解码后的视频帧并推送显示
 * 
 * @param arg 线程参数，指向video_play_info结构体
 * @return 线程返回值
 */
static void *video_play_decode_frame_task(void *arg)
{
    video_play_info *frame_info = (video_play_info *)arg;
    struct ak_vdec_frame frame; // 视频帧结构体
    int pitch_width, pitch_height, widht, height = 0; // 帧参数
    unsigned char *backup_addr = NULL; // 备份帧数据地址
    
    // 线程主循环
    while (frame_info->run == true)
    {
        memset(&frame, 0, sizeof(struct ak_vdec_frame)); // 清零帧结构体
        
        // 获取解码后的视频帧
        if (ak_vdec_get_frame(frame_info->handle_id, &frame) == 0)
        {
            // 首次播放逻辑处理
            if (video_first_play != 0x01)
            {
                if (video_first_play == 0x02 && video_play_state == PLAY_VIDEO_STATE_PAUSE)
                {
                    ak_sleep_ms(1);
                    ak_vdec_release_frame(frame_info->handle_id, &frame);
                    continue;
                }
                // 暂停状态或EOF处理
                if (video_play_state == PLAY_VIDEO_STATE_PAUSE || video_play_video_frame_eof == true || video_first_play == 0x00)
                {
                    if (backup_addr == NULL)
                    {
                          // 分配备份缓冲区
                        backup_addr = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VDEC, frame.frame_obj.data.data_size);
                    }
                    if (backup_addr != NULL)
                    {
                        // 保存帧参数和数据
                        widht = frame.width;
                        height = frame.height;
                        pitch_width = frame.frame_obj.data.pitch_width;
                        pitch_height = frame.frame_obj.data.pitch_height;
                        memcpy((void *)backup_addr, frame.frame_obj.data.data, frame.frame_obj.data.data_size);
                    }
                    video_first_play = video_first_play == 0x00 ? 0x01 : video_first_play;
                }

                // 计算视频帧播放时间戳
                unsigned long long v_pts = video_play_clock_base + video_play_video_frame_index * video_play_video_frame_duration;
                unsigned long long cur_pts = get_sys_ms(); // 获取当前系统时间
            
                // 时间戳同步播放逻辑 
                if(video_play_task_run)
                {
                if (((cur_pts >= v_pts) && ((cur_pts - v_pts) < 25)) || ((cur_pts < v_pts) && ((v_pts - cur_pts) < 100)))
                {
                     // 推送视频帧数据并刷新屏幕
                    video_raw_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height);
                    screen_force_refresh(); //视频回放任务有定时器触发ui的刷新，不需要再强制刷（考虑到如果定时器的周期太长，视频就会不流畅，还是强制刷）
                }
                else if (cur_pts < v_pts)
                {
                    // 等待到播放时间再推送
                    ak_sleep_ms(v_pts - cur_pts);
                    video_raw_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height);
                    screen_force_refresh(); //视频回放任务有定时器触发ui的刷新，不需要再强制刷（考虑到如果定时器的周期太长，视频就会不流畅，还是强制刷）
                }
                else
                {
                    // 跳过过时帧
                    // printf("skip play video frame \n\r");
                }
                }
            }
            ak_vdec_release_frame(frame_info->handle_id, &frame);
        }
        else if ((video_play_state == PLAY_VIDEO_STATE_PAUSE) && (backup_addr != NULL))
        {
            // 暂停状态下显示备份帧
            if(video_play_task_run)
            video_raw_push(backup_addr, 0, widht, height, pitch_width, pitch_height);
            ak_sleep_ms(40);
            screen_force_refresh();
        }

        ak_sleep_ms(1);
    }
     // 释放备份缓冲区
    if (backup_addr != NULL)
    {
        ak_mem_dma_free(backup_addr);
    }
    printf("===========<<< video play display thread finish >>>===========\n");
    ak_thread_exit(); // 退出线程
    return NULL;
}

/**
 * 视频播放解码视频任务线程函数
 * 负责从队列获取视频数据并发送到解码器
 * 
 * @param arg 线程参数，指向video_play_info结构体
 * @return 线程返回值
 */
static void *video_play_decode_video_task(void *arg)
{
    video_play_info *decode_info = (video_play_info *)arg;

#if 1
     // 创建并启动帧显示线程
    video_play_info frame_info;
    frame_info.handle_id = decode_info->handle_id;
    frame_info.run = true;
    extern bool media_thumb_decode_clear(int handle);
    media_thumb_decode_clear(decode_info->handle_id);// 清除解码器缓冲区
    ak_thread_create(&frame_info.thread_id, video_play_decode_frame_task, &frame_info, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
#endif

    //	struct ak_vdec_frame frame;
    while (decode_info->run)
    {
        video_play_queue *node = NULL;
        // 加锁获取视频队列节点
        ak_thread_mutex_lock(&video_play_video_head_mutex);
        if (queue_empty(&video_play_video_queue_head) == 0)
        {
            node = (video_play_queue *)queue_delete_next(&video_play_video_queue_head);
        }
        ak_thread_mutex_unlock(&video_play_video_head_mutex);
        if (node != NULL)
        {
            // 处理首次播放逻辑
            if (video_first_play == 0x02)
            {
                video_play_queue_node_del(0, node);
                continue;
            }
             // 发送视频数据到解码器
            extern bool thumb_stream_send(int, unsigned char *, int);
            // if(video_play_task_run)
            // {
            if (thumb_stream_send(decode_info->handle_id, node->data, node->len) == false)
            {
                perror("thumb_stream_send failed\n");
            }
            // }
             // 释放队列节点
            video_play_queue_node_del(0, node);
        }

#if 0
        memset(&frame,0,sizeof(struct ak_vdec_frame));
        if(ak_vdec_get_frame(decode_info->handle_id, &frame) == 0)
        {
            video_play_video_frame_index++;
            unsigned long long v_pts = video_play_clock_base + video_play_video_frame_index*video_play_video_frame_duration;
            unsigned long long cur_pts = get_sys_ms();
            if(1)//(((cur_pts >= v_pts)&&((cur_pts - v_pts) < 25))||((cur_pts < v_pts)&&((v_pts - cur_pts) < 25)))
            {
                video_raw_push(frame.frame_obj.data.data, 0, frame.width, frame.height);
            }
            else if(cur_pts < v_pts)
            {
                ak_sleep_ms(v_pts - cur_pts);
                video_raw_push(frame.frame_obj.data.data, 0, frame.width, frame.height);
            }
            else
            {
                //printf("skip play video frame \n\r");
            }
            ak_vdec_release_frame(decode_info->handle_id, &frame);
        }
#endif
        ak_sleep_ms(1);
    }

#if 1
    // 等待帧显示线程退出
    frame_info.run = false;
    ak_thread_join(frame_info.thread_id);
#endif

    // 清除解码器缓冲区
    media_thumb_decode_clear(decode_info->handle_id);
    printf("===========<<< video frame thread finish >>>===========\n");
    ak_thread_exit(); // 退出线程
    return NULL;
}

/**
 * 等待音频输出缓冲区可写入的空间达到阈值
 * @param threshold 缓冲区剩余空间阈值
 * @return 等待成功返回true，获取缓冲区状态失败返回false
 */
static bool video_play_audio_buffer_wait_clear(int threshold)
{
    // 外部声明：获取音频输出缓冲区状态
    // extern bool audio_output_buffer_get(int *, int *);

    int total, remain;

    while (1)
    {
        if (audio_output_buffer_get(&total, &remain) == false)
        {
            return false;
        }

        // 当剩余空间小于等于阈值时，认为缓冲区可写入
        if (remain <= threshold)
        {
            break;
        }
    }
    return true;
}

/**
 * 音频解码任务线程函数
 * @param arg 视频播放信息结构体指针
 * @return 线程退出指针
 */
static void *video_play_audio_decode_task(void *arg)
{
    video_play_info *decode_info = (video_play_info *)arg;

    // 等待音频缓冲区清空，阈值8000字节
    video_play_audio_buffer_wait_clear(8000);

    // 外部声明：打开音频输出设备
    // extern bool audio_output_open(enum ak_audio_channel_type ch, enum ak_audio_sample_rate rate, int vol, int gain);
    usleep(100 * 1000); // 等待100ms确保设备准备就绪
    // 打开单声道、16kHz采样率的音频输出，音量80，增益5
    audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 80, 5);

    // 计算音频帧持续时间（16kHz采样率，1通道，16位深度）
    int frame_duration = audio_frame_duration_get(16000, 1, 16);
    // printf("audio play duration:%d \n", frame_duration);

    // 循环处理音频帧直到播放停止
    while (decode_info->run == true)
    {
        video_play_queue *node = NULL;
        ak_thread_mutex_lock(&video_play_audio_head_mutex); // 加锁保护音频队列
        if (queue_empty(&video_play_audio_queue_head) == 0)
        {
            // 从队列中取出下一个音频帧节点
            node = (video_play_queue *)queue_delete_next(&video_play_audio_queue_head);
        }
        ak_thread_mutex_unlock(&video_play_audio_head_mutex); // 解锁队列

        if (node != NULL)
        {
            // 外部声明：向音频输出设备写入数据
            //  extern bool audio_output_write(unsigned char *, int);

            unsigned long long cur_pts = get_sys_ms(); // 获取当前系统时间(毫秒)
            // 计算当前音频帧的目标播放时间点
            unsigned long long a_pts = video_play_clock_base + video_play_audio_frame_index * frame_duration;
            
            // 确保音量为预设值80
            if (audio_output_volume_get() != 80)
            {
                audio_output_volume_set(80);
            }
            
            // 音频同步逻辑：根据当前时间与目标播放时间的差值决定处理方式
            if (((cur_pts >= a_pts) && ((cur_pts - a_pts) < 25)) || ((cur_pts < a_pts) && ((a_pts - cur_pts) < 100)))
            {
                // 时间差在允许范围内，直接播放
                audio_output_write(node->data, node->len);
            }
            else if (cur_pts < a_pts)
            {
                // 当前时间早于目标时间，等待到目标时间再播放
                ak_sleep_ms(a_pts - cur_pts);
                audio_output_write(node->data, node->len);
            }
            else
            {
                // 当前时间晚于目标时间，直接播放（可能产生丢帧）
                audio_output_write(node->data, node->len);
            }
            
            // 释放节点资源并更新帧索引
            video_play_queue_node_del(1, node);
            video_play_audio_frame_index++;
        }

        ak_sleep_ms(1); // 短暂休眠避免CPU占用过高
    }
    // printf("===========<<< audio frame thread finish >>>===========\n");
    ak_thread_exit();
    return NULL;
}

/**
 * 释放视频和音频队列资源
 */
static void video_play_queue_realease(void)
{
    // 释放视频队列
    ak_thread_mutex_lock(&video_play_video_head_mutex);
    while (!queue_empty(&video_play_video_queue_head))
    {
        video_play_queue *node = (video_play_queue *)queue_delete_next(&video_play_video_queue_head);
        if (node != NULL)
        {
            video_play_queue_node_del(0, node);
        }
    }
    ak_thread_mutex_unlock(&video_play_video_head_mutex);

    // 释放音频队列
    ak_thread_mutex_lock(&video_play_audio_head_mutex);
    while (!queue_empty(&video_play_audio_queue_head))
    {
        video_play_queue *node = (video_play_queue *)queue_delete_next(&video_play_audio_queue_head);
        if (node != NULL)
        {
            video_play_queue_node_del(1, node);
        }
    }
    ak_thread_mutex_unlock(&video_play_audio_head_mutex);

    // 释放原始视频数据
    video_raw_release_all();
}

/**
 * 从AVI文件读取并发送视频帧到队列
 */
static void video_play_video_frame_send()
{
    int data_len = 0;
    int key = 0;

    // 获取当前AVI视频帧大小
    long video_frame_size = AVI_cur_frame_size(avi_handle);
    if (video_frame_size > 0)
    {
        // 创建视频帧队列节点
        video_play_queue *node = video_play_queue_node_new(0, video_frame_size);
        if (node != NULL)
        {
            // 从AVI文件读取视频帧数据
            if ((data_len = (int)AVI_read_frame(avi_handle, (char *)node->data, &key)) > 0)
            {
                node->len = data_len;
                ak_thread_mutex_lock(&video_play_video_head_mutex);
                // printf("video_play_get_status frame\n");
                
                // 更新视频帧索引并插入队列
                video_play_video_frame_index++;
                queue_insert((queue_s *)node, &video_play_video_queue_head);
                ak_thread_mutex_unlock(&video_play_video_head_mutex);
                
                // 检查是否已读取完所有视频帧
                if (video_play_video_frame_index >= video_play_video_frame_total)
                {
                    video_play_video_frame_eof = true;
                }
            }
            else
            {
                // 读取失败时释放节点
                video_play_queue_node_del(0, node);
            }
        }
    }
}

/**
 * 从AVI文件读取并发送音频帧到队列
 */
static void video_play_audio_frame_send()
{
    int data_len = 0;
    // 创建音频帧队列节点（固定AVI音频帧大小）
    video_play_queue *node = video_play_queue_node_new(1, AVI_AUDIO_FRAME_SIZE);
    if (node != NULL)
    {
        // printf("video_play_queue_node_new.run \n");
        // 从AVI文件读取音频数据
        if ((data_len = (int)AVI_read_audio(avi_handle, (char *)node->data, AVI_AUDIO_FRAME_SIZE)) > 0)
        {
            node->len = data_len;
            ak_thread_mutex_lock(&video_play_audio_head_mutex);
            // printf("video_play_get_status audio\n");
            // 插入音频队列
            queue_insert((queue_s *)node, &video_play_audio_queue_head);
            ak_thread_mutex_unlock(&video_play_audio_head_mutex);
        }
        else
        {
            // 读取失败时释放节点
            video_play_queue_node_del(1, node);
        }
    }
}

/**
 * 视频播放主任务线程函数
 * @param arg 线程参数（未使用）
 * @return 线程退出指针
 */
static void *video_play_task(void *arg)
{
    video_play_thread_run = true;

    // 打开AVI视频文件（第二个参数1表示读取模式）
    avi_handle = AVI_open_input_file(video_play_file_path, 1);

    // 初始化视频和音频解码信息结构体
    video_play_info video_info = {
        .handle_id = -1,
        .run = false,
    };
    video_play_info audio_info = {
        .handle_id = -1,
        .run = false,
    };

    // 获取视频压缩格式并初始化对应解码器
    char *format = AVI_video_compressor(avi_handle);
    if (strcmp(format, "H264") == 0)
    {
        extern int media_h264_decode_handle_id_get(void);
        video_info.handle_id = media_h264_decode_handle_id_get();
    }
    else if (strcmp(format, "MJPG") == 0)
    {
        extern int media_mjpeg_decode_handle_id_get(void);
        video_info.handle_id = media_mjpeg_decode_handle_id_get();
    }

    // 创建视频解码线程
    if (video_info.handle_id != -1)
    {
        video_info.run = true;
        ak_thread_create(&video_info.thread_id, video_play_decode_video_task, &video_info, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    }

    // 获取音频参数并检查是否支持当前格式
    int audio_channels = AVI_audio_channels(avi_handle);
    int audio_rate = AVI_audio_rate(avi_handle);
    int audio_byte = AVI_audio_bits(avi_handle);
    // printf("channels:%d rate:%d byte:%d \n", audio_channels, audio_rate, audio_byte);
    if ((audio_channels == 1) && (audio_rate == 16000) && (audio_byte == 16))
    {
        audio_info.run = true;
        // 创建音频解码线程
        ak_thread_create(&audio_info.thread_id, video_play_audio_decode_task, &audio_info, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    }

    // 初始化播放状态变量
    video_play_video_frame_eof = false;
    video_play_video_frame_index = 0;
    video_play_audio_frame_index = 0;
    video_play_video_frame_total = AVI_video_frames(avi_handle);
    video_play_video_frame_duration = video_frame_duration_get(avi_handle);
    // printf("video frame duration:%d ms\n\r", video_play_video_frame_duration);

    // 设置播放时钟基准为当前系统时间
    video_play_clock_base = get_sys_ms();

    // 设置播放状态为播放中
    video_play_state = PLAY_VIDEO_STATE_PLAY;
    video_pause_play_first = true;
    video_first_play = 0;
    
    // 主播放循环
    while (video_play_task_run == true)
    {
        // 处理暂停状态
        if (video_play_state == PLAY_VIDEO_STATE_PAUSE)
        {
            if (video_pause_play_first)
            {
                // 暂停时若队列空且未播放到EOF，继续读取视频帧
                if (queue_empty(&video_play_video_queue_head) && video_play_video_frame_eof == false)
                {
                    if (video_play_video_frame_index < video_play_video_frame_total)
                    {
                        video_play_video_frame_send();
                    }
                }
            }
            video_pause_play_first = false;
            ak_sleep_ms(40);
            continue;
        }
        video_pause_play_first = true;
        
        // 处理视频帧读取逻辑
        if (video_play_video_frame_index < video_play_video_frame_total)
        {
            if (video_first_play == 0x00)
            {
                // 首次播放读取视频帧并等待10ms
                video_play_video_frame_send();
                usleep(10 * 1000);
                continue;
            }
            else if (video_first_play == 0x01)
            {
                // 重置播放状态（可能用于重新开始播放）
                video_first_play = 0x02;
                video_play_state = PLAY_VIDEO_STATE_PAUSE;
                extern bool media_thumb_decode_clear(int handle);
                media_thumb_decode_clear(video_info.handle_id);
                video_pause_play_first = false;
                video_play_video_frame_index = 0;
                video_play_audio_frame_index = 0;
                // 重置AVI文件播放位置
                AVI_set_video_position(avi_handle, video_play_video_frame_index);
                AVI_set_audio_position(avi_handle, video_play_audio_frame_index);
                continue;
            }
            else
            {
                video_first_play = 0x03;
                video_play_video_frame_send();
            }
        }
        
        // 若音频解码线程运行中，读取并发送音频帧
        if (audio_info.run == true)
        {
            video_play_audio_frame_send();
            // printf("audio_info.run \n");
        }
        
        // 检查是否播放到文件末尾
        if ((video_play_video_frame_index >= video_play_video_frame_total) && (queue_empty(&video_play_video_queue_head)) && (queue_empty(&video_play_audio_queue_head)))
        {
            video_play_video_frame_eof = true;
            video_play_state = PLAY_VIDEO_STATE_PAUSE;
            video_play_video_frame_index = 0;
            video_play_audio_frame_index = 0;

            // 重置播放位置到文件开头
            AVI_set_video_position(avi_handle, 0);
            AVI_set_audio_position(avi_handle, 0);
            printf("video play eof \n\r");
        }

        ak_sleep_ms(1); // 短暂休眠避免CPU占用过高
    }

    // 等待视频解码线程退出
    if (video_info.run == true)
    {
        video_info.run = false;
        ak_thread_join(video_info.thread_id);
    }

    // 等待音频解码线程退出
    if (audio_info.run == true)
    {
        audio_info.run = false;
        ak_thread_join(audio_info.thread_id);
    }

    // 关闭AVI文件并释放资源
    AVI_close(avi_handle);
    video_play_queue_realease();
    video_play_thread_run = false;

    // printf("===========<<< video play finish >>>===========\n");
    ak_thread_exit();
    return NULL;
}

/**
 * 初始化视频播放系统
 */
void video_play_init(void)
{
    // 初始化视频和音频队列
    queue_initialize(&video_play_video_queue_head);
    queue_initialize(&video_play_audio_queue_head);

    queue_initialize(&video_play_video_queue_free);
    queue_initialize(&video_play_audio_queue_free);

    // 初始化队列缓冲区并将节点加入空闲队列
    memset(video_play_video_queue_buffer, 0, sizeof(video_play_queue) * VIDEO_PLAY_QUEUE_MAX);
    memset(video_play_audio_queue_buffer, 0, sizeof(video_play_queue) * VIDEO_PLAY_QUEUE_MAX);
    for (int i = 0; i < VIDEO_PLAY_QUEUE_MAX; i++)
    {
        queue_insert((queue_s *)&video_play_video_queue_buffer[i], &video_play_video_queue_free);
        queue_insert((queue_s *)&video_play_audio_queue_buffer[i], &video_play_audio_queue_free);
    }

    // 初始化互斥锁
    ak_thread_mutex_init(&video_play_video_head_mutex, NULL);
    ak_thread_mutex_init(&video_play_audio_head_mutex, NULL);

    ak_thread_mutex_init(&video_play_video_free_mutex, NULL);
    ak_thread_mutex_init(&video_play_audio_free_mutex, NULL);
}

static const rom_bin_info *p_scr_act_img = NULL;

/**
 * 打开视频文件并启动播放任务
 * @param file 视频文件路径
 * @return 打开成功返回true，失败返回false
 */
bool video_play_open(const char *file)
{
    if (video_play_task_run == true)
    {
        // printf("video play thread working \n");
        return false;
    }

    // 等待之前的播放线程退出
    if (video_play_wait_thread_quit() == false)
    {
        // printf("video play thread wait fail \n");
        return false;
    }

    // 保存当前屏幕背景并设置为透明
    if (p_scr_act_img == NULL)
    {
        p_scr_act_img = lv_obj_get_style_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN);
    }
    lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);

    // 复制文件路径并启动播放任务
    memset(video_play_file_path, 0, sizeof(video_play_file_path));
    strcpy(video_play_file_path, file);

    video_play_task_run = true;
    ak_pthread_t thread_id;
    ak_thread_create(&thread_id, video_play_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    ak_thread_detach(thread_id);
    return true;
}

/**
 * 停止视频播放
 * @return 停止成功返回true，失败返回false
 */
bool video_play_stop(void)
{
    if (video_play_task_run == false)
    {
        return false;
    }
    video_play_task_run = false;
    video_play_state = PLAY_VIDEO_STATE_IDLE;
    
    // 恢复屏幕背景
    if (p_scr_act_img != NULL)
    {
        system_bg_fill_color_2(0x00, 0, 0, 1024, 600);//hlf
        lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, p_scr_act_img);
        p_scr_act_img = NULL;
    }

    return true;
}

/**
 * 暂停或继续视频播放
 * @return 操作成功返回true，失败返回false
 */
bool video_play_pause(void)
{
    if (video_play_state == PLAY_VIDEO_STATE_IDLE)
    {
        return false;
    }

    // 切换播放/暂停状态
    if (video_play_state == PLAY_VIDEO_STATE_PAUSE)
    {
        video_play_state = PLAY_VIDEO_STATE_PLAY;
        video_play_video_frame_eof = false;
        // 重新计算播放时钟基准（用于续播同步）
        video_play_clock_base = get_sys_ms() - video_play_video_frame_index * video_play_video_frame_duration;
    }
    else if (video_play_state == PLAY_VIDEO_STATE_PLAY)
    {
        video_play_state = PLAY_VIDEO_STATE_PAUSE;
    }
    return true;
}

/**
 * 获取当前播放状态
 * @return 1=播放中，2=暂停，0=空闲
 */
char video_play_get_status(void)
{
    return video_play_state == PLAY_VIDEO_STATE_PLAY ? 1 : video_play_state == PLAY_VIDEO_STATE_PAUSE ? 2
                                                                                                      : 0;
}

/**
 * 获取当前播放时长和总时长
 * @param cur 输出当前已播放时长(毫秒)
 * @param total 输出视频总时长(毫秒)
 * @return 操作成功返回true，失败返回false
 */
bool video_play_duration_get(int *cur, int *total)
{
    if (video_play_state == PLAY_VIDEO_STATE_IDLE)
    {
        return false;
    }

    // 计算总时长（总帧数×单帧时长）
    *total = video_play_video_frame_total * video_play_video_frame_duration;
    if (video_play_video_frame_eof == true)
    {
        // 播放到EOF时当前时长等于总时长
        *cur = *total;
    }
    else
    {
        // 正常播放时当前时长为已播放帧数×单帧时长
        *cur = video_play_video_frame_index * video_play_video_frame_duration;
    }
    return true;
}
