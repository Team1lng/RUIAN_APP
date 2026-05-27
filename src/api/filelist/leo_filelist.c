#include "file_api.h"
#include "ak_mem.h"
#include <dirent.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stdio.h"





static int sd_mixed_total = 0;
static int sd_mixed_new_total = 0;
static media_info *p_sd_mixed;

static int sd_photo_total = 0;
static int sd_photo_new_total = 0;
static media_info *p_sd_photo;

static int sd_video_total = 0;
static int sd_video_new_total = 0;
static media_info *p_sd_video;

static int sd_message_total = 0;
static int sd_message_new_total = 0;
static media_info *p_sd_message;

static int 	sd_gallery_total = 0;
static int  sd_gallery_new_total = 0;
static media_info *p_sd_gallery;

static int flash_photo_total = 0;
static int flash_photo_new_total = 0;
static media_info *p_flash_photo;

static int sd_picture_total = 0;
static int sd_picture_new_total = 0;
static media_info *p_sd_picture;

static int sd_ring_total = 0;
static int sd_ring_new_total = 0;
static media_info *p_sd_ring;

static int sd_products_picture_total = 0;
static int sd_products_picture_new_total = 0;
static media_info *p_sd_products_picture;

//新增flash_logo结构
static int flash_logo_total = 0;
static int flash_logo_new_total = 0;
static media_info *p_flash_logo;

static char sd_card_mount = 0;
static char sd_card_inserted = 0;
static ak_mutex_t file_list_mutex;

#define VIDEO_DOT ".AVI"
#define PHOTO_DOT ".JPG"
#define IMG_DOT   ".jpg"
#define AUDIO_DOT ".pcm"
#define RING_DOT_LOWER ".mp3"
#define RING_DOT_UPPER ".MP3"
static int _video_bad_path_check(const char *file, char *damaged_file)
{
    char bad_file_path[MEDIA_PATH_MAX];
    sprintf(bad_file_path,"%stemp", file);
    if (access(bad_file_path, F_OK) == 0) 
		{
        if (damaged_file != NULL) {
            strcpy(damaged_file, bad_file_path);
        }
        return 1;
    }
    return 0;
}
static int _photo_bad_path_check(const char* file,char* badfile){

    sprintf(badfile,"%s",file);
    if(access(badfile,F_OK) == 0){
        return 0;
    }
    return 1;
}

