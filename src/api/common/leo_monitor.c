#include "leo_api.h"
#include "stdbool.h"
#include "video_decode.h"
#include "file_api.h"
#include "network_common.h"
#include "../../layout/user_data.h"
#include "../audio/audio_input.h"
#include "../audio/audio_decode.h"
#include "stdio.h"


static MONITOR_CH monitor_channel = MON_CH_NONE;
void monitor_channel_set(MONITOR_CH ch)
{
	printf("set ch is %d\n",ch);
	monitor_channel = ch;
}



static void monitor_reset(void)
{
	extern bool rtsp_stream_close();
	rtsp_stream_close();
	video_decode_close();//关闭视频解码

	network_video_receive_package_close();//收包关闭

	network_video_send_package_close();//发包关闭

	//hlf:system_bg_fill_color -> system_bg_fill_color_2
	#ifndef _PLATFORM_800_1280
	extern bool system_bg_fill_color_2(unsigned int color,int x,int y ,int w,int h);
    system_bg_fill_color_2(0x00,0,0,1024,600);
	#else
	extern bool system_bg_fill_color(unsigned int color,int x,int y ,int w,int h);
    system_bg_fill_color(0x00,0,0,1024,600);
	#endif
}

void monitor_open(void)
{
	if(monitor_channel == MON_CH_NONE)
	{
		return ;
	}	

	monitor_reset(); // 重置所有监控相关状态（关闭所有通道，清理资源）
	if((monitor_channel >= MON_CH_UNIT_DOOR_1) && (monitor_channel <= MON_CH_DOOR_15))
	{
		//printf("======%s==========%d=======%d====\n",__func__,__LINE__,user_timestamp_get());
		 
		// 存储网络设备标识符的缓冲区
		char eth_p_id[128];
		memset(eth_p_id,0,sizeof(eth_p_id));
			//extern bool rtsp_stream_open(char* url);
		
		// 通过通道号获取对应的室外网络设备信息
		network_device device = get_outdoor_device_by_channel(monitor_channel);	

		//获取到套接字类别 和对方设备ip
		network_common_socket_eth_p_get(0,network_device_get_ip(device,user_data_get()->other.family_id),eth_p_id);
		
		printf("=========family_id  is %hd\n",user_data_get()->other.family_id);
		printf("=========eth_p_id id is %s\n",eth_p_id);
		printf("=========eth_p_id id is %s\n",eth_p_id);
		printf("=========eth_p_id id is %s\n",eth_p_id);

		 // 打开网络视频接收包处理（开始接收视频流数据）
		network_video_receive_package_open(eth_p_id);//while(1); //接收包开关打开
		
		// 打开视频解码功能，指定解码类型为0（可能是H.264），分辨率为1280x720
        // 注意：此处硬编码了分辨率，可能需要根据实际设备动态调整
		video_decode_open(0,1280, 720);//640, 360);//解码打开 解码类型 屏幕宽度 屏幕高度 ** 分辨率
	}
	// 处理网络摄像机通道1
	else if(monitor_channel == MON_CH_CCTV_1)
    {
		// video_decode_open(0,640, 360);
		// 根据主码流/子码流选择不同的解码分辨率
		if(user_data_get()->onvif_dev[0].stream == 0){
			 // 主码流使用1920x1088高分辨率
			video_decode_open(0,1920, 1088);
		}else{
			// 子码流使用配置的低分辨率参数
			video_decode_open(0,user_data_get()->onvif_dev[0].substream_hight, user_data_get()->onvif_dev[0].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[0].url);
		  // 打开RTSP流（开始接收网络摄像机视频）
		rtsp_stream_open(user_data_get()->onvif_dev[0].url);
	}
	else if(monitor_channel == MON_CH_CCTV_2)
	{
		if(user_data_get()->onvif_dev[1].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[1].substream_hight, user_data_get()->onvif_dev[1].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[1].url);
		rtsp_stream_open(user_data_get()->onvif_dev[1].url);
	}
	else if(monitor_channel == MON_CH_CCTV_3)
	{
		if(user_data_get()->onvif_dev[2].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[2].substream_hight, user_data_get()->onvif_dev[2].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[2].url);
		rtsp_stream_open(user_data_get()->onvif_dev[2].url);
	}	
	else if(monitor_channel == MON_CH_CCTV_4)
	{
		if(user_data_get()->onvif_dev[3].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[3].substream_hight, user_data_get()->onvif_dev[3].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[3].url);
		rtsp_stream_open(user_data_get()->onvif_dev[3].url);
	}
	else if(monitor_channel == MON_CH_CCTV_5)
	{
		if(user_data_get()->onvif_dev[4].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[4].substream_hight, user_data_get()->onvif_dev[4].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[4].url);
		rtsp_stream_open(user_data_get()->onvif_dev[4].url);
	}
	else if(monitor_channel == MON_CH_CCTV_6)
	{
		if(user_data_get()->onvif_dev[5].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[5].substream_hight, user_data_get()->onvif_dev[5].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[5].url);
		rtsp_stream_open(user_data_get()->onvif_dev[5].url);
	}
	else if(monitor_channel == MON_CH_CCTV_7)
	{
		if(user_data_get()->onvif_dev[6].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[6].substream_hight, user_data_get()->onvif_dev[6].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[6].url);
		rtsp_stream_open(user_data_get()->onvif_dev[6].url);
	}
	else if(monitor_channel == MON_CH_CCTV_8)
	{
		if(user_data_get()->onvif_dev[7].stream == 0){
			video_decode_open(0,1920, 1088);
		}else{
			video_decode_open(0,user_data_get()->onvif_dev[7].substream_hight, user_data_get()->onvif_dev[7].substream_low);
		}
		printf("\n+++++++++++++++++++%s++++++++++++++++++++\n",user_data_get()->onvif_dev[7].url);
		rtsp_stream_open(user_data_get()->onvif_dev[7].url);
	}
}



void monitor_close(void)
{
	monitor_reset();
	monitor_channel = MON_CH_NONE;
}


MONITOR_CH monitor_channel_get(void)
{
	return monitor_channel;
}


static bool audio_talk_status = false;
bool audio_talk_open(network_device device,bool is_mobilephone)
{
	printf("enter audio_talk_open\n");
	if(audio_talk_status == true)
	{
		printf("device audio talking \n");
		return false;
	}

	if(device == user_data_get()->other.network_device)
	{	
		return false;
	}
	char  eth_p_id [128];
	memset(eth_p_id,0,sizeof(eth_p_id));
	unsigned int slave_id = 0;
	bool falg = false;

	slave_id = network_device_get_ip(device,user_data_get()->other.family_id);

	printf("=========slave_id id is %d\n",slave_id);
	printf("=========slave_id id is %d\n",slave_id);
	printf("=========slave_id id is %d\n",slave_id);
	printf("=========slave_id id is %d\n",slave_id);
	printf("=========slave_id id is %d\n",slave_id);
	network_common_socket_eth_p_get(1,slave_id,eth_p_id);
	printf("=========eth_p_id id is %s\n",eth_p_id);

	network_audio_receive_package_open(eth_p_id);//收
	network_audio_send_package_open(eth_p_id);//发

	if(is_mobilephone == false)
	{
		
		audio_input_open();
		audio_decode_open(falg);

		audio_input_start();
		
		audio_decode_start(falg);
	}
	
	audio_talk_status = true;
	return true;
}

bool audio_play_open(bool is_mobilephone){


	audio_input_open();
	audio_decode_open(true);
	
	audio_input_start();	
	audio_decode_start(true);
	return true;
}

bool audio_talk_close(void)
{
	if(audio_talk_status == false)
	{
		printf("device audio not talk \n");
		return false;
	}
	network_audio_send_package_close();
	network_audio_receive_package_close();

	audio_input_close();
	audio_decode_close();

	audio_talk_status = false;
	return true;
}




bool is_audio_talk_open(void)
{
	return audio_talk_status;
}





static bool cctv_online_check_flag = false;

void cctv_online_check_ctrl(bool en)
{
	cctv_online_check_flag = en;
}

// static void *cctv_online_check_thread(void *arg)
// {
	
	// LOG_WHITE("************ cctv_online_check_thread [ok] ***********\n");
	// while(1)
	// {
	// 	usleep(1000*1000*10);
	// 	// network_device
	// 	if((user_data_get()->other.network_device == DEVICE_INDOOR_ID1) && (wlan_is_wifi_connect_success() == 1) && cctv_online_check_flag == true )
	// 	{
	// 		if((user_data_get()->onvif_dev[0].ip[0] != 0) || (user_data_get()->onvif_dev[1].ip[0] != 0))
	// 		{
	// 			sat_ipcamera_device_online_search();		/* 搜索 */
	// 			while (sat_ipcamera_status_get() == true)	/* 等待搜索完成 */
	// 				;

	// 			for(int j = 0; j < 2; j++)
	// 			{
	// 				bool dev_state = net_channel_state_get(DEVICE_CCTV_1+j)->online;
	// 				bool cur_state = false;
	// 				for(int i = 0; i < sat_ipcamera_online_num_get(); i++)
	// 				{

	// 					cur_state = (strcmp(user_data_get()->onvif_dev[j].ip, sat_ipcamera_ipaddr_get(i)) == 0) ? true : false;

	// 					if(net_channel_state_get(DEVICE_CCTV_1+j)->online != cur_state)
	// 					{
	// 						net_channel_state_get(DEVICE_CCTV_1+j)->online = cur_state;
	// 					}

	// 					if(cur_state == true)
	// 					{
	// 						break;
	// 					}

	// 				}

	// 				/* 更新状态 */
	// 				if(dev_state != cur_state)
	// 				{
	// 					LOG_CYAN("dev[%d] state:[%d]\n", (DEVICE_CCTV_1+j), cur_state ? 0x01 : 0x00);
	// 					device_online_state_push(DEVICE_CCTV_1 + j, cur_state ? 0x01 : 0x00);	
	// 					net_channel_state_get(DEVICE_CCTV_1+j)->online = cur_state;

	// 				}
	// 			}

	// 		}

			
	// 	}

		
				
	// }

	// ak_thread_exit();
	// return NULL;
// }

// void cctv_online_check_thread_create()
// {

// 	ak_pthread_t thread_id;
// 	ak_thread_create(&thread_id, cctv_online_check_thread, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
// 	ak_thread_detach(thread_id);

// }

