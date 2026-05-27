/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"

#include "../include/anyka/ak_common.h"
#include "../include/anyka/ak_common_graphics.h"
#include "../include/anyka/ak_tde.h"
#include "../include/anyka/ak_mem.h"
#include "../include/anyka/ak_vdec.h"

#include <linux/fb.h>

#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include "unistd.h"
#include "../include/anyka/ak_thread.h"
#include "../api/video/video_decode.h"
#include "../layout/user_time.h"
#define FBDEV_PATH "/dev/fb0"

#define LV_HOR_RES_MAX_CUSTOM (1280)
#define LV_VER_RES_MAX_CUSTOM (800)

#define LCD_FORMAT_24

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

#if LV_USE_GPU
// static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_blend(lv_disp_drv_t *disp_drv, lv_color_t *dest_buf, const lv_color_t *src_buf, const lv_area_t *src_area, const lv_area_t *dst_area, lv_opa_t opa, bool a_color);

static void gpu_fill(lv_disp_drv_t *disp_drv, lv_color_t *dest_buf, lv_coord_t dest_width,
					 const lv_area_t *fill_area, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;

static unsigned char *fbp = 0;
static unsigned long fbp_phyaddres = 0;
static long int screensize = 0;
static int fbfd = 0;
/***
**   日期:2022-05-28 16:13:10
**   作者: leo.liu
**   函数作用：禁止视频显示
**   参数说明:
***/
static bool lv_fb_video_display_preview = true;
/***
** 日期: 2022-04-26 14:59
** 作者: leo.liu
** 函数作用：进入视频模式使能
** 返回参数说明：
***/
static bool is_video_mode_enable = false;
/***
** 日期: 2022-04-26 14:59
** 作者: leo.liu
** 函数作用：进入视频模式使能
** 返回参数说明：
***/
static bool is_playback_video_mode = false;

#define FB_BUFFER_A 0
#define FB_BUFFER_B 1
#define FB_BUFFER_GET ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)
#define FB_BUFFER_SET ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);
#define FB_BUFFER vinfo.reserved[0]

static unsigned char *gui_buffer = NULL;
static unsigned long gui_phyaddres = 0;
static unsigned char *platform_address = NULL;
static unsigned char *system_bg_addres = NULL;
static unsigned char *system_bg_temp_addres = NULL;
static unsigned long system_bg_phyaddres = 0;

static sem_t lv_fb_sem_t;
static void *platform_refresh_screen_task(void *arg);
static unsigned char *lv_gui_addres = NULL;
void lv_port_disp_init(void)
{
	disp_init();

	static lv_disp_buf_t draw_buf_dsc_3;
	lv_gui_addres = (unsigned char *)ak_mem_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	lv_disp_buf_init(&draw_buf_dsc_3, lv_gui_addres, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);

	disp_drv.hor_res = LV_HOR_RES_MAX;
	disp_drv.ver_res = LV_VER_RES_MAX;

	disp_drv.flush_cb = disp_flush;

	/*Set a display buffer*/
	disp_drv.buffer = &draw_buf_dsc_3;

#if LV_USE_GPU
	disp_drv.gpu_blend_cb = gpu_blend;

	/*Fill a memory array with a color*/
	disp_drv.gpu_fill_cb = gpu_fill;
#endif
	lv_disp_drv_register(&disp_drv);

	ak_pthread_t thread_t;
	ak_thread_create(&thread_t, platform_refresh_screen_task, NULL, 200 * 1024, -1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
	// system("rmmod /usr/modules/ak_fb.ko");
	system("insmod /usr/modules/ak_fb.ko");
	system("insmod /usr/modules/ak_gui.ko");

	sdk_run_config config = {0};
	config.mem_trace_flag = SDK_RUN_NORMAL;
	ak_sdk_init(&config);
	ak_tde_open();

	/*You code here*/ 
	fbfd = open(FBDEV_PATH, O_RDWR | O_EXCL);
	if (fbfd == -1)
	{
		perror("Error: cannot open framebuffer device");
		return;
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
	{
		perror("Error reading fixed information");
		return;
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
	{
		perror("Error reading variable information");
		return;
	}

	vinfo.xres = vinfo.xres_virtual;
	vinfo.yres = vinfo.yres_virtual;
#ifdef LCD_FORMAT_24
	vinfo.bits_per_pixel = 24;
	vinfo.red.offset = 16;
	vinfo.red.length = 8;
	vinfo.green.offset = 8;
	vinfo.green.length = 8;
	vinfo.blue.offset = 0;
	vinfo.blue.length = 8;

#else
	vinfo.bits_per_pixel = 16;
	vinfo.red.offset = 11;
	vinfo.red.length = 5;
	vinfo.green.offset = 5;
	vinfo.green.length = 6;
	vinfo.blue.offset = 0;
	vinfo.blue.length = 5;
#endif
	ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);

	int bpp = vinfo.bits_per_pixel;
	screensize = vinfo.xres * vinfo.yres * bpp / 8;

	extern void *osal_fb_mmap_viraddr(int fb_len, int fb_fd);
	fbp = (unsigned char *)osal_fb_mmap_viraddr(screensize * 2, fbfd);

	fbp_phyaddres = finfo.smem_start;
	printf("fb buffer 1:%p,2:%p ,phy:%lu ,2:%lu \n", fbp, fbp + screensize, fbp_phyaddres, fbp_phyaddres + screensize);

	gui_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	// printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	ak_mem_dma_vaddr2paddr(gui_buffer, (unsigned long *)&gui_phyaddres);

	system_bg_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	ak_mem_dma_vaddr2paddr(system_bg_addres, (unsigned long *)&system_bg_phyaddres);
	

	FB_BUFFER = FB_BUFFER_A;
	struct ak_tde_layer layer;
#ifdef LCD_FORMAT_24
	layer.format_param = GP_FORMAT_RGB888;
#else
	layer.format_param = GP_FORMAT_RGB565;
#endif
	layer.width = vinfo.xres; // screen_vector.width;
	layer.height = vinfo.yres;
	layer.pos_left = 0;
	layer.pos_top = 0;
	layer.pos_width = vinfo.xres;
	layer.pos_height = vinfo.yres;
	layer.phyaddr = FB_BUFFER == FB_BUFFER_B ? (fbp_phyaddres + screensize) : fbp_phyaddres;
	ak_tde_opt_fillrect(&layer, 0x00);
	FB_BUFFER_SET;
	printf("The framebuffer device was mapped to memory successfully.\n");
}

static bool fb_dst_tde_layer_get(struct ak_tde_layer *dst, const lv_area_t *area_t)
{
#ifdef LCD_FORMAT_24
	dst->format_param = GP_FORMAT_RGB888;
#else
	dst->format_param = GP_FORMAT_RGB565;
#endif
	dst->width = vinfo.xres; // screen_vector.width;
	dst->height = vinfo.yres;
	dst->pos_left = 0;
	dst->pos_top = 0;
	dst->pos_width = vinfo.xres;
	dst->pos_height = vinfo.yres;
	dst->phyaddr = FB_BUFFER == FB_BUFFER_B ? (fbp_phyaddres + screensize) : fbp_phyaddres;
	return true;
}

static bool guid_dst_tde_layer_get(struct ak_tde_layer *dst)
{
#ifdef LCD_FORMAT_24
	dst->format_param = GP_FORMAT_RGB888;
#else
	dst->format_param = GP_FORMAT_RGB565;
#endif
	dst->width = LV_HOR_RES_MAX_CUSTOM; // screen_vector.width;
	dst->height = LV_VER_RES_MAX_CUSTOM;
	dst->pos_left = 0;
	dst->pos_top = 0;
	dst->pos_width = LV_HOR_RES_MAX_CUSTOM;
	dst->pos_height = LV_VER_RES_MAX_CUSTOM;
	if (platform_address == NULL)
	{
		platform_address = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_VER_RES_MAX_CUSTOM * LV_HOR_RES_MAX_CUSTOM * 3);
		ak_mem_dma_vaddr2paddr(platform_address, (unsigned long *)&dst->phyaddr);
	}

	return true;
}

static bool bg_tde_layer_get(struct ak_tde_layer *dst, const lv_area_t *area_t)
{
	dst->format_param = GP_FORMAT_RGB888;
	dst->width = LV_HOR_RES_MAX;
	dst->height = LV_VER_RES_MAX;
	dst->pos_left = area_t == NULL ? 0 : area_t->x1;
	dst->pos_top = area_t == NULL ? 0 : area_t->y1;
	dst->pos_width = area_t == NULL ? LV_HOR_RES_MAX : lv_area_get_width(area_t) - 1;
	dst->pos_height = area_t == NULL ? LV_VER_RES_MAX : lv_area_get_height(area_t) - 1;
	if (system_bg_addres == NULL)
	{
		system_bg_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
		ak_mem_dma_vaddr2paddr(system_bg_addres, (unsigned long *)&system_bg_phyaddres);
	}
	dst->phyaddr = system_bg_phyaddres;
	return true;
}

static bool gui_tde_layer_get(struct ak_tde_layer *dst, const lv_area_t *area_t)
{
	dst->format_param = GP_FORMAT_ARGB8888;
	dst->width = LV_HOR_RES_MAX; // screen_vector.width;
	dst->height = LV_VER_RES_MAX;
	dst->pos_left = area_t == NULL ? 0 : area_t->x1;
	dst->pos_top = area_t == NULL ? 0 : area_t->y1;
	dst->pos_width = area_t == NULL ? LV_HOR_RES_MAX : lv_area_get_width(area_t) - 1;
	dst->pos_height = area_t == NULL ? LV_VER_RES_MAX : lv_area_get_height(area_t) - 1;
	dst->phyaddr = gui_phyaddres;
	return true;
}

static void system_bg_decode_data_copy(struct ak_vdec_frame *frame)
{
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = frame->frame_obj.data.pitch_width;
	src.height = frame->frame_obj.data.pitch_height;
	src.pos_top = src.pos_left = 0;
	src.pos_width = frame->width;
	src.pos_height = frame->height;
	ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long *)&src.phyaddr);

	bg_tde_layer_get(&dst, NULL);
	if ((frame->width == LV_HOR_RES_MAX) && (frame->height == LV_VER_RES_MAX))
	{
		ak_tde_opt_blit(&src, &dst);
	}
	else
	{
		ak_tde_opt_scale(&src, &dst);
	}
}

