/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Config.h
 * Author: 33866
 *
 * Created on 14 February, 2019, 2:44 PM
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

class Config {
public:
    Config(const string filename);
    Config(const Config& orig) =  delete;
    void read_config();
    virtual ~Config();
    bool get_string_val(std::string s_key,std::string& s_val);
    bool get_int_val(std::string s_key,int& i_val);
    bool get_short_val(std::string s_key,short& i_val);
    boost::log::trivial::severity_level get_logging_lvl(std::string s_key);
private:
    std::string s_filename;
    std::unordered_map<std::string,std::string> config_map;
     
};



#endif /* CONFIG_H */

