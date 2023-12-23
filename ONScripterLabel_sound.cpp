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

SoundMusic* SoundMusic::s_soundMusic = NULL;

bool SoundMusic::Start(ONScripterLabel* onscripter, unsigned char* buffer, size_t length, const char* ext_hint)
{
    // Not done yet;
    // - looping needs to be completed
    // - SoundMusic needs to own the buffer the SDL_RWops is wrapping
    int freq;
    Uint16 format;
    int channels;
    Mix_QuerySpec(&freq, &format, &channels);
    Sound_AudioInfo info;
    info.channels = channels;
    info.format = format;
    info.rate = freq;
    Sound_Sample* sample = Sound_NewSampleFromMem(buffer, length, ".mid", &info, 1024 * 1024);

    if ((NULL == sample) || (sample->flags == SOUND_SAMPLEFLAG_ERROR)) {
        Sound_FreeSample(sample);
        delete[] buffer;
        return false;
    }

    if (s_soundMusic) {
        delete s_soundMusic;
    }

    s_soundMusic = new SoundMusic();
    s_soundMusic->m_sample = sample;
    s_soundMusic->m_buffer = buffer;

    Mix_CloseAudio();
    onscripter->openAudio(sample->desired.rate, sample->desired.format, sample->desired.channels);

    Mix_HookMusic(&Decode, s_soundMusic);

    return true;
}

void SDLCALL SoundMusic::Decode(void* self, Uint8* stream, int len)
{
    SoundMusic* data = (SoundMusic*)self;
    Sound_Sample* sample = data->m_sample;
    int bw = 0; /* bytes written to stream this time through the callback */

    while (bw < len)
    {
        int cpysize;  /* bytes to copy on this iteration of the loop. */

        if (data->m_decoded_bytes == 0) /* need more data! */
        {
            /* if there wasn't previously an error or EOF, read more. */
            if (((sample->flags & SOUND_SAMPLEFLAG_ERROR) == 0) &&
                ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0))
            {
                data->m_decoded_bytes = Sound_Decode(sample);
                data->m_decoded = static_cast<Uint8*>(sample->buffer);
            } /* if */

            if (data->m_decoded_bytes == 0)
            {
                /* ...there isn't any more data to read! */
                SDL_memset(stream + bw, '\0', len - bw);  /* write silence. */
                data->m_done = true;
                return;  /* we're done playback, one way or another. */
            } /* if */
        } /* if */

        /* we have data decoded and ready to write to the device... */
        cpysize = len - bw;  /* len - bw == amount device still wants. */
        if (cpysize > data->m_decoded_bytes)
            cpysize = data->m_decoded_bytes;  /* clamp to what we have left. */

        /* if it's 0, next iteration will decode more or decide we're done. */
        if (cpysize > 0)
        {
            /* write this iteration's data to the device. */
            SDL_memcpy(stream + bw, (Uint8*)data->m_decoded, cpysize);

            /* update state for next iteration or callback */
            bw += cpysize;
            data->m_decoded += cpysize;
            data->m_decoded_bytes -= cpysize;
        } /* if */
    } /* while */
}

SoundMusic::~SoundMusic()
{
    if (m_sample) {
        Sound_FreeSample(m_sample);
        m_sample = NULL;
    }

    if (m_buffer) {
        delete[] m_buffer;
        m_buffer = NULL;
    }
}