static void *bg_load_close_task(void *arg)
{
	int *handle_id = (int *)arg;
	ak_vdec_close(*handle_id);
	*handle_id = -1;
	ak_thread_exit();
	return NULL;
}

static void system_bg_resource_load(rom_bin_info *info)
{
	extern const unsigned char *get_rom_bin_base(void);
	struct ak_vdec_param param;
	memset(&param, 0, sizeof(struct ak_vdec_param));
	param.vdec_type = MJPEG_ENC_TYPE;
	param.output_type = AK_YUV420SP;
	param.sc_width = LV_HOR_RES_MAX;
	param.sc_height = LV_VER_RES_MAX;

	static int handle_id = -1;
	int delay_count = 0;
	while (handle_id != (-1))
	{
		ak_sleep_ms(1);
		if ((delay_count++) > 500)
		{
			printf("loading bg fail \n");
			return;
		}
	}
	ak_vdec_open(&param, &handle_id);
	ak_vdec_clear_buff(handle_id);

	const unsigned char *pdata = get_rom_bin_base() + info->offset;

	int dec_len = 0;
	int read_len = info->size;
	int send_len = 0;
	while (read_len > 0)
	{
		ak_vdec_send_stream(handle_id, &pdata[send_len], read_len, NONBLOCK, &dec_len);
		read_len -= dec_len;
		send_len += dec_len;
	}

	struct ak_vdec_frame frame = {0};

	int timeout = 500;
	while (timeout--)
	{
		memset(&frame, 0, sizeof(frame));
		if (ak_vdec_get_frame(handle_id, &frame) == 0)
		{
			system_bg_decode_data_copy(&frame);
			ak_vdec_release_frame(handle_id, &frame);
			break;
		}
		ak_sleep_ms(1);
	}

	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, bg_load_close_task, &handle_id, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
}

