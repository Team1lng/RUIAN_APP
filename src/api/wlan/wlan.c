//
// Created by michael on 2021/11/8.
//

#include "../../include/anyka/ak_thread.h"
#include "../../include/anyka/ak_common.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "wlan.h"
#include "../../lvgl/lvgl.h"

#include "../../include/tuya/tuya_sdk.h"

#include "../../include/tuya/tuya_ipc_api.h"
#include <signal.h>





static int scanned_wifi_index = 0;
static linked_info wifi_wlan0 = {0};
static wifi_info scanned_wifi[MAX_WIFI_SCAN] = {{"\0","\0",0,false,false}};

static void parsing_data(char *data, int index) {
    char *p_level = strchr(data, '-');
    char *p_flag = strchr(p_level, '[');
    char *p_ssid = strrchr(p_level, '\t') + 1;

    scanned_wifi[index].hidden = false;
    scanned_wifi[index].free = false;

    char tmp_level[8] = {0};
    char *stop_str;
    strncpy(tmp_level, p_level, p_flag - p_level);
    scanned_wifi[index].signal_level = strtol(tmp_level,&stop_str, 10);
    //printf("level: %d \n\r",scanned_wifi[index].signal_level);

    memset(scanned_wifi[index].flags, 0x00, sizeof(scanned_wifi[index].flags));
    strncpy(scanned_wifi[index].flags, p_flag, p_ssid - p_flag - 1);


    if (strcmp(scanned_wifi[index].flags, "[ESS]") == 0) {
        scanned_wifi[index].free = true;
		
    }
    memset(scanned_wifi[scanned_wifi_index].ssid, 0x00, sizeof(scanned_wifi[scanned_wifi_index].ssid));

    char unresolved[3] = {"\\x"};
    char* ret = strpbrk(p_ssid, unresolved);
    if (ret) {
        #if 0
       	//printf("Unresolved: %s \n\r",p_ssid);
        const char *unresolved[8] ={
                "Unresolved","미해결","Irresoluta","Нерешенный","لم تحل","Chưa giải quyết"
        };
        memset(scanned_wifi[scanned_wifi_index].ssid,0,sizeof (scanned_wifi[scanned_wifi_index].ssid));
        sprintf(scanned_wifi[scanned_wifi_index].ssid, "%s",unresolved[0]);//10
        #endif
        //解决ssid显示问题
        strncpy(scanned_wifi[scanned_wifi_index].ssid,p_ssid, strlen(p_ssid) - 1);
    } else {
        if ((strlen(p_ssid) == 1) && (strncmp(p_ssid, "\n", sizeof("\n")) == 0)) {
            const char *hidden[8] ={
                    "Hidden Wi-Fi","숨겨져 있고 충실한","Escondido y fiel","Скрытый и верный","هيدن وفي","Giấu kín và trung thành"
            };
            memset(scanned_wifi[scanned_wifi_index].ssid,0,sizeof (scanned_wifi[scanned_wifi_index].ssid));
            sprintf(scanned_wifi[scanned_wifi_index].ssid,  "%s",hidden[0]);//12
            scanned_wifi[index].hidden = true;
        } else {
            strncpy(scanned_wifi[scanned_wifi_index].ssid, p_ssid, strlen(p_ssid) - 1);
        }
    }
    
}
static int if_wlan_up(void){
    int ret = 0;
    FILE *pf = popen("cat /sys/class/net/wlan0/operstate","r");
    if(pf == NULL){
        perror("get wlan0 operstate error !\n\r");
        return ret;
    }
    char buffer[256] = {0};
    memset(buffer,0,sizeof (buffer));
    while(fgets(buffer,sizeof (buffer),pf)){
        //printf("buffer: %s \n\r",buffer);
        if(strncmp(buffer,"up",2) == 0){
            ret = 1;
        }else if(strncmp(buffer,"dormant",7) == 0){
            ret = 2;
        }
        else if(strncmp(buffer,"down",4) == 0){
            ret = 3;
        }
    }
    pclose(pf);
    pf = NULL;
    return ret;
}

