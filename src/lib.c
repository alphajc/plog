/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lib.c
 * Copyright (C) 2016 Gavin Chan <jiajun.chen@ww-it.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Gavin Chan'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * libplog IS PROVIDED BY Gavin Chan ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Gavin Chan OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "files_handle.h"
#include "mmgr.h"
#include "libplog.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

/* 最大的字符串长度加一 */
#define MAX_SIZE 200

static int log_fd;            /* 日志文件描述符 */
static int flag = 0;          /* 如果不会再打印日志了，置一 */
static pthread_t thrd;        /* 打印线程的ID */

/*
 * @function:get_level
 * @description:根据level返回对应字符串
 * @return:对应字符串
 * @called by:libplog_write
 * @author:Gavin
 *
 */
static char *
get_level(log_level_t level)
{
    switch(level)
    {
    case DEBUG: return "DEBUG";
    case INFO:  return "INFO";
    case WARN:  return "WARN";
    case ERROR: return "ERROR";
    case FATAL: return "FATAL";
    default:	return "";
    }
}

/*
 * @function:persistence
 * @description:将缓冲区数据写入磁盘（线程）
 * @calls:get_block_string
 *        release_full_block
 * @called by:libplog_init
 * @author:Gavin
 * @update: 2016.11.24 Gavin
 */
static void *
persistence(void *arg)
{
    char *page;
    int finished = 0;
	while (1)
	{
		page = get_block_string(&finished);
		if (file_is_full())
		{
			log_fd = switch_file(log_fd);
		}
        write(log_fd, page, strlen(page));
		if (1 == finished && 1 == flag)
		{
			break;
		}
	}

	return NULL;
}

/*
 * @function:plog_cache()
 * @description:缓存日志
 * @calls:memory_alloc  strlen
 *        memcpy
 * @param:log->待缓存的一条log
 * @called by:libplog_write
 * @author:Gavin
 * @update:2016.11.24 Gavin
 *
 */
int plog_cache(const char *log)
{
	void *begin;

	begin = memory_alloc (strlen(log));
	memcpy(begin, log, strlen(log));

	return 0;
}

/*
 * @function:libplog_write
 * @description:日志打印的入口函数，所有的调用最终从这里进入
 * @calls:strftime  localtime strlen
 *        snprintf  va_start  vsnprintf
 *        va_end    plog_cache
 * @param:file_name->产生日志的文件名
 *        func_name->产生日志的函数名
 *        line->产生日志的行数
 *        level->日志的级别
 *        msg,...->日志内容的格式化输出形式
 * @author:Gavin
 */
int
libplog_write(const char *file_name, const char *func_name,
		const int line, log_level_t level, char *fmt, ...)
{
	char log[MAX_SIZE] = {'\0'};
	size_t len = 0;
	time_t t;
	/* 获取时间	*/
	time(&t);
	strftime( log, MAX_SIZE, "[%Y%m%d%H%M%S] ",localtime(&t) );

	/* snprintf和vsnprintf，如果大于MAX_SIZE将被截断	*/
	len = strlen(log);
	snprintf(log + len, MAX_SIZE - len, "`%s`%s:%d %s\t", func_name,
	         file_name, line, get_level(level));
	len = strlen(log);
	va_list arg_ptr;
	va_start(arg_ptr, msg);
	vsnprintf(log + len, MAX_SIZE - len, fmt, arg_ptr);
	va_end(arg_ptr);
	/* 缓存	*/
	plog_cache(log);

	return (0);
}

/*
 * @function:libplog_init
 * @description:初始化日志模块，包括
 *									初始化内存管理模块，
 *									初始化互斥锁、条件变量
 *									打开文件描述符
 *									创建日志持久化线程
 * @calls:memory_init
 *        open
 *        pthread_create    perror
 * @author:Gavin
 * @update: 2016.11.28 Gavin
 */
int
libplog_init(const char *log_path, const char *project_name, const unsigned log_num, const size_t log_size)
{
	memory_init ();

    /* configure */
    set_log_path(log_path);
    set_project_name(project_name);
    set_log_num(log_num);
    set_log_size(log_size);

    log_fd = init_file();
/*
	if((log_fd = open("gc.log",O_RDWR | O_CREAT | O_APPEND, 0777)) == -1)
	{
		perror("open file gc.log failed.");
	}
*/

	if(0 != pthread_create(&thrd, NULL, persistence, NULL))
	{
		perror("create thread failed.");
	}

	return 0;
}

/*
 * @function:libplog_destroy
 * @description:销毁当初所创建的，并且等待线程结束
 * @calls:update_current_block  memory_destroy
 *        pthread_join
 *        close
 * @author:Gavin
 *
 */
int
libplog_destroy()
{
	flag = 1;
	update_current_block ();
	pthread_join(thrd, NULL);
	memory_destroy();

	close_file();

	return 0;
}
