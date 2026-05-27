#include "tuya_sdk.h"
#include "ak_thread.h"
#include <stdio.h>
#include <math.h>

#include <stdlib.h>
#include <sys/stat.h>

#include <unistd.h>

#include "tuya_ipc_api.h"
#include "string.h"
#include "memory.h"
#include "ak_common.h"
#include "leo_tuya_key_check.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_ipc_p2p.h"
#include "wlan.h"
#include "../../layout/user_data.h"
#include "../../layout/layout_define.h"

#define IPC_APP_STORAGE_PATH "/app/data/"
#define IPC_APP_UPGRADE_FILE "/tmp/"
#define IPC_APP_SD_BASE_PATH "/mnt/tf/"

static tuya_api_weather weather;

static char tuya_pid[IPC_PRODUCT_KEY_LEN + 1] = {0};
// static char tuya_uuid[IPC_UUID_LEN + 1] = {0};
// static char tuya_auth_key[IPC_AUTH_KEY_LEN + 1] = {0};

bool tuya_sdk_inited = false;

BOOL_T door_lock_state = false;
BOOL_T door_lock2_state = false;

tuya_api_weather tuya_weather_get()
{
    return weather;
}

static void tuya_dp_query_func(const TY_DP_QUERY_S *dp_duery)
{
    /*
     * 此函数基本上没有
     */
    printf("%s:%d %d\n", __func__, __LINE__, dp_duery->cnt);
}

static void tuya_upgrade_info_func(const FW_UG_S *fw)
{
    printf("Rev Upgrade Info \n\r");
    printf("fw->fw_url:%s \n\r", fw->fw_url);
    printf("fw->fw_md5:%s \n\r", fw->fw_md5);
    printf("fw->sw_ver:%s \n\r", fw->sw_ver);
    printf("fw->file_size:%u \n\r", fw->file_size);
}

// --------------- DP PROCESS ---------------

#define SWITCH_CHANNEL_CMD_HEAD "{\\\"cmd\\\":1,\\\"cc\\\":1,\\\"chs\\\":["
#define TUYA_CHANNEL_CMD_TAIL "]}\"}"

#define CHANNEL_RESULT_CMD_HEAD "{\\\"res\\\":1,\\\"err\\\":0,\\\"cc\\\":"
#define CHANNEL_RESULT_CMD_TAIL ",\\\"chs\\\":["

#define MON_CH_NONE 0
#define MON_CH_DOOR1 1
#define MON_CH_DOOR2 2
#define MON_CH_DOOR3 3
#define MON_CH_DOOR4 4
#define MON_CH_DOOR5 5
#define MON_CH_DOOR6 6
#define MON_CH_DOOR7 7
#define MON_CH_DOOR8 8
#define MON_CH_DOOR9 9
#define MON_CH_DOOR10 10
#define MON_CH_DOOR11 11
#define MON_CH_DOOR12 12
#define MON_CH_DOOR13 13
#define MON_CH_DOOR14 14
#define MON_CH_DOOR15 15
#define MON_CH_DOOR16 16
#define MON_CH_DOOR17 17
#define MON_CH_DOOR18 18
#define MON_CH_CCTV1 19
#define MON_CH_CCTV2 20
#define MON_CH_CCTV3 21
#define MON_CH_CCTV4 22
#define MON_CH_CCTV5 23
#define MON_CH_CCTV6 24
#define MON_CH_CCTV7 25
#define MON_CH_CCTV8 26
#define TUYA_LANGUAGE_TOTAL 15

static bool video_channel_state[26] = {0};
void set_tuya_channel_state(int channel, bool state)
{
    if (video_channel_state[channel - 1] != state)
    {
        video_channel_state[channel - 1] = state;
        tuya_channel_valid_report();
    }
}
static bool monitor_valid_channel_check(char channel)
{
    if ((channel >= 1) && (channel <= 26))
    {
        if (video_channel_state[channel - 1] == true)
        {
            return true;
        }
    }
    return false;
}

static int monitor_channel_flag = MON_CH_NONE;

void tuya_current_channel_set(int channel)
{
    monitor_channel_flag = channel;
}

int tuya_current_channel_get(void)
{
    return monitor_channel_flag;
}

static int tuya_current_language = 0;

void tuya_set_current_language(int language)
{
    tuya_current_language = language;
}

