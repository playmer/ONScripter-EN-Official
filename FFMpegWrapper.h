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


#include <vector>

#include <SDL.h>
#include <SDL_thread.h>
#include "kitchensink/kitchensink.h"

class Window;
class ONScripterLabel;
struct SDL_RWOps_AVIOContext;

class FFMpegWrapper
{
public:
    FFMpegWrapper() = default;
    ~FFMpegWrapper();

    int initialize(ONScripterLabel* onscripterLabel, Window* window, const char* filename, bool audio_open_flag, bool debug_flag);

    int play( bool click_flag );

    static void mixer_callback(FFMpegWrapper* userdata, Uint8* stream, int length);

private:
    void display_frame();
    void queue_audio();
    ONScripterLabel* m_onscripterLabel;
    Window* m_window;
    Kit_Source* m_source;
    Kit_Player* m_player;
    SDL_Texture* m_video_texture;

    bool m_has_audio = false;
    bool m_have_not_hit_audio = true;
    bool m_any_audio_left = true;

    //std::vector<uint8_t> scratch_audio_data;
    std::vector<uint8_t> audio_data;
    SDL_mutex* audio_data_mutex;
};


#endif // __FFMPEG_WRAPPER_H__
