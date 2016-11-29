//
// Created by gavin on 16-11-26.
//

#ifndef PLOG_CONFIG_H
#define PLOG_CONFIG_H

#include <sys/types.h>

void set_log_path(char *path);
const char *get_log_path();
void set_project_name(char *name);
const char *get_project_name();
void set_log_num(unsigned num);
const int get_log_num();
void set_log_size(size_t size);
const int get_log_size();

#endif //PLOG_CONFIG_H