int tuya_get_current_language(void)
{
    return tuya_current_language;
}
static bool tuya_valid_channel_get_str(int id, char *str)
{
    // printf("\n开始打印\n");
    // for(int i =0;i<5;i++){
    //     printf("===ser_data_get()->tuya_ch_name[%d].ch_name=>%s===========\n",i,user_data_get()->tuya_ch_name[i].ch_name);
    // }
    // printf("\n结束打印\n");

    char door_dp_str[128] = {0};
    switch (id)
    {
    case 1:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR1) == false)
        {
            return false;
        }
        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door1", "大门1", "Дверь1", "Porte1", "Puerta1", "Tür1", "باب1", "Dveře1", "1שער", "Porta1", "Door1", "Door1", "Door1","Kapı1", "Deur1"

        };
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 1, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[1].ch_name) > 0)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 1, user_data_get()->tuya_ch_name[1].ch_name);
        }
    }
    break;
    case 2:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR2) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door2", "大门2", "Дверь2", "Porte2", "Puerta2", "Tür2", "2باب", "Dveře2", "2שער", "Porta2", "Door2", "Door2", "Door2", "Kapı2", "Deur2"

        };
        printf("\n\r ###############MON_CH_DOOR2#################### \n\r");

        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 2, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[2].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 2, user_data_get()->tuya_ch_name[2].ch_name);
        }
    }
    break;

    case 3:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR3) == false)
        {
            return false;
        }
        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door0", "大门0", "Дверь0", "Porte0", "Puerta0", "Tür0", "باب0", "Dveře0", "0שער", "Porta0", "Door0", "Door0", "Door0", "Kapı0", "Deur0"

        };
        printf("\n\r ###############MON_CH_DOOR0#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 3, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[3].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 3, user_data_get()->tuya_ch_name[3].ch_name);
        }
    }

    break;

    case 4:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR4) == false)
        {
            return false;
        }
        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door1", "大门1", "Дверь1", "Porte1", "Puerta1", "Tür1", "باب1", "Dveře1", "1שער", "Porta1", "Door1", "Door1", "Door1", "Kapı1", "Deur1"

        };
        printf("\n\r ###############MON_CH_DOOR1#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 4, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[4].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 4, user_data_get()->tuya_ch_name[4].ch_name);
        }
    }
    break;
    case 5:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR5) == false)
        {
            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door2", "大门2", "Дверь2", "Porte2", "Puerta2", "Tür2", "2باب", "Dveře2", "2שער", "Porta2", "Door2", "Door2", "Door2", "Kapı2", "Deur2"

        };
        printf("\n\r ###############MON_CH_DOOR2#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 5, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[5].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 5, user_data_get()->tuya_ch_name[5].ch_name);
        }
    }
    break;

    case 6:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR6) == false)
        {
            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door3", "大门3", "Дверь3", "Porte3", "Puerta3", "Tür3", "3باب", "Dveře3", "3שער", "Porta3", "Door3", "Door3", "Door3", "Kapı3", "Deur3"

        };
        printf("\n\r ###############MON_CH_DOOR3#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 6, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[6].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 6, user_data_get()->tuya_ch_name[6].ch_name);
        }
    }
    break;

    case 7:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR7) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door4", "大门4", "Дверь4", "Porte4", "Puerta4", "Tür4", "4باب", "Dveře4", "4שער", "Porta4", "Door4", "Door4", "Door4", "Kapı4", "Deur4"

        };
        printf("\n\r ###############MON_CH_DOOR4#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 7, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[7].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 7, user_data_get()->tuya_ch_name[7].ch_name);
        }
    }
    break;

    case 8:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR8) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door5", "大门5", "Дверь5", "Porte5", "Puerta5", "Tür5", "5باب", "Dveře5", "5שער", "Porta5", "Door5", "Door5", "Door5", "Kapı5", "Deur5"

        };
        printf("\n\r ###############MON_CH_DOOR5#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 8, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[8].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 8, user_data_get()->tuya_ch_name[8].ch_name);
        }
    }
    break;

    case 9:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR9) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door6", "大门6", "Дверь6", "Porte6", "Puerta6", "Tür6", "6باب", "Dveře6", "6שער", "Porta6", "Door6", "Door6", "Door6", "Kapı6", "Deur6"

        };
        printf("\n\r ###############MON_CH_DOOR6#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 9, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[9].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 9, user_data_get()->tuya_ch_name[9].ch_name);
        }
    }
    break;
    case 10:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR10) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door7", "大门7", "Дверь7", "Porte7", "Puerta7", "Tür7", "7باب", "Dveře7", "7שער", "Porta7", "Door7", "Door7", "Door7", "Kapı7", "Deur7"

        };
        printf("\n\r ###############MON_CH_DOOR7#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 10, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[10].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 10, user_data_get()->tuya_ch_name[10].ch_name);
        }
    }
    break;
    case 11:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR11) == false)
        {
            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door8", "大门8", "Дверь8", "Porte8", "Puerta8", "Tür8", "8باب", "Dveře8", "8שער", "Porta8", "Door8", "Door8", "Door8", "Kapı8", "Deur8"

        };
        printf("\n\r ###############MON_CH_DOOR8#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 11, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[11].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 11, user_data_get()->tuya_ch_name[11].ch_name);
        }
    }
    break;
    case 12:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR12) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door9", "大门9", "Дверь9", "Porte9", "Puerta9", "Tür9", "9باب", "Dveře9", "9שער", "Porta9", "Door9", "Door9", "Door9", "Kapı9", "Deur9"

        };
        printf("\n\r ###############MON_CH_DOOR9#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 12, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[12].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 12, user_data_get()->tuya_ch_name[12].ch_name);
        }
    }
    break;
    case 13:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR13) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door10", "大门10", "Дверь10", "Porte10", "Puerta10", "Tür10", "10باب", "Dveře10", "10שער", "Porta10", "Door10", "Door10", "Door10", "Kapı10", "Deur10"

        };
        printf("\n\r ###############MON_CH_DOOR10#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 13, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[13].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 13, user_data_get()->tuya_ch_name[13].ch_name);
        }
    }
    break;
    case 14:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR14) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door11", "大门11", "Дверь11", "Porte11", "Puerta11", "Tür11", "11باب", "Dveře11", "11שער", "Porta11", "Door11", "Door11", "Door11", "Kapı11", "Deur11"

        };
        printf("\n\r ###############MON_CH_DOOR11#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 14, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[14].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 14, user_data_get()->tuya_ch_name[14].ch_name);
        }
    }
    break;
    case 15:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR15) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door12", "大门12", "Дверь12", "Porte12", "Puerta12", "Tür12", "12باب", "Dveře12", "12שער", "Porta12", "Door12", "Door12", "Door12","Kapı12", "Deur12"

        };
        printf("\n\r ###############MON_CH_DOOR12#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 15, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[15].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 15, user_data_get()->tuya_ch_name[15].ch_name);
        }
    }
    break;
    case 16:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR16) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door13", "大门13", "Дверь13", "Porte13", "Puerta13", "Tür13", "13باب", "Dveře13", "13שער", "Porta13", "Door13", "Door13", "Door13","Kapı13", "Deur13"

        };
        printf("\n\r ###############MON_CH_DOOR13#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 16, tmp_str[tuya_get_current_language()]);
        if (strlen(user_data_get()->tuya_ch_name[16].ch_name) > 1)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 16, user_data_get()->tuya_ch_name[16].ch_name);
        }
    }
    break;
    case 17:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR17) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door14", "大门14", "Дверь14", "Porte14", "Puerta14", "Tür14", "14باب", "Dveře14", "14שער", "Porta14", "Door14", "Door14", "Door14","Kapı14", "Deur14"

        };
        printf("\n\r ###############MON_CH_DOOR14#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 17, tmp_str[tuya_get_current_language()]);
        if (user_data_get()->tuya_ch_name[17].ch_name)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 17, user_data_get()->tuya_ch_name[17].ch_name);
        }
    }
    break;
    case 18:
    {
        if (monitor_valid_channel_check(MON_CH_DOOR18) == false)
        {

            return false;
        }

        char *tmp_str[TUYA_LANGUAGE_TOTAL] = {
            "Door15", "大门15", "Дверь15", "Porte15", "Puerta15", "Tür15", "15باب", "Dveře15", "15שער", "Porta15", "Door15", "Door15", "Door15","Kapı15", "Deur15"

        };
        printf("\n\r ###############MON_CH_DOOR15#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 18, tmp_str[tuya_get_current_language()]);
        if (user_data_get()->tuya_ch_name[18].ch_name)
        {
            sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 18, user_data_get()->tuya_ch_name[18].ch_name);
        }
    }
    break;
    case 19:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV1) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV1#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 19, "CCTV1");
    }
    break;
    case 20:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV2) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV2#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 20, "CCTV2");
    }
    break;
    case 21:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV3) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV3#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 21, "CCTV3");
    }
    break;
    case 22:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV4) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV4#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 22, "CCTV4");
    }
    break;
    case 23:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV5) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV5#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 23, "CCTV5");
    }
    break;
    case 24:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV6) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV6#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 24, "CCTV6");
    }
    break;
    case 25:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV7) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV7#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 25, "CCTV7");
    }
    break;
    case 26:
    {
        if (monitor_valid_channel_check(MON_CH_CCTV8) == false)
        {
            return false;
        }

        printf("\n\r ###############MON_CH_CCTV8#################### \n\r");
        sprintf(door_dp_str, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s\\\"},", 26, "CCTV8");
    }
    break;
    default:
        return false;
        break;
    }
    strcpy(str, door_dp_str);
    return true;
}