static bool scan_find_file(media_type type, const char *dir_path, media_info *p_info, int *p_total, int *p_total_new)
{
    char cmd_buffer[64] = {0};
    if(type == FILE_TYPE_SD_PRODUCTS_PICTURE)
    {
        sprintf(cmd_buffer, "ls %s -l | awk '{print $9}' | sort -k1.1n  |awk '{print i$0}' i=`pwd`'/'", dir_path);

    }
    else
    {
        sprintf(cmd_buffer, "find %s -type f -maxdepth 1", dir_path);
    }


    FILE *pf = popen(cmd_buffer, "r");
    char buffer[128] = {0};
    while (fgets(buffer, 128, pf))
	{

        /**********************
        判断文件是否为规范
        **********************/
        buffer[strlen(buffer) - 1] = '\0';
        char *p_file = strrchr(buffer, '/');

        if ((p_file == NULL)|| strlen(p_file ++) > 64 || strlen(p_file) < 4)
		{
			printf("file name error: %s \n\r",p_file);
            goto fail_next;
        }

        /**************************
        判断是否为视频切换为坏文件
        ***************************/
        if (((type != FILE_TYPE_FLASH_PHOTO) && (type != FILE_TYPE_SD_GALLERY)) && (_video_bad_path_check(buffer, NULL) != 0)) {

			printf("unknown file type \n\r");
            goto fail_next;
        }


        //printf("--- %s %d ---\n\r",p_file,strlen(p_file));
        /******************
        获取该文件所属类型
        *******************/
        char *ptr = strrchr(buffer, '.');
        if ((type == FILE_TYPE_SD_MIXED_PHOTO) &&(strcmp(ptr, PHOTO_DOT) == 0))
		{
            p_info->type = FILE_TYPE_SD_MIXED_PHOTO;
        }
		else if ((type == FILE_TYPE_SD_MIXED_VIDEO) && (strcmp(ptr, VIDEO_DOT) == 0)) 
		{
            p_info->type = FILE_TYPE_SD_MIXED_VIDEO;
        } 
		else if ((type == FILE_TYPE_SD_PHOTO) && (strcmp(ptr, PHOTO_DOT) == 0))
		{
            p_info->type = FILE_TYPE_SD_PHOTO;
        }
		else if ((type == FILE_TYPE_FLASH_PHOTO) && (strcmp(ptr, PHOTO_DOT) == 0))
		{
			p_info->type = FILE_TYPE_FLASH_PHOTO;
		}else if ((type == FILE_TYPE_FLASH_LOGO) && (strcmp(ptr, IMG_DOT) == 0))
		{
			p_info->type = FILE_TYPE_FLASH_LOGO;
		}
		else if ((type == FILE_TYPE_SD_VIDEO) && (strcmp(ptr, VIDEO_DOT) == 0))
		{
            p_info->type = FILE_TYPE_SD_VIDEO;
        }
		else if ((type == FILE_TYPE_SD_AUDIO) && (strcmp(ptr, AUDIO_DOT) == 0)) 
		{
            p_info->type = FILE_TYPE_SD_AUDIO;
        }else if ((type == FILE_TYPE_SD_PICTURE) && ((strcmp(ptr, IMG_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
		{
            p_info->type = FILE_TYPE_SD_PICTURE;
        }else if ((type == FILE_TYPE_SD_RING) && ((strcmp(ptr, RING_DOT_LOWER) == 0) || (strcmp(ptr, RING_DOT_UPPER) == 0)))
		{
            p_info->type = FILE_TYPE_SD_RING;
        }else if ((type == FILE_TYPE_SD_PRODUCTS_PICTURE) && ((strcmp(ptr, IMG_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
		{
            p_info->type = FILE_TYPE_SD_PRODUCTS_PICTURE;
        }
		else
		{
            goto fail_next;
        }

        /***********
        获取该文件名
        ************/
        strncpy(p_info->file_name, p_file, strlen(p_file));

        p_info->ch = p_file[16] - 48;
        p_info->mode = p_file[17] - 48;
		
#if 0
		struct stat st;
        stat(buffer, &st);
        if (st.st_ctime == st.st_mtime) {
            p_info->is_new = 1;
            (*p_total_new)++;

        } else {
            p_info->is_new = 0;
        }
#endif
        p_info++;
        (*p_total)++;

        if ((type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && ((*p_total) > SD_MIXED_MAX)){
            break;
        }else if ((type == FILE_TYPE_SD_PHOTO) && ((*p_total) >= SD_PHOTO_MAX)){
            break;
        } if ((type == FILE_TYPE_SD_VIDEO) && ((*p_total) >= SD_VIDEO_MAX)){
            break;
        } else if ((type == FILE_TYPE_SD_AUDIO) && ((*p_total) > SD_AUDIO_MAX)){
            break;
        } else if ((type == FILE_TYPE_SD_GALLERY) && ((*p_total) > SD_GALLERY_MAX)){
            break;
        }else if ((type == FILE_TYPE_FLASH_PHOTO) && ((*p_total) > FLASH_PHOTO_MAX)){
            break;
        }else if ((type == FILE_TYPE_SD_PICTURE) && ((*p_total) > SD_PICTURE_MAX)){
            break;
        }else if ((type == FILE_TYPE_SD_RING) && ((*p_total) > SD_RING_MAX)){
            break;
        }else if ((type == FILE_TYPE_SD_PRODUCTS_PICTURE) && ((*p_total) > SD_PROJECTS_PICTURE_MAX)){
            break;
        }
fail_next:
        memset(buffer, 0, sizeof(buffer));
    }
    pclose(pf);
    return true;
}

#define SD_MIXED_CACHE_PATH SD_MIXED_PATH".config"
#define SD_PHOTO_CACHE_PATH SD_PHOTO_PATH".config"
#define SD_VIDEO_CACHE_PATH SD_VIDEO_PATH".config"
#define FALSH_PHOTO_CACHE_PATH FLASH_PHOTO_PATH".config"
#define FALSH_LOGO_CACHE_PATH FLASH_LOGO_PATH".config"



static void sd_media_file_load(media_type type, media_info *p_info, int *p_total, int *p_total_new, int max)
{
    FILE *fp = NULL;
    if(type == FILE_TYPE_SD_MIXED||type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
	{

       
        fp = fopen(SD_MIXED_CACHE_PATH, "rb");
        if (fp == NULL)
		{
       
            system("touch "SD_MIXED_CACHE_PATH);
            printf(SD_MIXED_CACHE_PATH" not exit \n\r");
            return ;
        }
    }
	else if (type == FILE_TYPE_SD_PHOTO) 
   	{
        fp = fopen(SD_PHOTO_CACHE_PATH, "rb");
        if (fp == NULL) {
			system("touch "SD_PHOTO_CACHE_PATH);
            printf(SD_PHOTO_CACHE_PATH" not exit \n\r");
            return ;
        }
    }
	else if(type == FILE_TYPE_SD_VIDEO)
	{
        fp = fopen(SD_VIDEO_CACHE_PATH, "rb");
        if (fp == NULL) {
			system("touch "SD_VIDEO_CACHE_PATH);
            printf(SD_VIDEO_CACHE_PATH" not exit \n\r");
            return ;
        }
    }
	else if(type == FILE_TYPE_FLASH_PHOTO)
	{
        fp = fopen(FALSH_PHOTO_CACHE_PATH, "rb");
        if (fp == NULL)
		{
			system("touch "FALSH_PHOTO_CACHE_PATH);
            printf(FALSH_PHOTO_CACHE_PATH" not exit \n\r");
            return ;
        }
    }	else if(type == FILE_TYPE_FLASH_LOGO)
	{
        fp = fopen(FALSH_LOGO_CACHE_PATH, "rb");
        if (fp == NULL)
		{
			system("touch "FALSH_LOGO_CACHE_PATH);
            printf(FALSH_LOGO_CACHE_PATH" not exit \n\r");
            return ;
        }
    }else
	{
		return ;
	}

    int read_len = 0;
    media_info info;
    int read_size = sizeof(media_info);
    while ((read_len = fread(&info, 1, read_size, fp)) == read_size)
	{
        *p_info = info;
        (*p_total)++;
        if (info.is_new) {
            (*p_total_new)++;
        }
        if ((*p_total) >= max) 
		{
            break;
        }
		//printf("lock :%s\n",info.is_lock?"ture":"false");
        p_info++;
    }
    fclose(fp);
    printf("media total:%d \n\r", (*p_total));
    
}

static bool sd_file_sync(media_type type)
{
    FILE *fp;
    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
	{
        fp = fopen(SD_MIXED_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(SD_MIXED_PATH"not exit \n\r");
            return false;
        }
        fwrite(p_sd_mixed, sd_mixed_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
        // system("fsync -d /mnt/tf/media/");
    }
	else if (type == FILE_TYPE_SD_PHOTO)
	{
        fp = fopen(SD_PHOTO_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(SD_PHOTO_CACHE_PATH"not exit \n\r");
            return false;
        }
        fwrite(p_sd_photo, sd_photo_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
	else if (type == FILE_TYPE_FLASH_PHOTO)
	{
        fp = fopen(FALSH_PHOTO_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(FALSH_PHOTO_CACHE_PATH"not exit \n\r");
            return false;
        }
        fwrite(p_flash_photo, flash_photo_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }else if (type == FILE_TYPE_FLASH_LOGO)
	{
        fp = fopen(FALSH_LOGO_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(FALSH_LOGO_CACHE_PATH"not exit \n\r");
            return false;
        }
        fwrite(p_flash_logo, flash_logo_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
	else 
	{
        fp = fopen(SD_VIDEO_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(SD_VIDEO_CACHE_PATH"not exit \n\r");
            return false;
        }
        fwrite(p_sd_video, sd_video_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    return true;
}

static int scan_media_file(media_type type)
{
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && (access(SD_MIXED_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_MIXED_PATH);
        return -1;
    } else if ((type == FILE_TYPE_SD_PHOTO) && (access(SD_PHOTO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_PHOTO_PATH);
        return -1;
    } else if ((type == FILE_TYPE_SD_VIDEO) && (access(SD_VIDEO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_VIDEO_PATH);
        return -1;
    }else if ((type == FILE_TYPE_SD_AUDIO) && (access(SD_AUDIO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_AUDIO_PATH);
        return -1;
    } else if ((type == FILE_TYPE_SD_GALLERY) && (access(SD_GALLERY_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_GALLERY_PATH);
        return -1;
      }else if ((type == FILE_TYPE_FLASH_PHOTO) && (access(FLASH_PHOTO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", FLASH_PHOTO_PATH);
        system("mkdir "FLASH_PHOTO_PATH);
        return -1;

      }else if ((type == FILE_TYPE_FLASH_LOGO) && (access(FLASH_LOGO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", FLASH_LOGO_PATH);
        system("mkdir "FLASH_LOGO_PATH);
        return -1;

      }else if ((type == FILE_TYPE_SD_PICTURE) && (access(SD_PICTURE_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_PICTURE_PATH);

        return -1;

      }else if ((type == FILE_TYPE_SD_RING) && (access(SD_RING_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_RING_PATH);
        return -1;
      }else if ((type == FILE_TYPE_SD_PRODUCTS_PICTURE) && (access(SD_PRODUCTS_PICTURE_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_PRODUCTS_PICTURE_PATH);
        return -1;
      }
      else if (type >= FILE_TYPE_NONE) {

        printf("unknown file type \n\r");
        return -1;
    }


    char *dir_path = NULL;
    media_info *p_array = NULL;
    int *p_total = NULL;
    int *p_new_total = NULL;

	
	printf("scan_media_file type: %d \n\r",type);
	if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) {

	    dir_path = SD_MIXED_PATH;
	    sd_mixed_total = sd_mixed_new_total = 0;
	    p_array = p_sd_mixed;
	    p_total = &sd_mixed_total;
	    p_new_total = &sd_mixed_new_total;
	    sd_media_file_load(type, p_array, p_total, p_new_total, SD_MIXED_MAX);
	}else if (type == FILE_TYPE_SD_PHOTO) {

        dir_path = SD_PHOTO_PATH;
        sd_photo_total = sd_photo_new_total = 0;
        p_array = p_sd_photo;
        p_total = &sd_photo_total;
        p_new_total = &sd_photo_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total,SD_PHOTO_MAX);
    } else if (type == FILE_TYPE_SD_VIDEO) {

	    dir_path = SD_VIDEO_PATH;
        sd_video_total = sd_video_new_total = 0;
        p_array = p_sd_video;
        p_total = &sd_video_total;
        p_new_total = &sd_video_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total,SD_VIDEO_MAX);
    }else if (type == FILE_TYPE_SD_AUDIO) {

        dir_path = SD_AUDIO_PATH;
        sd_message_total = sd_message_new_total = 0;
        p_array = p_sd_message;
        p_total = &sd_message_total;
        p_new_total = &sd_message_new_total;
        scan_find_file(type, dir_path, p_array, p_total, p_new_total);
    } else if (type == FILE_TYPE_SD_GALLERY) {

        dir_path = SD_GALLERY_PATH;
        sd_gallery_total = sd_gallery_new_total = 0;
        p_array = p_sd_gallery;
        p_total = &sd_gallery_total;
        p_new_total = &sd_gallery_new_total;
        scan_find_file(type, dir_path, p_array, p_total, p_new_total);
     }else if (type == FILE_TYPE_FLASH_PHOTO) {
         dir_path = FLASH_PHOTO_PATH;
         flash_photo_total = flash_photo_new_total = 0;
         p_array = p_flash_photo;
         p_total = &flash_photo_total;
         p_new_total = &flash_photo_new_total;
		 sd_media_file_load(type, p_array, p_total, p_new_total,FLASH_PHOTO_MAX);
		 system("sync");
       //  scan_find_file(type, dir_path, p_array, p_total, p_new_total);
     }else if (type == FILE_TYPE_FLASH_LOGO) {
         dir_path = FLASH_LOGO_PATH;
         flash_logo_total = flash_logo_new_total = 0;
         p_array = p_flash_logo;
         p_total = &flash_logo_total;
         p_new_total = &flash_logo_new_total;
		//  sd_media_file_load(type, p_array, p_total, p_new_total,FLASH_PHOTO_MAX);
         		 scan_find_file(type,dir_path, p_array, p_total, p_new_total);
		//  scan_find_file(type,dir_path, p_array, p_total, p_new_total);
		 system("sync");
       //  scan_find_file(type, dir_path, p_array, p_total, p_new_total);
     }else if (type == FILE_TYPE_SD_PICTURE) {
         dir_path = SD_PICTURE_PATH;
         sd_picture_total = sd_picture_new_total = 0;
         p_array = p_sd_picture;
         p_total = &sd_picture_total;
         p_new_total = &sd_picture_new_total;
		 scan_find_file(type,dir_path, p_array, p_total, p_new_total);
     }else if (type == FILE_TYPE_SD_RING) {
        dir_path = SD_RING_PATH;
        sd_ring_total = sd_ring_new_total = 0;
        p_array = p_sd_ring;
        p_total = &sd_ring_total;
        p_new_total = &sd_ring_new_total;
        scan_find_file(type,dir_path, p_array, p_total, p_new_total);
     }else if (type == FILE_TYPE_SD_PRODUCTS_PICTURE) {
        dir_path = SD_PRODUCTS_PICTURE_PATH;
        sd_products_picture_total = sd_products_picture_new_total = 0;
        p_array = p_sd_products_picture;
        p_total = &sd_products_picture_total;
        p_new_total = &sd_products_picture_new_total;
        scan_find_file(type,dir_path, p_array, p_total, p_new_total);
     }



    return 0;
}

bool format_sd_flag = false;
static void sd_format_process(void)
{
    //system("mkfs.vfat /dev/mmcblk0");

    system("cd /");
    system("rm -rf /mnt/tf/*");

    system("umount "SD_BASE_PATH);
    if(access("/dev/mmcblk0p1", F_OK) == 0)
    {
        system("mkdosfs -F 32  -n VDP /dev/mmcblk0p1");
    }
    else
    {
        system("mkdosfs -F 32  -n VDP /dev/mmcblk0");
    }
    

    if (access(SD_BASE_PATH, F_OK) != 0) {

        system("mkdir "SD_BASE_PATH);
    } else {
        if(access("/dev/mmcblk0p1", F_OK) == 0)
        {
            system("mount /dev/mmcblk0p1 "SD_BASE_PATH);
        }
        else
        {
            system("mount /dev/mmcblk0 "SD_BASE_PATH);
        }
    }
    if(MIX_PHOTOS_AND_VIDEOS_FILE){
        if (access(SD_MIXED_PATH, F_OK) != 0) {

            system("mkdir "SD_MIXED_PATH);
        }
    }else{
        if (access(SD_PHOTO_PATH, F_OK) != 0) {

            system("mkdir "SD_PHOTO_PATH);
        }
        if (access(SD_VIDEO_PATH, F_OK) != 0) {

            system("mkdir "SD_VIDEO_PATH);
        }
    }
    if ((access(SD_AUDIO_PATH, F_OK) != 0) && AUDIO_MESSAGE_FILE_ENABLE) {
        system("mkdir "SD_AUDIO_PATH);
    }
    if ((access(SD_GALLERY_PATH, F_OK) != 0) && GALLERY_IMG_FILE_ENABLE) {
        system("mkdir "SD_GALLERY_PATH);
    }

    system("sync");
	
	//ak_sleep_ms(50);
	//system("umount "SD_BASE_PATH);
	//ak_sleep_ms(50);
	///system("mount /dev/mmcblk0 "SD_BASE_PATH);
}

bool copy_to_sd_flag = false;
static void copy_to_sd_process(void)
{
    if (access(SD_BACKUP_PATH, F_OK) != 0) {

        char buf[128] = {0};
        sprintf(buf, "mkdir %s", SD_BACKUP_PATH);
        system(buf);
        ak_sleep_ms(20);
        system("sync");
    }
    system("mv "FLASH_PHOTO_PATH"*.JPG "SD_BACKUP_PATH);
    ak_sleep_ms(20);
    system("sync");
    ak_sleep_ms(20);
}

char delete_file_flag = 0;
static void delete_file_process(void)
{
    if (delete_file_flag & DELETE_ALL_SD_PHOTO) { ///删除所有的sd卡内的photo

        if (access(SD_PHOTO_PATH, F_OK) == 0) {

			system("rm "SD_PHOTO_CACHE_PATH);
		
			printf("===>>>>%s/n",SD_PHOTO_PATH"*JPG");
			
            system("rm -rf "SD_PHOTO_PATH"*JPG");
			
			

            scan_media_file(FILE_TYPE_SD_PHOTO);//查看文件夹路径是否存在
            printf("\033[31m delete sd photo \n\r");
        }
    }

    if (delete_file_flag & DELETE_ALL_SD_VIDEO) { ///video

        if (access(SD_VIDEO_PATH, F_OK) == 0) {
			
			printf("====>>>delet sd video .config!!!!\n");
			
			system("rm "SD_VIDEO_CACHE_PATH);
			printf("====>>>delet all sd video!!!!! \n");
			
            system("rm -rf "SD_VIDEO_PATH"*AVI*");
			
			printf("====>>>delet sd photo over!!!!! \n");

            scan_media_file(FILE_TYPE_SD_VIDEO);

            printf("\033[31m delete sd video \n\r");
        }

    }
    if(MIX_PHOTOS_AND_VIDEOS_FILE){
        if (delete_file_flag & DELETE_ALL_MIXED) { ///mixed

            if (access(SD_MIXED_PATH, F_OK) == 0) {
				
				printf("====>>>delet all mix .config!!!!!\n");

                system("rm -rf "SD_MIXED_CACHE_PATH);

				printf("====>>>delet all mix !!!!! \n");
				
                system("rm -rf "SD_MIXED_PATH"*.*");

				
				printf("====>>>delet all mix over!!!!! \n");
                scan_media_file(FILE_TYPE_SD_MIXED);
                printf("\033[31m delete sd mixed file \n\r");
            }
        }

        if(delete_file_flag & DELETE_ALL_MIXED_PHOTO){
            for(int i= 0;i < sd_mixed_total;i++){
                media_info * media = media_info_get(FILE_TYPE_SD_MIXED,i);
                if(media->type == FILE_TYPE_SD_MIXED_PHOTO)
                {
                	printf("====>>>delet all mix photo !!!!! \n");
                    media_file_delete(FILE_TYPE_SD_MIXED, i);
                    
                    i--;
                }
            }
			printf("====>>>delet all mix photo over !!!!! \n");
        }

        if(delete_file_flag & DELETE_ALL_MIXED_VIDEO){
            for(int i= 0;i < sd_mixed_total;i++){
                media_info * media = media_info_get(FILE_TYPE_SD_MIXED,i);
                if(media->type == FILE_TYPE_SD_MIXED_VIDEO)
                {
                	printf("====>>>delet all mix video !!!!! \n");
                    media_file_delete(FILE_TYPE_SD_MIXED, i);
                    
                    i--;
                }
            }
			printf("====>>>delet all mix video over !!!!! \n");
        }
        scan_media_file(FILE_TYPE_SD_MIXED);
    }

    if (delete_file_flag & DELETE_ALL_FLASH_PHOTO) { ///photo

        if (access(FLASH_PHOTO_PATH, F_OK) == 0) {
			printf("====>>>delet falsh photo photo!!!!!\n");
			
            system("rm -rf "FLASH_PHOTO_PATH"*JPG");
			system("rm -rf "FLASH_PHOTO_PATH"*jpg");

			printf("====>>>delet falsh photo .config!!!!!\n");

			system("rm "FALSH_PHOTO_CACHE_PATH);
			
			printf("====>>>delet falsh photo over!!!!!\n");
            scan_media_file(FILE_TYPE_FLASH_PHOTO);
			

            printf("\033[31m delete flash photo \n\r");
        }
    }

    if (delete_file_flag & DELETE_ALL_MESSAGE) { ///message
        if (access(SD_AUDIO_PATH, F_OK) == 0) {
			printf("====>>>delet all SD_AUDIO meg!!!!!\n");
			
            system("rm -rf "SD_AUDIO_PATH"*pcm");
		

            scan_media_file(FILE_TYPE_SD_AUDIO);
			printf("====>>>delet all SD_AUDIO meg over!!!!!\n");

            printf("\033[31m delete sd message \n\r");
        }
    }
    if (delete_file_flag & DELETE_ALL_GALLERY) {
        if (access(SD_GALLERY_PATH, F_OK) == 0) {
			printf("====>>>delet all SD_GALLERY meg!!!!!\n");
			
            system("rm -rf "SD_GALLERY_PATH"*pcm");
		printf("====>>>delet all GALLERY_PATH meg over!!!!!\n");

            scan_media_file(FILE_TYPE_SD_GALLERY);

            printf("\033[31m delete sd message \n\r");
        }
    }

    if (delete_file_flag) {
        delete_file_flag = 0;
        ak_sleep_ms(20);
        system("sync");
    }
}

bool get_system_cmd_return_result()
{
    const char * result = SD_BASE_PATH" is a mountpoint";
    bool value = false;
    FILE *fp;
    char buffer[64];
    fp = popen("mountpoint /mnt/tf", "r");
    fgets(buffer, sizeof(buffer), fp);
    printf("%s", buffer);
    if(strncmp(buffer,result,strlen(result)) == 0)
    {
        value = true;
    }
    pclose(fp);
    return value;
}
static void *file_list_task(void *arg)
{
    char cur_insert = 0;
    while (1) {

        cur_insert = (access("/dev/mmcblk0", F_OK) == 0) ? 1 : 0;
        if (cur_insert != sd_card_inserted)
		{
            ak_thread_mutex_lock(&file_list_mutex);
            sd_card_inserted = cur_insert;
            sd_card_mount = cur_insert;
            if (sd_card_inserted) {

                if (access(SD_BASE_PATH, F_OK) != 0) {

					system("mkdir "SD_BASE_PATH);
                }
                system("umount "SD_BASE_PATH);
				if(access("/dev/mmcblk0p1", F_OK) == 0)
                {
                    system("mount /dev/mmcblk0p1 "SD_BASE_PATH);
                }
                else
                {
                    system("mount /dev/mmcblk0 "SD_BASE_PATH);
                }

                sd_card_mount =  get_system_cmd_return_result();
                if(sd_card_mount)
                {
                    if(MIX_PHOTOS_AND_VIDEOS_FILE)
                    {
                        if (access(SD_MIXED_PATH, F_OK) != 0) {

                            system("mkdir "SD_MIXED_PATH);
                        }
                        scan_media_file(FILE_TYPE_SD_MIXED);
                    }
                    else
                    {
                        if (access(SD_PHOTO_PATH, F_OK) != 0) {

                            system("mkdir "SD_PHOTO_PATH);
                        }
                        if (access(SD_VIDEO_PATH, F_OK) != 0) {

                            system("mkdir "SD_VIDEO_PATH);
                        }
                        scan_media_file(FILE_TYPE_SD_PHOTO);
                        scan_media_file(FILE_TYPE_SD_VIDEO);
                    }

                    if ((access(SD_AUDIO_PATH, F_OK) != 0) && AUDIO_MESSAGE_FILE_ENABLE) 
                    {

                        system("mkdir "SD_AUDIO_PATH);
                        scan_media_file(FILE_TYPE_SD_AUDIO);
                    }
                    if ((access(SD_GALLERY_PATH, F_OK) != 0) && GALLERY_IMG_FILE_ENABLE)
                    {

                        system("mkdir "SD_GALLERY_PATH);
                        scan_media_file(FILE_TYPE_SD_GALLERY);
                    }
                    if ((access(SD_PICTURE_PATH, F_OK) == 0) && BG_PICTURE_FILE_ENABLE)
                    {
                        scan_media_file(FILE_TYPE_SD_PICTURE);
                    }
                    if ((access(SD_RING_PATH, F_OK) == 0) && DOOR_RING_FILE_ENABLE)
                    {
                        scan_media_file(FILE_TYPE_SD_RING);
                    }
                    if ((access(SD_PRODUCTS_PICTURE_PATH, F_OK) == 0) && DOOR_RING_FILE_ENABLE)
                    {
                        scan_media_file(FILE_TYPE_SD_PRODUCTS_PICTURE);
                    }
                }

            }
			
			extern bool sdcard_status_change_push(char,char);
			sdcard_status_change_push(0x01,0x00);
			
            ak_thread_mutex_unlock(&file_list_mutex);
        }
        if (format_sd_flag)
		{
            sd_format_process();
            if(MIX_PHOTOS_AND_VIDEOS_FILE)
			{
                scan_media_file(FILE_TYPE_SD_MIXED);
            }
			else
			{
                scan_media_file(FILE_TYPE_SD_PHOTO);
                scan_media_file(FILE_TYPE_SD_VIDEO);
            }
            if(AUDIO_MESSAGE_FILE_ENABLE)
			{
                scan_media_file(FILE_TYPE_SD_AUDIO);
            }
            if(GALLERY_IMG_FILE_ENABLE)
			{
                scan_media_file(FILE_TYPE_SD_GALLERY);
            }

            format_sd_flag = false;
        }
        if (copy_to_sd_flag)
		{
            printf("----------------- 123\n\r");
            copy_to_sd_process();
            scan_media_file(FILE_TYPE_FLASH_PHOTO);
            copy_to_sd_flag = false;
        }
        if (delete_file_flag)
		{
            delete_file_process();
        }
        ak_sleep_ms(50);
    }
    ak_thread_exit();
    return NULL;
}

void start_format_sd_card(void){
    format_sd_flag = true;
}
bool format_sd_card_status(void){
    return format_sd_flag;
}
void start_copy_flash_photo_to_sd(void){
    copy_to_sd_flag = true;
}
bool copy_flash_photo_to_sd_status(void){
    return copy_to_sd_flag;
}
void start_delete_media(enum delete_flag item){
    delete_file_flag = item;
}
char delete_media_status(void){
    return delete_file_flag;
}



void media_file_list_init(void)
{
    ak_thread_mutex_init(&file_list_mutex, NULL);
    p_flash_photo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+FLASH_PHOTO_MAX));
    p_flash_logo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+FLASH_LOGO_MAX));
    scan_media_file(FILE_TYPE_FLASH_PHOTO);
    scan_media_file(FILE_TYPE_FLASH_LOGO);
    if(MIX_PHOTOS_AND_VIDEOS_FILE)
	{
        p_sd_mixed = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_MIXED_MAX));
        scan_media_file(FILE_TYPE_SD_MIXED);
    }
	else
	{
        p_sd_video = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_VIDEO_MAX));
        p_sd_photo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_PHOTO_MAX));
        scan_media_file(FILE_TYPE_SD_PHOTO);
        scan_media_file(FILE_TYPE_SD_VIDEO);
    }
    if(AUDIO_MESSAGE_FILE_ENABLE)
	{
        p_sd_message = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_AUDIO_MAX));
        scan_media_file(FILE_TYPE_SD_AUDIO);
    }
    if(GALLERY_IMG_FILE_ENABLE){
        p_sd_gallery = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_GALLERY_MAX));
        scan_media_file(FILE_TYPE_SD_GALLERY);
    }
    if(BG_PICTURE_FILE_ENABLE){
        p_sd_picture = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_PICTURE_MAX));
        scan_media_file(FILE_TYPE_SD_PICTURE);
    }
    if(DOOR_RING_FILE_ENABLE){
        p_sd_ring = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_RING_MAX));
        scan_media_file(FILE_TYPE_SD_RING);
    }
    if(PRODUCTS_PICTURE_FILE_ENABLE){
        p_sd_products_picture = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1+SD_PROJECTS_PICTURE_MAX));
        scan_media_file(FILE_TYPE_SD_PRODUCTS_PICTURE);
    }
    ak_pthread_t task_id;
    ak_thread_create(&task_id, file_list_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
}




bool is_sdcard_insert(void)
{
    ak_thread_mutex_lock(&file_list_mutex);
    char insert = sd_card_mount;
    ak_thread_mutex_unlock(&file_list_mutex);
    return insert ? true : false;
}



bool create_one_media_file(media_type type, char ch, char mode, char *path)
{
    if ((type != FILE_TYPE_FLASH_PHOTO) && ((is_sdcard_insert() == 0) || format_sd_card_status())) //SD卡没插入并且选择保存到SD卡中
	{
        return false;
    }
    media_info *p_array = NULL;
    char *file_path = NULL;
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && MIX_PHOTOS_AND_VIDEOS_FILE) {
      
        if (sd_mixed_total >=  SD_MIXED_MAX) 
		{	
			int i = 0;
            int total = sd_mixed_total;
			for(i = 0 ; i < total; i++)
			{
				if(p_sd_mixed[i].is_lock == false)
				{
					media_file_delete(type, i);
                    
                    break;
				}
			}
			if(i == total)//全部上锁，无法删除最旧一张
			{
				return false;
			}			
        }
            file_path = SD_MIXED_PATH;
            p_array = &p_sd_mixed[sd_mixed_total];
            sd_mixed_total++;
            sd_mixed_new_total++;
        }else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (sd_video_total >= SD_VIDEO_MAX)
		{
            int total=sd_video_total;
			int i = 0;
			for(i = 0 ; i < total; i++)
			{
				if(p_sd_video[i].is_lock == false)
				{
					media_file_delete(type, i);
                    
				}
                break;
			}
			if(i == total)
			{
				return false;
			}
        }
        file_path = SD_VIDEO_PATH;
        p_array = &p_sd_video[sd_video_total];
        sd_video_total++;
        sd_video_new_total++;
    } else if ((type == FILE_TYPE_SD_PHOTO)&& (!MIX_PHOTOS_AND_VIDEOS_FILE)) {


        if (sd_photo_total >=SD_VIDEO_MAX) 
		{
           int i = 0;
           int total=sd_photo_total;
			for(i = 0 ; i < total; i++)
			{
				if(p_sd_photo[i].is_lock == false)
				{
					media_file_delete(type, i);
                    
                    break;
				}
			}
			if(i == total)
			{
				return false;
			}
        }
        file_path = SD_PHOTO_PATH;
        p_array = &p_sd_photo[sd_photo_total];
        sd_photo_total++;
        sd_photo_new_total++;
    } else if ((type == FILE_TYPE_SD_AUDIO) && AUDIO_MESSAGE_FILE_ENABLE) {

        file_path = SD_AUDIO_PATH;
        p_array = &p_sd_message[sd_message_total];
        sd_message_total++;
        sd_message_new_total++;
        if (sd_message_total > SD_AUDIO_MAX) 
		{
            media_file_delete(type, 0);
            
        }
    } else if ((type == FILE_TYPE_SD_GALLERY) && GALLERY_IMG_FILE_ENABLE) {

        file_path = SD_GALLERY_PATH;
        p_array = &p_sd_gallery[sd_gallery_total];
        sd_gallery_total++;
        sd_gallery_new_total++;
        if (sd_gallery_total > SD_GALLERY_MAX) {
            media_file_delete(type, 0);
            
        }
    }
	else if (type == FILE_TYPE_FLASH_PHOTO)
   	{
        if (flash_photo_total >= FLASH_PHOTO_MAX)
		{
			int i = 0;
            int total=flash_photo_total;
			for( i = 0 ; i < total; i++)
			{
				if(p_flash_photo[i].is_lock == false)
				{
					media_file_delete(type, i);
                    
				}
                break;
			}
			if(i == total)
			{
				return false;
			}
        } 
        file_path = FLASH_PHOTO_PATH;
        p_array = &p_flash_photo[flash_photo_total];
        flash_photo_total++;
        flash_photo_new_total++;      
    }else if (type == FILE_TYPE_FLASH_LOGO)
   	{
        if (flash_logo_total >= FLASH_PHOTO_MAX)
		{
			int i = 0;
            int total=flash_logo_total;
			for( i = 0 ; i < total; i++)
			{
				if(p_flash_logo[i].is_lock == false)
				{
					media_file_delete(type, i);
                    
				}
                break;
			}
			if(i == total)
			{
				return false;
			}
        } 
        file_path = FLASH_LOGO_PATH;
        p_array = &p_flash_logo[flash_logo_total];
        flash_logo_total++;
        flash_logo_new_total++;      
    }else {
        printf("No this type file.\n\r");
        return false;
    }

    p_array->ch = ch;
    p_array->is_new = 1;
    p_array->mode = mode;
    p_array->type = type;
	p_array->is_lock = 0;
    struct ak_date date;
    ak_get_localdate(&date);
    do {

        if (type == FILE_TYPE_SD_AUDIO) {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, AUDIO_DOT);
        } else if (type == FILE_TYPE_SD_VIDEO || type == FILE_TYPE_SD_MIXED_VIDEO) {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, VIDEO_DOT);
        } else if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_FLASH_PHOTO) {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, PHOTO_DOT);
        }

        strcpy(path, file_path);
        strcat(path, p_array->file_name);

        date.second++;
        date.second %= 60;
        printf("%s exist\n\r",path);
    } while (access(path, F_OK) == 0);

    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_VIDEO || type == FILE_TYPE_SD_MIXED_PHOTO ||type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO||type ==FILE_TYPE_FLASH_PHOTO) {
        printf("----------------FILE_TYPE_SD_MIXED:%d\n\r ",type);
        if(sd_file_sync(type) == false)
        {
            return false;
        }

    }
    return true;
}


