/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   slidingWindow.h
 * Author: 33866
 *
 * Created on 13 February, 2019, 2:18 PM
 */

#ifndef SLIDINGWINDOW_H
#define SLIDINGWINDOW_H

#include "genLogger.h"
#include <ratio>
#include <chrono>
#include <thread>

#define MAX_SLIDING_WINDOW_SZ 500

using namespace std;
using namespace std::chrono;

class slidingWindow {
public:
    slidingWindow(int max_ords_in_window); // suppose 1000 . 
    slidingWindow(const slidingWindow& orig) = delete;
    virtual ~slidingWindow();
    void validate_slidingwindow();
private:
    steady_clock::time_point clock_start_time_pt; //start time of sliding window.
    milliseconds ml[MAX_SLIDING_WINDOW_SZ]; // array for storing the timestamp of each operation in sliding window.
    //milliseconds tmp_ml;
    int i_idx_pos; // current index poision
    int i_window_sz_milliseconds; // the size of the sliding windows in milisecond.
    int i_max_orders_in_window; //  the maximum numbers of operations allowed within the sliding window
    
};

#endif /* SLIDINGWINDOW_H */