static bool system_bg_enable = false;
void system_bg_enable_set(bool en)
{
	system_bg_enable = en;
}
bool system_bg_loading(rom_bin_info *info, bool extern_src)
{
	if (extern_src)
	{
		lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
		lv_disp_set_bg_image(lv_disp_get_default(), info);
	}
	else
	{
		lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
		system_bg_resource_load(info);
	}

	system_bg_enable = true;
	return true;
}

unsigned long system_bg_phyaddres_get(void)
{
	if (system_bg_addres == NULL)
	{
		system_bg_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
		// printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);

		ak_mem_dma_vaddr2paddr(system_bg_addres, (unsigned long *)&system_bg_phyaddres);
	}

	return system_bg_phyaddres;
}

unsigned char *system_bg_virtual_addres_get(void)
{
	if (system_bg_addres == NULL)
	{
		system_bg_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
		// printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);

		ak_mem_dma_vaddr2paddr(system_bg_addres, (unsigned long *)&system_bg_phyaddres);
	}

	return system_bg_addres;
}

bool system_bg_data_backup(void)
{
	if (system_bg_addres == NULL)
	{
		return false;
	}
	if (system_bg_temp_addres == NULL)
	{
		system_bg_temp_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	}
	memcpy(system_bg_temp_addres, system_bg_addres, LV_HOR_RES_MAX * LV_VER_RES_MAX * 2);
	/* 使用dma内存copy，会出现颜色失真*/
#if 0
	struct ak_tde_layer src,dst;
	bg_tde_layer_get(&src,NULL);

	dst = src;
	if(system_bg_temp_addres == NULL)
	{
		system_bg_temp_addres = (unsigned char*)ak_mem_dma_alloc(MODULE_ID_VO,LV_HOR_RES_MAX*LV_VER_RES_MAX*3/2);
	}
	ak_mem_dma_vaddr2paddr(system_bg_temp_addres, (unsigned long*)&dst.phyaddr);
	ak_tde_opt_blit(&src, &dst);
#endif
#if 0
	int fd = open("/mnt/yuvsp",O_WRONLY|O_CREAT);
	write(fd,system_bg_temp_addres,LV_HOR_RES_MAX*LV_VER_RES_MAX*3/2);
	close(fd);
#endif
	return true;
}

