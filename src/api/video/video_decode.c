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
#include "leo_api.h"
#include "ring_buffer.h"

/**
 * 视频解码模块 - 支持H.264、MJPEG和H.265格式
 * 
 * 该模块实现了视频流的解码、显示和相关控制功能
 */

static int decode_src_width = 0;      // 解码源视频宽度
static int decode_src_height = 0;     // 解码源视频高度

static bool video_decode_run = false;         // 视频解码运行标志
static bool video_decode_thread_run = false;  // 视频解码线程运行标志
static bool video_decode_ready = false;       // 视频解码就绪标志
static int video_decode_handle_id = -1;       // 视频解码器句柄

pthread_mutex_t video_main_display_mutex;     // 显示主互斥锁

static char video_decode_type = 0;  // 解码类型: 0:H.264 1:MJPEG 2:H.265

static ak_mutex_t video_decode_mutex;        // 视频解码互斥锁
static ring_buffer video_decode_ring_buffer;  // 视频解码环形缓冲区

bool video_auto_record_falg = false;  // 视频自动录制标志
bool tuya_motion_record = false;      // 涂鸦移动检测录制标志


/**
 * 打开视频解码设备
 * 
 * @return 成功返回true，失败返回false
 */
static bool video_decode_device_open(void)
{
    printf("video_decode_device_open start\n");
    
    // 初始化解码器参数
    struct ak_vdec_param param;
    memset(&param, 0, sizeof(struct ak_vdec_param));
    
    // 根据解码类型设置编码器类型
    param.vdec_type = video_decode_type == 1?MJPEG_ENC_TYPE:video_decode_type == 2?HEVC_ENC_TYPE:H264_ENC_TYPE;
    param.output_type = AK_YUV420SP;           // 输出格式为YUV420SP
    param.sc_width = decode_src_width;         // 设置缩放宽度
    param.sc_height = decode_src_height;       // 设置缩放高度
    param.stream_buf_size = 2 *1024*1024;      // 流缓冲区大小为2MB
    param.frame_buf_num = 3;                   // 帧缓冲区数量为3
    
    // 打开解码器
    if(ak_vdec_open(&param, &video_decode_handle_id))
    {
        perror("open vdec failed\n");
        return false;
    }
    
    // 清除解码器缓冲区
    ak_vdec_clear_buff(video_decode_handle_id);
    printf("video_decode_device_open finish\n");
    
    // 初始化环形缓冲区
    ring_buffer_init(&video_decode_ring_buffer,1024*1024,&video_decode_mutex);
    return true;    
}

/**
 * 关闭视频解码设备
 */
static void video_decode_device_close(void)
{
    // 关闭解码器
    if(video_decode_handle_id != -1)
    {
        ak_vdec_close(video_decode_handle_id);
        video_decode_handle_id = -1;
    }

    // 释放环形缓冲区资源
    ring_buffer_release(&video_decode_ring_buffer);    
}

/**
 * 发送视频数据流到解码器
 * 
 * @param pdata 视频数据指针
 * @param len   数据长度
 */
static void video_data_stream_send(unsigned char* pdata,int len)
{
    int dec_len = 0;      // 已解码数据长度
    int read_len = len;   // 剩余要发送的数据长度
    int send_len = 0;     // 已发送数据长度

	while(read_len > 0)
	{	
		// printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());	
		ak_vdec_send_stream(video_decode_handle_id, &pdata[send_len], read_len, NONBLOCK, &dec_len);	
        read_len -= dec_len;  // 更新剩余数据长度
        send_len += dec_len;  // 更新已发送数据长度
	}
}

/**
 * 视频解码线程函数 - 处理解码后的帧数据
 * 
 * @param arg 线程参数(未使用)
 * @return 线程返回值
 */
//static bool snap = true;
//static bool snap = true;
static void* video_decode_thread_task(void*arg)
{
	struct ak_vdec_frame frame = {0};

	int count = 0;
	// printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
	int frame_decodec_index = 0;
	while(1)
	{
		// printf("===22===%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
		memset(&frame,0,sizeof(struct ak_vdec_frame));
		
		if(ak_vdec_get_frame(video_decode_handle_id, &frame) == 0)
		{	
#if 0
			if(snap  == true)
			{
				FILE *fp = fopen("/etc/config/leo/640_360.yuv","wb+");
				if(fp == NULL)
				{
					printf("open yuv error\n");
					
				}else {

					int write_len = fwrite(frame.frame_obj.data.data,sizeof(char), frame.frame_obj.data.data_size, fp);
					if(write_len != frame.frame_obj.data.data_size){
						printf("write yuv error\n");
						
					}
					fclose(fp);
				}
				snap  = false;
			}
	#endif
			//cctv数据太多处理不过来会造成马赛克，适当丢弃一些数据
			frame_decodec_index++;
			if((frame.width > 1280)||(frame.height > 720)){
				if((frame_decodec_index % 2) ==0){
					ak_vdec_release_frame(video_decode_handle_id,  &frame);	
					if(video_auto_record_falg == false)
						video_auto_record_falg = true;
					continue;
				}
			}
			count = 0;
			video_raw_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height);//,frame.width,frame.height);
			extern void screen_force_refresh(void);
			screen_force_refresh();
			ak_vdec_release_frame(video_decode_handle_id,  &frame);	
			//视频稳定的标志位
			if(video_auto_record_falg == false)
				video_auto_record_falg = true;		
		}
		else if((count++) > 100)
		{
			int status = 0;
			ak_vdec_get_decode_finish(video_decode_handle_id, &status);
			
			if(status)
			{
				break;
			}
		}
		 ak_sleep_ms(1);
	}
	video_raw_release_all();
	video_auto_record_falg = false;
	printf("===========<<< video h264 stream stop  >>>===========\n");
	ak_thread_exit();
	return NULL;
}