static bool tuya_check_channel_valid(int channel)
{

    if ((channel < 1) || (channel > 26))
    {
        return false;
    }
    if (monitor_valid_channel_check(channel) == false)
    {
        return false;
    }

    return true;
}

int tuya_switch_channel_upload_results(int channel)
{
    for (int i = 0; i < 8; i++)
    {
        if (user_data_get()->onvif_dev[i].url[0] != 0)
        {
            video_channel_state[18 + i] = true; // 使能涂鸦通道的状态
        }
        else
        {
            video_channel_state[18 + i] = false; // 使能涂鸦通道的状态
        }
    }

    char dp_result_str[512] = {0};
    sprintf(dp_result_str, "%s%d%s", CHANNEL_RESULT_CMD_HEAD, channel, CHANNEL_RESULT_CMD_TAIL);

    bool is_valid_channel = false;

    char door_str[512] = {0};
    for (int i = 1; i < 27; i++)
    {

        memset(door_str, 0, sizeof(door_str));
        if (tuya_valid_channel_get_str(i, door_str) == true)
        {
            printf("\n已修改=>%s\n", door_str);
            strcat(dp_result_str, door_str);
            is_valid_channel = true;
        }
    }
    if (is_valid_channel == false)
    {
        return -1;
    }

    dp_result_str[strlen(dp_result_str) - 1] = '\0';
    strcat(dp_result_str, TUYA_CHANNEL_CMD_TAIL);
    return tuya_ipc_dp_report(NULL, KOCOM_DP_SWITCH_CHANNEL, PROP_STR, dp_result_str, 1);
}

