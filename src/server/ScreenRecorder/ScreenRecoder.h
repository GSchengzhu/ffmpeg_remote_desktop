#ifndef SCREEN_RECODER_H_
#define SCREEN_RECODER_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <cstddef>
#include <sys/shm.h>
#include <utility>
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

    FramePacket(std::vector<std::vector<unsigned char>> rgb,int width,int height,int depth)
        :m_rgb_data(std::move(rgb)),
         m_width(width),
         m_height(height),
         m_depth(depth)
         
    {
        
    }

    void saveToFile(std::string path);
    void saveToPng(std::string path);
    void saveToJpeg(std::string path);
    // void saveToYuv444(std::string path);
    // void saveToYuv422(std::string path);
    // void saveToYuv420(std::string path);

private:
    int m_width;
    int m_height;
    int m_depth;
    std::vector<std::vector<unsigned char>> m_rgb_data;
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