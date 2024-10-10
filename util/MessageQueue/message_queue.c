#include <errno.h>
#include <stdio.h> // Added for printf and fprintf
#include "message_queue.h"
// #include "Log.h" // Removed as we're replacing logging

int msgq_exit_flag;

int create_message_queue(int *initial_queue_id) 
{
    key_t key;
    int temp_global_queue_id;

    printf("message_queue.c: create_message_queue: Pointer received: %p\n", (void*)initial_queue_id);
    printf("message_queue.c: create_message_queue: Testing if it is working without making object file and shared library\n");

    if (initial_queue_id != NULL) {
        printf("message_queue.c: create_message_queue: Value pointed by initial_queue_id: %d\n", *initial_queue_id);
    } else {
        fprintf(stderr, "message_queue.c: create_message_queue: Received NULL pointer for initial_queue_id\n");
        return -1;
    }

    key = *initial_queue_id;
    // key = ftok("/mnt/c/Users/devdu/go-workspace/data_fwd_tap/cmd/main/mq_key_file", 'A');

    if(key == -1 ){
        fprintf(stderr, "message_queue.c: create_message_queue: Failed to get the unique key id using 'ftok': %s\n", strerror(errno));
        return -1;
    }

    if(key == 0){
        fprintf(stderr, "message_queue.c: create_message_queue: ftok returned 0, which is invalid.\n");
        return -1;
    }

    printf("message_queue.c: create_message_queue: Key retrieved from ftok : %d\n", key);
    printf("message_queue.c: create_message_queue: Initial Queue id is : %d\n", *initial_queue_id);
    printf("message_queue.c: create_message_queue: Address of Initial Queue id is : %p\n", (void*)initial_queue_id);

    temp_global_queue_id = msgget(key, (IPC_CREAT | IPC_EXCL | 0777));
    printf("message_queue.c: create_message_queue: Global Queue 1 id is : %d\n", temp_global_queue_id);
    printf("message_queue.c: create_message_queue: Global Queue 2 id is : %d\n", temp_global_queue_id);
    
    if (temp_global_queue_id == -1) {
        if (errno == EEXIST) {
            printf("message_queue.c: create_message_queue: Message queue already exists with key: %d\n", key);
            temp_global_queue_id = msgget(key, 0); 
            if (temp_global_queue_id == -1) {
                fprintf(stderr, "message_queue.c: create_message_queue: Failed to access existing message queue: %s\n", strerror(errno));
                return -1;
            }
            printf("message_queue.c: create_message_queue: Accessed existing message queue with ID: %d\n", temp_global_queue_id);
        } else {
            fprintf(stderr, "message_queue.c: create_message_queue: Failed to create message queue: %s\n", strerror(errno));
            fprintf(stderr, "message_queue.c: create_message_queue: Key used was: %d\n", key);
            fprintf(stderr, "message_queue.c: create_message_queue: Address of key is : %p\n", (void*)&key);
            return -1; 
        }
    } else {
        printf("message_queue.c: create_message_queue: Message queue created successfully with ID: %d\n", temp_global_queue_id);
    }
    
    return temp_global_queue_id;
}

// For the initial creation of the message queue, the `initial_queue_id` provided by the user, is used.
// On subsequent attempts to obtain the message queue, the return value of `msgget()` (global_queue_id) should be used.
// This ensures that either a new queue is created or an existing queue is accessed based on the given `queue_id`.

int destroy_message_queue(int global_queue_id) 
{
    if (msgctl(global_queue_id, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "message_queue.c: destroy_message_queue: Failed to remove message queue: %s\n", strerror(errno));
        return -1;
    } else {
        printf("message_queue.c: destroy_message_queue: Message queue removed successfully, ID: %d\n", global_queue_id);
        return 0;
    }
}

int send_message_to_queue(int global_queue_id, void *message, long message_length) 
{
    if (msgsnd(global_queue_id, message, message_length, IPC_NOWAIT) == -1) {
        fprintf(stderr, "message_queue.c: send_message_to_queue: Failed to send message to queue: %s\n", strerror(errno));
        return -1;
    }
    printf("message_queue.c: send_message_to_queue: Message sent to queue ID: %d, Length: %ld bytes\n", global_queue_id, message_length);
    
    size_t received_size = sizeof(*(struct message_board_lot_in *)message); 
    fprintf(stderr, "message_queue.c: send_message_to_queue: Size  Expected: %ld, Received: %zu\n", message_length, received_size);


    return 0;
}

int receive_message_from_queue(int global_queue_id, void *buffer, long *buffer_length) {

    long received_bytes = msgrcv(global_queue_id, buffer, *buffer_length, 0, 0);

    if (received_bytes == -1) {
        fprintf(stderr, "message_queue.c: receive_message_from_queue: Failed to receive message: %s\n", strerror(errno));
        return -1;
    } else {
        *buffer_length = received_bytes;
        printf("message_queue.c: receive_message_from_queue: Received message from queue ID: %d, Length: %ld bytes\n", global_queue_id, received_bytes);
        return 0;
    }
    return -1;
}

int get_queue_message_count(int global_queue_id, msgqnum_t *message_count) 
{
    struct msqid_ds queue_status;

    if (msgctl(global_queue_id, IPC_STAT, &queue_status) == -1) {
        fprintf(stderr, "message_queue.c: get_queue_message_count: Failed to retrieve message queue status: %s\n", strerror(errno));
        return -1;
    } else {
        *message_count = queue_status.msg_qnum;
        printf("message_queue.c: get_queue_message_count: Queue ID: %d has %llu messages\n", global_queue_id, (unsigned long long)*message_count);
        return 0;
    }
}

int flush_message_queue(int global_queue_id) 
{
    char temp_buffer[3072];
    long buffer_length = sizeof(temp_buffer);

    msgq_exit_flag = CONTINUE_READING;

    while (msgq_exit_flag == CONTINUE_READING) {
        if (msgrcv(global_queue_id, &temp_buffer, buffer_length, 0, IPC_NOWAIT) == -1) {
            if (errno == ENOMSG) {
                printf("message_queue.c: flush_message_queue: Message queue ID: %d is empty\n", global_queue_id);
                return 0;  
            } else if (errno != EINTR) {
                fprintf(stderr, "message_queue.c: flush_message_queue: Failed to flush message queue: %s\n", strerror(errno));
                return -1;
            }
        }
    }

    fprintf(stderr, "message_queue.c: flush_message_queue: Failed to flush message queue due to user signal. Returning with Error Code: %d\n", -1);
    return -1;
}

int send_board_lot_message(int global_queue_id, struct message_board_lot_in *message) {
    return send_message_to_queue(global_queue_id, message, sizeof(struct message_board_lot_in));
}

int send_log_on_message(int global_queue_id, struct message_log_on *message) {
    return send_message_to_queue(global_queue_id, message, sizeof(struct message_log_on));
}

int send_log_off_message(int global_queue_id, struct message_log_off *message) {
    return send_message_to_queue(global_queue_id, message, sizeof(struct message_log_off));
}
