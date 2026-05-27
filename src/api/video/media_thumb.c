#include "video_decode.h"
#include "ak_thread.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"
#include "queue.h"
#include "ak_mem.h"
#include "ak_vdec.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "lvgl.h"
#include "avilib.h"

static int media_mjpeg_handle_id = -1;
int media_h264_handle_id = -1;
static int media_thumb_width = 0;
static int media_thumb_height = 0;

static bool madia_thumb_task_run = false;
static bool media_thumb_thread = false;

static bool media_decode_device_ready = false;

static bool h264_decode_load = false;
int last_decode_res[2] = {0, 0};
int curr_decode_res[2] = {0, 0};
ak_mutex_t media_thumb_mutex;
/*************************************************************************
 * @brief  h264解码器分辨率检查，当上一次与这一次解码出来的分辨率不一致时，在回放界面会出现异常
 * @date   2023-03-10 07:50
 * @author xiaole
 * @return void
 * @note   检测到分辨率变化时会重新加载解码器
 **************************************************************************/
void h264_decode_res_check(void)
{
    // 检查上一次解码分辨率与当前解码分辨率是否一致
    if ((last_decode_res[0] != curr_decode_res[0]) || (last_decode_res[1] != curr_decode_res[1]))
    {
        // 打印前后分辨率对比信息
        printf("last:(%dx%d), curr:(%dx%d)\n", last_decode_res[0], last_decode_res[1], curr_decode_res[0], curr_decode_res[1]);
        printf("decode res is different, pls reload...\n");
        
        // 标记录解码器需要重新加载
        h264_decode_load = false;

        // 关闭当前H.264解码器
        ak_vdec_close(media_h264_handle_id);

        // 初始化解码器参数结构体
        struct ak_vdec_param param;
        memset(&param, 0, sizeof(struct ak_vdec_param));
        param.vdec_type = H264_ENC_TYPE;          // 设置解码器类型为H.264
        param.output_type = AK_YUV420SP;         // 设置输出格式为YUV420SP
        param.sc_width = media_thumb_width;      // 设置缩放宽度为媒体缩略图宽度
        param.sc_height = media_thumb_height;    // 设置缩放高度为媒体缩略图高度
        param.frame_buf_num = 3;                 // 设置帧缓冲区数量为3
        param.stream_buf_size = 4 * 1024 * 1024;  // 设置流缓冲区大小为4MB
        
        // 重新打开H.264解码器
        ak_vdec_open(&param, &media_h264_handle_id);
        
        // 清除解码器缓冲区
        ak_vdec_clear_buff(media_h264_handle_id);

        // 标记录解码器已加载完成
        h264_decode_load = true;

        // 更新上一次解码分辨率为当前分辨率
        last_decode_res[0] = curr_decode_res[0];
        last_decode_res[1] = curr_decode_res[1];
    }
}

/**
 * 初始化媒体缩略图互斥锁
 * 
 * 确保互斥锁只初始化一次
 */
static void media_thumb_mutex_init(void)
{
    static bool is_first = true;  // 静态标志位，确保只初始化一次
    if (is_first == true)
    {
        is_first = false;
        // 初始化互斥锁
        ak_thread_mutex_init(&media_thumb_mutex, NULL);
    }
}

/**
 * 清除媒体缩略图解码器缓冲区
 * 
 * @param handle 解码器句柄
 * @return 清除成功返回true，失败返回false
 */
bool media_thumb_decode_clear(int handle)
{
    if (handle == -1)  // 无效句柄检查
    {
        return false;
    }

    struct ak_vdec_frame frame = {0};  // 初始化帧结构体
    
    // 循环获取并释放帧，直到无帧可获取
    while (ak_vdec_get_frame(handle, &frame) == 0)
    {
        ak_vdec_release_frame(handle, &frame);
    }
    
    // 结束数据流输入
    ak_vdec_end_stream(handle);
    
    while (1)
    {
        struct ak_vdec_frame frame = {0};
        int status = 0;
        
        // 尝试获取帧并释放
        if (ak_vdec_get_frame(handle, &frame) == 0)
        {
            ak_vdec_release_frame(handle, &frame);
        }
        
        // 检查解码是否完成
        if (ak_vdec_get_decode_finish(handle, &status) == 0)
        {
            if (status)  // 解码完成
            {
                break;
            }
            ak_sleep_ms(1);  // 未完成则等待1ms继续检查
            continue;
        }
        break;
    }
    
    // 清除解码器缓冲区
    ak_vdec_clear_buff(handle);
    return true;
}