int wpa_cli_scan_wifi(bool *continue_flag) {
    //system("wpa_cli -i wlan0 scan");
    int ret = if_wlan_up();
    if(ret <= 0){
        printf("-0-\n\r");
        return -1;
    }
    FILE *fp = popen("wpa_cli -i wlan0 scan &", "r");
    if (fp == NULL) {
        perror("wpa_cli -i wlan0 scan open fail !\n\r");
        return -1;
    }
    char buffer[32] = {0};
    while (fgets(buffer, sizeof(buffer), fp) && (*continue_flag == true)) {
        
    }
    pclose(fp);

    //ak_sleep_ms(300);
    FILE *pf = popen("wpa_cli -i wlan0 scan_result &", "r");
    if (pf == NULL) {
        perror("wpa_cli_scan_wifi open fail !\n\r");
        return -1;
    }
    char tmp_buffer[1024 * 2] = {0};
    int scanned_index = 0;
    scanned_wifi_index = 0;
    while (fgets(tmp_buffer, sizeof(tmp_buffer), pf) && (*continue_flag == true)) {
        //printf(" %s \n\r", tmp_buffer);
        if (scanned_index >= 1) {

            parsing_data(tmp_buffer, scanned_wifi_index);
            scanned_wifi_index++;
            if (scanned_wifi_index >= MAX_WIFI_SCAN) {
                break;
            }
        }
        memset(tmp_buffer, 0, sizeof(tmp_buffer));
        scanned_index++;
    }
    pclose(pf);
    return scanned_wifi_index;
}

bool get_scanned_wifi_info(int index, wifi_info *info) {
    if ((index < scanned_wifi_index) && (info != NULL)) {
        *info = scanned_wifi[index];
        return true;
    }
    return false;
}

static bool connectwifi_level(void){
	for(int i = 0;i <= MAX_WIFI_SCAN;i++){
		if(strcmp(wifi_wlan0.wlan_ssid,scanned_wifi[i].ssid)  == 0){
			wifi_wlan0.level = scanned_wifi[i].signal_level;
			return true;
		}
	}
	return false;
}

bool wpa_cli_wlan_status(bool *continue_flag) {

    memset(&wifi_wlan0,0,sizeof (wifi_wlan0));

    FILE *pf = popen("wpa_cli -i wlan0 status", "r");
    if (pf == NULL) {
        perror(" wpa_cli_wlan_status open fail \n\r");
        return false;
    }
    char buffer[1024 * 1] = {0};
    memset(buffer,0,sizeof (buffer));
    while (fgets(buffer, sizeof(buffer), pf)&&(*continue_flag  == true)) {
        if (strncmp(buffer, "ssid=", 5) == 0) {
            char *p = strchr(buffer, '=') + 1;
            strncpy(wifi_wlan0.wlan_ssid,p, strlen(p) - 1);
            strcat(wifi_wlan0.wlan_ssid,"\0");
            //printf("1:%s \n\r",wifi_wlan0.wlan_ssid);
        } else if (strncmp(buffer, "key_mgmt=", 9) == 0) {
            char *p = strchr(buffer, '=') + 1;
            strncpy(wifi_wlan0.key_mgmt,p, strlen(p) - 1);
            strcat(wifi_wlan0.wlan_ssid,"\0");
            //printf("2:%s \n\r",wifi_wlan0.key_mgmt);
        } else if (strncmp(buffer, "wpa_state=COMPLETED", 19) == 0) {
            wifi_wlan0.completed = 1;
        } else if (strncmp(buffer, "ip_address=", 11) == 0) {
            char *p = strchr(buffer, '=') + 1;
            strncpy(wifi_wlan0.ip,p, strlen(p) - 1);
            strcat(wifi_wlan0.ip,"\0");
            //printf("3:%s \n\r",wifi_wlan0.ip);
        } else if (strncmp(buffer, "address=", 8) == 0) {
            char *p = strchr(buffer, '=') + 1;
            strncpy(wifi_wlan0.mac,p, strlen(p) - 1);
            strcat(wifi_wlan0.wlan_ssid,"\0");
            //printf("4:%s \n\r",wifi_wlan0.mac);
        }

    }
    pclose(pf);
	connectwifi_level();
    return true;
}


