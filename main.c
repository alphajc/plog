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

#define THRD_COUNT 20

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

