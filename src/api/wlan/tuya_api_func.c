#include "tuya_sdk.h"
#include "wifi_hwl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include "tuya_iot_config.h"
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "base_hwl.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "base_hwl.h"
#include "tuya_g711_utils.h"

#include "tuya_ipc_p2p.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_api.h"
#include "network_common.h"
#include "../../layout/layout_common.h"
// #include "os_sys_api.h"

// #include "leo_monitor.h"
#include <fcntl.h>

OPERATE_RET hwl_bnw_get_mac(OUT NW_MAC_S *mac)
{
	return 1;
}

OPERATE_RET hwl_bnw_set_mac(IN CONST NW_MAC_S *mac)
{
	return 1;
}

OPERATE_RET tuya_iot_reg_wf_lock_chan_cb(FUNC_WIFI_LOCK_CHANNEL_CB func_wifi_lock_ch_cb)
{
	return 1;
}

OPERATE_RET wf_nw_set_lock_chan_notify_cb(FUNC_WIFI_LOCK_CHANNEL_CB func_wifi_lock_ch_cb)
{
	return 1;
}

// #include "audio_package.h"
#include "wlan.h"

#define WLAN_DEV "wlan0"

OPERATE_RET hwl_bnw_get_ip(OUT NW_IP_S *ip)
{
	int sock;
	struct sockaddr_in *sin;
	struct ifreq ifr;
	linked_info wlan = {0};
	get_linked_wifi_info(&wlan);
	if (!wlan.completed)
	{
		return OPRT_COM_ERROR;
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket create failse...GetLocalIp!\n");
		return OPRT_COM_ERROR;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, WLAN_DEV, sizeof(ifr.ifr_name) - 1);

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		printf("%s:%d ioctl error\n", __func__, __LINE__);
		close(sock);
		return OPRT_COM_ERROR;
	}

	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	strcpy(ip->ip, inet_ntoa(sin->sin_addr));
	close(sock);
	return OPRT_OK;
}

BOOL_T hwl_bnw_station_conn(VOID)
{
	int sock;
	struct ifreq ifr;
	linked_info wlan = {0};
	get_linked_wifi_info(&wlan);
	if (!wlan.completed)
	{
		return OPRT_COM_ERROR;
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket create failse...GetLocalIp!\n");
		return OPRT_COM_ERROR;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, WLAN_DEV, sizeof(ifr.ifr_name) - 1);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
	{
		printf("%s:%d ioctl error\n", __func__, __LINE__);
		close(sock);
		return FALSE;
	}
	close(sock);

	if (0 == (ifr.ifr_flags & IFF_UP))
	{
		return FALSE;
	}

	return TRUE;
}
OPERATE_RET hwl_bnw_set_station_connect(IN CONST CHAR_T *ssid, IN CONST CHAR_T *passwd)
{
	return OPRT_COM_ERROR;
}
BOOL_T hwl_bnw_need_wifi_cfg(VOID)
{
	return FALSE;
}
OPERATE_RET hwl_bnw_station_get_conn_ap_rssi(OUT SCHAR_T *rssi)
{
	*rssi = 99;

	return OPRT_OK;
}