bool system_bg_data_recovery(void)
{
	if (system_bg_temp_addres == NULL)
	{
		return false;
	}

	memcpy(system_bg_addres, system_bg_temp_addres, LV_HOR_RES_MAX * LV_VER_RES_MAX * 2);

	/* 使用dma内存copy，会出现颜色失真*/
#if 0
	struct ak_tde_layer src,dst;
	bg_tde_layer_get(&dst,NULL);

	src = dst;
	ak_mem_dma_vaddr2paddr(system_bg_temp_addres, (unsigned long*)&src.phyaddr);
	ak_tde_opt_blit(&src, &dst);
#endif
	ak_mem_dma_free(system_bg_temp_addres);
	system_bg_temp_addres = NULL;
	return true;
}

bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h)
{
	if (system_bg_addres == NULL)
	{
		return false;
	}
#if 0
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_RGB565;
	src.pos_width = src.width = w;
	src.pos_height = src.height = h;
	src.pos_left = src.pos_top = 0;
	// printf("=========%s======%d====dma_alloc size is %d\n",__func__,__LINE__,  w * h * 2);
	unsigned char *addres = ak_mem_dma_alloc(MODULE_ID_VO, w * h * 2);

	ak_mem_dma_vaddr2paddr(addres, (unsigned long *)&src.phyaddr);
	ak_tde_opt_fillrect(&src, color);

	bg_tde_layer_get(&dst,NULL);
	dst.pos_left = x;
	dst.pos_top = y;
	dst.pos_width = w;
	dst.pos_height = h;
	ak_tde_opt_blit(&src, &dst);

	ak_mem_dma_free(addres);
#else
	// struct ak_tde_layer dst;
	// bg_tde_layer_get(&dst,NULL);
	// dst.pos_left = x;
	// dst.pos_top = y;
	// dst.pos_width = w;
	// dst.pos_height = h;
	// ak_tde_opt_fillrect(&dst, color);

	unsigned int *data = (unsigned int *)system_bg_addres + y * LV_HOR_RES_MAX + x;
	// int size = w * h;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			*data = 0xFF000000; // 将当前像素设为0xFF000000（不透明黑色）
			data += 1;			// 移动到下一个像素
		}
		data += LV_HOR_RES_MAX - w; // 移动到下一行的起始像素位置
	}

	
#endif
	return true;
}

//hlf:在视频回放页使用这个刷黑函数，否则删除视频会有花屏。
bool system_bg_fill_color_2(unsigned int color, int x, int y, int w, int h)
{
	struct ak_tde_layer bg_layer;
	lv_area_t area = {x, y, x+w, y+h};
	bg_tde_layer_get(&bg_layer, &area);
	ak_tde_opt_fillrect(&bg_layer, 0x000000);
	return true;
}