/**
 * 关闭媒体缩略图解码器
 * 
 * @param handle 解码器句柄
 */
static void media_thumb_decode_close(int handle)
{
    if (handle == -1)  // 无效句柄检查
    {
        return;
    }
    
    // 清除解码器缓冲区
    media_thumb_decode_clear(handle);
    
    printf("media vdec close\n");  // 打印关闭日志
    // 关闭解码器
    ak_vdec_close(handle);
}

/**
 * 获取H.264解码器加载状态
 * 
 * @return 已加载返回true，未加载返回false
 */
bool h264_decode_load_state_get()
{
    return h264_decode_load;  // 返回加载状态标志
}

/**
 * 等待H.264解码器加载完成
 * 
 * @return 加载完成返回true，超时返回false
 */
bool h264_decode_load_wait(void)
{
    int timeout = 300;  // 设置超时时间为300ms
    
    while (timeout--)  // 循环等待直到超时
    {
        if (h264_decode_load == true)  // 检查是否加载完成
        {
            return true;
        }

        ak_sleep_ms(1);  // 等待1ms
    }

    return false;  // 超时返回false
}

/**
 * 打开H.264解码器用于媒体缩略图
 */
static void media_thumb_h264_decode_open(void)
{
    // 初始化解码器参数结构体
    struct ak_vdec_param param;
    memset(&param, 0, sizeof(struct ak_vdec_param));
    param.vdec_type = H264_ENC_TYPE;          // 设置解码器类型为H.264
    param.output_type = AK_YUV420SP;         // 设置输出格式为YUV420SP
    param.sc_width = 1920;                    // 设置缩放宽度为1920
    param.sc_height = 1080;                   // 设置缩放高度为1080
    param.frame_buf_num = 2;                  // 设置帧缓冲区数量为2
    param.stream_buf_size = 307200 * 2;       // 设置流缓冲区大小为614400字节
    
    // 打开H.264解码器
    if (ak_vdec_open(&param, &media_h264_handle_id))
    {
        printf("open video h264 device fail \n\r");  // 打开失败日志
    }
    
    // 清除解码器缓冲区
    ak_vdec_clear_buff(media_h264_handle_id);
}

/**
 * 媒体缩略图打开任务线程函数
 * 
 * @param arg 线程参数(未使用)
 * @return 线程返回值
 */
static void *media_thumb_open_task(void *arg)
{
    media_thumb_thread = true;  // 标记录制线程正在运行

    // 检查MJPEG解码器句柄是否有效，无效则打开
    if (media_mjpeg_handle_id == -1)
    {
        // 初始化解码器参数结构体
        struct ak_vdec_param param;
        memset(&param, 0, sizeof(struct ak_vdec_param));
        param.vdec_type = MJPEG_ENC_TYPE;        // 设置解码器类型为MJPEG
        param.output_type = AK_YUV420SP;         // 设置输出格式为YUV420SP
        param.sc_width = media_thumb_width;      // 设置缩放宽度为媒体缩略图宽度
        param.sc_height = media_thumb_height;    // 设置缩放高度为媒体缩略图高度
        param.frame_buf_num = 2;                 // 设置帧缓冲区数量为2
        param.stream_buf_size = 307200;          // 设置流缓冲区大小为307200字节
        
        // 打开MJPEG解码器
        if (ak_vdec_open(&param, &media_mjpeg_handle_id))
        {
            printf("open video mjpeg device fail \n\r");  // 打开失败日志
        }
        
        // 清除解码器缓冲区
        ak_vdec_clear_buff(media_mjpeg_handle_id);

        h264_decode_load = true;  // 标记录解码器已加载
    }

    // 检查H.264解码器句柄是否有效，无效则打开
    if (media_h264_handle_id == -1)
    {
        media_thumb_h264_decode_open();  // 调用H.264解码器打开函数
    }
    
    media_decode_device_ready = true;  // 标记录制设备已准备好
    
    // 线程主循环，直到任务运行标志为false
    while (madia_thumb_task_run == true)
    {
        ak_sleep_ms(10);  // 休眠10ms减少CPU占用
    }

    // 加锁防止并发访问
    ak_thread_mutex_lock(&media_thumb_mutex);
    
    // 关闭MJPEG解码器
    media_thumb_decode_close(media_mjpeg_handle_id);
    // 关闭H.264解码器
    media_thumb_decode_close(media_h264_handle_id);
    
    media_decode_device_ready = false;  // 标记录制设备未准备好
    media_mjpeg_handle_id = -1;        // 重置MJPEG解码器句柄为无效
    media_h264_handle_id = -1;         // 重置H.264解码器句柄为无效
    
    // 解锁互斥锁
    ak_thread_mutex_unlock(&media_thumb_mutex);

    media_thumb_thread = false;  // 标记录制线程已停止
    printf("===========<<< media thumb finish >>>===========\n");  // 线程结束日志
    
    // 退出线程
    ak_thread_exit();
    return NULL;
}