/************************************************************************************************************
PULL CMD FORMAT:
{"cmd":1,"cc":1,"chs":[
 {"id":1,"n":"door1"},
 {"id":2,"n":"door2"},
 {"id":3,"n":"CCTV1"},
 {"id":4,"n":"CCTV2"}]}

PUSH CMD FORMAT:
"{\\\"res\\\":1,\\\"err\\\":0,\\\"cc\\\":1,\\\"chs\\\":[
 {\\\"id\\\":1,\\\"n\\\":\\\"door1\\\"},
 {\\\"id\\\":2,\\\"n\\\":\\\"door2\\\"},
 {\\\"id\\\":3,\\\"n\\\":\\\"CCTV1\\\"},
 {\\\"id\\\":4,\\\"n\\\":\\\"CCTV2\\\"}]}\"}"
************************************************************************************************************/

static int tuya_channel_change(TY_OBJ_DP_S *dp)
{
    char *down_head_str = {"{\"cmd\":1,\"cc\":"};
    if (strncmp(dp->value.dp_str, down_head_str, strlen(down_head_str)) != 0)
    {
        char *down_head_str2 = {"{\"cmd\":2,\"cc\":"};
        if (strncmp(dp->value.dp_str, down_head_str2, strlen(down_head_str2)) != 0)
        {
            printf("String error \n\r");
            return -1;
        }
        /*
        authot:zio
        data:20260119
        */        
        // cJSON *dp_data = (dp->value.dp_str);
        cJSON *dp_data = cJSON_Parse(dp->value.dp_str);
        if (dp_data == NULL) {
            // 解析失败，处理错误
            printf("JSON parse error\n");
            return -1;
        }
        cJSON *chs_json = cJSON_GetObjectItem(dp_data, "chs");
        int i = 0;
        cJSON *json_chs_arr = NULL;
        char *str_id = NULL;
        char *str_n = NULL;
        while ((json_chs_arr = cJSON_GetArrayItem(chs_json, i)) != NULL)
        {
            char temp_id[8];
            memset(temp_id, 0, sizeof(temp_id));
            str_id = cJSON_Print(cJSON_GetObjectItem(json_chs_arr, "id"));
            str_n = cJSON_Print(cJSON_GetObjectItem(json_chs_arr, "n"));
            strcpy(temp_id, str_id);
            int id_num = atoi(temp_id);
            if (strcmp(str_n, "\0") != 0)
            {
                int len = strlen(str_n);
                //char sub[32];
                memset(user_data_get()->tuya_ch_name[id_num].ch_name, 0, sizeof(user_data_get()->tuya_ch_name[id_num].ch_name));
                strncpy(user_data_get()->tuya_ch_name[id_num].ch_name, str_n + 1, len - 2);
            }
            printf("\n\n修改中===id=>%d,name=>%s\n\n", id_num, user_data_get()->tuya_ch_name[id_num].ch_name);
            user_data_save();
            i++;
        }
        tuya_switch_channel_upload_results(monitor_channel_get());
        return 0;
    }
    // 切换通道
    int channel_id = 0;
    for (int i = 0; i < 2; i++)
    {
        int num = dp->value.dp_str[strlen(down_head_str) + i] - 48;
        printf("===num is %d\n", num);
        if ((num >= 0) && (num <= 9))
        {
            channel_id = channel_id * pow(10, i) + num;
        }
    }
    if (tuya_check_channel_valid(channel_id) == false)
    {
        return -1;
    }
    printf("channel_id is %d\n", channel_id);
    extern bool tuya_monitor_swap_event(int ch);
    tuya_monitor_swap_event(channel_id);
    return 0;
}

int tuya_channel_valid_report(void)
{

    if (tuya_sdk_inited == false)
    {
        return -1;
    }

    char dp_str[512] = {0};
    char door_str[128] = {0};
    bool is_valid_channel = false;

    strcpy(dp_str, SWITCH_CHANNEL_CMD_HEAD);
    for (int i = 1; i < 27; i++)
    {

        memset(door_str, 0, sizeof(door_str));
        if (tuya_valid_channel_get_str(i, door_str) == true)
        {
            strcat(dp_str, door_str);
            is_valid_channel = true;
        }
    }
    if (is_valid_channel == false)
    {
        return -1;
    }
    dp_str[strlen(dp_str) - 1] = '\0';
    strcat(dp_str, TUYA_CHANNEL_CMD_TAIL);
    printf("report ch dp_str is %s\n", dp_str);
    return tuya_ipc_dp_report(NULL, KOCOM_DP_SWITCH_CHANNEL, PROP_STR, dp_str, 1);
}

static int tuya_dp_148_accessory_lock(TY_OBJ_DP_S *dp)
{

    if (dp == NULL || (dp->type != PROP_BOOL))
    {
        printf("Error! type invalid %d \n\r", dp->type);
        return -1;
    }
    extern bool tuya_monitor_unlock_event(bool state);
    tuya_monitor_unlock_event(dp->value.dp_bool);
    return 0;
}