/**
 * 控制视频模式的启用/禁用状态
 * 
 * @param en 为true时启用视频模式，为false时禁用视频模式
 * 
 * 功能说明：
 * 1. 当启用视频模式时，将屏幕背景设为透明，确保视频内容可以正常显示
 * 2. 当禁用视频模式时：
 *    - 若system_bg_addres为NULL（无系统背景图），则显示不透明背景
 *    - 若system_bg_addres不为NULL（有系统背景图），则显示透明背景
 * 3. 若传入状态与当前状态一致，则不执行任何操作
 */
void fb_video_mode_enable(bool en)
{
	 // 如果状态没有变化，直接返回
	if (is_video_mode_enable == en)
	{
		return;
	}
	// 处理禁用视频模式的情况
	if (en == false)
	{
		  // 若无系统背景图，则显示不透明背景
		if (system_bg_addres == NULL)
		{
			  
			lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		}
		 // 处理启用视频模式的情况
		else
		{			         
			lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		}
	}
	else// 启用视频模式时，强制设置背景为透明，确保视频内容可见
	{
		lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	}
	is_video_mode_enable = en; // 更新当前视频模式状态标志
}

void fb_playback_video_mode_enable(bool en)
{
	is_playback_video_mode = en;
}

bool is_fb_video_mode_enable(void)
{
	return is_video_mode_enable;
}


#ifdef _PLATFORM_800_1280
static void bg_flush_to_fb(void)
{
	struct ak_tde_layer src, dst, platform;

	lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	bg_tde_layer_get(&src, &area);
	guid_dst_tde_layer_get(&platform);

	ak_tde_opt_scale(&src, &platform);

	fb_dst_tde_layer_get(&dst, NULL);
	struct ak_tde_cmd opt;
	memcpy(&(opt.tde_layer_src), &platform, sizeof(struct ak_tde_layer));
	memcpy(&(opt.tde_layer_dst), &dst, sizeof(struct ak_tde_layer));

	opt.opt = GP_OPT_ROTATE;
	opt.rotate_param = AK_GP_ROTATE_270;
	ak_tde_opt(&opt);
}
#else
static void bg_flush_to_fb(void)
{
	struct ak_tde_layer src, dst;

	lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	bg_tde_layer_get(&src, &area);

	fb_dst_tde_layer_get(&dst, NULL);
	struct ak_tde_cmd opt;
	memcpy(&(opt.tde_layer_src), &src, sizeof(struct ak_tde_layer));
	memcpy(&(opt.tde_layer_dst), &dst, sizeof(struct ak_tde_layer));

	opt.opt = GP_OPT_BLIT;
	ak_tde_opt(&opt);
}
#endif

static int gui_area_count = 0;
static lv_area_t gui_area_group[32] = {{0}};

typedef struct _lv_port_disp_data_info
{
	unsigned int opt;
	lv_area_t gui_area_group[32];
	int alpha;
	int color_max;
	int color_min;
	int coloract;
	/* data */
} lv_port_disp_data_info;

static lv_port_disp_data_info lv_port_disp_data = {0};

lv_port_disp_data_info *lv_port_disp_get(void)
{
	return &lv_port_disp_data;
}

void lv_port_disp_data_set(int opt, unsigned int alpha, int color_min, int color_max, int coloract)
{
	lv_port_disp_data.opt = opt;
	lv_port_disp_data.alpha = alpha;
	lv_port_disp_data.color_min = color_min;
	lv_port_disp_data.color_max = color_max;
	lv_port_disp_data.coloract = coloract;
}

void gui_draw_area_set(const lv_area_t *area_t, int count)
{
	memcpy(gui_area_group, area_t, count * sizeof(lv_area_t));
	gui_area_count = count;
}

static void gui_flush_to_bg(void)
{
	struct ak_tde_layer src, bg_dst;
	gui_tde_layer_get(&src, NULL);
	bg_tde_layer_get(&bg_dst, NULL);
	if (/* is_video_mode_enable == true */ 1)
	{
		struct ak_tde_cmd opt;
		memcpy(&(opt.tde_layer_src), &src, sizeof(struct ak_tde_layer));
		memcpy(&(opt.tde_layer_dst), &bg_dst, sizeof(struct ak_tde_layer));

		opt.opt = lv_port_disp_data.opt;
		opt.alpha = lv_port_disp_data.alpha;
		opt.colorkey_param.color_min = lv_port_disp_data.color_min;
		opt.colorkey_param.color_max = lv_port_disp_data.color_max;
		opt.colorkey_param.coloract = lv_port_disp_data.coloract;

		for (int i = 0; i < gui_area_count; i++)
		{
			int x = gui_area_group[i].x1;
			int y = gui_area_group[i].y1;
			int w = gui_area_group[i].x2 - gui_area_group[i].x1;
			int h = gui_area_group[i].y2 - gui_area_group[i].y1;
			tde_layer_pos_init(opt.tde_layer_src, x, y, w, h);
			tde_layer_pos_init(opt.tde_layer_dst, x, y, w, h);
			opt.opt = GP_OPT_BLIT;
			// printf("x,y,w,h is %d\n",x,y,w,h);
			ak_tde_opt(&opt);
		}
	}
}

