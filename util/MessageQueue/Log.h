#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#define LOG_DIR "/mnt/c/Users/devdu/go-workspace/data_fwd_tap/logs"

void create_log_directory(void);
void get_log_file_name(char *log_file_name, size_t size);
void rotate_logs(void);
void init_log(void);
void write_log( char *level,  char *service_name,  char *function_name,  const char *format, ...);
void log_info( char *service_name,  char *function_name,  const char *format, ...);
void log_error( char *service_name,  char *function_name,  const char *format, ...);
void close_log(void);

#endif /* LOG_H */