static void p2p_media_init(IPC_MEDIA_INFO_S *info)
{
	memset(info, 0, sizeof(IPC_MEDIA_INFO_S));
	info->channel_enable[E_CHANNEL_VIDEO_MAIN] = TRUE; /* Whether to enable local HD video streaming */
	info->video_fps[E_CHANNEL_VIDEO_MAIN] = 30;		   /* FPS */
													   // info->video_gop[E_CHANNEL_VIDEO_MAIN] = 30;  /* GOP */
	info->video_gop[E_CHANNEL_VIDEO_MAIN] = 30;		   /* GOP */
													   // info->video_bitrate[E_CHANNEL_VIDEO_MAIN] = TUYA_VIDEO_BITRATE_2M; /* Rate limit */

	info->video_bitrate[E_CHANNEL_VIDEO_MAIN] = TUYA_VIDEO_BITRATE_1_5M;

	info->video_width[E_CHANNEL_VIDEO_MAIN] = 640;					 /* Single frame resolution of width*/
	info->video_height[E_CHANNEL_VIDEO_MAIN] = 360;					 /* Single frame resolution of height */
	info->video_freq[E_CHANNEL_VIDEO_MAIN] = 90000;					 /* Clock frequency */
	info->video_codec[E_CHANNEL_VIDEO_MAIN] = TUYA_CODEC_VIDEO_H264; /* Encoding format */
	tuya_ipc_ring_buffer_init(E_CHANNEL_VIDEO_MAIN, info->video_bitrate[E_CHANNEL_VIDEO_MAIN], info->video_fps[E_CHANNEL_VIDEO_MAIN], 0, NULL);

	info->channel_enable[E_CHANNEL_VIDEO_SUB] = TRUE; /* Whether to enable local HD video streaming */
	info->video_fps[E_CHANNEL_VIDEO_SUB] = 25;		  /* FPS */
													  // info->video_gop[E_CHANNEL_VIDEO_SUB] = 30;  /* GOP */
	info->video_gop[E_CHANNEL_VIDEO_SUB] = 30;		  /* GOP */
													  // info->video_bitrate[E_CHANNEL_VIDEO_SUB] = TUYA_VIDEO_BITRATE_2M; /* Rate limit */

	info->video_bitrate[E_CHANNEL_VIDEO_SUB] = TUYA_VIDEO_BITRATE_1_5M;

	info->video_width[E_CHANNEL_VIDEO_SUB] = 640;					/* Single frame resolution of width*/
	info->video_height[E_CHANNEL_VIDEO_SUB] = 360;					/* Single frame resolution of height */
	info->video_freq[E_CHANNEL_VIDEO_SUB] = 90000;					/* Clock frequency */
	info->video_codec[E_CHANNEL_VIDEO_SUB] = TUYA_CODEC_VIDEO_H264; /* Encoding format */
	tuya_ipc_ring_buffer_init(E_CHANNEL_VIDEO_SUB, info->video_bitrate[E_CHANNEL_VIDEO_SUB], info->video_fps[E_CHANNEL_VIDEO_SUB], 0, NULL);

	/* Audio stream configuration.
	Note: The internal P2P preview, cloud storage, and local storage of the SDK are all use E_CHANNEL_AUDIO data. */
	info->channel_enable[E_CHANNEL_AUDIO] = TRUE;					/* Whether to enable local sound collection */
	info->audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_PCM;		// TUYA_CODEC_AUDIO_PCM; //TUYA_CODEC_AUDIO_PCM /* Encoding format */
	#if defined(TUYA_AUDIO_SAMPLE) && (TUYA_AUDIO_SAMPLE == 16000)
	info->audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_16K;		// TUYA_AUDIO_SAMPLE_16K;//TUYA_AUDIO_SAMPLE_8K /* Sampling Rate */
	#else
	info->audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_8K;		// TUYA_AUDIO_SAMPLE_16K;//TUYA_AUDIO_SAMPLE_8K /* Sampling Rate */
	#endif
	info->audio_databits[E_CHANNEL_AUDIO] = TUYA_AUDIO_DATABITS_16; /* Bit width */
	info->audio_channel[E_CHANNEL_AUDIO] = TUYA_AUDIO_CHANNEL_MONO; // TUYA_AUDIO_CHANNEL_MONO;/* channel */
	info->audio_fps[E_CHANNEL_AUDIO] = 60;							/* Fragments per second */
	tuya_ipc_ring_buffer_init(E_CHANNEL_AUDIO, TUYA_VIDEO_BITRATE_2M /*  info->audio_sample[E_CHANNEL_AUDIO]*info->audio_databits[E_CHANNEL_AUDIO]/1024 */, info->audio_fps[E_CHANNEL_AUDIO], 0, NULL);

	// tuya_ipc_ring_buffer_init(E_CHANNEL_AUDIO, info->audio_sample[E_CHANNEL_AUDIO]*info->audio_databits[E_CHANNEL_AUDIO]/1024, info->audio_fps[E_CHANNEL_AUDIO], 0, NULL);

	// tuya_ipc_ring_buffer_init(E_CHANNEL_AUDIO,TUYA_VIDEO_BITRATE_1_5M/*  info->audio_sample[E_CHANNEL_AUDIO]*info->audio_databits[E_CHANNEL_AUDIO]/1024 */, info->audio_fps[E_CHANNEL_AUDIO],0,NULL);
}
static void p2p_status_change_func(TRANSFER_ONLINE_E stat)
{
}