/*
static void fb_flush_to_fb(void)
{
	FB_BUFFER_GET;
	FB_BUFFER = FB_BUFFER == FB_BUFFER_A ? FB_BUFFER_B : FB_BUFFER_A;
	if ((system_bg_enable == true) && (system_bg_addres != NULL))
	{
		bg_flush_to_fb();
	}
	gui_flush_to_bg();
	FB_BUFFER_SET;
}
 */

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	if (area->x2 < 0 || area->y2 < 0 || area->x1 > (int32_t)LV_HOR_RES_MAX || area->y1 > (int32_t)LV_VER_RES_MAX)
	{
		lv_disp_flush_ready(disp_drv);
		return;
	}
	//	video_main_display_lock();
	/***** 将图片tde biblt到gui layer上面 *****/
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_RGB565;
	src.width = lv_area_get_width(area);
	src.height = lv_area_get_height(area);
	// printf("w%d h%d \n", src.width, src.height);
	if (1) // ((src.width < 18) || (src.height < 18))
	{
		unsigned char *src_addr = lv_gui_addres;
		unsigned char *dst = gui_buffer + area->y1 * LV_HOR_RES_MAX * 4 + area->x1 * 4;
		for (int h = 0; h < src.height; h++)
		{
			memcpy(&dst[h * LV_HOR_RES_MAX * 4], &src_addr[h * src.width * 4], src.width * 4);
		}
	}
	else
	{
		src.pos_left = 0;
		src.pos_top = 0;
		src.width *= 2;
		src.pos_width = src.width;
		src.pos_height = src.height;
		ak_mem_dma_vaddr2paddr((void *)lv_gui_addres, (unsigned long *)&src.phyaddr);

		// memcpy(&dst, &src, sizeof(struct ak_tde_layer));
		dst.format_param = GP_FORMAT_RGB565;
		dst.width = LV_HOR_RES_MAX * 2;
		dst.height = LV_VER_RES_MAX;
		dst.pos_left = area->x1 * 2;
		dst.pos_top = area->y1;
		dst.pos_width = src.width;
		dst.pos_height = src.height;
		dst.phyaddr = gui_phyaddres;
		ak_tde_opt_blit(&src, &dst);
	}
	//	video_main_display_unlock();

	if (lv_disp_flush_is_last(disp_drv) == true)
	{
		int value;
		sem_getvalue(&lv_fb_sem_t, &value);
		if (value == 0)
		{
			sem_post(&lv_fb_sem_t);
		}
	}
	lv_disp_flush_ready(disp_drv);
}

void gui_raw_clear(void)
{
	video_main_display_lock();
	memset(gui_buffer, 0x00, 1024 * 600 * 4);
	video_main_display_unlock();
}
#ifdef _PLATFORM_800_1280
static void gui_flush_to_fb(void)
{
	struct ak_tde_layer src, dst, platform;

	// lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};

	gui_tde_layer_get(&src, NULL);
	guid_dst_tde_layer_get(&platform);

	ak_tde_opt_scale(&src, &platform);

	fb_dst_tde_layer_get(&dst, NULL);
	struct ak_tde_cmd opt;
	memcpy(&(opt.tde_layer_src), &platform, sizeof(struct ak_tde_layer));
	memcpy(&(opt.tde_layer_dst), &dst, sizeof(struct ak_tde_layer));

	opt.opt = GP_OPT_ROTATE;
	opt.rotate_param = AK_GP_ROTATE_270;
	ak_tde_opt(&opt);
}
#else
static void gui_flush_to_fb(void)
{
	struct ak_tde_layer src, dst;

	// lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};

	gui_tde_layer_get(&src, NULL);

		fb_dst_tde_layer_get(&dst, NULL);
	struct ak_tde_cmd opt;
	memcpy(&(opt.tde_layer_src), &src, sizeof(struct ak_tde_layer));
	memcpy(&(opt.tde_layer_dst), &dst, sizeof(struct ak_tde_layer));

	opt.opt = GP_OPT_BLIT;
	ak_tde_opt(&opt);
}
#endif