bool wpa_cli_connect_new_wifi(const char *ssid, const char *psk) {
    char tmp_configure[1024];
    if (ssid != NULL) {
        if ((psk != NULL) && (strlen(psk) >= 8)) {
            sprintf(tmp_configure,
                    "ctrl_interface=/var/run/wpa_supplicant"
                    "\nupdate_config=1"
                    "\nnetwork={"
                    "\n\t	scan_ssid=1"
                    "\n\t	key_mgmt=WPA-PSK"
                    "\n\t	ssid=\"%s\""
                    "\n\t	psk=\"%s\""
                    "\n}"
                    "\n", ssid, psk);
        } else {
            sprintf(tmp_configure,
                    "ctrl_interface=/var/run/wpa_supplicant"
                    "\nupdate_config=1"
                    "\nnetwork={"
                    "\n\t	scan_ssid=1"
                    "\n\t	key_mgmt=NONE"
                    "\n\t	ssid=\"%s\""
                    "\n}"
                    "\n", ssid);
        }
        FILE *fp = fopen("/tmp/wpa_supplicant.conf", "wb");
        fwrite(tmp_configure, strlen(tmp_configure), 1, fp);
        printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
        printf("tmp wpa_conf is %s\n",tmp_configure);
        printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
        fclose(fp);
        system("sync");
        if (access("/tmp/wpa_supplicant.conf", F_OK) == 0) {
			
            system("killall wpa_supplicant");
            system("killall udhcpc");
			
			printf("=========================>>>>>>connect begin!\n");
			
            system("wpa_supplicant -Dnl80211 -i wlan0 -c /tmp/wpa_supplicant.conf -B &");
            //ak_sleep_ms(1000);
			
			printf("=========================>>>>>>get ip!\n");
            system("udhcpc -i wlan0 -n 4 -R &");


            return true;
        }
    }
    return false;
}

// -1: psk error 1: connect  -2:other error
int wpa_cli_check_connect_status(void) {
    int ret = 0;
    char tmp_str[1024];
    memset(tmp_str, 0, 1024);
    system("killall wlan_check.sh");
    FILE *fp = popen("/etc/config/wlan_check.sh", "r");
    if (fp != NULL) {
        if (!feof(fp)) {
            fgets(tmp_str, sizeof(tmp_str), fp);
            if (strstr(tmp_str, "incorrect key")) {
                printf("-------- error wifi password \n\r");
                ret = -1;

            }else if (strstr(tmp_str, "not found")) {
                printf("-------- error not found \n\r");
                ret = -2;
            } else if (strstr(tmp_str, "connected")) {
                system("rm -rf /etc/config/wpa_supplicant.conf");
                system("cp -rf /tmp/wpa_supplicant.conf /etc/config/");
                system("sync");
                //system("killall udhcpc");
                ret = 1;
                printf("-------- connected \n\r");
                //tuya_ipc_reconnect_wifi();
            }
        }
    }
    pclose(fp);
    system("killall wlan_check.sh");
    system("rm -rf wpa_ctrl_*");
    return ret;
}



int wifi_connection_status_sucess(void){
	//ak_sleep_ms(1000);
	int result = 0;
	FILE* pf = popen("wpa_cli -i wlan0 status","r");
	if(pf == NULL)
	{
		printf("popen fail \n\r");
		return 0;
	}
	char buffer[1024*10] = {0};
	while(fgets(buffer,sizeof(buffer),pf))
	{
		if(strncmp(buffer,"wpa_state=COMPLETED",19) == 0)
		{  

			result = 1;
			break;
		}
	}
	
	pclose(pf);
	return result;
}




int current_wlan_signal_level(void) {
    int ret = 0;
    char tmp_str[1024];
    memset(tmp_str, 0, 1024);
    FILE *fp = popen("cat /proc/net/wireless |grep wlan0 |awk '{print $4}' ", "r");
    if (fp != NULL) {
        if (!feof(fp))
        {
            while (fgets(tmp_str, sizeof(tmp_str), fp)) {
                char tmp_level[8] = {0};
                char *p_start = strchr(tmp_str, '-');
                char *p_end = strchr(tmp_str, '.');
                strncpy(tmp_level, tmp_str, p_end - p_start);
                printf("Signal Level: %s dB\n\r", tmp_level);
                int level = atoi(tmp_level);
                return level;
            }
        }
    }
    pclose(fp);
    return ret;
}


