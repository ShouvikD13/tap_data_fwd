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

struct message {
    long message_type;  

    struct msg_struct_for_board_lot_in {
        int64_t li_msg;
        char data[338];
    } board_lot_in;  

    struct msg_struct_for_log_on {
        int64_t li_msg;
        char data[300];
    } log_on;  

    struct msg_struct_for_log_off {
        int64_t li_msg;
        char data[62];
    } log_off;  
};

/* Function Prototypes */


int fn_crt_msg_q(int *ptr_i_qid);

void fn_dstry_msg_q(int i_qid);

int fn_write_msg_q(int i_qid, void *ptr_v_msg, long int li_len);

int fn_read_msg_q(int i_qid, void *ptr_q_data, long int *ptr_li_len);

int fn_get_no_of_msg(int i_qid, unsigned int *ptr_li_no_msg);

int fn_flush_q(int i_qid);


#endif /* FN_MSGQ_H */
