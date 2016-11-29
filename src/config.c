//
// Created by gavin on 16-11-26.
//

#include "config.h"

static char *log_path;      /* 设置日志路径 */
static char *project_name;  /* 项目名称 */
static int log_num = 10;    /* 日志文件数 */
static size_t log_size = 1; /* 每个日志文件大小，单位兆字节 */
static int  print_level = 1;      /* 打印日志的级别，默认debug */
void
set_project_name(char *name)
{
    project_name = name;
}

const char *
get_project_name()
{
    return project_name;
}

void
set_log_num(unsigned num)
{
    log_num = num;
}

const int
get_log_num()
{
    return log_num;
}

void
set_log_size(size_t size)
{
    log_size = size;
}

const int
get_log_size()
{
    return log_size;
}

void
set_log_path(char *path)
{
    log_path = path;
}
const char
*get_log_path()
{
    return log_path;
}

void
set_print_level(int level)
{
    print_level = level;
}

const int
get_print_level()
{
    return print_level;
}