media_info *media_info_get(media_type type, int index)
{
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
                && MIX_PHOTOS_AND_VIDEOS_FILE) {
        if (index >= sd_mixed_total) {
            printf("====sd_mixed_total===%d\n",sd_mixed_total);
            printf("====index===%d\n",index);
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_mixed[index];
    }else if (type == FILE_TYPE_FLASH_PHOTO) {
        if (index >= flash_photo_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_flash_photo[index];
    }else if (type == FILE_TYPE_FLASH_LOGO) {
        if (index >= flash_logo_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_flash_logo[index];
    }else if ((type == FILE_TYPE_SD_PHOTO) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_photo_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_photo[index];
    } else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_video_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_video[index];
    } else if ((type == FILE_TYPE_SD_AUDIO)&& AUDIO_MESSAGE_FILE_ENABLE) {

        if (index >= sd_message_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_message[index];
    } else if ((type == FILE_TYPE_SD_GALLERY) && GALLERY_IMG_FILE_ENABLE) {

        if (index >= sd_gallery_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_gallery[index];
    }else if ((type == FILE_TYPE_SD_PICTURE) && BG_PICTURE_FILE_ENABLE) {

        if (index >= sd_picture_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_picture[index];
    }else if ((type == FILE_TYPE_SD_RING) && (DOOR_RING_FILE_ENABLE)) {

        if (index >= sd_ring_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_ring[index];
    }else if ((type == FILE_TYPE_SD_PRODUCTS_PICTURE) && (PRODUCTS_PICTURE_FILE_ENABLE)) {

        if (index >= sd_products_picture_total) {
            printf("get info error %d\n\r",__LINE__);
            return NULL;
        }
        return &p_sd_products_picture[index];
    }
    printf("type error ! \n\r");
    return NULL;
}


int media_file_total_get(media_type type, char is_new)
{

    if (type == FILE_TYPE_FLASH_PHOTO) {

        return is_new ? flash_photo_new_total : flash_photo_total;

    }else if (type == FILE_TYPE_FLASH_LOGO) {
        
        return is_new ? flash_logo_new_total : flash_logo_total;
        } else {

        if (is_sdcard_insert() == 0) {

            return -1;
        }
        if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) {

            return is_new ? sd_mixed_new_total : sd_mixed_total;
        }else if (type == FILE_TYPE_SD_PHOTO) {

            return is_new ? sd_photo_new_total : sd_photo_total;
        } else if (type == FILE_TYPE_SD_VIDEO) {

            return is_new ? sd_video_new_total : sd_video_total;
        } else if (type == FILE_TYPE_SD_AUDIO) {

            return is_new ? sd_message_new_total : sd_message_total;
        } else if (type == FILE_TYPE_SD_GALLERY) {

            return is_new ? sd_gallery_new_total : sd_gallery_total;
        }else if (type == FILE_TYPE_SD_PICTURE) {
            return is_new ? sd_picture_new_total : sd_picture_total;
        }else if (type == FILE_TYPE_SD_RING) {
            return is_new ? sd_ring_new_total : sd_ring_total;
        }else if (type == FILE_TYPE_SD_PRODUCTS_PICTURE) {
            return is_new ? sd_products_picture_new_total : sd_products_picture_total;
        }

    }

    printf("type error ! \n\r");
    return 0;
}
int media_file_lock_set(media_type type, int index,bool en)
{
	media_info *info  = NULL;
    if (type == FILE_TYPE_FLASH_PHOTO)
	{
        if (index >= flash_photo_total) {
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_flash_photo[index];

    }
	else if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)&& MIX_PHOTOS_AND_VIDEOS_FILE)
	{
        if (index >= sd_mixed_total) 
		{
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_mixed[index];
       
    }
	else if ((type == FILE_TYPE_SD_PHOTO) &&(!MIX_PHOTOS_AND_VIDEOS_FILE))
	{
        if (index >= sd_photo_total)
		{
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_photo[index];
    } 
	else if ((type == FILE_TYPE_SD_VIDEO) &&(!MIX_PHOTOS_AND_VIDEOS_FILE))
	{
        if (index >= sd_video_total)
		{
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_video[index];
    } 
	else if ((type == FILE_TYPE_SD_AUDIO) && AUDIO_MESSAGE_FILE_ENABLE)
	{
        if (index >= sd_message_total) 
		{
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_message[index];
    }
	else if ((type == FILE_TYPE_SD_GALLERY) && AUDIO_MESSAGE_FILE_ENABLE) 
	{
        if (index >= sd_gallery_total) 
		{
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_gallery[index];
    }
	else
	{
        printf("get info error type \n\r");
        return -1;
    }

	if(info->is_lock == en)
	{
		return 0;
	}

    info->is_lock = en;
	
    if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO|| type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO||type == FILE_TYPE_SD_MIXED||type ==FILE_TYPE_FLASH_PHOTO)
	{
        sd_file_sync(type);
		
    }
    return 0;
}


int media_file_new_clear(media_type type, int index)
{

    media_info *info  = NULL;
    //char *path = NULL;

    if (type == FILE_TYPE_FLASH_PHOTO) {

        if (index >= flash_photo_total) {
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_flash_photo[index];
     //   path = FLASH_PHOTO_PATH;
        if ((info->is_new) && (flash_photo_new_total > 0)) {

            flash_photo_new_total--;
        }

    }else if (type == FILE_TYPE_FLASH_LOGO) {

        if (index >= flash_logo_total) {
            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_flash_logo[index];
     //   path = FLASH_PHOTO_PATH;
        if ((info->is_new) && (flash_logo_new_total > 0)) {

            flash_logo_new_total--;
        }

    } else if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)&& MIX_PHOTOS_AND_VIDEOS_FILE) {

        if (index >= sd_mixed_total) {

            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_mixed[index];
      //  path = SD_MIXED_PATH;
        if ((info->is_new) && (sd_mixed_new_total > 0)) {
            sd_mixed_new_total--;
        }
    }else if ((type == FILE_TYPE_SD_PHOTO) &&(!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_photo_total) {

            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_photo[index];
      //  path = SD_PHOTO_PATH;
        if ((info->is_new) && (sd_photo_new_total > 0)) {
            sd_photo_new_total--;
        }
    } else if ((type == FILE_TYPE_SD_VIDEO) &&(!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_video_total) {

            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_video[index];
        //path = SD_VIDEO_PATH;
        if ((info->is_new) && (sd_video_new_total > 0)) {
            sd_video_new_total--;
        }
    } else if ((type == FILE_TYPE_SD_AUDIO) && AUDIO_MESSAGE_FILE_ENABLE) {

        if (index >= sd_message_total) {

            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_message[index];
       // path = SD_AUDIO_PATH;
        if ((info->is_new) && (sd_message_new_total > 0)) {
            sd_message_new_total--;
        }
    } else if ((type == FILE_TYPE_SD_GALLERY) && AUDIO_MESSAGE_FILE_ENABLE) {

        if (index >= sd_gallery_total) {

            printf("get info error %d\n\r",__LINE__);
            return -1;
        }
        info =  &p_sd_gallery[index];
      //  path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_gallery_new_total > 0)) {
            sd_gallery_new_total--;
        }
    }else{
        printf("get info error type \n\r");
        return -1;
    }

    if (info->is_new == 0) {

        return 0;
    }

    info->is_new = 0;
#if 0
    char file_path[MEDIA_PATH_MAX] = {0};
    strcat(file_path, path);
    strcat(file_path, info->file_name);

    struct stat st;
    chmod(file_path, S_IRUSR | S_IWUSR);
    stat(file_path, &st);
#endif
    if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO|| type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO||type ==FILE_TYPE_FLASH_PHOTO) {
        sd_file_sync(type);
		
    }
    return 0;
}



int media_file_delete(media_type type, int index)
{
    media_info *info  = NULL, *p_array = NULL;
    char file_path[64];
    int *total_file  = NULL, *new_total_file = NULL;;

    if (type == FILE_TYPE_FLASH_PHOTO)
	{
        if (index >= flash_photo_total)
            return -1;

        p_array = p_flash_photo;
        info = &p_flash_photo[index];
        strcpy(file_path, FLASH_PHOTO_PATH);
        strcat(file_path, info->file_name);

        total_file = &flash_photo_total;
        new_total_file = &flash_photo_new_total;

    }else if(type == FILE_TYPE_FLASH_LOGO){
         if (index >= flash_logo_total)
            return -1;

        p_array = p_flash_logo;
        info = &p_flash_logo[index];
        strcpy(file_path, FLASH_PHOTO_PATH);
        strcat(file_path, info->file_name);

        total_file = &flash_logo_total;
        new_total_file = &flash_logo_new_total;
    }
     else {

        if (is_sdcard_insert() == 0) {

            printf("no insert sd ,delete fail \n\r");
            return -1;
        }
        if ((type == FILE_TYPE_SD_MIXED|| type == FILE_TYPE_SD_MIXED_PHOTO ||type == FILE_TYPE_SD_MIXED_VIDEO)
            && (index < sd_mixed_total)
            && MIX_PHOTOS_AND_VIDEOS_FILE) {

            p_array = p_sd_mixed;
            info = &p_sd_mixed[index];
            strcpy(file_path, SD_MIXED_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_mixed_total;
            new_total_file = &sd_mixed_new_total;
        }else if ((type == FILE_TYPE_SD_PHOTO && index < sd_photo_total) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {
            p_array = p_sd_photo;
            info = &p_sd_photo[index];
            strcpy(file_path, SD_PHOTO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_photo_total;
            new_total_file = &sd_photo_new_total;
        } else if ((type == FILE_TYPE_SD_VIDEO && index < sd_video_total) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

            p_array = p_sd_video;
            info = &p_sd_video[index];
            strcpy(file_path, SD_VIDEO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_video_total;
            new_total_file = &sd_video_new_total;
        } else if ((type == FILE_TYPE_SD_AUDIO  && index < sd_message_total) && AUDIO_MESSAGE_FILE_ENABLE) {

            p_array = p_sd_message;
            info = &p_sd_message[index];
            strcpy(file_path, SD_AUDIO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_message_total;
            new_total_file = &sd_message_new_total;
        } else if ((type == FILE_TYPE_SD_GALLERY  && index < sd_gallery_total) && GALLERY_IMG_FILE_ENABLE) {

            p_array = p_sd_gallery;
            info = &p_sd_gallery[index];
            strcpy(file_path, SD_GALLERY_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_gallery_total;
            new_total_file = &sd_gallery_new_total;
        } else {
            printf("delete media file error.\n\r");
            return -1;
        }
    }

    if ((*total_file) <= 0) {

        printf("delete fail file total %d \n\r", *total_file);
        return -1;
    }
    if ((info->is_new) && ((*new_total_file) > 0)) {

        (*new_total_file)--;
    }

    remove(file_path);
    printf("del %d.%s \n\r", index, file_path);

    if (index < ((*total_file) - 1)) {

        memmove(&p_array[index], &p_array[index + 1], (((*total_file) - 1) - index)*sizeof(media_info));
    }
    (*total_file)--;

    if (type == FILE_TYPE_SD_PHOTO
        || type == FILE_TYPE_SD_VIDEO
        || type == FILE_TYPE_SD_MIXED
        || type == FILE_TYPE_SD_MIXED_PHOTO
        || type == FILE_TYPE_SD_MIXED_VIDEO
        ||type ==FILE_TYPE_FLASH_PHOTO)
    {
        sd_file_sync(type);
		
    }
    return 0;
}


int record_null_error_file_remove(const char *file , bool is_null)
{
	if (is_sdcard_insert() == 0) {
        printf("no insert sd\n\r");
        return -1;
    }

	char bad_file[MEDIA_PATH_MAX] = {0};
    if ((_video_bad_path_check(file, bad_file))) {

        remove(bad_file);
        int index = sd_video_total - 1;
        media_file_delete(p_sd_video[index].type, index);
        
		return 1;

    }else if(is_null){
	 	int index = sd_video_total - 1;
     	media_file_delete(p_sd_video[index].type, index);
        
		return 1;
    }
	return 0;
}

int snap_null_error_file_remove(const char *file,  bool is_null)
{
	char bad_file[MEDIA_PATH_MAX] = {0};
	if (_photo_bad_path_check(file, bad_file)){
		
        if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0) {
            int index = sd_photo_total - 1;
            media_file_delete(p_sd_photo[index].type, index);
            
		
			remove(bad_file);
			return 1;
        } else {
			
            int index = flash_photo_total - 1;
            media_file_delete(p_flash_photo[index].type, index);
            
			
			remove(bad_file);
			return 1;
        }
    }
	else if(is_null)
	{
		printf("+++++%s \n\r",file);
		if (is_sdcard_insert() == 0)
		{
			int index = flash_photo_total - 1;
            media_file_delete(p_flash_photo[index].type, index);
            
			return 1;
		}
		else  if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
		{
            int index = sd_photo_total - 1;
            media_file_delete(p_sd_photo[index].type, index);
            
			return 1;
        }
	}
	return 0;
}


int playback_bad_file_check(const char *file,int index)
{
	char bad_file[MEDIA_PATH_MAX] = {0};
    if ((_video_bad_path_check(file, bad_file))) {

        remove(bad_file);
        media_file_delete(p_sd_video[index].type, index);
        

		return 1;
    } else if (_photo_bad_path_check(file, bad_file)) {
		
        if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0) {

			remove(bad_file);
            media_file_delete(p_sd_photo[index].type, index);
            
			return 1;
        } else {

			remove(bad_file);
            media_file_delete(p_flash_photo[index].type, index);
            
			return 1;
        }
    }
	return 0;
}

int media_bad_path_check(const char* file){
    if (is_sdcard_insert() == 0){
        
        return -1;
    }

    char bad_file[MEDIA_PATH_MAX] = {0};
    if(MIX_PHOTOS_AND_VIDEOS_FILE)
	{
        if((_video_bad_path_check(file,bad_file)))
		{
            remove(bad_file);
            int index = sd_mixed_total - 1;
            media_file_delete(p_sd_mixed[index].type,index);
            
        }else if(_photo_bad_path_check(file,bad_file)){
            if(strncmp(file,SD_BASE_PATH,strlen(SD_BASE_PATH)) == 0){
                int index = sd_mixed_total - 1;
                media_file_delete(p_sd_mixed[index].type,index);
                
            }else{
                int index = flash_photo_total - 1;
                media_file_delete(p_flash_photo[index].type,index);
                
            }
        }
    }else{

        if((_video_bad_path_check(file,bad_file))){
            remove(bad_file);
            int index = sd_video_total - 1;
            media_file_delete(p_sd_video[index].type,index);
            

        }else if(_photo_bad_path_check(file,bad_file)){
            if(strncmp(file,SD_BASE_PATH,strlen(SD_BASE_PATH)) == 0){
                int index = sd_photo_total - 1;
                media_file_delete(p_sd_photo[index].type,index);
                
            }else{
                int index = flash_photo_total - 1;
                media_file_delete(p_flash_photo[index].type,index);
                
            }
        }
    }

    return 0;
}

/************************************************************
** 函数说明: 获取文件大小
** 作者: xiaoxiao
** 日期: 2023-03-22 19:25:57
** 参数说明: 
** 注意事项: 
************************************************************/
unsigned int file_size_get(char * file_path)
{
	FILE* fp = fopen(file_path,"r");
    if(fp == NULL){
        printf("open %s fail \n\r",file_path);
        return -1;
    }
    fseek(fp,0,SEEK_END);
    unsigned int file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);
	return file_size;
}  