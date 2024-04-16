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
    
    m_screen_size = height*width*window_image->bits_per_pixel/8;

    m_shm_segment_info = new XShmSegmentInfo();
    m_shm_segment_info->shmid = 1;
    
    m_x_shm_image = XShmCreateImage(m_display, DefaultVisual(m_display, 0), window_image->depth, ZPixmap, NULL, m_shm_segment_info, width,height);
    if(!m_x_shm_image)
    {
        LOG_ERROR << "XShmCreateImage create failed";
        return false;
    }
    XDestroyImage(window_image);

    m_shm_segment_info->shmid = shmget(IPC_PRIVATE, m_x_shm_image->bytes_per_line*m_x_shm_image->height, IPC_CREAT | 0777);
    if(m_shm_segment_info->shmid == -1)
    {
        LOG_ERROR << "shmget error";
        return false;
    }

    // XDestroyImage(m_x_shm_image);

    m_shm_segment_info->shmaddr = m_x_shm_image->data = (char*)shmat(m_shm_segment_info->shmid, 0, 0);
    if(!XShmAttach(m_display,m_shm_segment_info))
    {
        LOG_ERROR<< "xshmattach error";
        return false;
    }

    return true;
}

void X11DesktopCapture::Destory()
{
    if(m_shm_segment_info && m_shm_segment_info->shmid != -1 && m_display)
    {
        if(!XShmDetach(m_display, m_shm_segment_info))
        {
            LOG_ERROR << "XSHmDetach error";
            return;
        }
    }

    if(m_x_shm_image)
    {
        XDestroyImage(m_x_shm_image);
        m_x_shm_image = nullptr;
    }

    if(m_shm_segment_info && m_shm_segment_info->shmaddr != (char*)-1 && m_display)
    {
        shmdt(m_shm_segment_info->shmaddr);
    }

    if(m_shm_segment_info && m_shm_segment_info->shmid != -1 && m_display)
    {
        shmctl(m_shm_segment_info->shmid, IPC_RMID, 0);
    }

    if(m_shm_segment_info)
    {
        m_shm_segment_info->shmaddr = (char*)-1;
        m_shm_segment_info->shmid = -1;
        delete m_shm_segment_info;
        m_shm_segment_info = nullptr;
    }
}

void X11DesktopCapture::capture()
{
    if(!XShmGetImage(m_display, m_desktop_window, m_x_shm_image, 0,0,AllPlanes))
    {
        LOG_ERROR << "X11DesktopCapture::capture error";
        return;
    }

    FramePacket tmp(m_x_shm_image->data,m_screen_size,FramePacket::RGBA);
    tmp.saveToFile("./frame.rgba");

    return;
}

void FramePacket::saveToFile(std::string path)
{
    LOG_INFO << "frame packet size: " << m_size;
    FILE *fp = fopen(path.c_str(), "w");
    size_t written = fwrite(m_data.data(), 1, m_size, fp);
     LOG_INFO << "written size: " << written;
    fclose(fp);
}

