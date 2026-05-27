#ifndef _VIDEO_DECODE_H_
#define _VIDEO_DECODE_H_
#include "stdbool.h"
#include "pthread.h"
extern pthread_mutex_t video_main_display_mutex;
#define video_main_display_lock() pthread_mutex_lock(&video_main_display_mutex)
#define video_main_display_unlock() pthread_mutex_unlock(&video_main_display_mutex)
#define tde_layer_pos_init(layer, px, py, pw, ph) \
	layer.pos_left = px;                          \
	layer.pos_top = py;                           \
	layer.pos_width = pw;                         \
	layer.pos_height = ph;

#define tde_layer_layer_init(layer, fmt, w, h, px, py, pw, ph) \
	layer.format_param = fmt;                                  \
	layer.width = w;                                           \
	layer.height = h;                                          \
	tde_layer_pos_init(layer, px, py, pw, ph);
void video_decode_init(void);

bool video_decode_open(char,int src_width,int src_height);

bool video_decode_close(void);

bool video_decode_push(char,unsigned char* data ,int len);



void video_raw_init(int width,int height);

bool video_raw_push(unsigned char* addres,unsigned long phy,int width,int height,int pixel_width,int pixel_height);

//bool video_raw_push(unsigned char* addres,unsigned long phy,int width,int height);

bool video_raw_get(unsigned char* dst,int width,int height,int format);

void video_raw_release_all(void);



bool jpg_record(const char*file_path);

bool is_jpg_record_ing(void);

bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h);




void video_record_init(void);

bool video_record_start(const char* path,bool has_audio,int width,int height,char );

bool video_record_stop(void);

bool is_video_recording(void);

bool video_record_data_push(char,unsigned char* data,int len,bool is_video );

void video_play_init(void);

bool sent_tuya_record(const char*file_path);

void h264_decode_res_check(void);
bool h264_decode_load_state_get(void);
bool h264_decode_load_wait(void);

bool video_decode_wait_thread_quit(void);
#endif

