#include <stdio.h>
#include <unistd.h>

#include "sat_ipcamera.h"
#include "sat_user_common.h"



/**
 * @brief  
 * @date   2023-10-06 13:51:03
 * @author xiaole
 * @note
 * @param argv 传入的参数个数
 * @param argc argc[0] 脚本名 argc[1] 参数1 argc2 参数2 * @return 
 */
int main(int argv,char *argc[]){
    LOG_WHITE("username='%s', password='%s'\n", argc[1], argc[2]);

    char *username = argc[1];
    char *password = argc[2];

    if(argv == 1){
        LOG_YELLOW("Your login username and password have not been set\n");
        LOG_YELLOW("The default setting is \"admin\" \"123456\"\n");
        username = "admin";
        password = "123456";
    }



    sat_ipcamera_device_discover_search(0);

    while (sat_ipcamera_status_get())
    {
        // LOG_WHITE("busy...\n");
        usleep(1000*1000);
    }


    int total = sat_ipcamera_online_num_get();
    LOG_WHITE("online = %d\n", total);
    for (int i = 0; i < total; i++){        
        sat_ipcamera_user_password_set(i, username, password);
        if(sat_ipcamera_rtsp_url_get(i) == true){
            while (sat_ipcamera_status_get())
            {
                // LOG_WHITE("busy...\n");
                usleep(1000*1000);
            }

            int nstream = sat_ipcamera_profile_token_num_get(i);
            LOG_WHITE("token=%d\n", nstream);
            for (int s = 0; s < nstream; s++){
                LOG_WHITE("[%d]:%s\n", s, sat_ipcamera_rtsp_addr_get(i, s));
            }

        }
    
    }
    
}