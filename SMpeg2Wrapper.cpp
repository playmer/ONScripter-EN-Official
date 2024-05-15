/* -*- C++ -*-
 * 
 *  SMpeg2Wrapper::.cpp - avifile library wrapper class to play AVI video & audio stream
 *
 *  Copyright (c) 2001-2010 Ogapee. All rights reserved.
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

// This file won't be compiled if this flag isn't set, but for completeness sake it's being included here.

#include "SMpeg2Wrapper.h"
#include <SDL_mixer.h>
#include <stdlib.h>
#include <string.h>

#include "ONScripterLabel.h"
#include "Window.h"


/*
SMpeg2Wrapper::~SMpeg2Wrapper()
{
    if (NULL != m_software_renderer) SDL_DestroyRenderer(m_software_renderer);
    //if (NULL != m_render_surface) SDL_FreeSurface(m_render_surface);
    //if (NULL != m_player) Kit_ClosePlayer(m_player);
    //if (NULL != m_source) Kit_CloseSource(m_source);
}


void SMpeg2Wrapper::smpeg_update(void* data, SMPEG_Frame* frame)
{
    SMpeg2Wrapper* self = (SMpeg2Wrapper*)data;
    self->m_frame = frame;
    ++self->m_frame_count;
}


int SMpeg2Wrapper::initialize(ONScripterLabel* onscripterLabel, Window* window, const char* filename, bool audio_open_flag, bool debug_flag)
{
    m_onscripterLabel = onscripterLabel;
    m_window = window;

    // Load file
    unsigned long length = onscripterLabel->script_h.cBR->getFileLength(filename);

    if (length == 0) {
        snprintf(onscripterLabel->script_h.errbuf, MAX_ERRBUF_LEN,
            "couldn't load movie '%s'", filename);
        onscripterLabel->errorAndCont(onscripterLabel->script_h.errbuf);
        return 0;
    }

    unsigned char* movie_buffer = new unsigned char[length];
    onscripterLabel->script_h.cBR->getFile(filename, movie_buffer);

    m_smpeg_file = SMPEG_new_data(movie_buffer, length, &m_info, true);

    SMPEG_enableaudio(m_smpeg_file, true);
    SMPEG_enablevideo(m_smpeg_file, true);
    SMPEG_setvolume(m_smpeg_file, 100);


    SMPEG_setdisplay(m_smpeg_file, smpeg_update, this, m_lock);
    return 0;
}


bool SMpeg2Wrapper::hasAudioStream()
{
    return m_info.has_audio;
}

extern "C" void mixer_callback_external(void* userdata, Uint8 * stream, int len)
{
    SMpeg2Wrapper* self = (SMpeg2Wrapper*)userdata;
    SMpeg2Wrapper::mixer_callback(self, stream, len);
}


void SMpeg2Wrapper::mixer_callback(SMpeg2Wrapper* userdata, Uint8* stream, int length)
{
    SMpeg2Wrapper& self = *userdata;

    SDL_LockMutex(self.audio_data_mutex);

    // Compute how much we can send to SDL_mixer and copy it (do we need to pad it out?)
    size_t size_to_copy = std::min(self.audio_data.size(), static_cast<size_t>(length));
    memcpy(stream, self.audio_data.data(), size_to_copy);

    // Erase what we just copied
    self.audio_data.erase(self.audio_data.begin(), self.audio_data.begin() + size_to_copy);

    SDL_UnlockMutex(self.audio_data_mutex);
}


int SMpeg2Wrapper::playFull(bool click_flag)
{
    SDL_Rect rect;
    int ret = 0;

    SMPEG_play(m_smpeg_file);
    
    if (m_has_audio) {
        Mix_HookMusic(mixer_callback_external, this);
        audio_data_mutex = SDL_CreateMutex();
    }
    
    bool done_flag = false;
    
    while (!done_flag) {
        if (SMPEG_status(m_smpeg_file) == SMPEG_PLAYING) {
            // If we need to loop, then start the playback again.
            if (m_onscripterLabel->movie_loop_flag) {
                SMPEG_play(m_smpeg_file);
            }
            else {
                done_flag = true;
                continue;
            }
        }

        SDL_Event event;
        while (m_window->PollEvents(event)) {
            switch (event.type) {
                case SDL_KEYUP:
                    if (((SDL_KeyboardEvent*)&event)->keysym.sym == SDLK_RETURN ||
                      ((SDL_KeyboardEvent*)&event)->keysym.sym == SDLK_KP_ENTER ||
                      ((SDL_KeyboardEvent*)&event)->keysym.sym == SDLK_SPACE ||
                      ((SDL_KeyboardEvent*)&event)->keysym.sym == SDLK_ESCAPE)
                      done_flag = true;
                    break;
                case SDL_QUIT:
                    ret = 1;
                case SDL_MOUSEBUTTONUP:
                    done_flag = true;
                    break;
                default:
                    break;
            }
        }


        if (m_frame_count > m_current_frame) {
            SDL_Rect rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = m_onscripterLabel->temp_screen_surface->w;
            rect.h = m_onscripterLabel->temp_screen_surface->h;
            display_frame(&rect);
        }
        else {
            SDL_Delay(0);
        }
    }
    
    if (m_has_audio) {
        Mix_HookMusic(NULL, NULL);
        SDL_DestroyMutex(audio_data_mutex);
        audio_data_mutex = NULL;
        audio_data.clear();
    }
    
    return ret;
}

void SMpeg2Wrapper::playFrame(SDL_Rect* dst)
{
//    if ((Kit_GetPlayerState(m_player) == KIT_STOPPED) && m_onscripterLabel->movie_loop_flag) {
//        Kit_PlayerPlay(m_player);
//    }
//
//    // Refresh audio
//    SDL_LockMutex(audio_data_mutex);
//    size_t original_size = audio_data.size();
//    audio_data.resize(audio_data.size() + 32768, 0);
//    int bytes_read = Kit_GetPlayerAudioData(m_player, (unsigned char*)audio_data.data() + original_size, 32768);
//    if (bytes_read > -1) {
//        audio_data.resize(original_size + bytes_read);
//    }
//    else {
//        audio_data.resize(original_size);
//    }
//    SDL_UnlockMutex(audio_data_mutex);
//
//    // Refresh videotexture and render it
//    display_frame(dst);
}

void SMpeg2Wrapper::setVolume(int volume)
{
    Mix_VolumeMusic(volume);
}

void SMpeg2Wrapper::display_frame(SDL_Rect *dst)
{
    SDL_mutexP(m_lock);
    //SDL_assert(m_frame->image_width == texture_width);
    //SDL_assert(m_frame->image_height == texture_height);
    //SDL_UpdateTexture(texture, NULL, m_frame->image, m_frame->image_width);
    m_current_frame = m_frame_count;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(m_frame->image, m_frame->image_width, m_frame->image_height, 0, 0, SDL_PIXELFORMAT_YV12);
    SDL_BlitScaled(surface, NULL, m_onscripterLabel->temp_screen_surface, NULL);
    SDL_FreeSurface(surface);
    SDL_mutexV(m_lock);

    SDL_Rect real_dst_rect = Window::ScaleRectToPixels(m_onscripterLabel->temp_screen_surface, m_onscripterLabel->screen_surface, *dst);
    SDL_SoftStretchLinear(m_onscripterLabel->temp_screen_surface, dst, m_onscripterLabel->screen_surface, &real_dst_rect);
    SDL_UpdateWindowSurfaceRects(m_window->GetWindow(), &real_dst_rect, 1);
}

void SMpeg2Wrapper::play()
{
    SMPEG_play(m_smpeg_file);
    m_paused = false;
}

void SMpeg2Wrapper::pause()
{
    SMPEG_pause(m_smpeg_file);
    m_paused = true;
}

void SMpeg2Wrapper::stopMovie()
{
    SMPEG_stop(m_smpeg_file);
    m_paused = false;
}


MediaState SMpeg2Wrapper::getStatus()
{
    SMPEGstatus status = SMPEG_status(m_smpeg_file);

    if (m_paused) {
        return MediaState::Paused;
    }
    else if (status == SMPEGstatus::SMPEG_PLAYING) {
        return MediaState::Playing;
    }
    else if (status == SMPEGstatus::SMPEG_STOPPED) {
        return MediaState::Stopped;
    }
}

*/