int tuya_dp_148_response_accessory_lock(BOOL_T state)
{

    door_lock_state = state;
    printf("====door lock state is %d\n", door_lock_state);
    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = state;
    printf("====door lock state is %d\n", state);
    return tuya_ipc_dp_report(NULL, KOCOM_DP_LOCK, PROP_BOOL, &param, 1);
}

static int tuya_dp_239_accessory_lock(TY_OBJ_DP_S *dp)
{

    if (dp == NULL || (dp->type != PROP_BOOL))
    {
        printf("Error! type invalid %d \n\r", dp->type);
        return -1;
    }
    extern bool tuya_monitor_unlock2_event(bool state);
    tuya_monitor_unlock2_event(dp->value.dp_bool);
    return 0;
}

int tuya_dp_239_response_accessory_lock(BOOL_T state)
{

    door_lock2_state = state;
    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = state;
    return tuya_ipc_dp_report(NULL, KOCOM_DP_LOCK2, PROP_BOOL, &param, 1);
}

int tuya_dp_238_response_access_lock_support()
{

    char *str = "148,239";
    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    return tuya_ipc_dp_report(NULL, KOCOM_DP_ACCESS_LOCK_SUPPORT, PROP_STR, str, 1);
}

static int tuya_dp_232_absent_mode(TY_OBJ_DP_S *dp)
{

    if (dp == NULL || (dp->type != PROP_BOOL))
    {
        printf("Error! type invalid %d \n\r", dp->type);
        return -1;
    }
    extern bool tuya_monitor_absent_mode_event(bool state);
    tuya_monitor_absent_mode_event(dp->value.dp_bool);
    return 0;
}

int tuya_dp_232_response_absent_mode(BOOL_T state)
{

    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = state;
    return tuya_ipc_dp_report(NULL, KOCOM_DP_ABSENT_MODE, PROP_BOOL, &param, 1);
}

static int tuya_dp_236_home_mode(TY_OBJ_DP_S *dp)
{

    if (dp == NULL || (dp->type != PROP_BOOL))
    {
        printf("Error! type invalid %d \n\r", dp->type);
        return -1;
    }
    extern bool tuya_monitor_home_mode_event(bool state);
    tuya_monitor_home_mode_event(dp->value.dp_bool);
    return 0;
}

int tuya_dp_236_response_home_mode(BOOL_T state)
{

    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = state;
    return tuya_ipc_dp_report(NULL, KOCOM_DP_HOME_MODE, PROP_BOOL, &param, 1);
}

static int tuya_dp_237_sleep_mode(TY_OBJ_DP_S *dp)
{

    if (dp == NULL || (dp->type != PROP_BOOL))
    {
        printf("Error! type invalid %d \n\r", dp->type);
        return -1;
    }
    extern bool tuya_monitor_sleep_mode_event(bool state);
    tuya_monitor_sleep_mode_event(dp->value.dp_bool);
    return 0;
}

int tuya_dp_237_response_sleep_mode(BOOL_T state)
{

    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = state;
    return tuya_ipc_dp_report(NULL, KOCOM_DP_SLEEP_MODE, PROP_BOOL, &param, 1);
}

int tuya_dp_233_response_device_active(void)
{
    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = 1;
    return tuya_ipc_dp_report(NULL, KOCOM_DP_DEVICE_ACTIVE, PROP_BOOL, &param, 1);
}

int tuya_dp_234_response_abnormal_unlock(void)
{

    if (tuya_sdk_inited == false)
    {
        return -1;
    }
    BOOL_T param = 1;
    return tuya_ipc_dp_report(NULL, KOCOM_DP_ABNORMAL_UNLOCK, PROP_BOOL, &param, 1);
}

int tuya_dp_uploads_security_msg(char id, char *data, int size)
{

    NOTIFICATION_NAME_E name;
    if (id == 1)
    {
        name = NOTIFICATION_NAME_IO_ALARM;
    }
    if (id == 2)
    {
        name = NOTIFICATION_NAME_USER_IO;
    }
    tuya_ipc_notify_with_event(data, size, NOTIFICATION_CONTENT_JPEG, name);
    return 1;
}

