#ifndef SCREEN_RECODER_H_
#define SCREEN_RECODER_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <cstddef>
#include <sys/shm.h>

class X11DesktopCapture
{
public:
    X11DesktopCapture();

    bool init();

private:
    Display          *m_display = nullptr;
    Window            m_desktop_window = 0;
    XShmSegmentInfo  *m_shm_segment_info = nullptr;
    XImage           *m_x_shm_image = nullptr;
};



#endif