/**
 * 屏幕刷新任务 - 负责GUI和视频画面的显示更新
 * 此任务会循环刷新屏幕内容，根据不同模式(普通UI/视频监控)进行相应处理
 */
static void *platform_refresh_screen_task(void *arg)
{
    printf("***** refresh screen task create sccess ! *****\n");
    //	int average_frame_time = 0;	// 
    //	unsigned long long timestamp = user_timestamp_get();
    //	int frame_count = 0;
    unsigned long long pre_timestamp = 0;	// 上次视频帧时间戳
    unsigned long long cur_timestamp = 0;	// 当前时间戳
    
    // 屏幕刷新主循环
    while (1)
    {
        // 等待LVGL帧缓冲区信号量，确保与GUI渲染同步
        sem_wait(&lv_fb_sem_t);
        
        // 获取当前显示缓冲区并切换到另一个缓冲区(双缓冲机制)
        FB_BUFFER_GET;
        FB_BUFFER = FB_BUFFER == FB_BUFFER_A ? FB_BUFFER_B : FB_BUFFER_A;
        
        // 视频监控模式处理
        if (is_video_mode_enable == true) // 监控状态，没有视频的话，会有UI残留
        {
            // 从视频源获取原始图像数据
            extern bool video_raw_get(unsigned char *, int, int, int);
            if (video_raw_get(system_bg_addres, LV_HOR_RES_MAX, LV_VER_RES_MAX, GP_FORMAT_RGB888) == true)
            {
                // 获取新视频帧成功，更新时间戳
                cur_timestamp = user_timestamp_get();
                pre_timestamp = cur_timestamp;

                // 首次显示视频帧时，设置UI透明度以显示视频背景
                if (lv_fb_video_display_preview == false)
                {
                #ifndef _PLATFORM_800_1280
                    // 在刷出第一帧后，才在视频显示函数设置显示器和屏幕透明
                    lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
                    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
                #endif
                    lv_fb_video_display_preview = true;
                }
            }
            else
            {
                // 获取视频帧失败，检查是否超时
                cur_timestamp = user_timestamp_get();
                if (cur_timestamp - pre_timestamp < 500)
                {
                    // 短时间内没有新帧，继续等待
                    continue;
                }
                else
                {
                    // 超时无视频帧，恢复UI显示
                    lv_fb_video_display_preview = false;
                    pre_timestamp = cur_timestamp;
                }
            }
        }
        else
        {
            // 非视频监控模式处理
        #ifndef _PLATFORM_800_1280//防止退出监控抖动
            if(lv_fb_video_display_preview)
            {
                // system_bg_fill_color(0x00, 0, 0, 1024, 600); //填充颜色
                ak_sleep_ms(500);
            }
        #endif
            // 关闭视频预览模式，恢复正常UI显示
            lv_fb_video_display_preview = false;
        }
        
        // 根据不同模式选择不同的显示刷新方式
        if ((is_video_mode_enable == true) && (lv_fb_video_display_preview == true || is_playback_video_mode))
        {
            // 视频监控模式且有有效视频帧，执行视频叠加显示
            video_main_display_lock();	// 获取视频显示锁，防止冲突

            gui_flush_to_bg();		// 将GUI内容刷新到背景缓冲区
            bg_flush_to_fb();		// 将背景(含视频+GUI)刷新到帧缓冲区
            video_main_display_unlock();	// 释放视频显示锁
        }
        else
        {
            // 普通UI模式，直接将GUI内容刷新到帧缓冲区
            gui_flush_to_fb();
        }

        // 设置当前使用的帧缓冲区
        FB_BUFFER_SET;

        // 短暂延时，控制刷新频率
        usleep(10 * 1000);
    }

    return NULL;
}
/***
** 日期: 2022-05-13 10:14
** 作者: leo.liu
** 函数作用：强制刷新
** 返回参数说明：
***/
void screen_force_refresh(void)
{
	sem_post(&lv_fb_sem_t);
}



#endif