static void tuya_dp_handle_func(const TY_RECV_OBJ_DP_S *dp_rev)
{
    TY_OBJ_DP_S *dp_data = (TY_OBJ_DP_S *)(dp_rev->dps);
    printf("\n\r \033[33m TUYA: dpid:%d type:%d time:%d  ",
           dp_data->dpid,
           dp_data->type,
           dp_data->time_stamp);
    if (dp_data->type == PROP_BOOL)
    {
        printf("\033[33m  value:%d \033[37m \r\n", dp_data->value.dp_bool);
    }
    else if (dp_data->type == PROP_VALUE)
    {
        printf("\033[33m  value:%d \033[37m \r\n", dp_data->value.dp_value);
    }
    else if (dp_data->type == PROP_STR)
    {
        printf("\033[33m  value:%s \033[37m \r\n", dp_data->value.dp_str);
    }
    else if (dp_data->type == PROP_ENUM)
    {
        printf("\033[33m  value:%d \033[37m \r\n", dp_data->value.dp_enum);
    }
    switch (dp_data->dpid)
    {

    case KOCOM_DP_DOORBELL: /// UPLOAD
        // tuya_ipc_door_bell_press(DOORBELL_AC, NULL, NULL, NOTIFICATION_CONTENT_JPEG);
        break;
    case KOCOM_DP_LOCK:
        tuya_dp_148_accessory_lock(dp_data);
        break;
    case KOCOM_DP_PICTURE: /// UPLOAD
        break;
    case KOCOM_DP_ALARM_MSG: /// UPLOAD
        break;
    case KOCOM_DP_SWITCH_CHANNEL:
        tuya_channel_change(dp_data);
        break;
    case KOCOM_DP_LOCK2:
        tuya_dp_239_accessory_lock(dp_data);
        break;
    case KOCOM_DP_ABSENT_MODE:
        tuya_dp_232_absent_mode(dp_data);
        break;
    case KOCOM_DP_HOME_MODE:
        tuya_dp_236_home_mode(dp_data);
        break;
    case KOCOM_DP_SLEEP_MODE:
        tuya_dp_237_sleep_mode(dp_data);
        break;

    default:
        break;
    }
}
void tuya_response_mode_sync(void)
{
    int state = user_data_get()->other.mode;
    if (state == 0)
    {
        tuya_dp_232_response_absent_mode(false);
        tuya_dp_236_response_home_mode(false);
        tuya_dp_237_response_sleep_mode(true);
    }
    else if (state == 1)
    {
        tuya_dp_232_response_absent_mode(false);
        tuya_dp_236_response_home_mode(true);
        tuya_dp_237_response_sleep_mode(false);
    }
    else
    {
        tuya_dp_232_response_absent_mode(true);
        tuya_dp_236_response_home_mode(false);
        tuya_dp_237_response_sleep_mode(false);
    }
}

void all_device_mode_sync(void)
{
    tuya_response_mode_sync();

    network_cmd_data_init(data);
    data.cmd = NET_COMMON_CMD_MODE_SYNC;
    data.arg1 = user_data_get()->other.mode;
    data.arg2 = 0;
    data.device = DEVICE_GROUP;
    network_send_cmd_data(&data);
}

extern OPERATE_RET TUYA_APP_Enable_P2PTransfer(IN UINT_T max_users);

static bool tuya_app_enable_p2p_func(void)
{

    static bool is_first = true;
    if (is_first == false)
    {
        return true;
    }

    is_first = false;
    TUYA_APP_Enable_P2PTransfer(4);
    tuya_ipc_upload_skills();
    return true;
}

static void report_dp_func(void)
{
    tuya_channel_valid_report();
    tuya_dp_148_response_accessory_lock(door_lock_state);
    tuya_dp_239_response_accessory_lock(door_lock2_state);
    tuya_dp_238_response_access_lock_support();

    tuya_response_mode_sync();
}

bool is_online_tuya_cloud(void)
{

    if (tuya_sdk_inited == false)
    {
        return false;
    }

    return true;
    //  return tuya_ipc_get_mqtt_status() ? true : false;
}

int is_tuya_cloud_connected_num(void)
{
    if (is_online_tuya_cloud() == false)
    {
        return -1;
    }

    return tuya_ipc_get_client_online_num();
}

// hlf
static bool tuya_online_state = false;
bool tuya_api_online_check()
{
    return tuya_online_state;
}
static void tuya_status_change_func(const BYTE_T stat)
{
    printf("%s:%d status change:%d \n", __func__, __LINE__, stat);
    switch (stat)
    {
    case STAT_UNPROVISION:
        tuya_dp_233_response_device_active();
        break;
    case STAT_CLOUD_CONN:
    case STAT_MQTT_ONLINE:
        //	case GB_STAT_CLOUD_CONN:
        tuya_app_enable_p2p_func();
        report_dp_func();
        tuya_online_state = true;
        printf("\n\r============= mqtt is online =================\r\n");
        break;
    case WF_START_AP_ONLY:
    case STAT_MQTT_OFFLINE:
        tuya_online_state = false;
        printf("\n\r============= mqtt is off =================\r\n");
        break;
    default:
        printf("get status change stat %d\n", stat);
        break;
    }
}

static void tuya_reboot_func(void)
{
    printf("%s:%d status reboot\n", __func__, __LINE__);
}

static void tuya_rest_system_func(GW_RESET_TYPE_E type)
{
    printf("reset ipc success. please restart the ipc %d\n", type);

    // printf("rm -rf /etc/config/tuya_\n");
    // system("rm -rf /etc/config/tuya_user.db");
    // system("rm -rf /etc/config/tuya_user.db_bak");
    // system("rm -rf /etc/config/tuya_enckey.db");

    system("rm -rf /app/data/tuya_user.db");
	system("rm -rf /app/data/tuya_user.db_bak");
	system("rm -rf /app/data/tuya_enckey.db");
    system("rm -rf /app/data/log_seq_stat");
    system("sync");
    // system("reboot");
    usleep(1000 * 1000);
	exit(-1);
}

extern char tuya_uid[64];
extern char tuya_key[64];

