/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lib.h
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
#ifndef LIBPLOG_H
#define LIBPLOG_H

/*---------------------
 * 日志级别：
 *	  debug------1
 *	  info ------2
 *	  warn ------3
 *	  error------4
 *	  fatal------5
 *---------------------
 */

typedef enum {
	 DEBUG = 1,
	 INFO,
	 WARN,
	 ERROR,
	 FATAL
 } log_level_t;

int libplog_init ();
int libplog_destroy();
int libplog_write (const char *file_name, const char *func_name, 
		const int line, log_level_t level, char *msg, ...);

/*
 * @description:调用libplog_write(),传入调试级别码
 */
#define PLOG_DEBUG(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, DEBUG,  msg, ##args)

/*
 * @description:调用libplog_write(),传入提示级别码
 */
#define PLOG_INFO(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, INFO,  msg, ##args)

/*
 * @description:调用libplog_write(),传入警告级别码
 */
#define PLOG_WARN(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, WARN,  msg, ##args)

/*
 * @description:调用libplog_write(),传入错误级别码
 */
#define PLOG_ERROR(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, ERROR,  msg, ##args)

/*
 * @description:调用libplog_write(),传入崩溃级别码
 */
#define PLOG_FATAL(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, FATAL,  msg, ##args)

#endif /* endif */