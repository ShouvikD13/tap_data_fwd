#ifndef FN_MSGQ_H
#define FN_MSGQ_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/* Macro Definitions */
#define DONT_EXIT 0
#define EXIT 1

/* External variables */
extern int i_break_status_q; 

/* Function Prototypes */

/**
 * @brief Creates a message queue and returns its identifier.
 * @param ptr_i_qid Pointer to store the message queue identifier.
 * @return 0 on success, -1 on failure.
 */
int fn_crt_msg_q(int *ptr_i_qid);

/**
 * @brief Destroys the specified message queue.
 * @param i_qid Message queue identifier.
 * @param ptr_c_ServiceName Service name for error logging.
 * @param ptr_c_err_msg Error message for logging.
 */
void fn_dstry_msg_q(int i_qid, char *ptr_c_ServiceName, char *ptr_c_err_msg);

/**
 * @brief Writes a message to the specified message queue.
 * @param i_qid Message queue identifier.
 * @param ptr_v_msg Pointer to the message data.
 * @param li_len Length of the message.
 * @return 0 on success, -1 on failure.
 */
int fn_write_msg_q(int i_qid, void *ptr_v_msg, long int li_len);

/**
 * @brief Reads a message from the specified message queue.
 * @param i_qid Message queue identifier.
 * @param ptr_q_data Pointer to store the received message.
 * @param ptr_li_len Pointer to store the length of the received message.
 * @return 0 on success, -1 on failure.
 */
int fn_read_msg_q(int i_qid, void *ptr_q_data, long int *ptr_li_len);

/**
 * @brief Gets the number of messages in the specified message queue.
 * @param i_qid Message queue identifier.
 * @param ptr_li_no_msg Pointer to store the number of messages.
 * @return 0 on success, -1 on failure.
 */

int fn_get_no_of_msg(int i_qid, unsigned int *ptr_li_no_msg);

/**
 * @brief Flushes all messages from the specified message queue.
 * @param i_qid Message queue identifier.
 * @param ptr_c_ServiceName Service name for error logging.
 * @param ptr_c_err_msg Error message for logging.
 * @return 0 on success, -1 on failure.
 */
int fn_flush_q(int i_qid);

/**
 * @brief Sets the break status for message queue operations.
 * @return EXIT on signal caught.
 */
int fn_set_break_q(void);

#endif /* FN_MSGQ_H */
