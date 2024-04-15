#include "GlobalLog.h"
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

BOOST_LOG_GLOBAL_LOGGER_INIT(MyLogger, boost::log::sources::severity_logger_mt)

enum SeverityLevel
{
    Log_Info,
    Log_Notice,
    Log_Debug,
    Log_Warning,
    Log_Error,
    Log_Fatal
};

template<typename CharT, typename TraitsT>
inline std::basic_ostream<CharT,TraitsT>& operator<< (std::basic_ostream<CharT,TraitsT>& strm,SeverityLevel lvl)
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

    if(static_cast<int>(lvl) < sizeof(str)/sizeof(*str))
    {
        strm << str[lvl];
    }else {
        strm << static_cast<int>(lvl);
    }

    return strm;
}

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", SeverityLevel)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(uptime, "Uptime", attrs::timer::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(scope, "Scope", attrs::named_scope::value_type)



GlobalLog::GlobalLog()
{
    logging::formatter formatter_ = 
        expr::stream << "[" 
                     << expr::format_date_time(timestamp,"%H:%M:%S")
                     << "]"
                     << expr::if_(expr::has_attr(uptime))
                        [
                            expr::stream << "[" << expr::format_date_time(uptime,"%O:%M:%S") << "]"                           
                        ]
                     << expr::if_(expr::has_attr(scope))
                        [
                            expr::stream << "[" << expr::format_named_scope(scope,keywords::format = "%n") << "]"                           
                        ]
                     << "<" << severity << ">" << expr::message;
    
    logging::add_common_attributes();

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

    file_sink->set_filter(severity >= Log_Warning);
    file_sink->locked_backend()->scan_for_files();
    
    console_sink->set_formatter(formatter_);
    file_sink->set_formatter(formatter_);
    file_sink->locked_backend()->auto_flush(true);
    

    logging::core::get()->add_sink(console_sink);
    logging::core::get()->add_sink(file_sink);

}