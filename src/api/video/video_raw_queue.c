#include "leo_api.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"
#include "queue.h"
#include "ak_mem.h"
#include "ak_vdec.h"
#include "string.h"
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "ak_thread.h"


/**
 * 视频原始帧队列最大长度
 */
#define VIDEO_FRAME_QUEUE_MAX 1

/**
 * 视频帧默认宽度和高度
 */
#define VIDEO_FRAME_WIDTH 1024
#define VIDEO_FRAME_HEIGHT 600

/**
 * 视频原始帧结构定义
 * 包含双向链表指针和视频帧属性
 */
typedef struct
{
    void* prev;     // 前驱节点指针
    void* next;     // 后继节点指针
    int width;      // 帧宽度
    int height;     // 帧高度
    unsigned char* addres;  // 帧数据地址
}video_raw_frame;

static queue_s video_raw_queue_free;    // 空闲帧队列
static queue_s video_raw_queue_head;    // 已使用帧队列头
static ak_mutex_t video_raw_head_mutex; // 已使用队列互斥锁
static ak_mutex_t video_raw_free_mutex; // 空闲队列互斥锁
static video_raw_frame frame_buffer[VIDEO_FRAME_QUEUE_MAX]; // 帧缓冲区

/**
 * 初始化视频原始帧管理系统
 * 初始化互斥锁和队列，将所有帧节点放入空闲队列
 */
void video_raw_init(void)
{
    // 初始化互斥锁
    ak_thread_mutex_init(&video_raw_head_mutex, NULL);
    ak_thread_mutex_init(&video_raw_free_mutex, NULL);
    
    // 初始化队列
    queue_initialize(&video_raw_queue_head);
    queue_initialize(&video_raw_queue_free);
    
    // 清空帧缓冲区并将所有节点加入空闲队列
    memset(frame_buffer,0,sizeof(video_raw_frame)*VIDEO_FRAME_QUEUE_MAX);
    for(int i = 0 ; i < VIDEO_FRAME_QUEUE_MAX ; i++)
    {
        queue_insert((queue_s*)&frame_buffer[i], &video_raw_queue_free);
    }
}

/**
 * 从空闲队列获取新的视频帧节点并分配内存
 * @param width 帧宽度
 * @param height 帧高度
 * @return 成功返回帧节点指针，失败返回NULL
 */
static video_raw_frame* video_raw_queue_node_new(int width,int height)
{
    video_raw_frame* node = NULL;
    
    // 从空闲队列获取节点
    ak_thread_mutex_lock(&video_raw_free_mutex);
    if(queue_empty(&video_raw_queue_free) == 0)
    {    
        node = (video_raw_frame*)queue_delete_next(&video_raw_queue_free);
    }
    ak_thread_mutex_unlock(&video_raw_free_mutex);
    
    if(node == NULL)
    {
        return NULL;
    }

    // 释放原有内存并重新分配
    if(node->addres != NULL)
    {
        ak_mem_dma_free(node->addres);
    }
    //printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__, width*height*3/2);
    
    // 分配DMA内存，大小为YUV420SP格式所需空间
    node->addres = (unsigned char*)ak_mem_dma_alloc(MODULE_ID_VDEC, width*height*3/2);
    if(node->addres == NULL)
    {
        perror("video_raw_queue_node_new failed\n");
        return NULL;
    }
    
    // 设置帧属性
    node->width = width;
    node->height = height;
    return node;
}

/**
 * 将视频帧节点释放并放回空闲队列
 * @param node 待释放的帧节点指针
 */
static void video_raw_queue_node_del(video_raw_frame* node)
{
    if(node != NULL)
    {
        // 释放帧内存
        if(node->addres != NULL)
        {
            ak_mem_dma_free(node->addres);
            node->addres = NULL;
        }
        
        // 重置帧属性
        node->width = node->height = 0;
        
        // 将节点放回空闲队列
        ak_thread_mutex_lock(&video_raw_free_mutex);
        queue_insert((queue_s*)node, &video_raw_queue_free);
        ak_thread_mutex_unlock(&video_raw_free_mutex);
    }
}

/**
 * 视频帧缩放/复制操作
 * 使用TDE(Transform Display Engine)硬件加速进行图像转换
 * @param src_addres 源帧虚拟地址
 * @param phy 源帧物理地址(若为0则使用虚拟地址转换)
 * @param src_width 源帧实际宽度
 * @param src_height 源帧实际高度
 * @param pixel_width 源帧像素宽度
 * @param pixel_height 源帧像素高度
 * @param src_format 源帧格式
 * @param dst_addres 目标帧虚拟地址
 * @param dst_width 目标帧宽度
 * @param dst_height 目标帧高度
 * @param dst_format 目标帧格式
 */
