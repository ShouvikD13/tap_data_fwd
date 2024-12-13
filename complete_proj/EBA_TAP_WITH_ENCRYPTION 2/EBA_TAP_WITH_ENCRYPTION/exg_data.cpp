/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   exg_data.cpp
 * Author: 33866
 * 
 * Created on 4 February, 2019, 12:53 PM
 */

#include <sstream>
#include "exg_data.h"

/*
exg_data::exg_data() {
}

exg_data::exg_data(const exg_data& orig) {
}

exg_data::~exg_data() {
}

*/

std::string exg_data::print_hex(int i_buf_len)
{
    if(i_buf_len > 0)
    {
        int i_len = (i_buf_len > MAX_BUF_LEN)? MAX_BUF_LEN:i_buf_len;
        std::stringstream ss;
        for(int i_idx = 0;i_idx < i_buf_len; i_idx++)
        {
            ss << std::hex << c_exg_data[i_idx];
        }
        return ss.str();
    }
    return "";
}