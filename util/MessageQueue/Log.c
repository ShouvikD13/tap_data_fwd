#include "Log.h"
// Create log directory if it doesn't exist
void create_log_directory() {
    struct stat st = {0};

    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0700);
    }
}

// Generate log file name in the "ULOG.YYYY-MM-DD.log" format
void get_log_file_name(char *log_file_name, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(log_file_name, size, "ULOG.%Y-%m-%d.log", t);
}

// Rotate logs if necessary (based on date change)
void rotate_logs() {
    char new_log_file[1024];
    get_log_file_name(new_log_file, sizeof(new_log_file));

    // If log file date has changed, close the current file and open a new one
    if (strcmp(current_log_file, new_log_file) != 0) {
        if (log_file != NULL) {
            fclose(log_file);
        }

        snprintf(current_log_file, sizeof(current_log_file), "%s/%s", LOG_DIR, new_log_file);
        log_file = fopen(current_log_file, "a");
        if (log_file == NULL) {
            perror("Failed to open log file");
            exit(EXIT_FAILURE);
        }
    }
}

// Initialize the log file by creating the log directory and opening the log file
void init_log() {
    create_log_directory();

    // Initialize log file name based on the current date
    get_log_file_name(current_log_file, sizeof(current_log_file));
    snprintf(current_log_file, sizeof(current_log_file), "%s/%s", LOG_DIR, current_log_file);

    // Open the log file
    log_file = fopen(current_log_file, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

// Function to write logs in the desired format to both file and terminal
void write_log(const char *level, const char *service_name, const char *function_name, const char *format, ...) {
    if (log_file == NULL) {
        init_log();
    }

    // Check if a new log file is needed (rotate if the date has changed)
    rotate_logs();

    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    // Prepare variable arguments for the log message
    va_list args;
    va_start(args, format);

    // Write to the log file
    fprintf(log_file, "%s [%s] [%s] [%s] ", level, time_str, service_name, function_name);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    // Write to the terminal (stdout)
    printf("%s [%s] [%s] [%s] ", level, time_str, service_name, function_name);
    vprintf(format, args);
    printf("\n");

    va_end(args);
    fflush(log_file);  // Ensure the log is written immediately
    fflush(stdout);    // Ensure the terminal output is flushed
}

// Log an info-level message
void log_info(const char *service_name, const char *function_name, const char *message, ...) {
    va_list args;
    va_start(args, message);
    write_log("INFO", service_name, function_name, message, args);
    va_end(args);
}

// Log an error-level message
void log_error(const char *service_name, const char *function_name, const char *message, ...) {
    va_list args;
    va_start(args, message);
    write_log("ERROR", service_name, function_name, message, args);
    va_end(args);
}

// Close the log file
void close_log() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}
