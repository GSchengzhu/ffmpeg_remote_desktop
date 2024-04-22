#include "ScreenRecoder.h"
#include "../log/GlobalLog.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <csetjmp>
#include <cstddef>
#include <cstdio>
#include <libpng16/pngconf.h>
#include <string>

#include <libpng16/png.h>
#include <vector>



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
    LOG_INFO << "bits_per_pixel: " << m_x_shm_image->bits_per_pixel;
    LOG_INFO << "depth: " << m_x_shm_image->depth;
    LOG_INFO << "bytes_per_line: " << m_x_shm_image->bytes_per_line;
    
    std::vector<std::vector<unsigned char>> image_data;
    unsigned long red_mask = m_x_shm_image->red_mask;
    unsigned long green_mask = m_x_shm_image->green_mask;
    unsigned long blue_mask = m_x_shm_image->blue_mask;

    for(int y = 0; y < m_x_shm_image->height; y++)
    {
        std::vector<unsigned char> rowData;
        for(int x = 0; x < m_x_shm_image->width; x++)
        {
            unsigned long pixel = XGetPixel(m_x_shm_image, x, y);

            unsigned char red = (pixel & red_mask) >> 16;
            unsigned char green = (pixel & green_mask) >> 8;
            unsigned char blue = (pixel & blue_mask);
            rowData.push_back(red);
            rowData.push_back(green);
            rowData.push_back(blue);
        }
        image_data.push_back(rowData);
    }

    FramePacket test(image_data,m_x_shm_image->width,m_x_shm_image->height,m_x_shm_image->depth);
    // test.saveToPng("test.png");
    return;
}

void FramePacket::saveToFile(std::string path)
{
    FILE *fp = fopen(path.c_str(), "w");
    if(!fp)
    {
        LOG_ERROR << "open file failed";
        return;
    }

    for(auto i = 0; i < m_rgb_data.size(); i++)
    {
        fwrite(m_rgb_data[i].data(), 1, m_rgb_data[i].size(), fp);
    }
    fclose(fp);
}

void FramePacket::saveToPng(std::string path)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;
    
    FILE *fp = fopen(path.c_str(),"wb+");
    if(!fp)
    {
        LOG_ERROR << "fopen error";
        return;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
    {
        LOG_ERROR << "png_create_write_struct failed";
        return;
    }
    
    do {
        
        info_ptr = png_create_info_struct(png_ptr);
        if(!info_ptr)
        {
            LOG_ERROR << "png_create_info_struct failed";
            break;
        }
        
        if(setjmp(png_jmpbuf(png_ptr)))
        {
            LOG_ERROR << "set jmp error";
            break;
        }


        // png_init_io(png_ptr, fp);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, m_width, m_height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png_ptr, info_ptr);
        
        png_bytep row_pointers[m_height];
        
        if(m_height > PNG_UINT_32_MAX/ (sizeof(png_bytep)))
        {
            png_error(png_ptr,"Image is too tall to process to memory");
            break;
        }
        
        // unsigned char * image_ = (unsigned char*)->data;
        for(int k = 0; k < m_height; k++)
        {
            // row_pointers[k] = image_+ k * m_width;
            
            row_pointers[k] = (png_bytep)reinterpret_cast<unsigned char*>(m_rgb_data[k].data());
        }

        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, info_ptr);
    }while (false);
    
    png_destroy_write_struct(&png_ptr, NULL);
    fclose(fp);

}

void FramePacket::saveToJpeg(std::string path)
{
    

}