/**
 * 等待媒体缩略图线程退出
 * 
 * @return 线程已退出返回true，超时返回false
 */
bool media_thumb_wait_thread_quit(void)
{
    int timeout = 300;  // 设置超时时间为300ms
    
    while (timeout--)  // 循环等待直到超时
    {
        if (media_thumb_thread == false)  // 检查线程是否已退出
        {
            return true;
        }
        usleep(10 * 1000);  // 休眠10微秒
    }
    return false;  // 超时返回false
}

/**
 * 打开媒体缩略图设备
 * 
 * @param width  缩略图宽度
 * @param height 缩略图高度
 * @return 打开成功返回true，失败返回false
 */
bool media_thumb_device_open(int width, int height)
{
    media_thumb_mutex_init();  // 初始化互斥锁

    if (madia_thumb_task_run == true)  // 检查任务是否已在运行
    {
        return false;
    }

    if (media_thumb_wait_thread_quit() == false)  // 等待之前的线程退出
    {
        printf("thumb thread wait error \n");
        return false;
    }

    // 获取系统背景虚拟地址
    extern unsigned char *system_bg_virtual_addres_get(void);
    static rom_bin_info img = rom_bin_raw_get();  // 获取ROM二进制信息

    // 初始化ROM二进制数据
    rom_bin_raw_init(img, system_bg_virtual_addres_get(), LV_HOR_RES_MAX, LV_VER_RES_MAX);
    // 设置当前屏幕的背景图片
    lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img);

    // 设置媒体缩略图宽度和高度
    media_thumb_width = width;
    media_thumb_height = height;
    
    // 保存当前解码器分辨率以便比较
    curr_decode_res[0] = media_thumb_width;
    curr_decode_res[1] = media_thumb_height;

    last_decode_res[0] = media_thumb_width;
    last_decode_res[1] = media_thumb_height;

    media_decode_device_ready = false;  // 标记录制设备未准备好
    madia_thumb_task_run = true;       // 标记录制任务正在运行
    
    // 创建媒体缩略图打开任务线程
    ak_pthread_t thread_id;
    ak_thread_create(&thread_id, media_thumb_open_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    ak_thread_detach(thread_id);  // 分离线程以便自动回收资源
    return true;
}

/**
 * 获取H.264解码器句柄
 * 
 * @return H.264解码器句柄
 */
int media_h264_decode_handle_id_get(void)
{
    return media_h264_handle_id;  // 返回H.264解码器句柄
}

/**
 * 获取MJPEG解码器句柄
 * 
 * @return MJPEG解码器句柄
 */
int media_mjpeg_decode_handle_id_get(void)
{
    return media_mjpeg_handle_id;  // 返回MJPEG解码器句柄
}

/**
 * 关闭媒体缩略图设备
 * 
 * @return 关闭成功返回true，失败返回false
 */
bool media_thumb_device_close(void)
{
    if (madia_thumb_task_run == false)  // 检查任务是否在运行
    {
        return false;
    }
    
    // 清除当前屏幕的背景图片
    lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
    madia_thumb_task_run = false;  // 标记录制任务停止
    return true;
}

/**
 * 从文件读取JPEG数据
 * 
 * @param file_path JPEG文件路径
 * @param buffer    输出参数：指向存储JPEG数据的缓冲区指针
 * @param size      输出参数：JPEG数据大小(字节)
 * @return 成功返回true，失败返回false
 */