// 采样率转换函数: 8K -> 16K (通过线性插值实现)
// 输入: 8K采样的PCM数据(16位有符号整数)，输入长度(字节数)
// 输出: 16K采样的PCM数据，返回值为输出数据长度(字节数)
int audio_resample_8k_to_16k(const int16_t *input, int input_len, int16_t *output)
{
	int input_samples = input_len / 2;		// 每个采样占2字节
	int output_samples = input_samples * 2; // 16K是8K的2倍

	for (int i = 0; i < input_samples; i++)
	{
		// 保留原始采样点
		output[2 * i] = input[i];

		// 计算插值点(线性插值)
		if (i < input_samples - 1)
		{
			output[2 * i + 1] = (input[i] + input[i + 1]) / 2;
		}
		else
		{
			// 最后一个点使用前一个值
			output[2 * i + 1] = input[i];
		}
	}

	return output_samples * 2; // 返回字节数
}

static void p2p_audio_rev_form_app_func(const TRANSFER_AUDIO_FRAME_S *p_audio_frame, const UINT_T frame_no) // 接受涂鸦APP音频
{

	// Local play
	unsigned char pcm8k[640] = {0};
	unsigned char pcm16k[1280] = {0};
	//	unsigned int pcm_len = 0;

	//	tuya_g711_decode(TUYA_G711_MU_LAW, (unsigned short*)p_audio_frame->p_audio_buf,p_audio_frame->buf_len, pcm_data, &pcm_len);
	// audio_decode_api.write((char*)pcm_data, pcm_len);

	//	if(audio_package_api.pull_to_local){
	//		audio_package_api.send_write(p_audio_frame->p_audio_buf,p_audio_frame->buf_len);
	//	}
	// network_audio_send_package_push(0,(const char*)pcm_data,pcm_len);

	/*
		static unsigned char pcm_data[2*1024] = {0};
		static int pcm_size = 0;

		if((pcm_size + p_audio_frame->buf_len )> 2*1024)
		{
			audio_output_write(pcm_data,pcm_size);
			pcm_size = 0;
		}

		memcpy(&pcm_data[pcm_size],p_audio_frame->p_audio_buf,p_audio_frame->buf_len);
		pcm_size += p_audio_frame->buf_len;
		*/
	// 如果是8K采样率，先转换为16K
	if (p_audio_frame->audio_sample == 8000)
	{
// 分配转换后的缓冲区
#if 0
        int16_t *converted_buf = malloc(p_audio_frame->buf_len * 2);
        if (converted_buf) {
            // 进行采样率转换
            int converted_len = audio_resample_8k_to_16k(
                (const int16_t*)p_audio_frame->p_audio_buf,
                p_audio_frame->buf_len,
                converted_buf
            );
            
            // 发送转换后的16K音频数据
            network_audio_send_package_push(0, (const char*)converted_buf, converted_len);
            free(converted_buf);  // 释放缓冲区
        }
#else
		unsigned int relSize = 0;
		tuya_g711_decode(TUYA_G711_MU_LAW, (unsigned short *)p_audio_frame->p_audio_buf, p_audio_frame->buf_len, (unsigned char *)pcm8k, &relSize);
		relSize = audio_resample_8k_to_16k(
			(const int16_t *)pcm8k,
			relSize,
			(int16_t *)pcm16k);

		static int fd8k = -1;
		static int fd16k = -1;
		if (fd8k == -1)
		{
			fd8k = open("/tmp/8k.pcm", O_CREAT | O_WRONLY, 0777);
		}
		if (fd16k == -1)
		{
			fd16k = open("/tmp/16k.pcm", O_CREAT | O_WRONLY, 0777);
		}

		write(fd8k, p_audio_frame->p_audio_buf, p_audio_frame->buf_len);
		write(fd16k, p_audio_frame->p_audio_buf, p_audio_frame->buf_len);

		// tuya8KResample16K((short*)p_audio_frame->p_audio_buf,p_audio_frame->buf_len/2,(short*)pcm16kBuf,&resamplesize);
		// printf("pcm 8kbuf:%d -> reszie:%d\n",p_audio_frame->buf_len,resamplesize);
		network_audio_send_package_push(0, (const char *)pcm16k, relSize);
#endif
	}
	else
	{
		// 非8K采样率直接发送
		network_audio_send_package_push(0, (const char *)p_audio_frame->p_audio_buf, p_audio_frame->buf_len);
	}

	// static int fd = -1;
	// static int i = 0;

	// if (i == 0)
	// {
	// 	fd = open("/mnt/nfs/yxaudio.pcm", O_CREAT | O_WRONLY, 0644);
	// 	if (fd < 0)
	// 	{
	// 		perror("open audio file failed");
	// 		return;
	// 	}
	// }
	// 写入音频数据到文件
	// if (fd >= 0 && p_audio_frame && p_audio_frame->p_audio_buf && p_audio_frame->buf_len > 0)
	// {
	// 	ssize_t written = write(fd, p_audio_frame->p_audio_buf, p_audio_frame->buf_len);
	// 	if (written != p_audio_frame->buf_len)
	// 	{
	// 		perror("write audio file failed");
	// 	}
	// }
	// if (i++ == 500 && fd >= 0)
	// {
	// 	close(fd);
	// 	fd = -1;
	// }
}

