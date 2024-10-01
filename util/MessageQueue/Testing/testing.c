#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h> // Required for va_list

// Structure for the message
struct message {
    long msg_type;
    char msg_text[100]; // Message content
};

// Declaration of the function to test
int get_queue_message_count(int global_queue_id, unsigned long *message_count); // Use unsigned long instead

// Logging functions
void log_error(const char *file, const char *func, const char *fmt, const char *msg) {
    fprintf(stderr, "%s%s: %s\n", file, func, msg);
}

void log_info(const char *file, const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("%s%s: ", file, func);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

int main() {
    key_t key = ftok("msg_queue_file", 65); // Generate a unique key
    int msgid = msgget(key, 0666 | IPC_CREAT); // Create a message queue

    if (msgid == -1) {
        perror("msgget failed");
        return 1;
    }

    // Adding messages to the queue
    struct message msg;
    for (int i = 1; i <= 5; i++) { // Add 5 messages
        msg.msg_type = 1; // Set message type
        snprintf(msg.msg_text, sizeof(msg.msg_text), "Message %d", i);
        if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
            perror("msgsnd failed");
            msgctl(msgid, IPC_RMID, NULL); // Clean up
            return 1;
        }
    }

    // Now test get_queue_message_count
    unsigned long message_count; // Use unsigned long instead of msgqnum_t
    if (get_queue_message_count(msgid, &message_count) == 0) {
        printf("Message count retrieved: %lu\n", message_count);
    } else {
        printf("Failed to get message count.\n");
    }

    // Clean up the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl failed to remove message queue");
    }

    return 0;
}

int get_queue_message_count(int global_queue_id, unsigned long *message_count) 
{
    struct msqid_ds queue_status;

    if (msgctl(global_queue_id, IPC_STAT, &queue_status) == -1) {
        log_error("[message_queue.c]", " [get_queue_message_count] ",
                  "Failed to retrieve message queue status: %s", strerror(errno));
        return -1;
    } else {
        *message_count = queue_status.msg_qnum;
        log_info("[message_queue.c]", " [get_queue_message_count] ",
                 "Queue ID: %d has %lu messages", global_queue_id, *message_count);
        return 0;
    }
}
