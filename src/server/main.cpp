#include "log/GlobalLog.h"
#include "ScreenRecorder/ScreenRecoder.h"
#include <memory>

int main()
{
    GlobalLog* m_log = GlobalLog::getInstance();
    m_log->init();

    // Log_INFO << "this is test for log";
    
    X11DesktopCapture capture;
    capture.init();
    capture.capture();

    
    delete m_log;

    return 0;
}