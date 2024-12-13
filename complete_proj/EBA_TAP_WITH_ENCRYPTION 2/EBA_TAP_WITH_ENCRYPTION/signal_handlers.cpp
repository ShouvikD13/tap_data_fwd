/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <csignal>

volatile sig_atomic_t gi_signal_recvd = 0;

void sig_handler(int i_sig)
{
    gi_signal_recvd = i_sig;
}

void set_sig_handler()
{
    signal (SIGINT, SIG_IGN);
    signal (SIGHUP, SIG_IGN);
    signal (SIGTERM, SIG_IGN);
    
    signal (SIGUSR1, sig_handler);
    signal (SIGUSR2, sig_handler);
    
}


