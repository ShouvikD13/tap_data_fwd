#include <errno.h>
#include "message_queue.h"
#include "Log.h"

int msgq_exit_flag;


int create_message_queue(int *initial_queue_id) 
{
	int temp_global_queue_id;
	
	temp_global_queue_id = msgget( *initial_queue_id, (IPC_CREAT | IPC_EXCL | 0777) );

	if (temp_global_queue_id == -1) {
		log_error("[message_queue.c]", " [create_message_queue] ", "Failed to create message queue: %s", strerror(errno));
		return -1; 
	}
	log_info("[message_queue.c]", " [create_message_queue] ", "Message queue created successfully with ID: %d", temp_global_queue_id);
	return temp_global_queue_id;
}
// For the initial creation of the message queue, the `initial_queue_id` provided by the user, is used.
// On subsequent attempts to obtain the message queue, the return value of `msgget()` (global_queue_id) should be used.
// This ensures that either a new queue is created or an existing queue is accessed based on the given `queue_id`.

int destroy_message_queue(int global_queue_id) 
{
	if (msgctl(global_queue_id, IPC_RMID, NULL) == -1) {
		log_error("[message_queue.c]", " [destroy_message_queue] ", "Failed to remove message queue:");
		return -1
	} else {
		log_info("[message_queue.c]", " [destroy_message_queue] ", "Message queue removed successfully, ID: %d", global_queue_id);
		return 0
	}
}

int  send_message_to_queue(int global_queue_id, void *message, long message_length) 
{
	if (msgsnd(global_queue_id, message, message_length, IPC_NOWAIT) == -1) {
		log_error("[message_queue.c]", " [send_message_to_queue] ", "Failed to send message to queue: %s", strerror(errno));
		return -1;
	}
	log_info("[message_queue.c]", " [receive_message_from_queue] ", "Message sent to queue ID: %d, Length: %ld bytes", global_queue_id, message_length);
	return 0;
}

int receive_message_from_queue(int global_queue_id, void *buffer, long *buffer_length) {

    long received_bytes = msgrcv(global_queue_id, buffer, *buffer_length, 0, 0);

    if (received_bytes == -1) {
        if (errno != EINTR) {
            log_error("[message_queue.c]", " [receive_message_from_queue] ", "Failed to receive message: %s", strerror(errno));
            return -1;
        }
    } else {
        *buffer_length = received_bytes;

        log_info("[message_queue.c]", " [receive_message_from_queue] ", "Received message from queue ID: %d, Length: %ld bytes", global_queue_id, received_bytes);
        return 0;
    }

    log_warning("[message_queue.c]", " [receive_message_from_queue] ", "Queue reading interrupted by user signal. Returning with Error Code: %d", -1);
    return -1;
}

int get_queue_message_count(int global_queue_id, unsigned int *message_count) 
{
	struct msqid_ds queue_status;

	if (msgctl(global_queue_id, IPC_STAT, &queue_status) == -1) {
		log_error("[message_queue.c]", " [get_queue_message_count] " ,"Failed to retrieve message queue status: %s", strerror(errno));
		return -1;
	} else {
	
		*message_count = queue_status.msg_qnum;
		
		log_info("[message_queue.c]", " [get_queue_message_count] ", "Queue ID: %d has %d messages", global_queue_id , *message_count);
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
				log_info("[message_queue.c]", "[flush_message_queue]", "Message queue ID: %d is empty", global_queue_id);
				return 0;  
			} else if (errno != EINTR) {
				log_error("[message_queue.c]", " [flush_message_queue] ", "Failed to flush message queue: %s", strerror(errno));
				return -1;
			}
		}
	}

	log_error("[message_queue.c]", " [flush_message_queue] ", "Failed to flush message queue due to user signal Returning with Error Code : %d", -1);
	return -1;
}


int send_board_lot_message(int global_queue_id, struct message_board_lot_in *message) {
    return send_message_to_queue(global_queue_id, message, sizeof(*message));
}

int send_log_on_message(int global_queue_id, struct message_log_on *message) {
    return send_message_to_queue(global_queue_id, message, sizeof(*message));
}

int send_log_off_message(int global_queue_id, struct message_log_off *message) {
    return send_message_to_queue(global_queue_id, message, sizeof(*message));
}

 