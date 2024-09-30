#ifndef FN_MSGQ_H
#define FN_MSGQ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/*****  Macro Definitions ******/
#define CONTINUE_READING 0
#define STOP_READING 1


struct message_board_lot_in {
    int64_t li_msg;  
    char board_lot_in[338];
};

struct message_log_on {
    int64_t li_msg;
    char log_on[300];
};

struct message_log_off {
    int64_t li_msg;
    char log_off[62];
};

/* Function Prototypes */

int create_message_queue(int *initial_queue_id);
int destroy_message_queue(int global_queue_id);
int send_message_to_queue(int global_queue_id, void *message, long message_length);
int receive_message_from_queue(int global_queue_id, void *buffer, long *buffer_length);
int get_queue_message_count(int global_queue_id, unsigned int *message_count);
int flush_message_queue(int global_queue_id);
int send_board_lot_message(int global_queue_id, struct message_board_lot_in *message);
int send_log_on_message(int global_queue_id, struct message_log_on *message);
int send_log_off_message(int global_queue_id, struct message_log_off *message);
int receive_message_board_lot_in(int global_queue_id, struct message_board_lot_in *msg);
int receive_message_log_on(int global_queue_id, struct message_log_on *msg);
int receive_message_log_off(int global_queue_id, struct message_log_off *msg);


#endif /* FN_MSGQ_H */
