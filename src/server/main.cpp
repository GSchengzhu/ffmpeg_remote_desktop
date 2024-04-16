#include "log/GlobalLog.h"
#include <memory>

int main()
{
    GlobalLog* m_log = GlobalLog::getInstance();
    m_log->init();

    Log_INFO << "this is test for log";
    
    delete m_log;

    return 0;
}