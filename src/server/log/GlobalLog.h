#include <boost/log/core/core.hpp>
#include <boost/noncopyable.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <cstdio>
#include <memory>

class GlobalLog:public boost::noncopyable
{
public:
    // static std::shared_ptr<GlobalLog> getInstance();
    static GlobalLog* getInstance();
    enum SeverityLevel
    {
        Log_Info,
        Log_Notice,
        Log_Debug,
        Log_Warning,
        Log_Error,
        Log_Fatal
    };

    ~GlobalLog()
    {
        if(m_init)
        {
            boost::log::core::get()->remove_all_sinks();
            m_init = true;
            printf("destory\n");
        }
    }

    void init();

private:
    GlobalLog(){ m_init = false; }


private:
    bool m_init;
    // static std::shared_ptr<GlobalLog> ins;
    static GlobalLog* ins;
};

#include <boost/log/common.hpp>
BOOST_LOG_GLOBAL_LOGGER(MyLogger, boost::log::sources::severity_logger_mt<GlobalLog::SeverityLevel>)


#define LOG_DEBUG     BOOST_LOG_SEV(MyLogger::get(),GlobalLog::Log_Debug)
#define LOG_INFO      BOOST_LOG_SEV(MyLogger::get(),GlobalLog::Log_Info)
#define LOG_WARNING   BOOST_LOG_SEV(MyLogger::get(),GlobalLog::Log_Warning)
#define LOG_ERROR     BOOST_LOG_SEV(MyLogger::get(),GlobalLog::Log_Error)