static bool jpeg_data_get(const char *file_path, unsigned char **buffer, int *size)
{
    struct stat statbuf;
    
    // 获取文件状态信息
    stat(file_path, &statbuf);
    if (statbuf.st_size < 1)  // 检查文件大小是否有效
    {
        return false;
    }

    // 打开文件读取
    int fd = open(file_path, O_RDONLY);
    if (fd < 0)  // 检查文件打开是否成功
    {
        return false;
    }

    // 设置输出参数
    *size = statbuf.st_size;
    // 分配内存存储JPEG数据
    *buffer = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, *size);
    // 读取文件内容到缓冲区
    read(fd, *buffer, *size);
    close(fd);  // 关闭文件
    return true;
}

/**
 * 将缩略图数据流发送到解码器
 * 
 * @param handle 解码器句柄
 * @param pdata  数据流指针
 * @param len    数据流长度
 * @return 成功返回true，失败返回false
 */
bool thumb_stream_send(int handle, unsigned char *pdata, int len)
{
    int dec_len = 0;      // 已解码数据长度
    int read_len = len;   // 剩余要发送的数据长度
    int send_len = 0;     // 已发送数据长度
    
    // 循环发送数据直到全部发送完成
    while (read_len > 0)
    {
        // 非阻塞方式发送数据到解码器
        if ((ak_vdec_send_stream(handle, &pdata[send_len], read_len, NONBLOCK, &dec_len)) != 0)
        {
            perror("send frame failed\n");
            return false;
        }
        read_len -= dec_len;  // 更新剩余要发送的数据长度
        send_len += dec_len;  // 更新已发送数据长度
    }
    return true;
}

/**
 * 将缩略图帧复制到系统背景缓冲区
 * 
 * @param frame 视频帧结构体
 * @param x     目标位置X坐标
 * @param y     目标位置Y坐标
 * @param w     目标宽度
 * @param h     目标高度
 */
static void thumb_frame_copy_to_bg_sysetm(struct ak_vdec_frame *frame, int x, int y, int w, int h)
{
    struct ak_tde_layer src, dst;  // 定义源和目标图层结构

    // 分配内存用于存储转换后的RGB数据
    unsigned char *data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_AO, w * h * 3);
    
    // 配置源图层参数
    src.format_param = GP_FORMAT_YUV420SP;  // 源格式为YUV420SP
    src.width = frame->frame_obj.data.pitch_width;  // 源宽度
    src.height = frame->frame_obj.data.pitch_height;  // 源高度
    src.pos_left = 0;  // 源区域左上角X坐标
    src.pos_top = 0;   // 源区域左上角Y坐标
    src.pos_width = frame->width;  // 源区域宽度
    src.pos_height = frame->height;  // 源区域高度
    // 获取源数据物理地址
    ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long *)&src.phyaddr);

    // 配置目标图层参数
    dst.format_param = GP_FORMAT_RGB888;  // 目标格式为RGB888
    dst.width = w;  // 目标宽度
    dst.height = h;  // 目标高度
    dst.pos_left = 0;  // 目标区域左上角X坐标
    dst.pos_top = 0;   // 目标区域左上角Y坐标
    dst.pos_width = w;  // 目标区域宽度
    dst.pos_height = h;  // 目标区域高度
    // 获取目标数据物理地址
    ak_mem_dma_vaddr2paddr(data, (unsigned long *)&dst.phyaddr);

    // 调用TDE模块进行图像缩放转换(YUV420SP->RGB888)
    ak_tde_opt_scale(&src, &dst);
    
    // 获取系统背景虚拟地址
    extern unsigned char *system_bg_virtual_addres_get(void);
    unsigned char *dst_data = system_bg_virtual_addres_get() + y * LV_HOR_RES_MAX * 4 + x * 4;
    unsigned char *src_data = data;
    
    // 将RGB数据复制到系统背景缓冲区，添加Alpha通道
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            // 设置RGBA值(添加完全不透明的Alpha通道)
            dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 3] = 0xFF;  // Alpha通道
            dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 2] = src_data[y * w * 3 + x * 3 + 2];  // Red通道
            dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 1] = src_data[y * w * 3 + x * 3 + 1];  // Green通道
            dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 0] = src_data[y * w * 3 + x * 3 + 0];  // Blue通道
        }
    }
    
    // 释放临时分配的内存
    ak_mem_dma_free(data);
    //printf("read: %dx%d write:%d,%d %dx%d\n", LV_HOR_RES_MAX, LV_VER_RES_MAX, x, y, w, h);
}

