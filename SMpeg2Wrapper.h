/* -*- C++ -*-
 * 
 *  SMpeg2Wrapper.h - avifile library wrapper class to play AVI video & audio stream
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

#ifndef __SMPEG2_WRAPPER_H__
#define __SMPEG2_WRAPPER_H__


#include <vector>

#include <SDL.h>
#include <SDL_thread.h>
#include <smpeg.h>

class Window;
class ONScripterLabel;
struct SDL_RWOps_AVIOContext;

enum MediaState {
    Playing,
    Paused,
    Stopped,
    Closed
};

class SMpeg2Wrapper
{
public:
    SMpeg2Wrapper() = default;
    ~SMpeg2Wrapper();

    int initialize(ONScripterLabel* onscripterLabel, Window* window, const char* filename, bool audio_open_flag, bool debug_flag);

    int playFull( bool click_flag );
    void play();
    void pause();
    void stopMovie();
    void setVolume(int volume);

    void playFrame(SDL_Rect* dst);

    bool hasAudioStream();

    MediaState getStatus();

    static void mixer_callback(SMpeg2Wrapper* userdata, Uint8* stream, int length);

private:
    void display_frame(SDL_Rect* dst = NULL);
    void queue_audio();
    ONScripterLabel* m_onscripterLabel;
    Window* m_window;
    //SDL_Texture* m_video_texture = NULL;
    //SDL_Surface* m_render_surface = NULL;
    SDL_Renderer* m_software_renderer = NULL;
    SMPEG_Info m_info;
    SMPEG* m_smpeg_file = NULL;
    bool m_paused = false;
    

    static void smpeg_update(void* data, SMPEG_Frame* frame);

    SMPEG_Frame* m_frame;
    int m_frame_count;
    SDL_mutex* m_lock;

    int m_current_frame = 0;

    bool m_has_audio = false;
    bool m_have_not_hit_audio = true;
    bool m_any_audio_left = true;

    std::vector<uint8_t> audio_data;
    SDL_mutex* audio_data_mutex;
};

#endif // __SMPEG2_WRAPPER_H__
