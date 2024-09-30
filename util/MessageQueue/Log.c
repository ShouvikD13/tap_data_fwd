#include "Log.h"
#include <errno.h>

FILE *log_file = NULL;   
char current_log_file[1024];  

void create_log_directory() {
    struct stat st = {0};

    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0700);
    }
}

void get_log_file_name(char *log_file_name, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(log_file_name, size, "ULOG.%Y-%m-%d.log", t);
}

void rotate_logs() {
    char new_log_file[1024];
    get_log_file_name(new_log_file, sizeof(new_log_file));

    if (strcmp(current_log_file, new_log_file) != 0) {
        if (log_file != NULL) {
            fclose(log_file);
        }

       
        if (snprintf(current_log_file, sizeof(current_log_file), "%s/%s", LOG_DIR, new_log_file) >= sizeof(current_log_file)) {
            fprintf(stderr, "Log file path too long, truncating...\n");
            current_log_file[sizeof(current_log_file) - 1] = '\0'; 
        }

        log_file = fopen(current_log_file, "a");
        if (log_file == NULL) {
            perror("Failed to open log file");
            exit(EXIT_FAILURE);
        }
    }
}

void init_log() {
    create_log_directory();

    char log_file_name[1024];
    get_log_file_name(log_file_name, sizeof(log_file_name));

    
    if (snprintf(current_log_file, sizeof(current_log_file), "%s/%s", LOG_DIR, log_file_name) >= sizeof(current_log_file)) {
        fprintf(stderr, "Log file path too long, truncating...\n");
        current_log_file[sizeof(current_log_file) - 1] = '\0'; 
    }

    log_file = fopen(current_log_file, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void write_log(const char *level, const char *service_name, const char *function_name, const char *format, ...) {
    if (log_file == NULL) {
        init_log();
    }
    rotate_logs();

    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    
    va_list args;
    va_start(args, format);

    
    fprintf(log_file, "%s [%s] [%s] [%s] ", level, time_str, service_name, function_name);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    
    printf("%s [%s] [%s] [%s] ", level, time_str, service_name, function_name);
    vprintf(format, args);
    printf("\n");

    va_end(args);
    fflush(log_file);  
    fflush(stdout);    
}

void log_info(const char *service_name, const char *function_name, void *message, ...) {
    va_list args;
    va_start(args, message);
    write_log("INFO", service_name, function_name, message, args);
    va_end(args);
}

void log_error(const char *service_name, const char *function_name, void *message, ...) {
    va_list args;
    va_start(args, message);
    write_log("ERROR", service_name, function_name, message, args);
    va_end(args);
}

void close_log() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}
