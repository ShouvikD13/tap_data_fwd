/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   genLogger.h
 * Author: 33866
 *
 * Created on April 6, 2016, 7:02 PM
 */

#ifndef GENLOGGER_H
#define GENLOGGER_H


#define BOOST_LOG_DYN_LINK 1 // necessary when linking the boost_log library dynamically

#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

// the logs are also written to LOGFILE
//#define LOGFILE "logfile.log"

// just log messages with severity >= SEVERITY_THRESHOLD are written
//#define SEVERITY_THRESHOLD logging::trivial::debug
extern boost::log::trivial::severity_level _gbl_severity_threshold;

#define SET_LOG_SEVERITY_THRESHOLD(severity) _gbl_severity_threshold=severity;

// register a global logger
BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)

// just a helper macro used by the macros below - don't use it in your code
//#define LOG(severity) BOOST_LOG_SEV(logger::get(),boost::log::trivial::severity)

// ===== log macros =====
//#define LOG_TRACE   LOG(trace)
        
#define LOG_TRACE   BOOST_LOG_SEV(logger::get(),boost::log::trivial::trace)
#define LOG_DEBUG   BOOST_LOG_SEV(logger::get(),boost::log::trivial::debug)
#define LOG_INFO    BOOST_LOG_SEV(logger::get(),boost::log::trivial::info)
#define LOG_WARNING BOOST_LOG_SEV(logger::get(),boost::log::trivial::warning)
#define LOG_ERROR   BOOST_LOG_SEV(logger::get(),boost::log::trivial::error)
#define LOG_FATAL   BOOST_LOG_SEV(logger::get(),boost::log::trivial::fatal)

#define LOGPATH __FILE__ << ":" << __func__ << ":" << __LINE__ << ":"
#endif /* GENLOGGER_H */

