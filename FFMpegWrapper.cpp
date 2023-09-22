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



#include "FFMpegWrapper.h"
#include <SDL_mixer.h>
#include <stdlib.h>
#include <string.h>

#include "ONScripterLabel.h"
#include "Window.h"



FFMpegWrapper::~FFMpegWrapper()
{
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

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    m_video_texture = SDL_CreateTexture(
        window->GetRenderer(),
        pinfo.video.output.format,
        SDL_TEXTUREACCESS_STATIC,
        pinfo.video.output.width,
        pinfo.video.output.height);

    if (m_video_texture == NULL) {
        fprintf(stderr, "Error while attempting to create a video texture\n");
        return 1;
    }

    m_onscripterLabel->openAudio(pinfo.audio.output.samplerate, pinfo.audio.output.format, pinfo.audio.output.channels);




    return 0;
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


int FFMpegWrapper::play(bool click_flag)
{
    SDL_Rect rect;
    int ret = 0;

    Kit_PlayerPlay(m_player);
    
    if (m_has_audio)
    {
        Mix_HookMusic(mixer_callback_external, this);
        audio_data_mutex = SDL_CreateMutex();
    }
    
    bool done_flag = false;
    
    while (!done_flag)
    {
        if (Kit_GetPlayerState(m_player) == KIT_STOPPED) {
            done_flag = true;
            continue;
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
    //
    //    if (m_packet->stream_index == video_id) {
    //        display_frame();
    //    }
    //    else if (m_packet->stream_index == audio_id) {
    //        queue_audio();
    //    }
    //    av_packet_unref(m_packet);


        //if (m_have_not_hit_audio || m_any_audio_left)
        {
            SDL_LockMutex(audio_data_mutex);
            size_t original_size = audio_data.size();
            audio_data.resize(audio_data.size() + 32768, 0);
            int bytes_read = Kit_GetPlayerAudioData(m_player, (unsigned char*)audio_data.data() + original_size, 32768);
            if (bytes_read > -1)
            {
                audio_data.resize(original_size + bytes_read);
            }
            else {
                audio_data.resize(original_size);
            }
            SDL_UnlockMutex(audio_data_mutex);
        }

        // Refresh audio
        // 
        //int queued = SDL_GetQueuedAudioSize(audio_dev);
        //if (queued < AUDIOBUFFER_SIZE) {
        //    int need = AUDIOBUFFER_SIZE - queued;
        //
        //    while (need > 0) {
        //        ret = Kit_GetPlayerAudioData(
        //            m_player,
        //            (unsigned char*)audiobuf,
        //            AUDIOBUFFER_SIZE);
        //        need -= ret;
        //        if (ret > 0) {
        //            SDL_QueueAudio(audio_dev, audiobuf, ret);
        //        }
        //        else {
        //            break;
        //        }
        //    }
        //    // If we now have data, start playback (again)
        //    if (SDL_GetQueuedAudioSize(audio_dev) > 0) {
        //        SDL_PauseAudioDevice(audio_dev, 0);
        //    }
        //}

        // Refresh videotexture and render it
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

void FFMpegWrapper::display_frame()
{
    Kit_GetPlayerVideoData(m_player, m_video_texture);
    SDL_RenderClear(m_window->GetRenderer());
    m_onscripterLabel->DisplayTexture(m_video_texture);
    //time_t start = time(NULL);
    //if (avcodec_send_packet(m_video_context, m_packet) < 0) {
    //    printf("send packet\n");
    //    return;
    //}
    //if (avcodec_receive_frame(m_video_context, m_video_frame) < 0) {
    //    printf("receive frame\n");
    //    return;
    //}
    //
    //SDL_Rect rect = { 0, 0, video_width, video_height };
    //
    //SDL_UpdateYUVTexture(m_texture, &rect,
    //    m_video_frame->data[0], m_video_frame->linesize[0],
    //    m_video_frame->data[1], m_video_frame->linesize[1],
    //    m_video_frame->data[2], m_video_frame->linesize[2]);
    //SDL_RenderClear(m_window->GetRenderer());
    //
    //m_onscripterLabel->DisplayTexture(m_texture);
    //
    //time_t end = time(NULL);
    //double diffms = difftime(end, start) / 1000.0;
    //if (diffms < fpsrendering) {
    //    uint32_t diff = (uint32_t)((fpsrendering - diffms) * 1000);
    //    //printf("diffms: %f, delay time %d ms.\n", diffms, diff);
    //    SDL_Delay(diff);
    //}
}
