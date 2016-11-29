/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2016 Gavin Chan <jiajun.chen@ww-it.cn>
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
 * logtest-sample IS PROVIDED BY Gavin Chan ``AS IS'' AND ANY EXPRESS
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
#include "libplog.h"
#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>

#define THRD_COUNT 	20
#define MSG_SIZE		898

/*
 * @description:调用log_print(),传入调试级别码
 */
#define PLOG_DEBUG(msg, args...) \
		log_print(__FILE__, __FUNCTION__, __LINE__, DEBUG,  msg, ##args)

/*
 * @description:调用log_print(),传入提示级别码
 */
#define PLOG_INFO(msg, args...) \
		log_print(__FILE__, __FUNCTION__, __LINE__, INFO,  msg, ##args)

/*
 * @description:调用log_print(),传入警告级别码
 */
#define PLOG_WARN(msg, args...) \
		log_print(__FILE__, __FUNCTION__, __LINE__, WARN,  msg, ##args)

/*
 * @description:调用log_print(),传入错误级别码
 */
#define PLOG_ERROR(msg, args...) \
		log_print(__FILE__, __FUNCTION__, __LINE__, ERROR,  msg, ##args)

/*
 * @description:调用log_print(),传入崩溃级别码
 */
#define PLOG_FATAL(msg, args...) \
		log_print(__FILE__, __FUNCTION__, __LINE__, FATAL,  msg, ##args)


void log_print(const char *file_name, const char *func_name,
			   const int line, log_level_t level, char *fmt, ...)
{
	size_t len = 0;
	char msg[MSG_SIZE];
	/* snprintf和vsnprintf，如果大于MAX_SIZE将被截断	*/
	snprintf(msg, MSG_SIZE, "`%s`%s:%d %s\t", func_name,
			 file_name, line, get_level(level));
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	len = strlen(msg);
	vsnprintf(msg + len, MSG_SIZE - len, fmt, arg_ptr);
    libplog_write(level, msg);
	va_end(arg_ptr);
}

void *
print_log(void *arg)
{
	for(int i = 0; i < 100; ++i)
	{	
		PLOG_ERROR("我是测试字符串 %s,第 %d次打印，我是线程%s\n","张鹏",i, (char *)arg);
		PLOG_DEBUG("我是测试字符串 %s,第 %d次打印，我是线程%s\n","灿灿",i, (char *)arg);
		PLOG_INFO("我是测试字符串 %s,第 %d次打印，我是线程%s\n","何敏",i, (char *)arg);
		PLOG_FATAL("我是测试字符串 %s,第 %d次打印，我是线程%s\n","小华",i, (char *)arg);
		PLOG_WARN("我是测试字符串 %s,第 %d次打印，我是线程%s\n","小玲",i, (char *)arg);
	}

	return NULL;
}

int 
main()
{
	pthread_t thrd[THRD_COUNT];
	char thrdname[20][10]={"thread-A","thread-B","thread-C","thread-D","thread-E","thread-F","thread-G","thread-H","thread-I","thread-J",
						   "thread-K","thread-L","thread-M","thread-N","thread-O","thread-P","thread-Q","thread-R","thread-S","thread-T"};
	libplog_init (".", "test", 10, 1);
	for(int j = 0; j < THRD_COUNT; ++j)
	{
		pthread_create(&thrd[j], NULL, print_log, (void *)thrdname[j]);
	}
	for(int j = 0; j < THRD_COUNT; ++j)
	{
		pthread_join(thrd[j], NULL);
	}
	libplog_destroy ();
	
	return (0);
}