/**
 * 显示缩略图帧
 * 
 * @param handle 解码器句柄
 * @param x      显示位置X坐标
 * @param y      显示位置Y坐标
 * @param w      显示宽度
 * @param h      显示高度
 * @return 成功返回true，失败返回false
 */
static bool thumb_frame_display(int handle, int x, int y, int w, int h)
{
    struct ak_vdec_frame frame = {0};  // 初始化帧结构体
    
    // 尝试获取解码后的帧
    if (ak_vdec_get_frame(handle, &frame) == 0)
    {
        // 如果是H.264解码器，检查分辨率是否变化
        if (handle == media_h264_handle_id)
        {
            curr_decode_res[0] = frame.width;   // 记录当前宽度
            curr_decode_res[1] = frame.height;  // 记录当前高度
            
            // 检查分辨率是否变化
            if ((last_decode_res[0] != curr_decode_res[0]) || (last_decode_res[1] != curr_decode_res[1]))
            {
                // 释放帧并检查解码器分辨率
                ak_vdec_release_frame(handle, &frame);
                h264_decode_res_check();
                return false;
            }
        }
        
        // 将帧复制到系统背景缓冲区显示
        thumb_frame_copy_to_bg_sysetm(&frame, x, y, w, h);
        // 释放帧资源
        ak_vdec_release_frame(handle, &frame);
        return true;
    }
    return false;
}

/**
 * 加载JPEG缩略图
 * 
 * @param x          显示位置X坐标
 * @param y          显示位置Y坐标
 * @param w          显示宽度
 * @param h          显示高度
 * @param file_path  JPEG文件路径
 * @return 成功返回true，失败返回false
 */
static bool jpeg_thumb_load(int x, int y, int w, int h, const char *file_path)
{
    unsigned char *jpg_buffer = NULL;  // JPEG数据缓冲区
    int jpg_size = 0;                  // JPEG数据大小
    
    // 从文件读取JPEG数据
    if (jpeg_data_get(file_path, &jpg_buffer, &jpg_size) == false)
    {
        return false;
    }

    int timeout = 1000;  // 设置超时计数
    bool is_first = true;  // 首次处理标志
    
    // 循环处理直到超时或成功显示
    while (timeout--)
    {
        // 等待媒体解码设备准备好
        if (media_decode_device_ready == false)
        {
            ak_sleep_ms(1);  // 休眠1ms
            continue;
        }

        // 首次处理时清除解码器缓冲区
        if (is_first == true)
        {
            media_thumb_decode_clear(media_mjpeg_handle_id);
            is_first = false;
        }

        // 发送JPEG数据流到解码器
        thumb_stream_send(media_mjpeg_handle_id, jpg_buffer, jpg_size);
        
        // 尝试显示解码后的帧
        if (thumb_frame_display(media_mjpeg_handle_id, x, y, w, h) == true)
        {
            break;  // 成功显示则退出循环
        }
        
        ak_sleep_ms(1);  // 休眠1ms
    }

    // 释放JPEG数据缓冲区
    ak_mem_free(jpg_buffer);

    return true;
}

/**
 * 等待视频缩略图设备准备就绪
 * 
 * @return 准备好返回true，超时返回false
 */
static bool video_thumb_wait_ready(void)
{
    int timeout = 1000;  // 设置超时计数
    
    // 循环等待直到超时或设备准备好
    while (timeout--)
    {
        if (media_decode_device_ready == true)
        {
            return true;
        }
        ak_sleep_ms(1);  // 休眠1ms
    }
    return false;
}

/**
 * 加载视频缩略图
 * 
 * @param x          显示位置X坐标
 * @param y          显示位置Y坐标
 * @param w          显示宽度
 * @param h          显示高度
 * @param file_path  视频文件路径
 * @return 成功返回true，失败返回false
 */