SoundChunk* SoundChunk::Create(unsigned char* buffer, size_t length, const char* ext_hint)
{
    int freq;
    Uint16 format;
    int channels;
    Mix_QuerySpec(&freq, &format, &channels);
    Sound_AudioInfo info;
    info.channels = channels;
    info.format = format;
    info.rate = freq;
    Sound_Sample* sample = Sound_NewSampleFromMem(buffer, length, ext_hint, &info, 1024 * 1024);
    Sound_DecodeAll(sample);

    if ((NULL == sample) || (sample->flags == SOUND_SAMPLEFLAG_ERROR))
    {
        Sound_FreeSample(sample);
        delete[] buffer;
        return NULL;
    }

    SoundChunk* soundChunk = new SoundChunk();
    soundChunk->m_sample = sample;
    soundChunk->m_chunk.abuf = static_cast<Uint8*>(sample->buffer);
    soundChunk->m_chunk.alen = sample->buffer_size;
    soundChunk->m_chunk.volume = 100;
    soundChunk->m_chunk.allocated = 0;
    soundChunk->m_buffer = buffer;
}

SoundChunk::~SoundChunk()
{
    if (m_sample) {
        Sound_FreeSample(m_sample);
    }

    if (m_buffer) delete[] m_buffer;
}

int ONScripterLabel::playSound(const char *filename, int format, bool loop_flag, int channel)
{
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

    unsigned char *buffer = new(std::nothrow) unsigned char[length];
    if (buffer == NULL) {
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                    "failed to load sound file [%s] (%lu bytes)",
                    filename, length);
        errorAndCont( script_h.errbuf, "unable to allocate buffer", "Memory Issue" );
        return SOUND_NONE;
    }
    script_h.cBR->getFile( filename, buffer );

    // if (format & SOUND_OGG) should be Mix_Chunk,
    // else should be Mix_Music/Mix_HookMusic
    if (format & (SOUND_OGG | SOUND_OGG_STREAMING)){
        if (format & SOUND_OGG) {
            SoundChunk *soundChunk = SoundChunk::Create(buffer, length, ".OGG");
            if (soundChunk == NULL)
            {
                fprintf(stderr, "Issue loading ogg file: %s\n", SDL_GetError());
                return SOUND_OTHER;
            }

            return playWave(soundChunk, format, loop_flag, channel) == 0 ? SOUND_OGG : SOUND_OTHER;
        }

        if (!SoundMusic::Start(this, buffer, length, ".mp3"))
        {
            fprintf(stderr, "Issue loading mp3 file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        return SOUND_SEQMUSIC;
    }

    /* check for WMA (i.e. ASF header format) */
    if ( IS_ASF_HDR(buffer) ){
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
        "sound file '%s' is in WMA format, skipping", filename);
        errorAndCont(script_h.errbuf);
        delete[] buffer;
        return SOUND_OTHER;
    }

    /* check for AVI header format */
    if ( IS_AVI_HDR(buffer) ){
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
        "sound file '%s' is in AVI format, skipping", filename);
        errorAndCont(script_h.errbuf);
        delete[] buffer;
        return SOUND_OTHER;
    }

    // Should be Mix_Chunk,
    if (format & SOUND_WAVE){
        SoundChunk *soundChunk = SoundChunk::Create(buffer, length, ".wav");
        if (soundChunk == NULL)
        {
            fprintf(stderr, "Issue loading wave file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        return playWave(soundChunk, format, loop_flag, channel) == 0 ? SOUND_WAVE : SOUND_OTHER;
    }

    // Should be streaming (Mix_Music/Mix_HookMusic)
    if ((format & SOUND_MP3) &&
        !(IS_MIDI_HDR(buffer) && (format & SOUND_SEQMUSIC))){ //bypass MIDIs
        if (!SoundMusic::Start(this, buffer, length, ".mp3"))
        {
            fprintf(stderr, "Issue loading mp3 file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        return SOUND_SEQMUSIC;
    }

    // Should be streaming (Mix_Music/Mix_HookMusic)
    if (format & SOUND_SEQMUSIC){

        SoundChunk *soundChunk = SoundChunk::Create(buffer, length, ".MID");
        //Mix_Chunk* chunk = Mix_LoadWAV_RW(SDL_RWFromMem(buffer, length), 1);
        if (soundChunk == NULL)
        {
            fprintf(stderr, "Issue loading Midi file: %s\n", SDL_GetError());
            return SOUND_OTHER;
        }

        return playWave(soundChunk, format, loop_flag, channel) == 0 ? SOUND_SEQMUSIC : SOUND_OTHER;
    }

    delete[] buffer;

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

int ONScripterLabel::playWave(SoundChunk *soundChunk, int format, bool loop_flag, int channel)
{
    Mix_Pause( channel );
    if ( wave_sample[channel] ) {
      Mix_ChannelFinished(NULL);
      delete wave_sample[channel];
      wave_sample[channel] = NULL;
    }
    wave_sample[channel] = soundChunk;

    if (!soundChunk) return -1;

    if      (channel < ONS_MIX_CHANNELS)
        Mix_Volume( channel, !volume_on_flag? 0 : channelvolumes[channel] * 128 / 100 );
    else if (channel == MIX_CLICKVOICE_CHANNEL)
        Mix_Volume( channel, !volume_on_flag? 0 : se_volume * 128 / 100 );
    else if (channel == MIX_BGM_CHANNEL)
        Mix_Volume( channel, !volume_on_flag? 0 : music_volume * 128 / 100 );
    else
        Mix_Volume( channel, !volume_on_flag? 0 : se_volume * 128 / 100 );

    if (!(format & SOUND_PRELOAD)) {
        Mix_ChannelFinished(waveCallback);
        Mix_PlayChannel(channel, wave_sample[channel]->GetChunk(), loop_flag ? -1 : 0);
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

    Mix_VolumeMusic(!volume_on_flag? 0 : music_volume);
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

    if (music_struct.voice_sample && *(music_struct.voice_sample))
        volume /= 2;
    if (Mix_GetMusicHookData() != NULL) { // for streamed MP3 & OGG
        if ( mp3_sample ) Mix_Volume(mp3_channel, !volume_on_flag ? 0 : volume);
        else music_struct.volume = volume; // ogg
    } else if (Mix_Playing(MIX_BGM_CHANNEL) == 1) { // wave
        Mix_Volume( MIX_BGM_CHANNEL, !volume_on_flag? 0 : volume * 128 / 100 );
    } else if (Mix_PlayingMusic() == 1) { // midi
        Mix_VolumeMusic( !volume_on_flag? 0 : volume * 128 / 100 );
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
        if ( mp3_sample ) Mix_Volume(mp3_channel, do_mute ? 0 : music_vol); // mp3
        if ( async_movie ) async_movie->setVolume( do_mute? 0 : music_vol ); // async mpeg
        else music_struct.is_mute = do_mute; // ogg
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

    if (async_movie) delete async_movie;

    if (audio_open_flag) Mix_CloseAudio();

    FFMpegWrapper* video_player = new FFMpegWrapper();
    if (video_player->initialize(this, m_window, filename, audio_open_flag, false) != 0) {
        delete video_player;
        return 0;
    }

    surround_rects = new SDL_Rect[4];
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
        async_movie->play();
        if (!async_movie->hasAudioStream() && movie_loop_flag) {
            timer_silentmovie_id = SDL_AddTimer(100, silentmovieCallback, this);
        }
        return 0;
    }

    video_player->playFull(true);

    if (audio_open_flag) {
        Mix_CloseAudio();
        openAudio();
    }

    delete video_player;

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

    if ( mp3_sample ){
        Mix_Pause(MIX_BGM_CHANNEL);
        Mix_ChannelFinished(NULL);
        Mix_FreeChunk(mp3_sample);
        mp3_sample = NULL;
    }

    if ( wave_sample[MIX_BGM_CHANNEL] ){
        Mix_Pause( MIX_BGM_CHANNEL );
        Mix_ChannelFinished(NULL);
        delete wave_sample[MIX_BGM_CHANNEL];
        wave_sample[MIX_BGM_CHANNEL] = NULL;
    }

    if ( !continue_flag ){
        setStr( &music_file_name, NULL );
        music_play_loop_flag = false;
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
