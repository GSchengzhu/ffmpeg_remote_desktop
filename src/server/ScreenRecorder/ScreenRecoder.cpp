#include "ScreenRecoder.h"
#include "../log/GlobalLog.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>


bool X11DesktopCapture::init()
{
    m_display = XOpenDisplay(NULL);
    if(!m_display)
    {
        LOG_ERROR << "open display error"; 
        return false;
    }

    m_desktop_window = RootWindow(m_display, 0);
    if(!m_desktop_window)
    {
        LOG_ERROR << "ROOT WINDOW GET FAILED";
        return false;
    }
    
    int height = DisplayHeight(m_display, 0);
    int width = DisplayWidth(m_display, 0);

    XImage *window_image = XGetImage(m_display, m_desktop_window, 0, 0, width, height, AllPlanes, ZPixmap);
    if(!window_image)
    {
        LOG_ERROR << "xgetimage error";
        return false;
    }

    XDestroyImage(window_image);

    m_shm_segment_info = new XShmSegmentInfo();
    m_shm_segment_info->shmid = 1;
    m_x_shm_image = XShmCreateImage(m_display, Visual *, unsigned int, int, char *, XShmSegmentInfo *, unsigned int, unsigned int)


}