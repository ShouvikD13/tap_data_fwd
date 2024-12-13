/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Config.cpp
 * Author: 33866
 * 
 * Created on 14 February, 2019, 2:44 PM
 */


#include <fstream>
#include <sstream>
#include "Config.h"

Config::Config(const std::string filename) {
    s_filename = filename;
}

/*Config::Config(const Config& orig) {
}
 */

Config::~Config() {
}

void Config::read_config(){
    std::ifstream ifs;
    std::string s_line,s_tmp_key,s_tmp_val;
    
    ifs.open (s_filename, std::ifstream::in);    
    while(std::getline(ifs,s_line)){
        if(s_line.length() == 0 || s_line.at(0) == '#')
        {
            continue;
        }
        std::istringstream is_line(s_line);
        s_tmp_key.clear();
        s_tmp_val.clear();
        
        if( std::getline(is_line, s_tmp_key, '=') )
        {
            if( std::getline(is_line, s_tmp_val) ) 
            {
                config_map[s_tmp_key]= s_tmp_val;
            }
        }
        
    }
    ifs.close();

}

bool Config::get_string_val(std::string s_key, std::string& s_val)
{
    if(config_map.count(s_key) == 1)
    {
        s_val = config_map[s_key];
        return true;
    }
    return false;
}

bool Config::get_int_val(std::string s_key, int& i_val) 
{
    if(config_map.count(s_key) == 1)
    {
        bool b_ret_val;
        try{
            i_val = std::stoi(config_map[s_key]);
            b_ret_val = true;
        }
        catch(std::exception e){
            b_ret_val = false;
        }
        return b_ret_val;
    }
    return false;
}

bool Config::get_short_val(std::string s_key, short& si_val) 
{
    if(config_map.count(s_key) == 1)
    {
        bool b_ret_val;
        try{
            si_val = boost::lexical_cast<short>(config_map[s_key]);
            b_ret_val = true;
        }
        catch(std::exception e){
            b_ret_val = false;
        }
        return b_ret_val;
    }
    return false;
}

boost::log::trivial::severity_level Config::get_logging_lvl(std::string s_key)
{
    std::string s_val;
    if(config_map.count(s_key) == 1)
    {
        s_val = config_map[s_key];
        if(s_val == "TRACE")
        {
            return boost::log::trivial::trace;
        }
        else if(s_val == "DEBUG" )
        {
            return boost::log::trivial::debug;
        }
        else if(s_val == "INFO" )
        {
            return boost::log::trivial::info;
        }
        else if(s_val == "WARNING")
        {
            return boost::log::trivial::warning;
        }
        else if(s_val == "ERROR")
        {
            return boost::log::trivial::error;
        }
        else if(s_val == "FATAL")
        {
            return boost::log::trivial::fatal;
        }else /* default will be set to INFO */
        {
            return boost::log::trivial::info;
        }
    }
    return boost::log::trivial::info; /* default will be set to INFO */
}