static OPERATE_RET ipc_app_sync_utc_time(VOID)
{
	TIME_T time_utc;
	INT_T time_zone;
	printf("Get Server Time \n");
	OPERATE_RET ret = tuya_ipc_get_service_time_force(&time_utc, &time_zone);

	if (ret != OPRT_OK)
	{
		return ret;
	}
	return OPRT_OK;
}
void tuya_ipc_ring_buffer_video_release_data(void)
{

	// tuya_ipc_ring_buffer_clean_user_state(E_CHANNEL_VIDEO_MAIN,E_USER_P2P_USER);
	//  tuya_ipc_ring_buffer_clean_user_state(E_CHANNEL_VIDEO_MAIN,E_USER_P2P_USER);

	// tuya_ipc_ring_buffer_clean_user_state(E_CHANNEL_AUDIO,E_USER_P2P_USER);
	if (is_online_tuya_cloud() == false)
	{
		return;
	}

	printf("tuya ringuffer clean ..............\n");
	tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_VIDEO_MAIN, E_USER_P2P_USER);
	tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_VIDEO_SUB, E_USER_P2P_USER);
	tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_AUDIO, E_USER_P2P_USER);
}

static INT_T p2p_event_func(const TRANSFER_EVENT_E event, const PVOID_T args)
{
	printf("Receivr tuya event 0x%x\n", event);
	switch (event)
	{
	case TRANS_LIVE_VIDEO_START: // 涂鸦获取视频
	{
		int on_line = tuya_ipc_get_client_online_num();
		if (on_line > 1)
		{
			break;
		}
		ipc_app_sync_utc_time();
		tuya_ipc_ring_buffer_video_release_data();
		const unsigned char nal[] = {0, 0, 0, 1, 0X67, 0, 0, 0, 1, 0X68, 0x00, 0x00, 0x00, 0x01, 0x65};
		unsigned char data[1024] = {0};
		extern unsigned long long user_timestamp_get();
		memcpy(data,nal, sizeof(nal));
		tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, 1024, E_VIDEO_PB_FRAME, user_timestamp_get());
		tuya_ipc_ring_buffer_append_data(E_CHANNEL_AUDIO, data, 1024, E_CHANNEL_AUDIO, user_timestamp_get());

		extern bool tuya_monitor_enter_event(void);
		tuya_monitor_enter_event();
		printf("=============>>> tuya video start \n");
	}
	break;

	case TRANS_LIVE_VIDEO_STOP: // tuya退出视频
	{
		int on_line = tuya_ipc_get_client_online_num();
		if (on_line > 1)
		{
			break;
		}
		// tuya_ipc_ring_buffer_video_release_data();
		const unsigned char nal[] = {0, 0, 0, 1, 0X67, 0, 0, 0, 1, 0X68, 0x00, 0x00, 0x00, 0x01, 0x65};
		unsigned char data[1024] = {0};
		extern unsigned long long user_timestamp_get();
		memcpy(data,nal, sizeof(nal));
		tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, 1024, E_VIDEO_PB_FRAME, user_timestamp_get());
		tuya_ipc_ring_buffer_append_data(E_CHANNEL_AUDIO, data, 1024, E_CHANNEL_AUDIO, user_timestamp_get());
		//	tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_SUB, data, 1024, E_VIDEO_PB_FRAME, user_timestamp_get());
		extern bool tuya_monitor_quit_event(void);
		tuya_monitor_quit_event();
		printf("=============>>> tuya video stop \n");
	}
	break;

	case TRANS_LIVE_AUDIO_START: //
	{
#if 0
			int on_line = tuya_ipc_get_client_online_num();
			if(on_line > 1)
			{
				break;
			}
#endif
		//			audio_push_to_tuya_open();
		printf("=============>>> tuya audio start \n");
	}
	break;

	case TRANS_LIVE_AUDIO_STOP:
	{
#if 0
        	int on_line = tuya_ipc_get_client_online_num();
			if(on_line > 1)
			{
				break;
			}
#endif
		/***插入关键帧***/
		// tuya_ipc_ring_buffer_video_release_data();
		// const unsigned char nal[] = {0, 0, 0, 1, 0X67, 0, 0, 0, 1, 0X68, 0x00, 0x00, 0x00, 0x01, 0x65};
		// unsigned char data[1024] = {0};
		// extern unsigned long long user_timestamp_get();
		// memcpy(data, nal, sizeof(nal));
		// tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, 1024, E_VIDEO_PB_FRAME, user_timestamp_get());
		// tuya_ipc_ring_buffer_append_data(E_CHANNEL_AUDIO, data, 1024, E_CHANNEL_AUDIO, user_timestamp_get());
		//			audio_push_to_tuya_close();
		printf("=============>>> tuya audio stop \n");
	}
	break;

	case TRANS_SPEAKER_START: // 传输到涂鸦的
	{
		int on_line = tuya_ipc_get_client_online_num();
		if (on_line > 1)
		{
			break;
		}
		// amp_enable(true);
		// audio_pull_to_local_open();
		extern bool tuya_monitor_talk_event(bool state);
		tuya_monitor_talk_event(true);
		printf("=============>>> tuya speaker start \n");
	}
	break;

	case TRANS_SPEAKER_STOP:
	{
		int on_line = tuya_ipc_get_client_online_num();
		if (on_line > 1)
		{
			break;
		}
		// amp_enable(false);
		//			audio_pull_to_local_close();
		extern bool tuya_monitor_talk_event(bool state);
		tuya_monitor_talk_event(false);
		printf("=============>>> tuya speaker stop \n");
	}
	break;

	default:

		break;
	}
	return TRANS_EVENT_SUCCESS;
}

OPERATE_RET TUYA_APP_Enable_P2PTransfer(IN UINT_T max_users)
{
	TUYA_IPC_TRANSFER_VAR_S p2p_var = {0};

	p2p_var.online_cb = p2p_status_change_func;
	p2p_var.on_rev_audio_cb = p2p_audio_rev_form_app_func;

	p2p_var.rev_audio_codec = TUYA_CODEC_AUDIO_G711U;
	p2p_var.audio_sample = TUYA_AUDIO_SAMPLE_8K;
	p2p_var.audio_databits = TUYA_AUDIO_DATABITS_16;
	p2p_var.audio_channel = TUYA_AUDIO_CHANNEL_MONO;
	/*end*/
	p2p_var.on_event_cb = p2p_event_func;
	p2p_var.live_quality = TRANS_LIVE_QUALITY_MAX; // TRANS_LIVE_QUALITY_MAX;
	p2p_var.max_client_num = max_users;

	p2p_media_init(&p2p_var.AVInfo);
	tuya_ipc_tranfser_init(&p2p_var);
	return OPRT_OK;
}