static bool video_thumb_load(int x, int y, int w, int h, const char *file_path)
{
    // 打开AVI视频文件
    avi_t *avi_file = AVI_open_input_file(file_path, 1);
    if (avi_file == NULL)
    {
        printf("%s %d  %s\n", __func__, __LINE__, file_path);
        return false;
    }

    int *handle_id = NULL;  // 解码器句柄指针
    
    // 获取视频压缩格式
    char *format = AVI_video_compressor(avi_file);
    if (strcmp(format, "H264") == 0)
    {
        handle_id = &media_h264_handle_id;  // 使用H.264解码器
    }
    else if (strcmp(format, "MJPG") == 0)
    {
        handle_id = &media_mjpeg_handle_id;  // 使用MJPEG解码器
    }

    int iskeyframe;  // 关键帧标志
    int total = AVI_video_frames(avi_file);  // 获取视频总帧数

    char *frame_buffer = NULL;  // 帧数据缓冲区
    int i = 0;                  // 帧计数器
    bool is_first = true;       // 首次处理标志

    // 遍历视频帧
    for (i = 0; i < total; i++)
    {
        // 设置视频位置到指定帧
        if (AVI_set_video_position(avi_file, i) != 0)
        {
            break;
        }

        // 获取当前帧大小
        int frame_size = AVI_frame_size(avi_file, i);

        if (frame_size <= 0)  // 检查帧大小是否有效
        {
            continue;
        }
        
        // 分配内存存储帧数据
        frame_buffer = (char *)ak_mem_alloc(MODULE_ID_VDEC, frame_size);
        if (frame_buffer == NULL)
        {
            break;
        }

        // 读取帧数据
        if (AVI_read_frame(avi_file, frame_buffer, &iskeyframe) < 0)
        {
            break;
        }

        // 等待视频缩略图设备准备就绪
        if (video_thumb_wait_ready() == false)
        {
            break;
        }

        // 首次处理时清除解码器缓冲区
        if (is_first == true)
        {
            media_thumb_decode_clear(*handle_id);
            is_first = false;
        }
        
        // 发送帧数据到解码器
        thumb_stream_send(*handle_id, (unsigned char *)frame_buffer, frame_size);
        
        // 释放帧数据缓冲区
        ak_mem_free(frame_buffer);
        frame_buffer = NULL;
        
        // 尝试显示解码后的帧，成功则退出循环
        if (thumb_frame_display(*handle_id, x, y, w, h) == true)
        {
            break;
        }
    }

    // 释放可能未释放的帧数据缓冲区
    if (frame_buffer != NULL)
    {
        ak_mem_free(frame_buffer);
    }
    
    // 关闭AVI文件
    AVI_close(avi_file);
    return true;
}

/**
 * 加载媒体缩略图(支持JPEG和AVI)
 * 
 * @param x          显示位置X坐标
 * @param y          显示位置Y坐标
 * @param w          显示宽度
 * @param h          显示高度
 * @param file_path  媒体文件路径
 * @return 成功返回true，失败返回false
 */
bool media_thumb_load(int x, int y, int w, int h, const char *file_path)
{
    // 查找文件扩展名
    char *p = strrchr(file_path, '.');
    if (p == NULL)  // 检查是否有扩展名
    {
        return false;
    }
    
    // 检查扩展名长度
    int str_len = strlen(p);
    if (str_len != 4)
    {
        return false;
    }

    // 等待视频缩略图设备准备就绪
    if (video_thumb_wait_ready() == false)
    {
        return false;
    }

    bool result = true;  // 结果标志
    
    // 加锁防止并发访问
    ak_thread_mutex_lock(&media_thumb_mutex);
    
    // 检查媒体解码设备是否准备好
    if (media_decode_device_ready == false)
    {
        ak_thread_mutex_unlock(&media_thumb_mutex);
        return result;
    }

    // 根据文件扩展名选择处理方式
    if ((p[1] == 'j') || (p[1] == 'J'))  // JPEG文件
    {
        result = jpeg_thumb_load(x, y, w, h, file_path);
    }
    else if ((p[1] == 'A') || (p[1] == 'a'))  // AVI文件
    {
        result = video_thumb_load(x, y, w, h, file_path);
        if (result == false)
        {
            // 视频加载失败处理
        }
    }
    
    // 解锁互斥锁
    ak_thread_mutex_unlock(&media_thumb_mutex);
    return result;
}