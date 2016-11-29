//
// Created by gavin on 16-11-26.
//

#include "files_handle.h"
#include "config.h"
#include <memory.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

static char **g_file_name_list; /* 申请的文件名数组 */
static int g_current_fd;        /* 当前文件描述符 */
static size_t g_log_size;       /* 每个日志文件大小 */
static int g_log_num;           /* 日志文件数 */
static int g_exist_log_num;     /* 已经创建的日志数 */
static int g_old_index;         /* 最旧的文件索引 */

static const void
generate_file_name(char *file_name)
{
    char strtime[MAX_STRTIME_LEN];
    struct timeval tv;
    struct tm* pTime;

    gettimeofday(&tv, NULL);
    pTime = localtime(&tv.tv_sec);

    strncpy(file_name, get_log_path(), max_file_name_len);
    if (file_name[strlen(file_name)-1] != '/')
        strcat(file_name, "/");
    strncat(file_name, get_project_name(), max_file_name_len);
    strncat(file_name, "_", max_file_name_len);
    snprintf(strtime, MAX_STRTIME_LEN, "%04d%02d%02d%02d%02d%02d%03d%03d", pTime->tm_year+1900, \
            pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
            tv.tv_usec/1000,tv.tv_usec%1000);
    strncat(file_name, strtime, max_file_name_len);
    strncat(file_name, ".log", max_file_name_len);
}

const int 
file_is_full()
{
    off_t size;

    size = lseek(g_current_fd, 0, SEEK_END);
    if (-1 == size)
    {
        perror("lseek error");
        exit(1);
    }

    if (size >= g_log_size * 1024 * 1024)
        return 1;

    return 0;
}

const int 
switch_file(int old_fd)
{
    char filename[max_file_name_len];

    close(old_fd);
    generate_file_name(filename);

    if (g_log_num > g_exist_log_num)
    {
        g_file_name_list[g_exist_log_num] = (char **)malloc(max_file_name_len);
        strncpy(g_file_name_list[g_exist_log_num], filename, max_file_name_len);
        if ((g_current_fd = open(filename,O_WRONLY | O_CREAT, 0777)) == -1)
        {
            perror("open file failed in switch_file");
        }
        g_exist_log_num++;
    }
    else if (g_log_num == g_exist_log_num)
    {
        remove(g_file_name_list[g_old_index]);
        g_file_name_list[g_old_index] = (char **)malloc(max_file_name_len);
        strncpy(g_file_name_list[g_old_index], filename, max_file_name_len);
        if ((g_current_fd = open(filename,O_WRONLY | O_CREAT, 0777)) == -1)
        {
            perror("open file failed in switch_file");
        }
    }
    else
    {
        printf("the log's num error.\n");
    }

    return g_current_fd;
}

const int 
init_file() 
{
    char filename[max_file_name_len];
    g_log_size = get_log_size();
    g_log_num = get_log_num();
    generate_file_name(filename);

    g_file_name_list = (char **)malloc(g_log_num * sizeof(char *));
    *g_file_name_list = (char *)malloc(max_file_name_len);
    strncpy(*g_file_name_list, filename, max_file_name_len);

    if((g_current_fd = open(filename,O_WRONLY | O_CREAT, 0777)) == -1)
    {
        perror("open file failed in init_file");
    }
    g_exist_log_num = 1;
    g_old_index = 0;

    return g_current_fd;
}

void close_file() {
    close(g_current_fd);
}