static void video_raw_biblt(unsigned char* src_addres,unsigned long phy,int src_width,int src_height,int pixel_width,int pixel_height,  enum ak_gp_format src_format,
                            unsigned char* dst_addres,int dst_width,int dst_height,  enum ak_gp_format dst_format)
{
    struct ak_tde_layer dst,src;
    
    // 配置源图层参数
    src.format_param = src_format;
    src.width = pixel_width;
    src.height = pixel_height;
    src.pos_left = 0;
    src.pos_top = 0;
    src.pos_width = src_width;
    src.pos_height = src_height;
    
    // 设置源图层物理地址
    if(phy != 0)
    {
        src.phyaddr = phy;
    }
    else
    {
        ak_mem_dma_vaddr2paddr(src_addres, (unsigned long *)&src.phyaddr);
    }

    // 配置目标图层参数
    dst.format_param = dst_format;
    dst.width = dst_width;
    dst.height = dst_height;
    dst.pos_left = 0;
    dst.pos_top = 0;
    dst.pos_width = dst_width;
    dst.pos_height = dst_height;
    
    // 获取目标图层物理地址
    ak_mem_dma_vaddr2paddr(dst_addres, (unsigned long *)&dst.phyaddr);
    
    // 根据源和目标尺寸关系选择缩放或直接复制
    if((src_width != dst_width)||(src_height != dst_height))
    {
        ak_tde_opt_scale(&src,&dst);  // 缩放操作
    }
    else
    {
        ak_tde_opt_blit(&src,&dst);   // 直接复制
    }
}

/**
 * 将视频帧数据压入原始帧队列
 * @param addres 源视频帧数据地址
 * @param phy 源视频帧物理地址
 * @param width 源视频帧宽度
 * @param height 源视频帧高度
 * @param pixel_width 源视频帧像素宽度
 * @param pixel_height 源视频帧像素高度
 * @return 成功返回true，失败返回false
 */
bool video_raw_push(unsigned char* addres,unsigned long phy,int width,int height,int pixel_width,int pixel_height)
{
    // 从空闲队列获取新节点
    video_raw_frame* node  = video_raw_queue_node_new(VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT);
    if(node == NULL)
    {    
        //printf("video update skip frame \n\r");
        return false;
    }
    
    // 将源帧数据缩放/复制到新节点
    video_raw_biblt(addres,phy,width,height,pixel_width,pixel_height,GP_FORMAT_YUV420SP,node->addres,node->width,node->height,GP_FORMAT_YUV420SP);
    
    // 将节点加入已使用队列
    ak_thread_mutex_lock(&video_raw_head_mutex);
    queue_insert((queue_s*)node, &video_raw_queue_head);
    ak_thread_mutex_unlock(&video_raw_head_mutex);
    
    return true;
}

/**
 * 从原始帧队列获取视频帧数据
 * @param dst 目标缓冲区地址
 * @param width 目标宽度
 * @param height 目标高度
 * @param foramt 目标格式(-1表示默认YUV420SP)
 * @return 成功返回true，失败返回false
 */
bool video_raw_get(unsigned char* dst,int width,int height,int foramt)
{
    bool result = false;
    video_raw_frame *node = NULL;
    
    // 从已使用队列获取节点
    ak_thread_mutex_lock(&video_raw_head_mutex);
    if(queue_empty(&video_raw_queue_head) == 0 )
    {
        node =  (video_raw_frame *)queue_delete_next(&video_raw_queue_head);
    }
    ak_thread_mutex_unlock(&video_raw_head_mutex);
    
    if(node != NULL)
    {
        // 设置默认格式
        if(foramt == -1)
        {
            foramt =  GP_FORMAT_YUV420SP;
        }
        
        // 将帧数据缩放/复制到目标缓冲区
        video_raw_biblt(node->addres, 0 ,node->width,node->height,node->width,node->height,GP_FORMAT_YUV420SP,dst,width,height,foramt);
        
        // 释放节点
        video_raw_queue_node_del(node);
        result =  true;
    }
    return result;
}

/**
 * 释放所有视频原始帧资源
 * 清空已使用队列并释放所有帧内存
 */
void video_raw_release_all(void)
{
    // 锁定已使用队列并释放所有节点
    ak_thread_mutex_lock(&video_raw_head_mutex);
    while(!queue_empty (&video_raw_queue_head))
    {
        video_raw_frame *node = (video_raw_frame *)queue_delete_next(&video_raw_queue_head);
        video_raw_queue_node_del(node);
    }    
    ak_thread_mutex_unlock(&video_raw_head_mutex);
}