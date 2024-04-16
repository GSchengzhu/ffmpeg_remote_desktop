#include "GlobalLog.h"
#include <boost/log/sources/global_logger_storage.hpp>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>

#include <boost/log/attributes.hpp>
#include <boost/log/attributes/timer.hpp>

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/log/attributes/named_scope.hpp>
#include <memory>
#include <mutex>

BOOST_LOG_GLOBAL_LOGGER_DEFAULT(MyLogger, boost::log::sources::severity_logger_mt<GlobalLog::SeverityLevel>);

// enum SeverityLevel
// {
//     Log_Info,
//     Log_Notice,
//     Log_Debug,
//     Log_Warning,
//     Log_Error,
//     Log_Fatal
// };

template<typename CharT, typename TraitsT>
inline std::basic_ostream<CharT,TraitsT>& operator<< (std::basic_ostream<CharT,TraitsT>& strm,GlobalLog::SeverityLevel lvl)
{
    static const char* const str[] = 
    {
        "Info",
        "Notice",
        "Debug",
        "Warning",
        "Error",
        "Fatal"
    };
    if(static_cast<size_t>(lvl) < sizeof(str)/sizeof(*str))
    {
        strm << str[lvl];
    }else {
        strm << static_cast<size_t>(lvl);
    }
    

    return strm;
}

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(log_severity, "Severity", GlobalLog::SeverityLevel)
BOOST_LOG_ATTRIBUTE_KEYWORD(log_timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(log_uptime, "Uptime", attrs::timer::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(log_scope, "Scope", attrs::named_scope::value_type)

// GlobalLog* GlobalLog::ins = NULL;

// std::shared_ptr<GlobalLog> GlobalLog::ins = NULL;
GlobalLog* GlobalLog::ins = NULL;

void GlobalLog::init()
{
    // logging::formatter formatter_ = 
    //     expr::stream << "[" 
    //                  << expr::format_date_time(timestamp,"%H:%M:%S")
    //                  << "]"
    //                  << expr::if_(expr::has_attr(uptime))
    //                     [
    //                         expr::stream << "[" << expr::format_date_time(uptime,"%O:%M:%S") << "]"                           
    //                     ]
    //                  << expr::if_(expr::has_attr(scope))
    //                     [
    //                         expr::stream << "[" << expr::format_named_scope(scope,keywords::format = "%n") << "]"                           
    //                     ]
    //                  << "<" << severity << ">" << expr::message;

    logging::formatter formatter_ = 
        expr::stream << "[" 
                     << expr::format_date_time(log_timestamp,"%Y-%m-%d %H:%M:%S")
                     << "]"
                    //  << expr::format_date_time(log_uptime,"%O:%M:%S")
                     << expr::format_named_scope(log_scope,keywords::format = "%n")
                     << "<" << log_severity << ">" << expr::message;
    
    logging::add_common_attributes();
    logging::core::get()->add_thread_attribute("Scope",attrs::named_scope());
    logging::core::get()->add_global_attribute("Uptime", attrs::timer());

    auto console_sink = logging::add_console_log();
    auto file_sink = logging::add_file_log(
        keywords::file_name = "%Y-%m-%d_%N.log", //文件名
        keywords::rotation_size = 10*1024*1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0,0,0)
    );

    file_sink->locked_backend()->set_file_collector(
        sinks::file::make_collector(
            keywords::target = "logs",
            keywords::max_size = 50*1024*1024,
            keywords::min_free_space = 100*1024*1024
        )
    );

    // file_sink->set_filter(log_severity >= Log_Warning);
    file_sink->locked_backend()->scan_for_files();
    
    console_sink->set_formatter(formatter_);
    file_sink->set_formatter(formatter_);
    file_sink->locked_backend()->auto_flush(true);
    

    logging::core::get()->add_sink(console_sink);
    logging::core::get()->add_sink(file_sink);


    
    m_init = true;
}

GlobalLog* GlobalLog::getInstance()
{
    static std::once_flag once_;
    std::call_once(once_, [&]{
        ins = new GlobalLog;
        // ins = std::make_shared<GlobalLog>();
    });

    return ins;
}