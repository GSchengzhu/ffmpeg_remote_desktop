#include <boost/noncopyable.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>


class GlobalLog:public boost::noncopyable
{
public:
    GlobalLog();

    enum SeverityLevel
{
    Log_Info,
    Log_Notice,
    Log_Debug,
    Log_Warning,
    Log_Error,
    Log_Fatal
    };

private:
};


BOOST_LOG_GLOBAL_LOGGER(MyLogger, boost::log::sources::severity_logger_mt<>())


#define Log_Debug()  BOOST_LOG_SEV(MyLogger::get(),GlobalLog::SeverityLevel::Log_Debug)
#define Log_INFO()  BOOST_LOG_SEV(MyLogger::get(),GlobalLog::SeverityLevel::Log_Info)