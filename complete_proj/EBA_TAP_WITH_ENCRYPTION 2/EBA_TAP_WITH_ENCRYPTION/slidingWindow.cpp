/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   slidingWindow.cpp
 * Author: 33866
 * 
 * Created on 13 February, 2019, 2:18 PM
 */

#include "slidingWindow.h"
#include "genLogger.h"

slidingWindow::slidingWindow(int max_ords_in_window) :i_max_orders_in_window(max_ords_in_window)
{
    clock_start_time_pt = steady_clock::now(); //this indicates the time of object creation of slidingWindow.
    i_window_sz_milliseconds = 1000; /* window size is 1 sec i.e. 1000 milliseconds */
    LOG_TRACE << "Inside slidingWindow" ;
    i_idx_pos = 0;
    milliseconds tmp_ml = std::chrono::duration_cast<std::chrono::milliseconds>(clock_start_time_pt - clock_start_time_pt);
    for(int i_idx=0;i_idx < MAX_SLIDING_WINDOW_SZ;i_idx++)
    {
        ml[i_idx] = tmp_ml;
    }
    
    LOG_TRACE << "End slidingWindow" ;
}

/*
slidingWindow::slidingWindow(const slidingWindow& orig) {
}
 */

slidingWindow::~slidingWindow() {
}

void slidingWindow::validate_slidingwindow(){
    
    auto current_time_pt = steady_clock::now(); //current time point
    milliseconds tmp_ml = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_pt - clock_start_time_pt);
    //Total time duration

    int i_diff = tmp_ml.count() - ml[i_idx_pos].count(); //  difference in time durations of previous operation and current opearation
    LOG_DEBUG << LOGPATH << "time tmp_ml: " << tmp_ml.count(); 
    if (i_diff < i_window_sz_milliseconds) // i_window_sz_milliseconds -> 1000
    {   // so if difference is less than 1000 ms (1sec) then sleep for "i_window_sz_milliseconds - i_diff"
        int i_sleep = i_window_sz_milliseconds - i_diff;
        LOG_DEBUG << LOGPATH << "sleeping for i_sleep time: " << i_sleep ; 
        std::this_thread::sleep_for(std::chrono::milliseconds(i_sleep));
        current_time_pt = steady_clock::now();
        tmp_ml = std::chrono::duration_cast<std::chrono::milliseconds>(current_time_pt - clock_start_time_pt);
    }
    ml[i_idx_pos] = tmp_ml; // adding the current time in array
    i_idx_pos = (i_idx_pos >= (i_max_orders_in_window - 1))? 0 : ++i_idx_pos ;
    
    // here we are increasing the index-position utill we reachup to "i_max_orders_in_window ". 

    // now the question is what are we achieving from this ?
    // How it is affecting the current operation.

}