static ak_pthread_t wlan_thread_id;
static bool wlan_thread_run = false;
static bool wlan_turn_on = false;



void get_linked_wifi_info(linked_info* info){
    *info = wifi_wlan0;
}
/*
static bool wlan_udhcpc_status(bool *continue_flag){

    system("killall udhcpc");
    bool ret = false;
    FILE *pf = popen("udhcpc -i wlan0 -n 4 -R &","r");
	
    if(pf == NULL){
        perror("udhcpc -i wlan0 error !\n\r");
        return ret;
    }
     char buffer[256] = {0};
    memset(buffer,0,sizeof (buffer));
    while(fgets(buffer,sizeof (buffer),pf) && (*continue_flag == true)){

        printf("buffer: %s \n\r",buffer);
        if(strncmp(buffer,"adding dns ",11) == 0){
            char *p = strchr(buffer,'s') + 1;
            printf("DNS: %s \n\r",p);
            ret = true;
        }
    }
    pclose(pf);
    pf = NULL;
    return ret;
}
*/

static void printf_connected_info(void){
    if (wifi_wlan0.completed) {
        wifi_wlan0.level = current_wlan_signal_level();
        printf("ssid: %s\n\r",wifi_wlan0.wlan_ssid);
        printf("mac: %s \n\r",wifi_wlan0.mac);
        printf("ip: %s \n\r",wifi_wlan0.ip);
        printf("key_mgmt %s \n\r",wifi_wlan0.key_mgmt);
        printf("level: %d  \n\r", wifi_wlan0.level);
    }
}

static void kill_background_thread(void){

    system("killall wpa_supplicant");
    system("killall udhcpc");
}
static void start_connect_process(void){
    system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B");
}

static void *check_wlan_task(void *arg) {
    bool run_once_flag = false;
    bool connected_info_active = false;
    scanned_wifi_index = 0;

    signal(SIGCHLD,SIG_IGN);

    kill_background_thread();//杀掉所有在运行的wifi服务器和客户端
    bool *run = (bool *) arg;

    int udhcpc_cnt = 0;
    while (*run) {
        if(wlan_turn_on){
            // Once: start connect wi-fi//第一次进入 连接wifi
            if(run_once_flag == false){
                start_connect_process();
                system("udhcpc -i wlan0 -n 4 -R &");
                udhcpc_cnt = 50;
                run_once_flag = true;
                printf("udhcpc -i wlan0 -n 4 -R & !\n\r");
            }else if(connected_info_active == false){
                // Get udhcpc status
                if(if_wlan_up() == 1){//获取wifi状态 在线

                    // Update connected wi-fi info
                    scanned_wifi_index = wpa_cli_scan_wifi(&wlan_turn_on);//扫描wifi
                    wpa_cli_wlan_status(&wlan_turn_on);//获取状态
                    printf_connected_info();//打印连接的标志
                    connected_info_active = true;
                }else{//没有连接的wifi
                    connected_info_active = false;
                    if(if_wlan_up() == 2){
                        if(scanned_wifi_index == 0 ){
                           
                            scanned_wifi_index = wpa_cli_scan_wifi(&wlan_turn_on);
                        }
                        if(udhcpc_cnt && (--udhcpc_cnt == 0)){
                            system("killall udhcpc");
                           
                        }
                    }
                }
            }else if(connected_info_active == true){
                if(scanned_wifi_index == 0){
                    wpa_cli_wlan_status(&wlan_turn_on);
                    scanned_wifi_index = wpa_cli_scan_wifi(&wlan_turn_on);
                }
            }else{
                printf("---%s------%d-----\n\r",__func__ ,__LINE__);
            }
        }else{
            if(connected_info_active == true){
                scanned_wifi_index = 0;
                connected_info_active = false;
                run_once_flag = false;
                kill_background_thread();
                ak_sleep_ms(500);
            }else if(if_wlan_up() && run_once_flag){
                
                scanned_wifi_index = 0;
                connected_info_active = false;
                run_once_flag = false;
                kill_background_thread();
            }
        }
        ak_sleep_ms(200);
    }
    printf("!!!!!--- DA MIE ----!!!!!\n\r");
    *run = false;
    ak_thread_exit();
    return NULL;
}

