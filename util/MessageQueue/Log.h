#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#define LOG_DIR "./logs"

void create_log_directory(void);
void get_log_file_name(char *log_file_name, size_t size);
void rotate_logs(void);
void init_log(void);
void write_log(const char *level, const char *service_name, const char *function_name, const char *format, ...);
void log_info(const char *service_name, const char *function_name, void *message, ...);
void log_error(const char *service_name, const char *function_name, void *message, ...);
void close_log(void);

#endif /* LOG_H */
