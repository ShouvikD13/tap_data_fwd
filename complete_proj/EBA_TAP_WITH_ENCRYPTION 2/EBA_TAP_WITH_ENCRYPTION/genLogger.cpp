/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "genLogger.h"

#include <boost/log/core/core.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <ostream>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", logging::trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(threadid, "ThreadID", attrs::current_thread_id::value_type)

logging::trivial::severity_level _gbl_severity_threshold = logging::trivial::debug;

BOOST_LOG_GLOBAL_LOGGER_INIT(logger, src::severity_logger_mt<boost::log::trivial::severity_level>) {
    src::severity_logger_mt<boost::log::trivial::severity_level> logger;

    // add attributes
    logger.add_attribute("LineID", attrs::counter<unsigned int>(1));     // lines are sequentially numbered
    
    logger.add_attribute("TimeStamp", attrs::local_clock());             // each log line gets a timestamp
    
    logger.add_attribute("ThreadID", attrs::current_thread_id());
    
    logging::add_file_log
    (
        keywords::file_name = "EBATAP%Y%m%d_%H%M%S_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0) ,
        keywords::auto_flush = true,
        keywords::format = /*"[%TimeStamp%]: %Message%"*/ (
          expr::stream
        << std::setw(7) << std::setfill('0') << line_id << /* std::setfill(' ') << */ "|"
        << expr::format_date_time(timestamp, "%Y%m%d%H%M%S.%f") << " "
        << "[" << threadid << ":" << logging::trivial::severity << "]"
        << " - " << expr::smessage
            ) /* ,
            keywords::filter = (logging::trivial::severity >= logging::trivial::info) */
   
    );
    
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= _gbl_severity_threshold
    );
    
    // add a text sink
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

    // add a logfile stream to our sink
    //sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>(LOGFILE));

    // add "console" output stream to our sink
    sink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

    // specify the format of the log message
    logging::formatter formatter = expr::stream
        << std::setw(7) << std::setfill('0') << line_id << /* std::setfill(' ') << */ "|"
        << expr::format_date_time(timestamp, "%Y%m%d%H%M%S.%f") << " "
        << "[" << threadid << ":" << logging::trivial::severity << "]"
        << " - " << expr::smessage;
    
    sink->set_formatter(formatter);

    // just log messages with severity >= SEVERITY_THRESHOLD are written
    //sink->set_filter(severity != SEVERITY_THRESHOLD);

    // "register" our sink
    logging::core::get()->add_sink(sink);

    return logger;
}