bool create_check_wlan_task(void) {
    if(wlan_thread_run){
        return false;
    }
    wlan_thread_run = true;
    ak_thread_create(&wlan_thread_id, check_wlan_task, &wlan_thread_run, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    return true;
}

bool destroy_check_wlan_task(void) {
    if (wlan_thread_run == false) {
        return false;
    }
    wlan_thread_run = false;
    ak_thread_join(wlan_thread_id);
    wlan_thread_id = 0;
    return true;
}


void update_wifi_list(void){
    scanned_wifi_index = 0;
}

int get_max_wifi_list_index(void){
    return scanned_wifi_index;
}

void turn_on_wlan_connect(void){
    wlan_turn_on = true;
	start_connect_process();
	system("udhcpc -i wlan0 -n 4 -R &");
    update_wifi_list();
}

void turn_off_wlan_connect(void){
    wlan_turn_on = false;
	kill_background_thread();//关闭wifi的客户端和服务器
}



void turn_on_walan_reconnect(void){
  
  	//system("wpa_cli -i wlan0 enable_network 0");
	//printf("############wlan0 up!\n");
    system("wpa_cli -i wlan0 reconnect");
	printf("$$$$$$$$$$$$$$$$$$udhcpc wlan0!\n");
	//update_wifi_list();

}


void turn_off_wlan_break(void){//关闭wifi连接
	
	//system("wpa_cli -i wlan0 disable_network 0");

	system("wpa_cli -i wlan0 disconnect");
	printf("@@@@@@@@@@@wlan0 down!\n");

}



void CloseConnectingWifi(void){

	system("ifconfig wlan0 down");
//	system("killall wpa_supplicant");
//	system("killall udhcpc");
//	system("killall udhcpd");
//	system("killall hostapd");
}



void StartConnectingWifi(void){
	//CloseConnectingWifi();
	//ak_sleep_ms(2000);
	//system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B");
	system("ifconfig wlan0 up");
	
	system("killall udhcpc");
	system("udhcpc -b -i wlan0 &");
//	system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B");
}



bool net_work_ping(const char *address)
{
	// if(wlan_is_wifi_connect_success() != 1) return false;

	#define BUF_SIZE	(64)
	
	char buf[BUF_SIZE] = {0};
	int ret = sprintf(buf, "ping -W 2 -c 2 -i 0.1 %s", address);
	// LOG_WHITE("buf:%s\n", buf);

	if(ret > BUF_SIZE)	printf("Data leakage, memory offside !!!\n");

	FILE *fp = popen(buf, "r");
	if(fp == NULL)
	{
		printf("err: open [%s] fail...\n", buf);
		pclose(fp);
		return false;
	}


	char buffer[256] = {0};
	while (fgets(buffer, sizeof(buffer), fp))
	{
		char *pstr = strstr(buffer, "ttl=");
        if (pstr != NULL)
        {
			pclose(fp);
			return true;            
        }
        memset(buffer, 0, sizeof(buffer));
	}

	pclose(fp);
	return false;

}

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

void reset_wifi_password(void)
{
    const char *conf_path = "/etc/config/wpa_supplicant.conf";
    const char *tmp_path = "/tmp/wpa_tmp.conf";

    // 1. 备份原文件权限（兼容系统原有配置）
    struct stat st;
    if (stat(conf_path, &st) != 0) {
        return;
    }

    // 2. 逐行替换psk，生成临时文件
    FILE *fp = fopen(conf_path, "r");
    if (!fp) return;

    FILE *tmp_fp = fopen(tmp_path, "w");
    if (!tmp_fp) {
        fclose(fp);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "psk=") != NULL) {
            fprintf(tmp_fp, "        psk=\"847413626\"\n");
        } else {
            fputs(line, tmp_fp);
        }
    }

    fclose(fp);
    fclose(tmp_fp);

    // 3. 覆盖原文件，强制恢复权限
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "cp -f %s %s", tmp_path, conf_path);
    system(cmd);
    unlink(tmp_path);

    chmod(conf_path, 0600);
    chown(conf_path, st.st_uid, st.st_gid);
}