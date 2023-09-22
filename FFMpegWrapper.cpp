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


#ifdef USE_AVIFILE

#include "FFMpegWrapper.h"
#include <SDL_mixer.h>
#include <stdlib.h>
#include <string.h>

#include "ONScripterLabel.h"
#include "Window.h"

// Referenced https://gist.github.com/mashingan/e94d41e21c6e0ce4c9f19cea72a57dc4


// FIXME: "We'll need to implement something like this https://stackoverflow.com/a/20610535 when we load from archives"
// 
//class my_iocontext_private
//{
//private:
//    my_iocontext_private(my_iocontext_private const &);
//    my_iocontext_private& operator = (my_iocontext_private const &);
//
//public:
//    my_iocontext_private(IInputStreamPtr inputStream)
//       : inputStream_(inputStream)
//       , buffer_size_(kBufferSize)
//       , buffer_(static_cast<unsigned char*>(::av_malloc(buffer_size_))) {
//        ctx_ = ::avio_alloc_context(buffer_, buffer_size_, 0, this,
//          &my_iocontext_private::read, NULL, &my_iocontext_private::seek); 
//    }
//
//    ~my_iocontext_private() { 
//        ::av_free(ctx_);
//        ::av_free(buffer_); 
//    }
//
//    void reset_inner_context() { ctx_ = NULL; buffer_ = NULL; }
//
//    static int read(void *opaque, unsigned char *buf, int buf_size) {
//        my_iocontext_private* h = static_cast<my_iocontext_private*>(opaque);
//        return h->inputStream_->Read(buf, buf_size); 
//    }
//
//    static int64_t seek(void *opaque, int64_t offset, int whence) {
//        my_iocontext_private* h = static_cast<my_iocontext_private*>(opaque);
//
//        if (0x10000 == whence)
//            return h->inputStream_->Size();
//
//        return h->inputStream_->Seek(offset, whence); 
//    }
//
//    ::AVIOContext *get_avio() { return ctx_; }
//
//private:
//    IInputStreamPtr inputStream_; // abstract stream interface, You can adapt it to TMemoryStream  
//    int buffer_size_;
//    unsigned char * buffer_;  
//    ::AVIOContext * ctx_;
//};

FFMpegWrapper::~FFMpegWrapper()
{
    SDL_DestroyTexture(m_texture);
}