bool tuya_sdk_start(void)
{
    TUYA_IPC_ENV_VAR_S env;

    memset(&env, 0, sizeof(TUYA_IPC_ENV_VAR_S));

    strcpy(env.storage_path, IPC_APP_STORAGE_PATH);
    strcpy(env.product_key, tuya_pid);
    // strcpy(env.uuid, "tuyaf0fc7625f1d03ac5");
    // strcpy(env.auth_key, "KQwVLrV9CRMXrQrmTyY68Z8eJ3totQGN");
    // strcpy(user_data_get()->tuya_uid, "mtkj008ee1dc3d59eba9");
    // strcpy(user_data_get()->tuya_key, "AkiBjjcH5fQaZXzxFQADboHAAJUZSDsP");

    strcpy(env.uuid, tuya_uid);
    strcpy(env.auth_key, tuya_key);
    strcpy(env.dev_sw_version, "1.2.3");
    strcpy(env.dev_serial_num, "tuya_ipc");

    env.dev_obj_dp_cb = tuya_dp_handle_func;
    env.dev_dp_query_cb = tuya_dp_query_func;

    env.status_changed_cb = tuya_status_change_func;

    env.gw_ug_cb = tuya_upgrade_info_func;

    env.gw_restart_cb = tuya_reboot_func;
    env.gw_rst_cb = tuya_rest_system_func;

    env.mem_save_mode = false; /* true;//false; */
    tuya_ipc_init_sdk(&env);
    tuya_ipc_set_region(REGION_CN);
    tuya_ipc_start_sdk(WIFI_INIT_NULL, NULL);
    return true;
}

extern linked_info link_info;

void find_link_wifi(void)
{

    memset(&link_info, 0, sizeof(linked_info));
    bool a = true;
    wpa_cli_scan_wifi(&a);
    extern bool wpa_cli_wlan_status(bool *continue_flag);
    wpa_cli_wlan_status(&a); // 获取wifi状态 再获取链接WiFi的信息

    get_linked_wifi_info(&link_info);
}

bool wifi_work_restart(void)
{

    system("killall wpa_supplicant");
    system("killall udhcpc");
    ak_sleep_ms(100);
    system("wpa_supplicant -Dnl80211 -i wlan0 -c /etc/config/wpa_supplicant.conf -B &");
    ak_sleep_ms(1200);
    system("udhcpc -b -i wlan0 &");
    ak_sleep_ms(1000);

    // create_check_wlan_task();
    return true;
}

static char tuya_qrcode_str[64] = {0};
char *tuya_qrcode_str_get(void)
{
    if (tuya_qrcode_str[0] == 0)
    {
        printf("\n\ntuya_qrcode_str=>NULL \n");

        return NULL;
    }
    return tuya_qrcode_str;
}

static bool _tuya_api_weather_get(tuya_api_weather *nm);
void *tuya_wifi_sdk_init_task(void *arg)
{
    printf("tuya sdk init start... \n");

    // tuya_uuid_and_key_read(tuya_uid, tuya_key);
    // tuya_sdk_start();
    // tuya_sdk_inited = true;
    //printf("tuya sdk init finish \n");

    while (1)
    { 
        if(is_online_tuya_cloud())
        {
            if (tuya_qrcode_str[0] == 0)
            {
                tuya_ipc_get_qrcode(NULL, tuya_qrcode_str, 64);
            }
            if(get_cur_layout() == pLAYOUT(home))
            {
                _tuya_api_weather_get(&weather);
            }
        }


        sleep(2);
    }

    ak_thread_exit();
    return NULL;
}
void standby_timeout_callback(void)
{
    printf("==========>>> standby timeout <<<=========== \n");
    goto_layout(pLAYOUT(standby));
}

