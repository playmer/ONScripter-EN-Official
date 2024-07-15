/* -*- C++ -*-
 *
 *  ONScripterLabel_sound.cpp - Methods for playing sound for ONScripter-EN
 *
 *  Copyright (c) 2001-2011 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2008-2011 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
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
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Modified by Haeleth, Autumn 2006, to better support OS X/Linux packaging.

// Modified by Mion, April 2009, to update from
// Ogapee's 20090331 release source code.

// Modified by Mion, November 2009, to update from
// Ogapee's 20091115 release source code.

#include "ONScripterLabel.h"
#include <new>
#ifdef LINUX
#include <signal.h>
#endif

#ifdef USE_AVIFILE
//#include "AVIWrapper.h"
#include "FFMpegWrapper.h"
#endif

extern "C" void waveCallback(int channel);

struct WAVE_HEADER{
    char chunk_riff[4];
    char riff_length[4];
    // format chunk
    char fmt_id[8];
    char fmt_size[4];
    char data_fmt[2];
    char channels[2];
    char frequency[4];
    char byte_size[4];
    char sample_byte_size[2];
    char sample_bit_size[2];
} header;
struct WAVE_DATA_HEADER{
    // data chunk
    char chunk_id[4];
    char data_length[4];
} data_header;
static void setupWaveHeader( unsigned char *buffer, int channels, int bits,
                             unsigned long rate, unsigned long data_length,
                             unsigned int extra_bytes=0, unsigned char *extra_ptr=NULL );

extern bool ext_music_play_once_flag;

extern "C"{
    extern void mp3callback( void *userdata, Uint8 *stream, int len );
    extern void oggcallback( void *userdata, Uint8 *stream, int len );
    extern Uint32 SDLCALL cdaudioCallback( Uint32 interval, void *param );
    extern Uint32 SDLCALL silentmovieCallback( Uint32 interval, void *param );
#if defined(MACOSX) //insani
    extern Uint32 SDLCALL seqmusicSDLCallback( Uint32 interval, void *param );
#endif
}
extern void seqmusicCallback( int sig );
extern void musicCallback( int sig );
extern SDL_TimerID timer_cdaudio_id;
extern SDL_TimerID timer_silentmovie_id;

#if defined(MACOSX) //insani
extern SDL_TimerID timer_seqmusic_id;
#endif

#define TMP_SEQMUSIC_FILE "tmp.mus"
#define TMP_MUSIC_FILE "tmp.mus"

#define SWAP_SHORT_BYTES(sptr){          \
            Uint8 *bptr = (Uint8 *)sptr; \
            Uint8 tmpb = *bptr;          \
            *bptr = *(bptr+1);           \
            *(bptr+1) = tmpb;            \
        }

//WMA header format
#define IS_ASF_HDR(buf)                           \
         ((buf[0] == 0x30) && (buf[1] == 0x26) && \
          (buf[2] == 0xb2) && (buf[3] == 0x75) && \
          (buf[4] == 0x8e) && (buf[5] == 0x66) && \
          (buf[6] == 0xcf) && (buf[7] == 0x11))

//AVI header format
#define IS_AVI_HDR(buf)                         \
         ((buf[0] == 'R') && (buf[1] == 'I') && \
          (buf[2] == 'F') && (buf[3] == 'F') && \
          (buf[8] == 'A') && (buf[9] == 'V') && \
          (buf[10] == 'I'))

//MIDI header format
#define IS_MIDI_HDR(buf)                         \
         ((buf[0] == 'M') && (buf[1] == 'T') && \
          (buf[2] == 'h') && (buf[3] == 'd') && \
          (buf[4] == 0)  && (buf[5] == 0) && \
          (buf[6] == 0)  && (buf[7] == 6))


Mix_MusicType detect_music_type(SDL_RWops* src);



int ONScripterLabel::playSound(const char *filename, int format, bool loop_flag, int channel)
{
    printf("%s\n", filename);

    if ( !audio_open_flag ) return SOUND_NONE;

    long length = script_h.cBR->getFileLength( filename );
    if (length == 0) return SOUND_NONE;

    //Mion: account for mode_wave_demo setting
    //(i.e. if not set, then don't play non-bgm wave/ogg during skip mode)
    if (!mode_wave_demo_flag &&
        ( (skip_mode & SKIP_NORMAL) || ctrl_pressed_status )) {
        if ((format & (SOUND_OGG | SOUND_WAVE)) &&
            ((channel < ONS_MIX_CHANNELS) || (channel == MIX_WAVE_CHANNEL) ||
             (channel == MIX_CLICKVOICE_CHANNEL)))
            return SOUND_NONE;
    }

    sound_buffer.resize(length);
    script_h.cBR->getFile(filename, sound_buffer.data());
    SDL_RWops* reader = SDL_RWFromMem(sound_buffer.data(), sound_buffer.size());
    Mix_MusicType type = detect_music_type(reader);

    /* check for WMA (i.e. ASF header format) */
    if (IS_ASF_HDR(sound_buffer.data())) {
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
            "sound file '%s' is in WMA format, skipping", filename);
        errorAndCont(script_h.errbuf);
        return SOUND_OTHER;
    }
    /* check for AVI header format */
    else if (IS_AVI_HDR(sound_buffer.data())) {
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
            "sound file '%s' is in AVI format, skipping", filename);
        errorAndCont(script_h.errbuf);
        return SOUND_OTHER;
    }
    /* check for OGG */
    else if ((type == MUS_OGG) && (format & (SOUND_OGG | SOUND_OGG_STREAMING))) {
        Mix_Chunk* chunk = Mix_LoadWAV_RW(reader, SDL_TRUE);
        if (chunk == NULL)
        {
            fprintf(stderr, "Issue loading ogg file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        return playWave(chunk, format, loop_flag, channel) == 0 ? SOUND_OGG : SOUND_OTHER;
    }
    /* check for MP3 */
    else if ((format & SOUND_MP3) &&
        !(IS_MIDI_HDR(sound_buffer.data()) && (format & SOUND_SEQMUSIC))) { //bypass MIDIs
        music_info = Mix_LoadMUSType_RW(reader, MUS_MP3, SDL_TRUE);
        if (music_info == NULL)
        {
            fprintf(stderr, "Issue loading mp3 file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        Mix_HaltMusic();
        int ret = Mix_PlayMusic(music_info, loop_flag) == 0 ? SOUND_MP3 : SOUND_OTHER;
        Mix_VolumeMusic(calculateVolume(music_volume));
        return ret;
    }
    else if ((type == MUS_MID) && (format & SOUND_SEQMUSIC)) {
        seqmusic_info = Mix_LoadMUSType_RW(reader, MUS_MID, SDL_TRUE);
        if (seqmusic_info == NULL)
        {
            fprintf(stderr, "Issue loading midi file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        int seqmusic_looping = loop_flag ? -1 : 0;
        Mix_HaltMusic();
        int ret = Mix_PlayMusic(seqmusic_info, seqmusic_looping) == 0 ? SOUND_SEQMUSIC : SOUND_OTHER;
        Mix_VolumeMusic(calculateVolume(music_volume));
        return ret;
    }
    else if (format & SOUND_WAVE) {
        if (strncmp((char*)sound_buffer.data(), "RIFF", 4) != 0) {
            // bad (encrypted?) header; need to recreate
            // assumes the first 128 bytes are bad (encrypted)
            // _and_ that the file contains uncompressed PCM data
            char* fmtname = new char[strlen(filename) + strlen(".fmt") + 1];
            sprintf(fmtname, "%s.fmt", filename);
            unsigned int fmtlen = script_h.cBR->getFileLength(fmtname);
            if (fmtlen >= 8) {
                // a file called filename + ".fmt" exists, of appropriate size;
                // read fmt info
                unsigned char* buffer2 = new unsigned char[fmtlen];
                script_h.cBR->getFile(fmtname, buffer2);

                int channels, bits;
                unsigned long rate = 0, data_length = 0;

                channels = buffer2[0];
                for (int i = 5; i > 1; i--) {
                    rate = (rate << 8) + buffer2[i];
                }
                bits = buffer2[6];
                if (fmtlen >= 12) {
                    // read the data_length
                    for (int i = 11; i > 7; i--) {
                        data_length = (data_length << 8) + buffer2[i];
                    }
                }
                else {
                    // no data_length provided, fake it from the buffer length
                    data_length = length - sizeof(WAVE_HEADER) - sizeof(WAVE_DATA_HEADER);
                }
                unsigned char fill = 0;
                if (bits == 8) fill = 128;
                for (int i = 0; (i < 128 && i < length); i++) {
                    //clear the first 128 bytes (encryption noise)
                    sound_buffer[i] = fill;
                }
                if (fmtlen > 12) {
                    setupWaveHeader(sound_buffer.data(), channels, bits, rate, data_length,
                        fmtlen - 12, buffer2 + 12);
                }
                else {
                    setupWaveHeader(sound_buffer.data(), channels, bits, rate, data_length);
                }
                if ((bits == 8) && (fmtlen < 12)) {
                    //hack: clear likely "pad bytes" at the end of the buffer
                    //      (only on 8-bit samples when the fmt file doesn't
                    //      include the data length)
                    int i = 1;
                    while (i < 5 && sound_buffer[length - i] == 0) {
                        sound_buffer[length - i] = fill;
                        i++;
                    }
                }
                delete[] buffer2;
            }
            delete[] fmtname;
        }
        Mix_Chunk* chunk = Mix_LoadWAV_RW(reader, 1);
        if (playWave(chunk, format, loop_flag, channel) == 0) {
            return SOUND_WAVE;
        }
    }

    return SOUND_OTHER;
}

void ONScripterLabel::playCDAudio()
{
    if (!audio_open_flag) return;

    if ( cdaudio_flag ){
        if ( cdrom_info ){
            int length = cdrom_info->track[current_cd_track - 1].length / 75;
            SDL_CDPlayTracks( cdrom_info, current_cd_track - 1, 0, 1, 0 );
            timer_cdaudio_id = SDL_AddTimer( length * 1000, cdaudioCallback, NULL );
        }
    }
    else{
        //if CD audio is not available, search the "cd" subfolder
        //for a file named "track01.mp3" or similar, depending on the
        //track number; check for mp3, ogg and wav files
        char filename[256];
        sprintf( filename, "cd\\track%2.2d.mp3", current_cd_track );
        int ret = playSound( filename, SOUND_MP3, cd_play_loop_flag, MIX_BGM_CHANNEL);
        if (ret == SOUND_MP3) return;

        sprintf( filename, "cd\\track%2.2d.ogg", current_cd_track );
        ret = playSound( filename, SOUND_OGG, cd_play_loop_flag, MIX_BGM_CHANNEL);
        if (ret == SOUND_OGG) return;

        sprintf( filename, "cd\\track%2.2d.wav", current_cd_track );
        ret = playSound( filename, SOUND_WAVE, cd_play_loop_flag, MIX_BGM_CHANNEL );
    }
}

int ONScripterLabel::calculateVolume(int volume /* between [0, 100]*/)
{
    return !volume_on_flag ? 0 : volume * MIX_MAX_VOLUME / 100;
}

int ONScripterLabel::playWave(Mix_Chunk* chunk, int format, bool loop_flag, int channel)
{
    Mix_HaltChannel(channel);
    if (Mix_Playing(channel) != 0) {
        Mix_ChannelFinished(NULL);
    }
    if (wave_sample[channel]) {
        Mix_FreeChunk(wave_sample[channel]);
    }

    wave_sample[channel] = chunk;

    if (!chunk) return -1;

    if (channel < ONS_MIX_CHANNELS)
        Mix_Volume(channel, calculateVolume(channelvolumes[channel]));
    else if (channel == MIX_CLICKVOICE_CHANNEL)
        Mix_Volume(channel, calculateVolume(se_volume));
    else if (channel == MIX_BGM_CHANNEL)
        Mix_Volume(channel, calculateVolume(music_volume));
    else
        Mix_Volume(channel, calculateVolume(se_volume));

    if (!(format & SOUND_PRELOAD)) {
        Mix_ChannelFinished(waveCallback);
        Mix_PlayChannel(channel, wave_sample[channel], loop_flag ? -1 : 0);
    }

    return 0;
}

int ONScripterLabel::playExternalMusic(bool loop_flag)
{
    int music_looping = loop_flag ? -1 : 0;
#ifdef LINUX
    signal(SIGCHLD, musicCallback);
    if (music_cmd) music_looping = 0;
#endif

    Mix_SetMusicCMD(music_cmd);

    char music_filename[256];
    sprintf(music_filename, "%s%s", script_h.save_path, TMP_MUSIC_FILE);
    if ((music_info = Mix_LoadMUS(music_filename)) == NULL){
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                 "can't load music file %s", music_filename );
        errorAndCont(script_h.errbuf);
        return -1;
    }

    // Mix_VolumeMusic( music_volume );
    Mix_PlayMusic(music_info, music_looping);

    return 0;
}

int ONScripterLabel::playSequencedMusic(bool loop_flag)
{
    Mix_SetMusicCMD(seqmusic_cmd);

    char seqmusic_filename[256];
    sprintf(seqmusic_filename, "%s%s", script_h.save_path, TMP_SEQMUSIC_FILE);
    seqmusic_info = Mix_LoadMUS(seqmusic_filename);
    if (seqmusic_info == NULL) {
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                 "error in sequenced music file %s", seqmusic_filename );
        errorAndCont(script_h.errbuf, Mix_GetError());
        return -1;
    }

    int seqmusic_looping = loop_flag ? -1 : 0;

#ifdef LINUX
    signal(SIGCHLD, seqmusicCallback);
    if (seqmusic_cmd) seqmusic_looping = 0;
#endif

    Mix_VolumeMusic(calculateVolume(music_volume));
#if defined(MACOSX) //insani
    // Emulate looping on MacOS ourselves to work around bug in SDL_Mixer
    seqmusic_looping = 0;
    Mix_PlayMusic(seqmusic_info, seqmusic_looping);
    timer_seqmusic_id = SDL_AddTimer(1000, seqmusicSDLCallback, NULL);
#else
    Mix_PlayMusic(seqmusic_info, seqmusic_looping);
#endif
    current_cd_track = -2;

    return 0;
}

int ONScripterLabel::playingMusic()
{
    if (audio_open_flag && 
        ( (Mix_GetMusicHookData() != NULL) ||
          (Mix_Playing(MIX_BGM_CHANNEL) == 1) ||
          (Mix_PlayingMusic() == 1) ))
        return 1;
    else
        return 0;
}

int ONScripterLabel::setCurMusicVolume( int volume )
{
    if (!audio_open_flag) return 0;

    printf("Music Volume Change: %d\n", volume);

    if (music_struct.voice_sample && *(music_struct.voice_sample))
        volume /= 2;
    if (Mix_GetMusicHookData() != NULL) { // for streamed MP3 & OGG
        music_struct.volume = volume; // ogg
    } else if (Mix_Playing(MIX_BGM_CHANNEL) == 1) { // wave
        Mix_Volume( MIX_BGM_CHANNEL, calculateVolume(volume) );
    } else if (Mix_PlayingMusic() == 1) { // midi
        Mix_VolumeMusic( calculateVolume(volume) );
    }

    return 0;
}

int ONScripterLabel::setVolumeMute( bool do_mute )
{
    if (!audio_open_flag) return 0;

    int music_vol = music_volume;
    if (music_struct.voice_sample && *(music_struct.voice_sample)) //bgmdown
        music_vol /= 2;
    if (Mix_GetMusicHookData() != NULL) { // for streamed MP3 & OGG
#ifdef USE_AVIFILE
        if ( async_movie ) 
            async_movie->setVolume( do_mute? 0 : music_vol ); // async mpeg
        else
#endif
        {
            music_struct.is_mute = do_mute; // ogg
        }
    } else if (Mix_Playing(MIX_BGM_CHANNEL) == 1) { // wave
        Mix_Volume( MIX_BGM_CHANNEL, do_mute? 0 : music_vol * 128 / 100 );
    } else if (Mix_PlayingMusic() == 1) { // midi
        Mix_VolumeMusic( do_mute? 0 : music_vol * 128 / 100 );
    }
    for ( int i=1 ; i<ONS_MIX_CHANNELS ; i++ ) {
        if ( wave_sample[i] )
            Mix_Volume( i, do_mute? 0 : channelvolumes[i] * 128 / 100 );
     }
    if ( wave_sample[MIX_LOOPBGM_CHANNEL0] )
        Mix_Volume( MIX_LOOPBGM_CHANNEL0, do_mute? 0 : se_volume * 128 / 100 );
    if ( wave_sample[MIX_LOOPBGM_CHANNEL1] )
        Mix_Volume( MIX_LOOPBGM_CHANNEL1, do_mute? 0 : se_volume * 128 / 100 );

    return 0;
}

int ONScripterLabel::playMPEG( const char *filename, bool async_flag, bool use_pos, int xpos, int ypos, int width, int height )
{
    int ret = 0;

#if USE_AVIFILE
    if (async_movie) delete async_movie;

    if (audio_open_flag) Mix_CloseAudio();

    FFMpegWrapper* video_player = new FFMpegWrapper();
    if (video_player->initialize(this, m_window, filename, audio_open_flag, false) != 0) {
        delete video_player;
        return 0;
    }

    for (int i = 0; i < 4; ++i) {
        surround_rects[i].x = surround_rects[i].y = 0;
        surround_rects[i].w = surround_rects[i].h = 0;
    }

    if (use_pos) {
        async_movie_rect.x = xpos;
        async_movie_rect.y = ypos;
        async_movie_rect.w = width;
        async_movie_rect.h = height;
        //sur_rect[0] = { 0, 0, screen_width, ypos };
        //sur_rect[1] = { 0, ypos, xpos, height };
        //sur_rect[2] = { xpos + width, ypos, screen_width - (xpos + width), height };
        //sur_rect[3] = { 0, ypos + height, screen_width, screen_height - (ypos + height) };
        surround_rects[0].w = surround_rects[3].w = screen_width;
        surround_rects[0].h = surround_rects[1].y = surround_rects[2].y = ypos;
        surround_rects[1].w = xpos;
        surround_rects[1].h = surround_rects[2].h = height;
        surround_rects[2].x = xpos + width;
        surround_rects[2].w = screen_width - (xpos + width);
        surround_rects[3].y = ypos + height;
        surround_rects[3].h = screen_height - (ypos + height);
    } else {
        async_movie_rect.x = 0;
        async_movie_rect.y = 0;
        async_movie_rect.w = screen_width;
        async_movie_rect.h = screen_height;
    }

    if (async_flag){
        async_movie = video_player;
#ifdef USE_AVIFILE
        async_movie->play();
        if (!async_movie->hasAudioStream() && movie_loop_flag) {
            timer_silentmovie_id = SDL_AddTimer(100, silentmovieCallback, this);
        }
#endif
        return 0;
    }

    video_player->playFull(true);

    if (audio_open_flag) {
        Mix_CloseAudio();
        openAudio();
    }

    delete video_player;
#endif

    return ret;
}

int ONScripterLabel::playAVI( const char *filename, bool click_flag )
{
#ifdef USE_AVIFILE
    if ( audio_open_flag ) Mix_CloseAudio();

    FFMpegWrapper avi;
    if ( avi.initialize(this, m_window, filename, audio_open_flag, false) == 0) {
        if (avi.playFull( click_flag )) return 1;
    }
    //delete[] absolute_filename;

    if ( audio_open_flag ){
        Mix_CloseAudio();
        openAudio();
    }

#if 0
    char *absolute_filename = new char[ strlen(archive_path) + strlen(filename) + 1 ];
    sprintf( absolute_filename, "%s%s", archive_path, filename );
    for ( unsigned int i=0 ; i<strlen( absolute_filename ) ; i++ )
        if ( absolute_filename[i] == '/' ||
             absolute_filename[i] == '\\' )
            absolute_filename[i] = DELIMITER;

    if ( audio_open_flag ) Mix_CloseAudio();

    AVIWrapper *avi = new AVIWrapper();
    if ( avi->init( absolute_filename, false ) == 0 &&
         avi->initAV( screen_surface, audio_open_flag ) == 0 ){
        if (avi->play( click_flag )) return 1;
    }
    delete avi;
    delete[] absolute_filename;

    if ( audio_open_flag ){
        Mix_CloseAudio();
        openAudio();
    }
#endif
#else
    errorAndCont( "avi: avi video playback is disabled." );
#endif

    return 0;
}

//void ONScripterLabel::stopMovie(SMPEG *mpeg)
//{
//#ifndef MP3_MAD
//    if (mpeg) {
//        SMPEG_Info info;
//        SMPEG_getinfo(mpeg, &info);
//        SMPEG_stop( mpeg );
//        if (info.has_audio){
//            Mix_HookMusic( NULL, NULL );
//        }
//        SMPEG_delete( mpeg );
//
//        dirty_rect.add( async_movie_rect );
//    }
//
//    if (movie_buffer) delete[] movie_buffer;
//    movie_buffer = NULL;
//    if (surround_rects) delete[] surround_rects;
//    surround_rects = NULL;
//#endif
//}

void ONScripterLabel::stopBGM( bool continue_flag )
{
    if ( cdaudio_flag && cdrom_info ){
        extern SDL_TimerID timer_cdaudio_id;

        clearTimer( timer_cdaudio_id );
        if (SDL_CDStatus( cdrom_info ) >= CD_PLAYING )
            SDL_CDStop( cdrom_info );
    }

    if ( wave_sample[MIX_BGM_CHANNEL] ){
        Mix_Pause( MIX_BGM_CHANNEL );
        //Mix_HaltChannel( MIX_BGM_CHANNEL );
        Mix_ChannelFinished(NULL);
        delete wave_sample[MIX_BGM_CHANNEL];
        wave_sample[MIX_BGM_CHANNEL] = NULL;
    }

    if ( !continue_flag ){
        setStr( &music_file_name, NULL );
        music_play_loop_flag = false;
    }

    if (music_info) {
        Mix_HaltMusic();
        Mix_FreeMusic(music_info);
        music_info = NULL;
    }

    if ( seqmusic_info ){

#if defined(MACOSX) //insani
        clearTimer( timer_seqmusic_id );
#endif

        ext_music_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( seqmusic_info );
        seqmusic_info = NULL;
    }
    if ( !continue_flag ){
        setStr( &seqmusic_file_name, NULL );
        seqmusic_play_loop_flag = false;
    }

    if ( music_info ){
        ext_music_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( music_info );
        music_info = NULL;
    }

    if ( !continue_flag ) current_cd_track = -1;
}

void ONScripterLabel::stopDWAVE( int channel )
{
    if (!audio_open_flag) return;

    //avoid stopping dwave outside array
    if (channel < 0) channel = 0;
    else if (channel >= ONS_MIX_CHANNELS) channel = ONS_MIX_CHANNELS-1;

    if ( wave_sample[channel] ){
        Mix_Pause( channel );
        if ( !channel_preloaded[channel] || channel == 0 ){
            //don't free preloaded channels, _except_:
            //always free voice channel, for now - could be
            //messy for bgmdownmode and/or voice-waiting FIXME
            Mix_ChannelFinished(NULL);
            delete wave_sample[channel];
            wave_sample[channel] = NULL;
            channel_preloaded[channel] = false;
        }
    }
    if ((channel == 0) && bgmdownmode_flag)
        setCurMusicVolume( music_volume );
}

void ONScripterLabel::stopAllDWAVE()
{
    if (!audio_open_flag) return;

    for (int ch=0; ch<ONS_MIX_CHANNELS ; ch++) {
        if ( wave_sample[ch] ){
            Mix_Pause( ch );
            if ( !channel_preloaded[ch] || ch == 0 ){
                //always free voice channel sample, for now - could be
                //messy for bgmdownmode and/or voice-waiting FIXME
                Mix_ChannelFinished(NULL);
                delete wave_sample[ch];
                wave_sample[ch] = NULL;
            }
        }
    }
    // just in case the bgm was turned down for the voice channel,
    // set the bgm volume back to normal
    if (bgmdownmode_flag)
        setCurMusicVolume( music_volume );
}

void ONScripterLabel::playClickVoice()
{
    if      ( clickstr_state == CLICK_NEWPAGE ){
        if ( clickvoice_file_name[CLICKVOICE_NEWPAGE] )
            playSound(clickvoice_file_name[CLICKVOICE_NEWPAGE],
                      SOUND_WAVE|SOUND_OGG, false, MIX_CLICKVOICE_CHANNEL);
    }
    else if ( clickstr_state == CLICK_WAIT ){
        if ( clickvoice_file_name[CLICKVOICE_NORMAL] )
            playSound(clickvoice_file_name[CLICKVOICE_NORMAL],
                      SOUND_WAVE|SOUND_OGG, false, MIX_CLICKVOICE_CHANNEL);
    }
}

void setupWaveHeader(unsigned char* buffer, int channels, int bits,
    unsigned long rate, unsigned long data_length,
    unsigned int extra_bytes, unsigned char* extra_ptr)
{
    memcpy(header.chunk_riff, "RIFF", 4);
    unsigned long riff_length = sizeof(WAVE_HEADER) + sizeof(WAVE_DATA_HEADER) +
        data_length + extra_bytes - 8;
    header.riff_length[0] = riff_length & 0xff;
    header.riff_length[1] = (riff_length >> 8) & 0xff;
    header.riff_length[2] = (riff_length >> 16) & 0xff;
    header.riff_length[3] = (riff_length >> 24) & 0xff;
    memcpy(header.fmt_id, "WAVEfmt ", 8);
    header.fmt_size[0] = 0x10 + extra_bytes;
    header.fmt_size[1] = header.fmt_size[2] = header.fmt_size[3] = 0;
    header.data_fmt[0] = 1; header.data_fmt[1] = 0; // PCM format
    header.channels[0] = channels; header.channels[1] = 0;
    header.frequency[0] = rate & 0xff;
    header.frequency[1] = (rate >> 8) & 0xff;
    header.frequency[2] = (rate >> 16) & 0xff;
    header.frequency[3] = (rate >> 24) & 0xff;

    int sample_byte_size = channels * bits / 8;
    unsigned long byte_size = sample_byte_size * rate;
    header.byte_size[0] = byte_size & 0xff;
    header.byte_size[1] = (byte_size >> 8) & 0xff;
    header.byte_size[2] = (byte_size >> 16) & 0xff;
    header.byte_size[3] = (byte_size >> 24) & 0xff;
    header.sample_byte_size[0] = sample_byte_size;
    header.sample_byte_size[1] = 0;
    header.sample_bit_size[0] = bits;
    header.sample_bit_size[1] = 0;

    memcpy(data_header.chunk_id, "data", 4);
    data_header.data_length[0] = (char)(data_length & 0xff);
    data_header.data_length[1] = (char)((data_length >> 8) & 0xff);
    data_header.data_length[2] = (char)((data_length >> 16) & 0xff);
    data_header.data_length[3] = (char)((data_length >> 24) & 0xff);

    memcpy(buffer, &header, sizeof(header));
    if (extra_bytes > 0) {
        if (extra_ptr != NULL)
            memcpy(buffer + sizeof(header), extra_ptr, extra_bytes);
        else
            memset(buffer + sizeof(header), 0, extra_bytes);
    }
    memcpy(buffer + sizeof(header) + extra_bytes, &data_header, sizeof(data_header));
}

Mix_MusicType detect_music_type(SDL_RWops* src)
{
    Uint8 magic[12];

    if (SDL_RWread(src, magic, 1, 12) != 12) {
        Mix_SetError("Couldn't read first 12 bytes of audio data");
        return MUS_NONE;
    }
    SDL_RWseek(src, -12, RW_SEEK_CUR);

    /* WAVE files have the magic four bytes "RIFF"
       AIFF files have the magic 12 bytes "FORM" XXXX "AIFF" */
    if (((SDL_memcmp(magic, "RIFF", 4) == 0) && (SDL_memcmp((magic + 8), "WAVE", 4) == 0)) ||
        (SDL_memcmp(magic, "FORM", 4) == 0)) {
        return MUS_WAV;
    }

    /* Ogg Vorbis files have the magic four bytes "OggS" */
    if (SDL_memcmp(magic, "OggS", 4) == 0) {
        SDL_RWseek(src, 28, RW_SEEK_CUR);
        SDL_RWread(src, magic, 1, 8);
        SDL_RWseek(src, -36, RW_SEEK_CUR);
        if (SDL_memcmp(magic, "OpusHead", 8) == 0) {
            return MUS_OPUS;
        }
        if (magic[0] == 0x7F && SDL_memcmp(magic + 1, "FLAC", 4) == 0) {
            return MUS_FLAC;
        }
        return MUS_OGG;
    }

    /* FLAC files have the magic four bytes "fLaC" */
    if (SDL_memcmp(magic, "fLaC", 4) == 0) {
        return MUS_FLAC;
    }

    /* WavPack files have the magic four bytes "wvpk" */
    if (SDL_memcmp(magic, "wvpk", 4) == 0) {
        return MUS_WAVPACK;
    }

    /* MIDI files have the magic four bytes "MThd" */
    if (SDL_memcmp(magic, "MThd", 4) == 0) {
        return MUS_MID;
    }

    /* RIFF MIDI files have the magic four bytes "RIFF" and then "RMID" */
    if ((SDL_memcmp(magic, "RIFF", 4) == 0) && (SDL_memcmp(magic + 8, "RMID", 4) == 0)) {
        return MUS_MID;
    }

    if (SDL_memcmp(magic, "ID3", 3) == 0 ||
        /* see: https://bugzilla.libsdl.org/show_bug.cgi?id=5322 */
        (magic[0] == 0xFF && (magic[1] & 0xE6) == 0xE2)) {
        return MUS_MP3;
    }

    /* GME Specific files */
    if (SDL_memcmp(magic, "ZXAY", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "GBS\x01", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "GYMX", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "HESM", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "KSCC", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "KSSX", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "NESM", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "NSFE", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "SAP\x0D", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "SNES", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "Vgm ", 4) == 0)
        return MUS_GME;
    if (SDL_memcmp(magic, "\x1f\x8b", 2) == 0)
        return MUS_GME;

    /* Assume MOD format.
     *
     * Apparently there is no way to check if the file is really a MOD,
     * or there are too many formats supported by libmodplug or libxmp.
     * The mod library does this check by itself. */
    return MUS_MOD;
}