/**
 * 视频解码任务线程函数 - 主解码控制线程
 * 
 * @param arg 线程参数(未使用)
 * @return 线程返回值
 */
static void* video_decode_task(void* arg)
{
    video_decode_thread_run = true;  // 标记解码线程正在运行

    // 打开视频解码设备
    if(video_decode_device_open() == false)
    {
        perror("open video decode device failed\n");
    }

//	//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
	// 创建视频解码线程
    ak_pthread_t  stream_thread_id;
    ak_thread_create(&stream_thread_id, video_decode_thread_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);

    // 分配数据缓冲区
    char* buffer = ak_mem_alloc(MODULE_ID_VDEC, 100*1024);
    int read_len = 0;
    
    video_decode_ready = true;  // 标记解码准备就绪
    
    // 主循环，从环形缓冲区读取数据并发送到解码器
	while(video_decode_run == true)
	{
		// 从环形缓冲区读取数据
		read_len = ring_buffer_read(&video_decode_ring_buffer, buffer, 100*1024);
		if(read_len > 0)
		{
				//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
			
			// 发送数据到解码器
			video_data_stream_send((unsigned char*)buffer,read_len);
			ak_sleep_ms(10);
		}
		else
		{
			 // 无数据可读，休眠10ms
			ak_sleep_ms(10);
		}
	}
	  video_decode_ready = false;  // 标记解码不再就绪

    // 释放数据缓冲区
    ak_mem_free(buffer);

    // 结束解码器数据流
    ak_vdec_end_stream(video_decode_handle_id);

    // 等待解码线程退出
    ak_thread_join(stream_thread_id);

    // 关闭视频解码设备
    video_decode_device_close();

    video_decode_thread_run = false;  // 标记解码线程已停止
    video_auto_record_falg = false;   // 标记视频自动录制停止
    
    printf("===========<<<video h264 decode finish >>>===========\n");
    ak_thread_exit();  // 退出线程
    return NULL;
}

/**
 * 等待视频解码线程退出
 * 
 * @return 成功等待返回true，超时返回false
 */
bool video_decode_wait_thread_quit(void)
{
    int timeout = 300;  // 设置超时时间
    
    // 循环等待线程退出
    while(timeout--)
    {
        if(video_decode_thread_run == false)
        {
            return true;
        }
        usleep(10*1000);  // 休眠10ms
    }
    return false;
}

/**
 * 打开视频解码器
 * 
 * @param type       解码类型(0:H.264 1:MJPEG 2:H.265)
 * @param src_width  源视频宽度
 * @param src_height 源视频高度
 * @return 成功返回true，失败返回false
 */
bool video_decode_open(char type,int src_width,int src_height)
{
    // 检查解码器是否已在运行
    if(video_decode_run == true)
    {
        printf("video decode open after \n");
        return false;
    }
    
    // 等待之前的解码线程退出
    if( video_decode_wait_thread_quit() == false)
    {
        printf("video decode thread wait error \n");
        return false;
    }

    // 设置解码参数
    decode_src_width = src_width;
    decode_src_height = src_height;
    video_decode_type = type;

    // 初始化解码状态
    video_decode_ready = false;
    video_decode_run = true;
    
    // 创建视频解码任务线程
    ak_pthread_t video_decode_thread = 0;
    ak_thread_create(&video_decode_thread, video_decode_task , NULL , ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
    ak_thread_detach(video_decode_thread);  //cyy：分离线程，使其可自动回收资源
    
    return true;
}

/**
 * 初始化视频解码模块
 */
void video_decode_init(void)
{
    // 初始化视频解码互斥锁
    ak_thread_mutex_init(&video_decode_mutex, NULL);
}

/**
 * 关闭视频解码器
 * 
 * @return 成功返回true，失败返回false
 */
bool video_decode_close(void)
{
    // 检查解码器是否在运行
    if(video_decode_run == false)
    {
        return false;
    }
    
    // 标记解码器停止运行
    video_decode_run = false;
    return true;
}

/**
 * 推送视频数据到解码器
 * 
 * @param type  数据类型(0:H.264 1:MJPEG 2:H.265)
 * @param data  视频数据指针
 * @param len   数据长度
 * @return 成功返回true，失败返回false
 */
bool video_decode_push(char type,unsigned char* data ,int len)
{
    // 检查解码器状态和数据类型
    if((video_decode_run == false)||(video_decode_ready == false))
    {
        return false;
    }
    
    // 检查数据类型是否匹配
    if(type != video_decode_type)
    {
        return false;
    }
    
    // 将数据写入环形缓冲区
    ring_buffer_write(&video_decode_ring_buffer,(char*)data, len);
    return true;
}
