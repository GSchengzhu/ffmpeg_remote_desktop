#ifndef SCREEN_RECODER_H_
#define SCREEN_RECODER_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <cstddef>
#include <sys/shm.h>
#include <vector>
#include <string>


class FramePacket
{
public:
    enum FrameType
    {
        RGBA = 0,
        YUV444,
        YUV422,
        YUV420
    };

    FramePacket(char* begin,int byte_size,FramePacket::FrameType type)
        :m_data(begin,begin+byte_size),
         m_size(byte_size),
         m_type(type)
    {

    }

    void saveToFile(std::string path);

    // void 

private:
    int m_size;
    FrameType m_type;
    std::vector<unsigned char> m_data;
};


class X11DesktopCapture
{
public:
    X11DesktopCapture() = default;
    ~X11DesktopCapture()
    {
        Destory();
    }
    void capture();
    bool init();

private:
    void Destory();

private:
    Display          *m_display = nullptr;
    Window            m_desktop_window = 0;
    XShmSegmentInfo  *m_shm_segment_info = nullptr;
    XImage           *m_x_shm_image = nullptr;
    int               m_screen_size = 0;
};



#endif