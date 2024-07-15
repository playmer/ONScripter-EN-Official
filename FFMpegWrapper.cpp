/* -*- C++ -*-
 * 
 *  FFMpegWrapper::.cpp - avifile library wrapper class to play AVI video & audio stream
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
#ifdef USE_AVIFILE

#include "FFMpegWrapper.h"
#include <SDL_mixer_ext.h>
#include <stdlib.h>
#include <string.h>

#include "ONScripterLabel.h"
#include "Window.h"



FFMpegWrapper::~FFMpegWrapper()
{
    if (NULL != m_software_renderer) SDL_DestroyRenderer(m_software_renderer);
    if (NULL != m_render_surface) SDL_FreeSurface(m_render_surface);
    if (NULL != m_player) Kit_ClosePlayer(m_player);
    if (NULL != m_source) Kit_CloseSource(m_source);
}

int FFMpegWrapper::initialize(ONScripterLabel* onscripterLabel, Window* window, const char* filename, bool audio_open_flag, bool debug_flag)
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
    m_source = Kit_CreateSourceFromRW(SDL_RWFromMem(movie_buffer, length));
    if (m_source == NULL) {
        fprintf(stderr, "Unable to load file '%s': %s\n", filename, Kit_GetError());
        return 1;
    }

    m_player = Kit_CreatePlayer(
        m_source,
        Kit_GetBestSourceStream(m_source, KIT_STREAMTYPE_VIDEO),
        Kit_GetBestSourceStream(m_source, KIT_STREAMTYPE_AUDIO),
        Kit_GetBestSourceStream(m_source, KIT_STREAMTYPE_SUBTITLE),
        0, 0);

    if (m_player == NULL) {
        fprintf(stderr, "Unable to create player: %s\n", Kit_GetError());
        return 1;
    }

    Kit_PlayerInfo pinfo;
    Kit_GetPlayerInfo(m_player, &pinfo);

    // Make sure there is video in the file to play first.
    if (Kit_GetPlayerVideoStream(m_player) == -1) {
        fprintf(stderr, "File contains no video!\n");
        return 1;
    }

    m_has_audio = Kit_GetPlayerAudioStream(m_player) != -1;

    m_onscripterLabel->openAudio(pinfo.audio.output.samplerate, pinfo.audio.output.format, pinfo.audio.output.channels);

    m_render_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        pinfo.video.output.width,
        pinfo.video.output.height,
        m_onscripterLabel->temp_screen_surface->format->BitsPerPixel,
        m_onscripterLabel->temp_screen_surface->format->format);

    m_software_renderer = SDL_CreateSoftwareRenderer(m_render_surface);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    m_video_texture = SDL_CreateTexture(
        m_software_renderer,
        pinfo.video.output.format,
        SDL_TEXTUREACCESS_STATIC,
        pinfo.video.output.width,
        pinfo.video.output.height);

    if (m_video_texture == NULL) {
        fprintf(stderr, "Error while attempting to create a video texture\n");
        return 1;
    }

    return 0;
}


bool FFMpegWrapper::hasAudioStream()
{
    return Kit_GetPlayerAudioStream(m_player) != -1;
}

extern "C" void mixer_callback_external(void* userdata, Uint8 * stream, int len)
{
    FFMpegWrapper& ffmpegWrapper = *(FFMpegWrapper*)userdata;
    ffmpegWrapper.mixer_callback((FFMpegWrapper*)userdata, stream, len);
}


void FFMpegWrapper::mixer_callback(FFMpegWrapper* userdata, Uint8* stream, int length)
{
    FFMpegWrapper& ffmpegWrapper = *(FFMpegWrapper*)userdata;

    SDL_LockMutex(ffmpegWrapper.audio_data_mutex);

    // Compute how much we can send to SDL_mixer and copy it (do we need to pad it out?)
    size_t size_to_copy = std::min(ffmpegWrapper.audio_data.size(), static_cast<size_t>(length));
    memcpy(stream, ffmpegWrapper.audio_data.data(), size_to_copy);

    // Erase what we just copied
    ffmpegWrapper.audio_data.erase(ffmpegWrapper.audio_data.begin(), ffmpegWrapper.audio_data.begin() + size_to_copy);

    SDL_UnlockMutex(ffmpegWrapper.audio_data_mutex);
}


int FFMpegWrapper::playFull(bool click_flag)
{
    SDL_Rect rect;
    int ret = 0;

    Kit_PlayerPlay(m_player);
    
    if (m_has_audio) {
        Mix_HookMusic(mixer_callback_external, this);
        audio_data_mutex = SDL_CreateMutex();
    }
    
    bool done_flag = false;
    
    while (!done_flag) {
        if (Kit_GetPlayerState(m_player) == KIT_STOPPED) {
            // If we need to loop, then start the playback again.
            if (m_onscripterLabel->movie_loop_flag) {
                Kit_PlayerPlay(m_player);
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


        
        // Refresh audio
        SDL_LockMutex(audio_data_mutex);
        size_t original_size = audio_data.size();
        audio_data.resize(audio_data.size() + 32768, 0);
        int bytes_read = Kit_GetPlayerAudioData(m_player, (unsigned char*)audio_data.data() + original_size, 32768);
        if (bytes_read > -1) {
            audio_data.resize(original_size + bytes_read);
        }
        else {
            audio_data.resize(original_size);
        }
        SDL_UnlockMutex(audio_data_mutex);
        
        // Refresh videotexture and render it
        SDL_RenderClear(m_window->GetRenderer());
        display_frame();

        SDL_Delay(1);
    }
    
    if (m_has_audio) {
        Mix_HookMusic(NULL, NULL);
        SDL_DestroyMutex(audio_data_mutex);
        audio_data_mutex = NULL;
        audio_data.clear();
    }
    
    return ret;
}

void FFMpegWrapper::playFrame(SDL_Rect* dst)
{
    if ((Kit_GetPlayerState(m_player) == KIT_STOPPED) && m_onscripterLabel->movie_loop_flag) {
        Kit_PlayerPlay(m_player);
    }

    // Refresh audio
    SDL_LockMutex(audio_data_mutex);
    size_t original_size = audio_data.size();
    audio_data.resize(audio_data.size() + 32768, 0);
    int bytes_read = Kit_GetPlayerAudioData(m_player, (unsigned char*)audio_data.data() + original_size, 32768);
    if (bytes_read > -1) {
        audio_data.resize(original_size + bytes_read);
    }
    else {
        audio_data.resize(original_size);
    }
    SDL_UnlockMutex(audio_data_mutex);

    // Refresh videotexture and render it
    display_frame(dst);
}

void FFMpegWrapper::setVolume(int volume)
{
    Mix_VolumeMusic(volume);
}

void FFMpegWrapper::display_frame(SDL_Rect *dst)
{
    Kit_GetPlayerVideoData(m_player, m_video_texture);

    auto err = SDL_RenderCopy(m_software_renderer, m_video_texture, NULL /*&src_rect*/, dst);
    SDL_RenderPresent(m_software_renderer);

    SDL_BlitScaled(m_render_surface, NULL, m_onscripterLabel->temp_screen_surface, NULL);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = m_onscripterLabel->temp_screen_surface->w;
    rect.h = m_onscripterLabel->temp_screen_surface->h;

    SDL_Rect real_dst_rect = Window::ScaleRectToPixels(m_onscripterLabel->temp_screen_surface, m_onscripterLabel->screen_surface, rect);
    SDL_SoftStretchLinear(m_onscripterLabel->temp_screen_surface, &rect, m_onscripterLabel->screen_surface, &real_dst_rect);
    SDL_UpdateWindowSurfaceRects(m_window->GetWindow(), &real_dst_rect, 1);
}

void FFMpegWrapper::play()
{
    Kit_PlayerPlay(m_player);
}

void FFMpegWrapper::pause()
{
    Kit_PlayerPause(m_player);
}

void FFMpegWrapper::stopMovie()
{
    Kit_PlayerStop(m_player);
}


Kit_PlayerState FFMpegWrapper::getStatus()
{
    return Kit_GetPlayerState(m_player);
}

#endif