int FFMpegWrapper::initialize(ONScripterLabel* onscripterLabel, Window* window, const char* filename, bool audio_open_flag, bool debug_flag)
{
    m_onscripterLabel = onscripterLabel;
    m_window = window;

    format_context.value = avformat_alloc_context();

    char bufmsg[1024];
    if (avformat_open_input(&format_context, filename, NULL, NULL) < 0) {
        printf("Cannot open %s", filename);
        return 1;
    }

    if (avformat_find_stream_info(format_context, NULL) < 0) {
        printf("Cannot find stream info %s. Quitting.\n", filename);
        return 1;
    }

    const AVCodec* video_codec = NULL;
    const AVCodec* audio_codec = NULL;
    AVCodecParameters* video_parameters = NULL;
    AVCodecParameters* audio_parameters = NULL;
    for (int i = 0; i < format_context->nb_streams; i++) {
        AVCodecParameters* localparam = format_context->streams[i]->codecpar;
        const AVCodec* localcodec = avcodec_find_decoder(localparam->codec_id);
        if (localparam->codec_type == AVMEDIA_TYPE_VIDEO && !video_codec) {
            video_codec = localcodec;
            video_parameters = localparam;
            video_id = i;
            AVRational rational = format_context->streams[i]->avg_frame_rate;
            fpsrendering = 1.0 / ((double)rational.num / (double)(rational.den));
        }
        else if (localparam->codec_type == AVMEDIA_TYPE_AUDIO && !audio_codec) {
            audio_codec = localcodec;
            audio_parameters = localparam;
            audio_id = i;
        }
        if (video_codec && audio_codec) { break; }
    }

    if (video_parameters == NULL) {
        fprintf(stderr, "Couldn't find a set of video_parameters when opening video, exiting");
        return 2;
    }

    m_video_context.value = avcodec_alloc_context3(video_codec);
    m_audio_context.value = avcodec_alloc_context3(audio_codec);

    if (avcodec_parameters_to_context(m_video_context, video_parameters) < 0) {
        printf("Error at: avcodec_parameters_to_context(m_video_context, video_parameters)\n");
        return 2;
    }
    if (avcodec_parameters_to_context(m_audio_context, audio_parameters) < 0) {
        printf("Error at: avcodec_parameters_to_context(m_audio_context, audio_parameters)\n");
        return 2;
    }
    if (avcodec_open2(m_video_context, video_codec, NULL) < 0) {
        printf("Error at: avcodec_open2(m_video_context, video_codec, NULL)\n");
        return 2;
    }
    if (avcodec_open2(m_audio_context, audio_codec, NULL) < 0) {
        printf("Error at: avcodec_open2(m_audio_context, audio_codec, NULL)\n");
        return 2;
    }

    if (m_audio_context)
    {
        // FIXME: This will hurt your ears currently, something about this setup produces bad audio.
        //m_onscripterLabel->openAudio(audio_parameters->bit_rate, MIX_DEFAULT_FORMAT, audio_parameters->channels);
    }

    m_video_frame.value = av_frame_alloc();
    m_audio_frame.value = av_frame_alloc();
    m_packet.value = av_packet_alloc();
    video_width = video_parameters->width;
    video_height = video_parameters->height;

    m_texture = SDL_CreateTexture(m_window->GetRenderer(), SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_width, video_height);

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

void FFMpegWrapper::queue_audio()
{
    if (avcodec_send_packet(m_audio_context, m_packet) < 0) {
        printf("send packet\n");
        return;
    }
    if (avcodec_receive_frame(m_audio_context, m_audio_frame) < 0) {
        printf("receive frame\n");
        return;
    }
    int size;
    int bufsize = av_samples_get_buffer_size(&size, m_audio_context->channels, m_audio_frame->nb_samples, static_cast<AVSampleFormat>(m_audio_frame->format), 0);
    bool isplanar = av_sample_fmt_is_planar(static_cast<AVSampleFormat>(m_audio_frame->format)) == 1;
    for (int ch = 0; ch < m_audio_context->channels; ch++) {
        SDL_LockMutex(audio_data_mutex);
        if (!isplanar) {
            const uint8_t* data_start = m_audio_frame->data[ch];
            audio_data.insert(audio_data.end(), data_start, data_start + m_audio_frame->linesize[ch]);
        }
        else {
            const uint8_t* data_start = m_audio_frame->data[0] + size * ch;
            audio_data.insert(audio_data.end(), data_start, data_start + size);
        }
        SDL_UnlockMutex(audio_data_mutex);
    }
}

int FFMpegWrapper::play(bool click_flag)
{
    SDL_Rect rect;
    int ret = 0;

    if (m_audio_context)
    {
        Mix_HookMusic(mixer_callback_external, this);
        audio_data_mutex = SDL_CreateMutex();
    }

    bool done_flag = false;

    while ((av_read_frame(format_context, m_packet) >= 0) && (ret != 1))
    {
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

        if (m_packet->stream_index == video_id) {
            display_frame();
        }
        else if (m_packet->stream_index == audio_id) {
            queue_audio();
        }
        av_packet_unref(m_packet);
    }

    if (m_audio_context)
    {
        Mix_HookMusic(NULL, NULL);
        SDL_DestroyMutex(audio_data_mutex);
        audio_data_mutex = NULL;
        audio_data.clear();
    }

    return ret;
}

void FFMpegWrapper::display_frame()
{
    time_t start = time(NULL);
    if (avcodec_send_packet(m_video_context, m_packet) < 0) {
        printf("send packet\n");
        return;
    }
    if (avcodec_receive_frame(m_video_context, m_video_frame) < 0) {
        printf("receive frame\n");
        return;
    }

    SDL_Rect rect = { 0, 0, video_width, video_height };
    
    SDL_UpdateYUVTexture(m_texture, &rect,
        m_video_frame->data[0], m_video_frame->linesize[0],
        m_video_frame->data[1], m_video_frame->linesize[1],
        m_video_frame->data[2], m_video_frame->linesize[2]);
    SDL_RenderClear(m_window->GetRenderer());

    m_onscripterLabel->DisplayTexture(m_texture);
    
    time_t end = time(NULL);
    double diffms = difftime(end, start) / 1000.0;
    if (diffms < fpsrendering) {
        uint32_t diff = (uint32_t)((fpsrendering - diffms) * 1000);
        //printf("diffms: %f, delay time %d ms.\n", diffms, diff);
        SDL_Delay(diff);
    }
}

#endif
