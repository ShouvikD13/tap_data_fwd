#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <MessageQueue.h>

int msgq_exit_flag;

/******************************************************************************/
/*	Function Name	:	create_message_queue                                  */ 
/*                                                                            */ 
/*  Input			:	int *						                          */ 
/*                                                                            */ 
/*  Output			:	Message queue ID on success, -1 on failure            */ 
/*                                                                            */ 
/*	Description		:	Creates a message queue with the given type (Send/    */
/*                      Receive/Broadcast) and returns the queue ID.          */
/*                      On success, 0 is returned, otherwise, logs error      */
/*                      and returns -1.                                       */ 
/******************************************************************************/
int create_message_queue(int *queue_id) 
{
	int temp_id;

	// Obtain the message queue identifier based on type
	temp_id = msgget(
		msgq_type_key,				  // 'msgq_type_key' is the key for the message queue
		(IPC_CREAT | IPC_EXCL | 0777)  // Create new queue with read, write, exec permissions
	);

	if (temp_id == -1) {
		log_error("MessageQueue", "Failed to create message queue: %s", strerror(errno));
		return -1;  // Return failure if queue cannot be created
	}

    *queue_id = temp_id;  // Assign the queue identifier
	log_info("MessageQueue", "Message queue created successfully with ID: %d", *queue_id);
	return 0;
}

/******************************************************************************/
/*	Function Name	:	destroy_message_queue                                 */
/*                                                                            */ 
/*  Input			:	int queue_id   										  */ 
/*                                                                            */ 
/*  Output			:	Removes the message queue                             */ 
/*                                                                            */ 
/*	Description		:	Removes the message queue for the given queue ID.     */
/*                      Logs an error on failure.                             */ 
/******************************************************************************/
void destroy_message_queue(int queue_id) 
{
	if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
		log_error("MessageQueue", "Failed to remove message queue: %s", strerror(errno));
	} else {
		log_info("MessageQueue", "Message queue removed successfully, ID: %d", queue_id);
	}
}

/******************************************************************************/
/*	Function Name	:	send_message_to_queue                                 */
/*                                                                            */ 
/*  Input			:	int queue_id, void *message, long message_length      */ 
/*                                                                            */ 
/*  Output			:	0 on success, -1 on failure                           */ 
/*                                                                            */ 
/*	Description		:	Sends a message to the specified queue. If the queue  */
/*                      is full, it returns -1 and logs an error.             */ 
/******************************************************************************/
int send_message_to_queue(int queue_id, void *message, long message_length) 
{
	if (msgsnd(queue_id, message, message_length, IPC_NOWAIT) == -1) {
		log_error("MessageQueue", "Failed to send message to queue: %s", strerror(errno));
		return -1;
	}

	log_info("MessageQueue", "Message sent to queue ID: %d, Length: %ld bytes", queue_id, message_length);
	return 0;
}

/******************************************************************************/
/*	Function Name	:	receive_message_from_queue                            */
/*                                                                            */ 
/*  Input			:	int queue_id, void *buffer, long *buffer_length       */ 
/*                                                                            */ 
/*  Output			:	Returns the number of bytes received on success, -1   */ 
/*                      on failure                                            */ 
/*                                                                            */ 
/*	Description		:	Receives a message from the queue in FIFO order.      */
/*                      Waits for message if queue is empty unless interrupted*/
/******************************************************************************/
int receive_message_from_queue(int queue_id, void *buffer, long *buffer_length) 
{
	long received_bytes;
	msgq_exit_flag = CONTINUE_READING;

	while (msgq_exit_flag == CONTINUE_READING) {
		received_bytes = msgrcv(queue_id, buffer, *buffer_length, 0, 0);

		if (received_bytes == -1) {
			if (errno != EINTR) {
				log_error("MessageQueue", "Failed to receive message: %s", strerror(errno));
				return -1;
			}
		} else {
			// Store the number of bytes received
			*buffer_length = received_bytes;
			log_info("MessageQueue", "Received message from queue ID: %d, Length: %ld bytes", queue_id, received_bytes);
			return 0;
		}
	}

	log_error("MessageQueue", "Queue reading interrupted by user signal");
	return -1;
}

/******************************************************************************/
/*	Function Name	:	get_queue_message_count                               */
/*                                                                            */ 
/*  Input			:	int queue_id, unsigned int *message_count             */ 
/*                                                                            */ 
/*  Output			:	0 on success, -1 on failure                           */ 
/*                                                                            */ 
/*	Description		:	Retrieves the number of messages in the specified     */
/*                      message queue. Logs an error on failure.              */
/******************************************************************************/
int get_queue_message_count(int queue_id, unsigned int *message_count) 
{
	struct msqid_ds queue_status;

	// Get the status of the queue
	if (msgctl(queue_id, IPC_STAT, &queue_status) == -1) {
		log_error("MessageQueue", "Failed to retrieve message queue status: %s", strerror(errno));
		return -1;
	} else {
		// Store the number of messages in the queue
		*message_count = queue_status.msg_qnum;
		log_info("MessageQueue", "Queue ID: %d has %d messages", queue_id, *message_count);
		return 0;
	}
}

/******************************************************************************/
/*	Function Name	:	flush_message_queue                                   */
/*                                                                            */ 
/*  Input			:	int queue_id										  */ 
/*                                                                            */ 
/*  Output			:	Flushes all messages from the queue                   */ 
/*                                                                            */ 
/*	Description		:	Reads and discards all messages in the queue. If no   */
/*                      messages are available, returns 0. Logs errors on     */
/*                      failure.                                              */ 
/******************************************************************************/
int flush_message_queue(int queue_id) 
{
	char temp_buffer[3072];
	long buffer_length = sizeof(temp_buffer);

	msgq_exit_flag = CONTINUE_READING;

	while (msgq_exit_flag == CONTINUE_READING) {
		if (msgrcv(queue_id, &temp_buffer, buffer_length, 0, IPC_NOWAIT) == -1) {
			if (errno == ENOMSG) {
				log_info("MessageQueue", "Message queue ID: %d is empty", queue_id);
				return 0;  // No messages to read
			} else if (errno != EINTR) {
				log_error("MessageQueue", "Failed to flush message queue: %s", strerror(errno));
				return -1;
			}
		}
	}

	log_error("MessageQueue", "Failed to flush message queue due to user signal");
	return -1;
}

/******************************************************************************/
/*	Function Name	:	set_queue_exit_flag                                   */
/*                                                                            */ 
/*	Description		:	Sets the flag to indicate that the queue reading      */
/*                      should be stopped due to a user signal.               */ 
/******************************************************************************/
int set_queue_exit_flag() 
{
	msgq_exit_flag = STOP_READING;
	log_info("MessageQueue", "Message queue exit flag set");
}
