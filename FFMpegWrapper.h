/* -*- C++ -*-
 * 
 *  FFMpegWrapper.h - avifile library wrapper class to play AVI video & audio stream
 *
 *  Copyright (c) 2001-2008 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FFMPEG_WRAPPER_H__
#define __FFMPEG_WRAPPER_H__

#ifdef USE_AVIFILE

#include <vector>

#include <SDL.h>
#include <SDL_thread.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class Window;

class FFMpegWrapper
{
public:
    enum { AVI_STOP = 0,
           AVI_PLAYING = 1
    };
    FFMpegWrapper() = default;
    ~FFMpegWrapper();

    int initialize(Window* window, const char* filename, bool audio_open_flag, bool debug_flag);

    int play( bool click_flag );

private:
    void display_frame();
    void queue_audio();
    static void mixer_callback(void* userdata, Uint8* stream, int length);

    static void CleanupFormatContext(AVFormatContext** format_context)
    {
      avformat_close_input(format_context);
      avformat_free_context(*format_context);
    }

    template <typename T, void (tFunc)(T**)>
    struct AVWrapper
    {
        AVWrapper() = default;
        ~AVWrapper()
        {
            if (value)
              tFunc(&value);

            value = NULL;
        }

        operator T*()
        {
          return value;
        }

        T* operator->()
        {
            return value;
        }

        T* get()
        {
            return value;
        }

        T** operator&()
        {
            return &value;
        }

        T* value = NULL;
    };

    AVWrapper<AVFormatContext, CleanupFormatContext> format_context;
    int video_id = -1;
    int audio_id = -1;
    double fpsrendering = 0.0;
    AVWrapper<AVCodecContext, avcodec_free_context> m_video_context;
    AVWrapper<AVCodecContext, avcodec_free_context> m_audio_context;
    AVWrapper<AVFrame, av_frame_free> m_video_frame;
    AVWrapper<AVFrame, av_frame_free> m_audio_frame;
    AVWrapper<AVPacket, av_packet_free> m_packet;

    std::vector<uint8_t> audio_data;
    SDL_mutex* audio_data_mutex;

    Window* m_window = NULL;
    SDL_Texture* m_texture = NULL;

    int video_width;
    int video_height;
};

#endif

#endif // __FFMPEG_WRAPPER_H__
