//
// Created by gavin on 16-11-26.
//

#ifndef PLOG_FILES_HANDLE_H
#define PLOG_FILES_HANDLE_H

#define MAX_STRTIME_LEN 50

static const int max_file_name_len = 256;

const int file_is_full();        /* 判断当前文件是否写满 */
const int switch_file(int fd);  /* 切换文件,返回切换后的文件描述符 */
const int init_file();          /* 初始化文件管理模块，返回第一个文件描述符 */
void close_file();              /* 关闭最后当前文件描述符 */

#endif //PLOG_FILES_HANDLE_H