bool tuya_wifi_sdk_init(const char *pid, bool wifi_enable)
{

    if (tuya_sdk_inited)
    {
        return false;
    }

    strcpy(tuya_pid, pid);
    wifi_work_restart();
    find_link_wifi();

    if (wifi_enable == false)
    {
        CloseConnectingWifi();
    }

    int tuya_status;
    if (access(IPC_APP_SD_BASE_PATH "tuya_index.txt", F_OK) == 0) // 量产烧录
    {
        tuya_status = tuya_key_and_uuid_init(true);
    }
    else
    {
        tuya_status = tuya_key_and_uuid_init(false); // 手动指定烧录
    }

    ak_pthread_t pthread_id;
    ak_thread_create(&pthread_id, tuya_wifi_sdk_init_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    ak_thread_detach(pthread_id);
    if (tuya_status == 0) // 需要注册
    {
        if (access(IPC_APP_SD_BASE_PATH "tuya_index.txt", F_OK) == 0)
        {
            FILE *fp = fopen(IPC_APP_SD_BASE_PATH "tuya_index.txt", "r+");
            if (fp == NULL)
            {
                perror("open fauled\n");
                exit(-1);
            }
            char index_buf[64];
            int index = 0;
            size_t ret;
            ret = fread(index_buf, 1, sizeof(index_buf), fp);
            if (ret < 0)
            {
                perror("read fauled\n");
                exit(-1);
            }
            else
            {
                index_buf[ret] = '\0';
            }

            index = atoi(index_buf);

            if (tuya_key_and_key_xls_register(index))
            {
                index++;
                sprintf(index_buf, "%d\r\n", index);
                fclose(fp);
                fp = NULL;
                fp = fopen(IPC_APP_SD_BASE_PATH "tuya_index.txt", "w+");
                fwrite(index_buf, strlen(index_buf), 1, fp);
                fclose(fp);
                tuya_uuid_and_key_read(tuya_uid, tuya_key);
                tuya_sdk_start();

                tuya_sdk_inited = true;
#if (POLISH == 0)
                goto_layout(pLAYOUT(home));
#endif
#if POLISH
                goto_layout(pLAYOUT(logo));
#endif
            }
            else
            {
                goto_layout(pLAYOUT(tuya_register));
            }
        }
        else
        {
            goto_layout(pLAYOUT(tuya_register));
        }
        return true;
    }

    else if (tuya_status == -1)
    {
        printf("tuya_key is no\n");
    }

    else if (tuya_status == 1)
    { // 已经注册了
        tuya_uuid_and_key_read(tuya_uid, tuya_key);
        tuya_sdk_start();
        tuya_sdk_inited = true;
        printf("tuya_conf is ok\n");
        // ak_pthread_t pthread_id;
        // ak_thread_create(&pthread_id, tuya_wifi_sdk_init_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
        // ak_thread_detach(pthread_id);
    }



    rtc_time_sync();
#if (POLISH == 0)
    goto_layout(pLAYOUT(home));
#endif
#if POLISH
    goto_layout(pLAYOUT(logo));
#endif

    standby_timer_open(60000, standby_timeout_callback);

    return true;
}

/***
** 日期:2022-06-18 08:05:12
** 作者: leo.liu
** 函数作用：获取天气信息
** 参数说明:
***/
OPERATE_RET http_gw_ipc_custom_msg(IN CONST CHAR_T *api_name, IN CONST CHAR_T *api_version, IN CONST CHAR_T *message, OUT cJSON **result);
static bool _tuya_api_weather_get(tuya_api_weather *nm)
{
    if (tuya_api_online_check() == false)
    {
        return false;
    }
    /* 预报 */
    const char *weather_choose[] = {
        "{  \
            \"codes\":[ \
            \"w.date.1\", \
            \"w.currdate\", \
            \"w.conditionNum\", \
            \"w.temp\", \
            \"w.humidity\", \
            \"w.pressure\", \
            \"w.pm10\", \
            \"w.pm25\", \
            \"w.aqi\", \
            \"w.thigh\", \
            \"w.tlow\", \
            \"c.city\", \
            ] \
            }"};
    // "{\"codes\":[\"w.conditionNum\", \"w.temp\", \"w.humidity\",\"w.pressure\", \"w.pm10\", \"w.pm25\", \"w.aqi\"]}"
    cJSON *result = NULL;
    if (http_gw_ipc_custom_msg("tuya.device.public.data.get", "1.0", *weather_choose, &result))
    {
        printf("================http_gw_ipc_custom_msg\n");
        return false;
    }
    cJSON *data_json = cJSON_GetArrayItem(result, 0);
    cJSON *condition_json = cJSON_GetObjectItem(data_json, "w.conditionNum");
    cJSON *temp_json = cJSON_GetObjectItem(data_json, "w.temp");
    cJSON *humidity_json = cJSON_GetObjectItem(data_json, "w.humidity");
    cJSON *pressure_json = cJSON_GetObjectItem(data_json, "w.pressure");
    cJSON *pm10_json = cJSON_GetObjectItem(data_json, "w.pm10");
    cJSON *pm25_json = cJSON_GetObjectItem(data_json, "w.pm25");
    cJSON *aqi_json = cJSON_GetObjectItem(data_json, "w.aqi");
    cJSON *thigh_json = cJSON_GetObjectItem(data_json, "w.thigh.0");
    cJSON *tlow_json = cJSON_GetObjectItem(data_json, "w.tlow.0");
    if (temp_json != NULL)
    {
        sscanf(cJSON_Print(temp_json), "%d", &(nm->temp));
    }
    if (condition_json != NULL)
    {
        sscanf(cJSON_Print(condition_json), "\"%d\"", &(nm->condition));
    }
    if (humidity_json != NULL)
    {
        sscanf(cJSON_Print(humidity_json), "%d", &(nm->humidity));
    }
    if (pressure_json != NULL)
    {
        sscanf(cJSON_Print(pressure_json), "%d", &(nm->pressure));
    }
    if (pm10_json != NULL)
    {
        sscanf(cJSON_Print(pm10_json), "%d", &(nm->pm10));
    }
    if (pm25_json != NULL)
    {
        sscanf(cJSON_Print(pm25_json), "%d", &(nm->pm25));
    }
    if (aqi_json != NULL)
    {
        sscanf(cJSON_Print(aqi_json), "%d", &(nm->aqi));
    }
    if (thigh_json != NULL)
    {
        sscanf(cJSON_Print(thigh_json), "%d", &(nm->thigh));
    }
    if (tlow_json != NULL)
    {
        sscanf(cJSON_Print(tlow_json), "%d", &(nm->tlow));
    }
    // printf("=============>>>>data_json====>%s\n", cJSON_Print(data_json));
    cJSON_Delete(result);
    return true;
}
