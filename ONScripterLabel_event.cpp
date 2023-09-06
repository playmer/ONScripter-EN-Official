/* -*- C++ -*-
 *
 *  ONScripterLabel_event.cpp - Event handler of ONScripter-EN
 *
 *  Copyright (c) 2001-2009 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2007-2010 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  Copyright (c) 2023 Galladite
 *
 *  galladite@yandex.com
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

// Modified by Mion, November 2009, to update from
// Ogapee's 20091115 release source code.

#include "ONScripterLabel.h"
#ifdef LINUX
#include <sys/types.h>
#include <sys/wait.h>
#endif
#ifdef WIN32
#include <windows.h>
#include "SDL_syswm.h"
#endif

// This sets up the fade event flag for use in bgm fadeout and fadein.
#define BGM_FADEOUT 0
#define BGM_FADEIN  1

#define EDIT_MODE_PREFIX "[EDIT MODE]  "
#define EDIT_SELECT_STRING "Music vol (m)  SE vol (s)  Voice vol (v)  Numeric variable (n)  Exit (Esc)"
#define EDIT_VOLUME_STRING "Music vol (m)  SE vol (s)  Voice vol (v)  Exit (Esc)"

static SDL_TimerID timer_id = NULL;
static SDL_TimerID break_id = NULL;
SDL_TimerID timer_cdaudio_id = NULL;
SDL_TimerID anim_timer_id = NULL;

SDL_TimerID timer_bgmfade_id = NULL;
SDL_TimerID timer_silentmovie_id = NULL;

// The reason we have a separate midi loop timer id here is that on Mac OS X, looping midis via SDL will cause SDL itself
// to hard crash after the first play.  So, we work around that by manually causing the midis to loop.  This OS X midi
// workaround is the work of Ben Carter.  Recommend for integration.  [Seung Park, 20060621]
#ifdef MACOSX
SDL_TimerID timer_seqmusic_id = NULL;
#endif
bool ext_music_play_once_flag = false;

void ONScripterLabel::clearTimer(SDL_TimerID &timer_id)
{
    if (timer_id != NULL ) {
        SDL_RemoveTimer( timer_id );
        timer_id = NULL;
    }
}

extern long decodeOggVorbis(ONScripterLabel::MusicStruct *music_struct, Uint8 *buf_dst, long len, bool do_rate_conversion);

/* **************************************** *
 * Callback functions
 * **************************************** */
extern "C" void mp3callback( void *userdata, Uint8 *stream, int len )
{
    SMPEG* mpeg_sample = (SMPEG*)userdata;
    int returned = SMPEG_playAudio(mpeg_sample, stream, len);
    
    if (returned == 0 ){
        Window::SendCustomEventStatic(ONS_SOUND_EVENT);
    }
}

extern "C" void oggcallback( void *userdata, Uint8 *stream, int len )
{
    if (decodeOggVorbis((ONScripterLabel::MusicStruct*)userdata, stream, len, true) == 0){
        Window::SendCustomEventStatic(ONS_SOUND_EVENT);
    }
}

extern "C" Uint32 SDLCALL animCallback( Uint32 interval, void *param )
{
    ONScripterLabel::clearTimer( anim_timer_id );

    Window::SendCustomEventStatic(ONS_ANIM_EVENT);

    return 0;
}

extern "C" Uint32 SDLCALL breakCallback(Uint32 interval, void *param)
{
    ONScripterLabel::clearTimer(break_id);

    Window::SendCustomEventStatic(ONS_BREAK_EVENT);

    return 0;
}

extern "C" Uint32 SDLCALL timerCallback( Uint32 interval, void *param )
{
    ONScripterLabel::clearTimer( timer_id );

    Window::SendCustomEventStatic(ONS_TIMER_EVENT);

    return 0;
}

extern "C" Uint32 cdaudioCallback( Uint32 interval, void *param )
{
    ONScripterLabel::clearTimer( timer_cdaudio_id );

    Window::SendCustomEventStatic(ONS_CDAUDIO_EVENT);

    return 0;
}

extern "C" Uint32 SDLCALL bgmfadeCallback( Uint32 interval, void *param )
{
    Window::SendCustomEventStatic(ONS_BGMFADE_EVENT, (param == NULL) ? 0 : 1);

    return interval;
}

extern "C" Uint32 SDLCALL silentmovieCallback( Uint32 interval, void *param )
{
    SMPEG **mpeg = (SMPEG **)param;
    if (*mpeg && (SMPEG_status(*mpeg) != SMPEG_PLAYING)){
        SMPEG_play( *mpeg );
    } else if (*mpeg == NULL){
        ONScripterLabel::clearTimer( timer_silentmovie_id );
        return 0;
    }

    return interval;
}

// Pushes the midi loop event onto the stack.  Part of a workaround for ONScripter
// crashing in Mac OS X after a midi is looped for the first time.  Recommend for
// integration.  This is the work of Ben Carter.  [Seung Park, 20060621]
#if defined(MACOSX)
extern "C" Uint32 seqmusicSDLCallback( Uint32 interval, void *param )
{
    Window::SendCustomEventStatic(ONS_SEQMUSIC_EVENT);
	return interval;
}
#endif

void seqmusicCallback( int sig )
{
#ifdef LINUX
    int status;
    wait( &status );
#endif
    if ( !ext_music_play_once_flag ){
        Window::SendCustomEventStatic(ONS_SEQMUSIC_EVENT);
    }
}

void musicCallback( int sig )
{
#ifdef LINUX
    int status;
    wait( &status );
#endif
    if ( !ext_music_play_once_flag ){
        Window::SendCustomEventStatic(ONS_MUSIC_EVENT);
    }
}

extern "C" void waveCallback( int channel )
{
    Window::SendCustomEventStatic(ONS_WAVE_EVENT, channel);
}


/* **************************************** *
 * OS Dependent Input Translation
 * **************************************** */
#ifndef IPODLINUX
//struct keychk {
//    bool set;
//    Uint16 unicode;
//    keychk(): set(false), unicode(0) {};
//};
//static keychk unikey[SDLK_LAST+1];
#endif

//FIXME: https://wiki.libsdl.org/SDL2/MigrationGuide
SDL_Keysym ONScripterLabel::transKey(SDL_Keysym key, bool isdown)
{
//#ifdef IPODLINUX
//    switch(key.sym){
//      case SDLK_m:      key.sym = SDLK_UP;      break; /* Menu               */
//      case SDLK_d:      key.sym = SDLK_DOWN;    break; /* Play/Pause         */
//      case SDLK_f:      key.sym = SDLK_RIGHT;   break; /* Fast forward       */
//      case SDLK_w:      key.sym = SDLK_LEFT;    break; /* Rewind             */
//      case SDLK_RETURN: key.sym = SDLK_RETURN;  break; /* Action             */
//      case SDLK_h:      key.sym = SDLK_ESCAPE;  break; /* Hold               */
//      case SDLK_r:      key.sym = SDLK_UNKNOWN; break; /* Wheel clockwise    */
//      case SDLK_l:      key.sym = SDLK_UNKNOWN; break; /* Wheel ctrclockwise */
//      default: break;
//    }
//#else
//    //printf("got key: %d (unicode %d)\n", event.key.keysym.sym, event.key.keysym.unicode);
//
//    // check against unicode
//    if (isdown) { // unicode field only available for keydown; save for keyup
//        unikey[key.sym].unicode = key.unicode;
//        unikey[key.sym].set = true;
//    }
//    else if (unikey[key.sym].set)
//        key.unicode = unikey[key.sym].unicode;
//
//    //account for switched-around keys in some layouts
//    if ((key.unicode & 0xFF80) == 0){
//        // ASCII
//        if ((key.unicode >= '0') && (key.unicode <= '9'))
//            key.sym = SDLKey(SDLK_0 + (int)key.unicode - '0');
//        else if ((key.unicode >= 'a') && (key.unicode <= 'z'))
//            key.sym = SDLKey(SDLK_a + (int)key.unicode - 'a');
//        else if ((key.unicode >= 'A') && (key.unicode <= 'Z'))
//            key.sym = SDLKey(SDLK_a + (int)key.unicode - 'A');
//        else if (key.unicode == ',')
//            key.sym = SDLK_COMMA;
//    }
//#endif

    return key;
}

SDL_Keycode transJoystickButton(Uint8 button)
{
#ifdef PSP
    SDLKey button_map[] = { SDLK_ESCAPE, /* TRIANGLE */
                            SDLK_RETURN, /* CIRCLE   */
                            SDLK_SPACE,  /* CROSS    */
                            SDLK_RCTRL,  /* SQUARE   */
                            SDLK_o,      /* LTRIGGER */
                            SDLK_s,      /* RTRIGGER */
                            SDLK_DOWN,   /* DOWN     */
                            SDLK_LEFT,   /* LEFT     */
                            SDLK_UP,     /* UP       */
                            SDLK_RIGHT,  /* RIGHT    */
                            SDLK_0,      /* SELECT   */
                            SDLK_a,      /* START    */
                            SDLK_UNKNOWN,/* HOME     */ /* kernel mode only */
                            SDLK_UNKNOWN,/* HOLD     */};
    return button_map[button];
#endif
    return SDLK_UNKNOWN;
}

SDL_KeyboardEvent transJoystickAxis(SDL_JoyAxisEvent &jaxis)
{
    static int old_axis=-1;

    SDL_KeyboardEvent event;

    SDL_Keycode axis_map[] = {SDLK_LEFT,  /* AL-LEFT  */
                         SDLK_RIGHT, /* AL-RIGHT */
                         SDLK_UP,    /* AL-UP    */
                         SDLK_DOWN   /* AL-DOWN  */};

    int axis = -1;
    /* rerofumi: Jan.15.2007 */
    /* ps3's pad has 0x1b axis (with analog button) */
    if (jaxis.axis < 2){
        axis = ((3200 > jaxis.value) && (jaxis.value > -3200) ? -1 :
                (jaxis.axis * 2 + (jaxis.value>0 ? 1 : 0) ));
    }

    if (axis != old_axis){
        if (axis == -1){
            event.type = SDL_KEYUP;
            event.keysym.sym = axis_map[old_axis];
        }
        else{
            event.type = SDL_KEYDOWN;
            event.keysym.sym = axis_map[axis];
        }
        old_axis = axis;
    }
    else{
        event.keysym.sym = SDLK_UNKNOWN;
    }

    return event;
}

void ONScripterLabel::flushEventSub( SDL_Event &event )
{
    //event related to streaming media
    if ( event.type == ONS_SOUND_EVENT ){
        if (async_movie) {
            if ((SMPEG_status(async_movie) != SMPEG_PLAYING) && (movie_loop_flag))
                SMPEG_play( async_movie );
        } else if ( music_play_loop_flag ||
             (cd_play_loop_flag && !cdaudio_flag ) ){
            stopBGM( true );
            if (music_file_name)
                playSound(music_file_name, SOUND_OGG_STREAMING|SOUND_MP3, true);
            else
                playCDAudio();
        }
        else{
            stopBGM( false );
        }
    }

// Mion: integrating insani's fadeout code & adding fadein, support for non-mp3 bgm
// The event handler for the mp3 fadeout event itself.
// Simply sets the volume of the mp3 being played lower and lower until it's 0,
// and until the requisite mp3 fadeout time has passed.  [Seung Park, 20060621]
    else if ((event.type == ONS_BGMFADE_EVENT) &&
             (event.user.code == BGM_FADEOUT)){
        Uint32 cur_fade_duration = mp3fadeout_duration;
        if (skip_mode & (SKIP_NORMAL | SKIP_TO_EOP | SKIP_TO_WAIT) ||
            ctrl_pressed_status) {
            cur_fade_duration = 0;
            setCurMusicVolume( 0 );
        }
        Uint32 tmp = SDL_GetTicks() - mp3fade_start;
        if ( tmp < cur_fade_duration ) {
            tmp = cur_fade_duration - tmp;
            tmp *= music_volume;
            tmp /= cur_fade_duration;

            setCurMusicVolume( tmp );
        } else {
            clearTimer( timer_bgmfade_id );

            event_mode &= ~WAIT_TIMER_MODE;
            stopBGM( false );
            //set break event to return to script processing
            clearTimer(break_id);

            Window::SendCustomEventStatic(ONS_BREAK_EVENT);
        }
    }

    else if ((event.type == ONS_BGMFADE_EVENT) &&
             (event.user.code == BGM_FADEIN)){
        Uint32 cur_fade_duration = mp3fadein_duration;
        if (skip_mode & (SKIP_NORMAL | SKIP_TO_EOP | SKIP_TO_WAIT) ||
            ctrl_pressed_status) {
            cur_fade_duration = 0;
            setCurMusicVolume( music_volume );
        }
        Uint32 tmp = SDL_GetTicks() - mp3fade_start;
        if ( tmp < cur_fade_duration ) {
            tmp *= music_volume;
            tmp /= cur_fade_duration;

            setCurMusicVolume( tmp );
        } else {
            clearTimer( timer_bgmfade_id );

            event_mode &= ~WAIT_TIMER_MODE;
            //set break event to return to script processing
            clearTimer(break_id);
            Window::SendCustomEventStatic(ONS_BREAK_EVENT);
        }
    }

    else if ( event.type == ONS_CDAUDIO_EVENT ){
        if ( cd_play_loop_flag ){
            stopBGM( true );
            playCDAudio();
        }
        else{
            stopBGM( false );
        }
    }
    else if ( event.type == ONS_SEQMUSIC_EVENT ){
#if defined(MACOSX) //insani
        if (!Mix_PlayingMusic())
        {
            ext_music_play_once_flag = !seqmusic_play_loop_flag;
            Mix_FreeMusic( seqmusic_info );
            playSequencedMusic(seqmusic_play_loop_flag);
        }
#else
        ext_music_play_once_flag = !seqmusic_play_loop_flag;
        Mix_FreeMusic( seqmusic_info );
        playSequencedMusic(seqmusic_play_loop_flag);
#endif
    }
    else if ( event.type == ONS_MUSIC_EVENT ){
        ext_music_play_once_flag = !music_play_loop_flag;
        Mix_FreeMusic(music_info);
        playExternalMusic(music_play_loop_flag);
    }
    else if ( event.type == ONS_WAVE_EVENT ){ // for processing btntime2 and automode correctly
        int ch = event.user.code;
        if ( wave_sample[ch] ){
            if ( (ch >= ONS_MIX_CHANNELS) || (ch == 0) ||
                 !channel_preloaded[ch] ) {
                //don't free preloaded channels, _except_:
                //always free voice channel, for now - could be
                //messy for bgmdownmode and/or voice-waiting FIXME
                Mix_ChannelFinished(NULL);
                Mix_FreeChunk( wave_sample[ch] );
                wave_sample[ch] = NULL;
            }
            if (ch == MIX_LOOPBGM_CHANNEL0 &&
                loop_bgm_name[1] &&
                wave_sample[MIX_LOOPBGM_CHANNEL1])
                Mix_ChannelFinished(waveCallback);
                Mix_PlayChannel(MIX_LOOPBGM_CHANNEL1,
                                wave_sample[MIX_LOOPBGM_CHANNEL1], -1);
            if (ch == 0) {
                channel_preloaded[ch] = false;
                if (bgmdownmode_flag)
                    setCurMusicVolume( music_volume );
            }
        }
    }
}

void ONScripterLabel::flushEvent()
{
    SDL_Event event;
    while (m_window->PollEvents(event))
        flushEventSub( event );
}

void ONScripterLabel::advancePhase( int count )
{
    clearTimer(timer_id);

    resetCursorTime( count );
    timer_id = SDL_AddTimer( count, timerCallback, NULL );
}

void ONScripterLabel::advanceAnimPhase( int count )
{
    if ( anim_timer_id == NULL ){
        resetRemainingTime(count);
        anim_timer_id = SDL_AddTimer( count, animCallback, NULL );
    }
}

void ONScripterLabel::waitEventSub(int count)
{
    if (break_id != NULL){ // already in wait queue
        return;
    }

    //use WAIT_NO_ANIM_MODE to avoid animation refresh (e.g. in effect mode)
    //use count<0 to prevent generating an automatic break event (i.e. run until "done")
    int no_anim = false;
    if (event_mode & (WAIT_INPUT_MODE | WAIT_TEXTBTN_MODE)) {
        if ( (skip_mode & SKIP_NORMAL) || ctrl_pressed_status ||
             (event_mode & WAIT_NO_ANIM_MODE) )
            no_anim = true;
    }
    if (event_mode & (WAIT_TIMER_MODE | WAIT_TEXTOUT_MODE)){
        if (no_anim){
            clearTimer(anim_timer_id);
        } else {
            int duration = proceedCursorAnimation();
            if (duration >= 0){
                advancePhase();
            }

            duration = proceedAnimation();
            if (duration >= 0){
                advanceAnimPhase();
            }
        }

        if (count > 0){
            break_id = SDL_AddTimer(count, breakCallback, NULL);
        }
    }
    
    if ((count >= 0) && (break_id == NULL)){
        Window::SendCustomEventStatic(ONS_BREAK_EVENT);
    }

    runEventLoop();

    clearTimer(break_id);
}

bool ONScripterLabel::waitEvent( int count )
{
    while(1){
        waitEventSub( count );
        if ( system_menu_mode == SYSTEM_NULL ) break;
        if ( rgosub_label ) {
            system_menu_mode = SYSTEM_NULL;
            char *tmp = script_h.rgosub_wait_pos[script_h.cur_rgosub_wait];
            if(select_release & SELECT_RELEASE_REQUIRED) {
                select_release |= SELECT_RELEASE_RGOSUB;
                // This tells the loop to break and call the gosubReal
                // manually, among other things
                return true;
            } else {
            gosubReal( rgosub_label, tmp, false, clickstr_state,
                       script_h.rgosub_wait_1byte[script_h.cur_rgosub_wait]);
            script_h.cur_rgosub_wait = script_h.num_rgosub_waits = 0;
            return true;
            }
        }
        //if ( executeSystemCall() ) return true;
        int ret = executeSystemCall();
        if      (ret == 1) return true;
        else if (ret == 2) return false;
    }

    return false;
}

void ONScripterLabel::trapHandler()
{
    trap_mode = TRAP_NONE;
    stopCursorAnimation( clickstr_state );
    setCurrentLabel( trap_dest );
}

/* **************************************** *
 * Event handlers
 * **************************************** */
bool ONScripterLabel::mouseMoveEvent( SDL_MouseMotionEvent *event )
{
    current_button_state.x = event->x;
    current_button_state.y = event->y;

    if ( event_mode & WAIT_BUTTON_MODE ){
        mouseOverCheck( current_button_state.x, current_button_state.y );
        if (getmouseover_flag &&
            (current_over_button >= getmouseover_min) &&
            (current_over_button <= getmouseover_max)){
            current_button_state.set(current_over_button);
            volatile_button_state.set(current_over_button);
            playClickVoice();
            stopCursorAnimation( clickstr_state );
            return true;
        }
        else if (btnarea_flag &&
                 ( ((btnarea_pos < 0) && (event->y > -btnarea_pos)) ||
                   ((btnarea_pos > 0) && (event->y < btnarea_pos)) )){
            current_button_state.set(-4);
            volatile_button_state.set(-4);
            playClickVoice();
            stopCursorAnimation( clickstr_state );
            return true;
        }

    }
    return false;
}

bool ONScripterLabel::mouseWheelEvent(SDL_MouseWheelEvent* event)
// returns true if should break out of the event loop
{
    if (variable_edit_mode) return false;

    if (event_mode & WAIT_BUTTON_MODE)
        last_keypress = KEYPRESS_NULL;

    //any mousepress clears automode, on the release
    if (automode_flag) {
        if (event->type == SDL_MOUSEBUTTONUP) {
            automode_flag = false;
            if (getskipoff_flag && (event_mode & WAIT_BUTTON_MODE)) {
                current_button_state.set(-61);
                volatile_button_state.set(-61);
                return true;
            }
        }
        return false;
    }

    // FIXME: Probably abstract all this stuff into a function to call in this and mousePressEvent
    //trap that mouseclick!
    //if ( ((event->button == SDL_BUTTON_RIGHT) && (trap_mode & TRAP_RIGHT_CLICK)) ||
    //     ((event->button == SDL_BUTTON_LEFT)  && (trap_mode & TRAP_LEFT_CLICK)) ){
    //    trapHandler();
    //    return true;
    //}

    current_button_state.reset();
    current_button_state.x = event->x;
    current_button_state.y = event->y;
    current_button_state.down_flag = false;
    if (getskipoff_flag && (skip_mode & SKIP_NORMAL) &&
        (event_mode & WAIT_BUTTON_MODE)) {
        skip_mode &= ~SKIP_NORMAL;
        current_button_state.set(-60);
        volatile_button_state.set(-60);
        return true;
    }

    skip_mode &= ~SKIP_NORMAL;

    if ((event->y > 0 /*SDL_BUTTON_WHEELUP*/) &&
        ((event_mode & WAIT_TEXT_MODE) ||
          (usewheel_flag && (event_mode & WAIT_BUTTON_MODE)) ||
          (system_menu_mode == SYSTEM_LOOKBACK))) {
        current_button_state.set(-2);
        if (event_mode & WAIT_TEXT_MODE) system_menu_mode = SYSTEM_LOOKBACK;
    }
    else if ((event->y < 0/*SDL_BUTTON_WHEELDOWN*/) &&
        ((enable_wheeldown_advance_flag && (event_mode & WAIT_TEXT_MODE)) ||
          (usewheel_flag && (event_mode & WAIT_BUTTON_MODE)) ||
          (system_menu_mode == SYSTEM_LOOKBACK))) {
        if (event_mode & WAIT_TEXT_MODE) {
            current_button_state.set(0);
        }
        else {
            current_button_state.set(-3);
        }
    }
    else return false;

    if (current_button_state.valid_flag)
        volatile_button_state.set(current_button_state.button);

    if (event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE)) {
        if (system_menu_mode == SYSTEM_NULL) playClickVoice();
        stopCursorAnimation(clickstr_state);
        return true;
    }
    else
        return false;
}

bool ONScripterLabel::mousePressEvent( SDL_MouseButtonEvent *event )
// returns true if should break out of the event loop
{
    if ( variable_edit_mode ) return false;

    if (event_mode & WAIT_BUTTON_MODE)
        last_keypress = KEYPRESS_NULL;

    //any mousepress clears automode, on the release
    if ( automode_flag ){
        if ( event->type == SDL_MOUSEBUTTONUP ){
            automode_flag = false;
            if (getskipoff_flag && (event_mode & WAIT_BUTTON_MODE)){
                current_button_state.set(-61);
                volatile_button_state.set(-61);
                return true;
            }
        }
        return false;
    }

    //trap that mouseclick!
    if ( ((event->button == SDL_BUTTON_RIGHT) && (trap_mode & TRAP_RIGHT_CLICK)) ||
         ((event->button == SDL_BUTTON_LEFT)  && (trap_mode & TRAP_LEFT_CLICK)) ){
        trapHandler();
        return true;
    }

    current_button_state.reset();
    current_button_state.x = event->x;
    current_button_state.y = event->y;
    current_button_state.down_flag = false;
    if (getskipoff_flag && (skip_mode & SKIP_NORMAL) &&
        (event_mode & WAIT_BUTTON_MODE)){
        skip_mode &= ~SKIP_NORMAL;
        current_button_state.set(-60);
        volatile_button_state.set(-60);
        return true;
    }

    skip_mode &= ~SKIP_NORMAL;

    //right-click
    if ((event->button == SDL_BUTTON_RIGHT) &&
        (event->type == SDL_MOUSEBUTTONUP) &&
        ( (rmode_flag && (event_mode & WAIT_TEXT_MODE)) ||
          (event_mode & (WAIT_BUTTON_MODE | WAIT_RCLICK_MODE)) )){
        current_button_state.set(-1);
        if (rmode_flag && (event_mode & WAIT_TEXT_MODE)){
            if (root_rmenu_link.next)
                system_menu_mode = SYSTEM_MENU;
            else
                system_menu_mode = SYSTEM_WINDOWERASE;
        }
    }
#if 0
    // Used for debugging right-click issues - Galladite 2023-2-25
    //    ( (rmode_flag && (event_mode & WAIT_TEXT_MODE)) ||
    //      (event_mode & (WAIT_BUTTON_MODE | WAIT_RCLICK_MODE)) )){
    else if ( (event->button == SDL_BUTTON_RIGHT) &&
                (event->type == SDL_MOUSEBUTTONUP) ){
        printf("Debug: caught unreceived right-click\n");
        printf("rmode_flag: %d\n", rmode_flag);
        if (event_mode & WAIT_TEXT_MODE)
            printf("event_mode: WAIT_TEXT_MODE\n");
        if (event_mode & WAIT_BUTTON_MODE)
            printf("event_mode: WAIT_BUTTON_MODE\n");
        if (event_mode & WAIT_RCLICK_MODE)
            printf("event_mode: WAIT_RCLICK_MODE\n");

        if (rmode_flag && (event_mode & WAIT_TEXT_MODE))
            printf("Cond 1: success\n");
        else {
            printf("Cond 1: failure - ");
            if (!rmode_flag)
                printf("rmode_flag failed: %d\n", rmode_flag);
            else if (!(event_mode & WAIT_TEXT_MODE))
                printf("event_mode failed\n");
            else
                printf("an unexpected error occurred\n");
        }
        if (event_mode & (WAIT_BUTTON_MODE | WAIT_RCLICK_MODE))
            printf("Cond 2: success\n");
        else
            printf("Cond 2: failure\n");
        printf("\n\n");
        return false;
    }
#endif
    //left-click
    else if ( (event->button == SDL_BUTTON_LEFT) &&
              ((event->type == SDL_MOUSEBUTTONUP) || btndown_flag) ){
        current_button_state.set(current_over_button);
        if ( event_mode & WAIT_TEXTOUT_MODE) {
            skip_mode |= (SKIP_TO_WAIT | SKIP_TO_EOL);
        }
        skip_effect = true;

        if ( event->type == SDL_MOUSEBUTTONDOWN )
            current_button_state.down_flag = true;
    }
    //middle-click
    else if ( (event->button == SDL_BUTTON_MIDDLE) &&
              ((event->type == SDL_MOUSEBUTTONUP) || btndown_flag) ){
        if (!getmclick_flag)
            return false;
        current_button_state.set(-70);
        if ( event->type == SDL_MOUSEBUTTONDOWN )
            current_button_state.down_flag = true;
    }
    else return false;

    if (current_button_state.valid_flag)
        volatile_button_state.set(current_button_state.button);

    if (event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE)){
        if (system_menu_mode == SYSTEM_NULL) playClickVoice();
        stopCursorAnimation( clickstr_state );
        return true;
    } else
        return false;
}

void ONScripterLabel::variableEditMode( SDL_KeyboardEvent *event )
{
    if (event_mode & WAIT_BUTTON_MODE)
        last_keypress = KEYPRESS_NULL;

    int  i;
    const char* var_name;
    char var_index[12];

    switch ( event->keysym.sym ) {
      case SDLK_m:
        if ( (variable_edit_mode != EDIT_SELECT_MODE) &&
             (variable_edit_mode != EDIT_VOLUME_MODE) )
            return;
        variable_edit_mode = EDIT_MP3_VOLUME_MODE;
        variable_edit_num = music_volume;
        break;

      case SDLK_s:
        if ( (variable_edit_mode != EDIT_SELECT_MODE) &&
             (variable_edit_mode != EDIT_VOLUME_MODE) )
            return;
        variable_edit_mode = EDIT_SE_VOLUME_MODE;
        variable_edit_num = se_volume;
        break;

      case SDLK_v:
        if ( (variable_edit_mode != EDIT_SELECT_MODE) &&
             (variable_edit_mode != EDIT_VOLUME_MODE) )
            return;
        variable_edit_mode = EDIT_VOICE_VOLUME_MODE;
        variable_edit_num = voice_volume;
        break;

      case SDLK_n:
        if ( variable_edit_mode != EDIT_SELECT_MODE ) return;
        variable_edit_mode = EDIT_VARIABLE_INDEX_MODE;
        variable_edit_num = 0;
        break;

      case SDLK_9: case SDLK_KP_9: variable_edit_num = variable_edit_num * 10 + 9; break;
      case SDLK_8: case SDLK_KP_8: variable_edit_num = variable_edit_num * 10 + 8; break;
      case SDLK_7: case SDLK_KP_7: variable_edit_num = variable_edit_num * 10 + 7; break;
      case SDLK_6: case SDLK_KP_6: variable_edit_num = variable_edit_num * 10 + 6; break;
      case SDLK_5: case SDLK_KP_5: variable_edit_num = variable_edit_num * 10 + 5; break;
      case SDLK_4: case SDLK_KP_4: variable_edit_num = variable_edit_num * 10 + 4; break;
      case SDLK_3: case SDLK_KP_3: variable_edit_num = variable_edit_num * 10 + 3; break;
      case SDLK_2: case SDLK_KP_2: variable_edit_num = variable_edit_num * 10 + 2; break;
      case SDLK_1: case SDLK_KP_1: variable_edit_num = variable_edit_num * 10 + 1; break;
      case SDLK_0: case SDLK_KP_0: variable_edit_num = variable_edit_num * 10 + 0; break;

      case SDLK_MINUS: case SDLK_KP_MINUS:
        if ( (variable_edit_mode == EDIT_VARIABLE_NUM_MODE) &&
             (variable_edit_num == 0) )
            variable_edit_sign = -1;
        break;

      case SDLK_BACKSPACE:
        if ( variable_edit_num ) variable_edit_num /= 10;
        else if ( variable_edit_sign == -1 ) variable_edit_sign = 1;
        break;

      case SDLK_RETURN: case SDLK_KP_ENTER:
        switch( variable_edit_mode ){

          case EDIT_VARIABLE_INDEX_MODE:
            variable_edit_index = variable_edit_num;
            variable_edit_num = script_h.getVariableData(variable_edit_index).num;
            if ( variable_edit_num < 0 ){
                variable_edit_num = -variable_edit_num;
                variable_edit_sign = -1;
            }
            else{
                variable_edit_sign = 1;
            }
            break;

          case EDIT_VARIABLE_NUM_MODE:
            script_h.setNumVariable( variable_edit_index, variable_edit_sign * variable_edit_num );
            break;

          case EDIT_MP3_VOLUME_MODE:
            music_volume = variable_edit_num;
            setCurMusicVolume(music_volume);
            break;

          case EDIT_SE_VOLUME_MODE:
            se_volume = variable_edit_num;
            for ( i=1 ; i<ONS_MIX_CHANNELS ; i++ )
                if ( wave_sample[i] )
                    Mix_Volume( i, !volume_on_flag? 0 : se_volume * 128 / 100 );
            if ( wave_sample[MIX_LOOPBGM_CHANNEL0] )
                Mix_Volume( MIX_LOOPBGM_CHANNEL0, !volume_on_flag? 0 : se_volume * 128 / 100 );
            if ( wave_sample[MIX_LOOPBGM_CHANNEL1] )
                Mix_Volume( MIX_LOOPBGM_CHANNEL1, !volume_on_flag? 0 : se_volume * 128 / 100 );
            break;

          case EDIT_VOICE_VOLUME_MODE:
            voice_volume = variable_edit_num;
            if ( wave_sample[0] )
                Mix_Volume( 0, !volume_on_flag? 0 : voice_volume * 128 / 100 );

          default:
            break;
        }
        if ( variable_edit_mode == EDIT_VARIABLE_INDEX_MODE )
            variable_edit_mode = EDIT_VARIABLE_NUM_MODE;
        else if (edit_flag)
            variable_edit_mode = EDIT_SELECT_MODE;
        else
            variable_edit_mode = EDIT_VOLUME_MODE;
        break;

      case SDLK_ESCAPE:
        if ( (variable_edit_mode == EDIT_SELECT_MODE) ||
             (variable_edit_mode == EDIT_VOLUME_MODE) ){
            variable_edit_mode = NOT_EDIT_MODE;
            m_window->SetWindowCaption( DEFAULT_WM_TITLE, DEFAULT_WM_ICON );
            SDL_Delay( 100 );
            m_window->SetWindowCaption( wm_title_string, wm_icon_string );
            return;
        }
        if (edit_flag)
            variable_edit_mode = EDIT_SELECT_MODE;
        else
            variable_edit_mode = EDIT_VOLUME_MODE;

      default:
        break;
    }

    if ( variable_edit_mode == EDIT_SELECT_MODE ){
        sprintf( wm_edit_string, "%s%s", EDIT_MODE_PREFIX, EDIT_SELECT_STRING );
    }
    else if ( variable_edit_mode == EDIT_VOLUME_MODE ){
        sprintf( wm_edit_string, "%s%s", EDIT_MODE_PREFIX, EDIT_VOLUME_STRING );
    }
    else if ( variable_edit_mode == EDIT_VARIABLE_INDEX_MODE ) {
        sprintf( wm_edit_string, "%s%s%d", EDIT_MODE_PREFIX, "Variable Index?  %", variable_edit_sign * variable_edit_num );
    }
    else if ( variable_edit_mode >= EDIT_VARIABLE_NUM_MODE ){
        int p=0;

        switch( variable_edit_mode ){

          case EDIT_VARIABLE_NUM_MODE:
            sprintf( var_index, "%%%d", variable_edit_index );
            var_name = var_index; p = script_h.getVariableData(variable_edit_index).num; break;

          case EDIT_MP3_VOLUME_MODE:
            var_name = "MP3 Volume"; p = music_volume; break;

          case EDIT_VOICE_VOLUME_MODE:
            var_name = "Voice Volume"; p = voice_volume; break;

          case EDIT_SE_VOLUME_MODE:
            var_name = "Sound effect Volume"; p = se_volume; break;

          default:
            var_name = "";
        }
        sprintf( wm_edit_string, "%sCurrent %s=%d  New value? %s%d",
                 EDIT_MODE_PREFIX, var_name, p, (variable_edit_sign==1)?"":"-", variable_edit_num );
    }

    m_window->SetWindowCaption( wm_edit_string, wm_icon_string );
}

void ONScripterLabel::shiftCursorOnButton( int diff )
//moves the mouse cursor to the new button "moused-over" via keystroke
{
    int num = 0;
    ButtonLink *button = root_button_link.next;
    while (button) {
        button = button->next;
        ++num;
    }

    shortcut_mouse_line += diff;
    if      (shortcut_mouse_line < 0)    shortcut_mouse_line = num - 1;
    else if (shortcut_mouse_line >= num) shortcut_mouse_line = 0;
    
    button = root_button_link.next;
    for (int i = 0; i < shortcut_mouse_line; ++i)
        button = button->next;

    if (button) {
        SDL_Rect clip = {0, 0, button->select_rect.w, button->select_rect.h};
        //center the position on the button.
        int x = button->select_rect.x + (button->select_rect.w / 2);
        int y = button->select_rect.y + (button->select_rect.h / 2);
        if (x < 0) clip.x -= x;
        else if (x > screen_width){
            clip.w = 0;
            x = screen_width - 1;
        }
        else if (x+clip.w > screen_width) clip.w = screen_width - x;
        if (y < 0) clip.y -= y;
        else if (x > screen_width){
            clip.h = 0;
            y = screen_height - 1;
        }
        else if (y+clip.h > screen_height) clip.h = screen_height - y;
        if (transbtn_flag && (clip.x < (Sint16) clip.w) && (clip.y < (Sint16) clip.h)){
            AnimationInfo *anim = NULL;
            if ( button->button_type == ButtonLink::SPRITE_BUTTON ||
                 button->button_type == ButtonLink::EX_SPRITE_BUTTON )
                anim = &sprite_info[ button->sprite_no ];
            else
                anim = button->anim[0];
            SDL_Rect pos = anim->findOpaquePoint(&clip);
            x += pos.x;
            y += pos.y;
        } else {
            x += clip.x;
            y += clip.y;
        }

        m_window->WarpMouse(x, y);
    }
}

bool ONScripterLabel::keyDownEvent( SDL_KeyboardEvent *event )
// returns true if should break out of the event loop
{
    if (event_mode & WAIT_BUTTON_MODE)
        last_keypress = event->keysym.sym;

    int last_ctrl_status = ctrl_pressed_status;

    switch ( event->keysym.sym ) {
      case SDLK_RCTRL:
        ctrl_pressed_status  |= 0x01;
      case SDLK_LCTRL:
        if (event->keysym.sym == SDLK_LCTRL)
            ctrl_pressed_status  |= 0x02;
        if (last_ctrl_status != ctrl_pressed_status)
            skip_effect = true; // allow short-circuiting the current effect with ctrl
        //Ctrl key: do skip in text
        if (event_mode & (WAIT_INPUT_MODE | WAIT_TEXTOUT_MODE | WAIT_TEXTBTN_MODE)){
            current_button_state.set(0);
            volatile_button_state.set(0);
            playClickVoice();
            stopCursorAnimation( clickstr_state );
            return true;
        }
        if (event_mode & (WAIT_SLEEP_MODE)){
            stopCursorAnimation( clickstr_state );
            return true;
        }
        break;
      case SDLK_RSHIFT:
        shift_pressed_status |= 0x01;
        break;
      case SDLK_LSHIFT:
        shift_pressed_status |= 0x02;
        break;
#ifdef MACOSX
      case SDLK_LMETA:
        apple_pressed_status |= 1;
        break;
      case SDLK_RMETA:
        apple_pressed_status |= 1;
        break;
#endif
      default:
        break;
    }

    return false;
}

void ONScripterLabel::keyUpEvent( SDL_KeyboardEvent *event )
{
    if (event_mode & WAIT_BUTTON_MODE)
        last_keypress = event->keysym.sym;

    switch ( event->keysym.sym ) {
      case SDLK_RCTRL:
        ctrl_pressed_status  &= ~0x01;
        break;
      case SDLK_LCTRL:
        ctrl_pressed_status  &= ~0x02;
        break;
      case SDLK_RSHIFT:
        shift_pressed_status &= ~0x01;
        break;
      case SDLK_LSHIFT:
        shift_pressed_status &= ~0x02;
        break;
#ifdef MACOSX
      case SDLK_LMETA:
        apple_pressed_status &= ~1;
        break;
      case SDLK_RMETA:
        apple_pressed_status &= ~2;
        break;
#endif
      default:
        break;
    }
}

bool ONScripterLabel::keyPressEvent( SDL_KeyboardEvent *event )
// returns true if should break out of the event loop
{
    //reset the button state
    current_button_state.reset();
    current_button_state.down_flag = false;

    //any keypress clears automode, on the keyup
    if ( automode_flag ){
        if ( event->type == SDL_KEYUP ){
            automode_flag = false;
            if (getskipoff_flag && (event_mode & WAIT_BUTTON_MODE)){
                current_button_state.set(-61);
                volatile_button_state.set(-61);
                return true;
            }
        }
        return false;
    }

    if ( event->type == SDL_KEYUP ){
        if ( variable_edit_mode ){
            variableEditMode( event );
            return false;
        }

        //'m' is for mute (toggle)
        if ( (event->keysym.sym == SDLK_m) && !ctrl_pressed_status ){
            volume_on_flag = !volume_on_flag;
            setVolumeMute(!volume_on_flag);
            printf("turned %s volume mute\n", !volume_on_flag?"on":"off");
        }

        //'v' is for entering Volume Edit Mode
        if ( (event->keysym.sym == SDLK_v) && !ctrl_pressed_status ){
            // set to windowed mode, so that the caption-based edit menu shows
            menu_windowCommand();
            variable_edit_mode = EDIT_VOLUME_MODE;
            variable_edit_sign = 1;
            variable_edit_num = 0;
            sprintf( wm_edit_string, "%s%s", EDIT_MODE_PREFIX, EDIT_VOLUME_STRING );
            m_window->SetWindowCaption( wm_edit_string, wm_icon_string );
        }

        //'z' is for entering Edit Mode (if enabled)
        if ( edit_flag && (event->keysym.sym == SDLK_z) && !ctrl_pressed_status ){
            // set to windowed mode, so that the caption-based edit menu shows
            menu_windowCommand();
            variable_edit_mode = EDIT_SELECT_MODE;
            variable_edit_sign = 1;
            variable_edit_num = 0;
            sprintf( wm_edit_string, "%s%s", EDIT_MODE_PREFIX, EDIT_SELECT_STRING );
            m_window->SetWindowCaption( wm_edit_string, wm_icon_string );
        }
    }

    //'s', Return, Enter, or Space will clear (regular) skip mode
    if ((event->type == SDL_KEYUP) &&
        ( (event->keysym.sym == SDLK_RETURN) ||
          (event->keysym.sym == SDLK_KP_ENTER) ||
          (event->keysym.sym == SDLK_SPACE) ||
          (event->keysym.sym == SDLK_s) )){
        if (getskipoff_flag && (skip_mode & SKIP_NORMAL) &&
            (event_mode & WAIT_BUTTON_MODE)){
            skip_mode &= ~SKIP_NORMAL;
            current_button_state.set(-60);
            volatile_button_state.set(-60);
            return true;
        }
        skip_mode &= ~SKIP_NORMAL;
    }

    //Shift-'q' is for Quit
    if (((shift_pressed_status && (event->keysym.sym == SDLK_q)) 
#ifdef MACOSX
        || (apple_pressed_status && (event->keysym.sym == SDLK_q)) 
#endif
       ) && (current_mode == NORMAL_MODE)) {

        endCommand();
    }

    //trap that 'left-click'!
    if ((trap_mode & TRAP_LEFT_CLICK) &&
        ( (event->keysym.sym == SDLK_RETURN) ||
          (event->keysym.sym == SDLK_KP_ENTER) ||
          (event->keysym.sym == SDLK_SPACE) )){
        trapHandler();
        return true;
    }
    //trap that 'right-click'!
    else if ((trap_mode & TRAP_RIGHT_CLICK) &&
             (event->keysym.sym == SDLK_ESCAPE)){
        trapHandler();
        return true;
    }

    //so many ways to 'left-click' a button
    if ((event_mode & WAIT_BUTTON_MODE) &&
        ( (((event->type == SDL_KEYUP) || btndown_flag) &&
           ( (!getenter_flag && (event->keysym.sym == SDLK_RETURN)) ||
             (!getenter_flag && (event->keysym.sym == SDLK_KP_ENTER)) )) ||
          (( spclclk_flag || !useescspc_flag ) &&
           (event->keysym.sym == SDLK_SPACE)) )){
        if ( (event->keysym.sym == SDLK_RETURN) ||
             (event->keysym.sym == SDLK_KP_ENTER) ||
             (spclclk_flag && (event->keysym.sym == SDLK_SPACE)) ){
            current_button_state.set(current_over_button);
            volatile_button_state.set(current_over_button);
            if ( event->type == SDL_KEYDOWN )
                current_button_state.down_flag = true;
        }
        else{
            current_button_state.set(0);
            volatile_button_state.set(0);
        }
        skip_effect = true;
        playClickVoice();
        stopCursorAnimation( clickstr_state );
        return true;
    }

    if ( event->type == SDL_KEYDOWN ) return false;

    if ((event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE)) &&
        ( (autoclick_time == 0) || (event_mode & WAIT_BUTTON_MODE) )){
        //Esc is for 'right-click' (sometimes)
        if (!useescspc_flag && (event->keysym.sym == SDLK_ESCAPE)){
            current_button_state.set(-1);
            if (rmode_flag && (event_mode & WAIT_TEXT_MODE)){
                if (root_rmenu_link.next)
                    system_menu_mode = SYSTEM_MENU;
                else
                    system_menu_mode = SYSTEM_WINDOWERASE;
            }
        }
        else if (useescspc_flag && (event->keysym.sym == SDLK_ESCAPE)){
            current_button_state.set(-10);
        }
        else if (!spclclk_flag && useescspc_flag && (event->keysym.sym == SDLK_SPACE)){
            current_button_state.set(-11);
        }
        //'h' or left-arrow for page-up
        else if (( (!getcursor_flag && (event->keysym.sym == SDLK_LEFT)) ||
                   (event->keysym.sym == SDLK_h) ) &&
                 ( (event_mode & WAIT_TEXT_MODE) ||
                   (usewheel_flag && !getcursor_flag &&
                    (event_mode & WAIT_BUTTON_MODE)) || 
                   (system_menu_mode == SYSTEM_LOOKBACK) )){
            current_button_state.set(-2);
            if (event_mode & WAIT_TEXT_MODE) system_menu_mode = SYSTEM_LOOKBACK;
        }
        //'l' or right-arrow for page-down
        else if (( (!getcursor_flag && (event->keysym.sym == SDLK_RIGHT)) ||
                   (event->keysym.sym == SDLK_l) ) &&
                 ( (enable_wheeldown_advance_flag && 
                    (event_mode & WAIT_TEXT_MODE)) ||
                   (usewheel_flag && (event_mode & WAIT_BUTTON_MODE)) ||
                   (system_menu_mode == SYSTEM_LOOKBACK) )){
            if (event_mode & WAIT_TEXT_MODE){
                current_button_state.set(0);
            }
            else{
                current_button_state.set(-3);
            }
        }
        //'k', 'p', or up-arrow for shift to mouseover next button
        else if (( (!getcursor_flag && (event->keysym.sym == SDLK_UP)) ||
                   (event->keysym.sym == SDLK_k) ||
                   (event->keysym.sym == SDLK_p) ) &&
                 (event_mode & WAIT_BUTTON_MODE)){
            shiftCursorOnButton(1);
            return false;
        }
        //'j', 'n', or down-arrow for shift to mouseover previous button
        else if (( (!getcursor_flag && (event->keysym.sym == SDLK_DOWN)) ||
                   (event->keysym.sym == SDLK_j) ||
                   (event->keysym.sym == SDLK_n)) &&
                 (event_mode & WAIT_BUTTON_MODE)){
            shiftCursorOnButton(-1);
            return false;
        }
        else if (getpageup_flag && (event->keysym.sym == SDLK_PAGEUP)){
            current_button_state.set(-12);
        }
        else if (getpagedown_flag && (event->keysym.sym == SDLK_PAGEDOWN)){
            current_button_state.set(-13);
        }
        else if ( (getenter_flag && (event->keysym.sym == SDLK_RETURN)) ||
                  (getenter_flag && (event->keysym.sym == SDLK_KP_ENTER)) ){
            current_button_state.set(-19);
        }
        else if (gettab_flag && (event->keysym.sym == SDLK_TAB)){
            current_button_state.set(-20);
        }
        else if (getcursor_flag && (event->keysym.sym == SDLK_UP)){
            current_button_state.set(-40);
        }
        else if (getcursor_flag && (event->keysym.sym == SDLK_RIGHT)){
            current_button_state.set(-41);
        }
        else if (getcursor_flag && (event->keysym.sym == SDLK_DOWN)){
            current_button_state.set(-42);
        }
        else if (getcursor_flag && (event->keysym.sym == SDLK_LEFT)){
            current_button_state.set(-43);
        }
        else if (getinsert_flag && (event->keysym.sym == SDLK_INSERT)){
            current_button_state.set(-50);
        }
        else if (getzxc_flag && (event->keysym.sym == SDLK_z)){
            current_button_state.set(-51);
        }
        else if (getzxc_flag && (event->keysym.sym == SDLK_x)){
            current_button_state.set(-52);
        }
        else if (getzxc_flag && (event->keysym.sym == SDLK_c)){
            current_button_state.set(-53);
        }
        else if ( getfunction_flag ){
            if      ( event->keysym.sym == SDLK_F1 )
                current_button_state.set(-21);
            else if ( event->keysym.sym == SDLK_F2 )
                current_button_state.set(-22);
            else if ( event->keysym.sym == SDLK_F3 )
                current_button_state.set(-23);
            else if ( event->keysym.sym == SDLK_F4 )
                current_button_state.set(-24);
            else if ( event->keysym.sym == SDLK_F5 )
                current_button_state.set(-25);
            else if ( event->keysym.sym == SDLK_F6 )
                current_button_state.set(-26);
            else if ( event->keysym.sym == SDLK_F7 )
                current_button_state.set(-27);
            else if ( event->keysym.sym == SDLK_F8 )
                current_button_state.set(-28);
            else if ( event->keysym.sym == SDLK_F9 )
                current_button_state.set(-29);
            else if ( event->keysym.sym == SDLK_F10 )
                current_button_state.set(-30);
            else if ( event->keysym.sym == SDLK_F11 )
                current_button_state.set(-31);
            else if ( event->keysym.sym == SDLK_F12 )
                current_button_state.set(-32);
        }
        if ( current_button_state.valid_flag ){
            volatile_button_state.set(current_button_state.button);
            stopCursorAnimation( clickstr_state );

            return true;
        }
    }

    //catch 'left-button click' that fell through?
    if ((event_mode & WAIT_INPUT_MODE) && !key_pressed_flag &&
        ( (autoclick_time == 0) || (event_mode & WAIT_BUTTON_MODE) )){
        //check for "button click"
        if ( (event->keysym.sym == SDLK_RETURN) ||
             (event->keysym.sym == SDLK_KP_ENTER) ||
             (event->keysym.sym == SDLK_SPACE) ){
            key_pressed_flag = true;
            skip_effect = true;
            current_button_state.set(0);
            volatile_button_state.set(0);
            playClickVoice();
            stopCursorAnimation( clickstr_state );

            return true;
        }
    }

    if ((event_mode & (WAIT_INPUT_MODE | WAIT_TEXTBTN_MODE)) &&
        !key_pressed_flag){
        //'s' is for skip mode
        if ((event->keysym.sym == SDLK_s) && !automode_flag &&
            !ctrl_pressed_status){
            if (!(skip_mode & SKIP_NORMAL))
                skip_effect = true; // short-circuit a current effect
            skip_mode |= SKIP_NORMAL;
            printf("toggle skip to true\n");
            key_pressed_flag = true;
            current_button_state.set(0);
            volatile_button_state.set(0);
            stopCursorAnimation( clickstr_state );

            return true;
        }
        //'o' is for one-page mode toggle
        else if ( (event->keysym.sym == SDLK_o) && !ctrl_pressed_status ){
            if (skip_mode & SKIP_TO_EOP)
                skip_mode &= ~SKIP_TO_EOP;
            else
                skip_mode |= SKIP_TO_EOP;
            printf("toggle draw one page flag to %s\n",
                   (skip_mode & SKIP_TO_EOP?"true":"false"));
            if (skip_mode & SKIP_TO_EOP){
                current_button_state.set(0);
                volatile_button_state.set(0);
                stopCursorAnimation( clickstr_state );

                return true;
            }
        }
        //'a' is for automode
        else if ((event->keysym.sym == SDLK_a) && !ctrl_pressed_status &&
                 mode_ext_flag && !automode_flag){
            automode_flag = true;
            skip_mode &= ~SKIP_NORMAL;
            printf("change to automode\n");
            key_pressed_flag = true;
            current_button_state.set(0);
            volatile_button_state.set(0);
            stopCursorAnimation( clickstr_state );

            return true;
        }
        //toggle default text speed
        else if ( event->keysym.sym == SDLK_0 ){
            if (++text_speed_no > 2) text_speed_no = 0;
            sentence_font.wait_time = -1;
        }
        //slow default text speed
        else if ( event->keysym.sym == SDLK_1 ){
            text_speed_no = 0;
            sentence_font.wait_time = -1;
        }
        //medium default text speed
        else if ( event->keysym.sym == SDLK_2 ){
            text_speed_no = 1;
            sentence_font.wait_time = -1;
        }
        //fast default text speed
        else if ( event->keysym.sym == SDLK_3 ){
            text_speed_no = 2;
            sentence_font.wait_time = -1;
        }
    }

        //'f' is for fullscreen toggle
        if ( (event->keysym.sym == SDLK_f) && !ctrl_pressed_status ){
            if ( fullscreen_mode ) menu_windowCommand();
            else                   menu_fullCommand();
        }

    //using insani's skippable wait
    if ( event_mode & WAIT_SLEEP_MODE) {
        if (event->keysym.sym == SDLK_s )
        {
            skip_mode |= SKIP_TO_WAIT;
            skip_mode &= ~SKIP_NORMAL;
            key_pressed_flag = true;
        }
    }
    if ((skip_mode & SKIP_TO_WAIT) && 
        ((event->keysym.sym == SDLK_RETURN) ||
         (event->keysym.sym == SDLK_KP_ENTER) ||
         (event->keysym.sym == SDLK_SPACE) )) {
        skip_mode &= ~SKIP_TO_WAIT;
        key_pressed_flag = true;
    }
    if ((event_mode & WAIT_TEXTOUT_MODE) &&
        ((event->keysym.sym == SDLK_RETURN) ||
         (event->keysym.sym == SDLK_KP_ENTER) ||
         (event->keysym.sym == SDLK_SPACE) )) {
        skip_mode |= (SKIP_TO_WAIT | SKIP_TO_EOL);
        key_pressed_flag = true;
    }

#if defined(WIN32) && defined(USE_MESSAGEBOX)
    if ((event->keysym.sym == SDLK_F1) && (version_str != NULL)){
        //F1 is for Help (on Windows), so show the About dialog box
        menu_windowCommand();
        HWND pwin = NULL;
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(m_window->GetWindow(), &info))
          pwin = info.info.win.window;
        MessageBox(pwin, version_str, "About",
                   MB_OK|MB_ICONINFORMATION);

        key_pressed_flag = true;
    }
#endif

    return false;
}

void ONScripterLabel::animEvent( void )
{
    if (!(event_mode & WAIT_NO_ANIM_MODE)){
        int duration = proceedAnimation();

        if ( duration >= 0 ){
            if (anim_timer_id == NULL)
                flush(refreshMode() | (draw_cursor_flag?REFRESH_CURSOR_MODE:0));
            advanceAnimPhase( duration );
        }
    }
}

void ONScripterLabel::timerEvent( void )
{
    int duration = proceedCursorAnimation();

    if ( duration >= 0 ){
        flush(refreshMode() | (draw_cursor_flag?REFRESH_CURSOR_MODE:0));
        advancePhase( duration );
    }

    volatile_button_state.reset();
}

enum WhatToDo
{
  Nothing,
  Break,
  Return
};

int ONScripterLabel::HandleGamepadEvent(SDL_Event& event, bool had_automode, bool& ctrl_toggle)
{
    SDL_KeyboardEvent keyEvent{};
    bool relevantButton = false;

    switch (event.cbutton.button)
    {
        // Treat these as keyboard buttons
      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
      case SDL_CONTROLLER_BUTTON_B:
        keyEvent.keysym.sym = SDLK_ESCAPE; relevantButton = true; break;
      case SDL_CONTROLLER_BUTTON_A:
      case SDL_CONTROLLER_BUTTON_Y:
        keyEvent.keysym.sym = SDLK_SPACE; relevantButton = true; break;
      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
      case SDL_CONTROLLER_BUTTON_X:
        keyEvent.keysym.sym = SDLK_RETURN; relevantButton = true; break;
      case SDL_CONTROLLER_BUTTON_BACK:
      case SDL_CONTROLLER_BUTTON_GUIDE:
      case SDL_CONTROLLER_BUTTON_START:
        keyEvent.keysym.sym = SDLK_ESCAPE; relevantButton = true; break;
        //case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        //case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
      case SDL_CONTROLLER_BUTTON_DPAD_UP:
        keyEvent.keysym.sym = SDLK_UP; relevantButton = true; break;
      case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        keyEvent.keysym.sym = SDLK_DOWN; relevantButton = true; break;
      case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        keyEvent.keysym.sym = SDLK_LEFT; relevantButton = true; break;
      case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        keyEvent.keysym.sym = SDLK_RIGHT; relevantButton = true; break;
    }

    if (relevantButton)
    {
      if (event.cbutton.state == SDL_PRESSED)
      {
          keyEvent.type = SDL_KEYDOWN;

          bool ret = keyDownEvent(&keyEvent);
          ctrl_toggle ^= (ctrl_pressed_status != 0);
          //allow skipping sleep waits with start of ctrl keydown
          ret |= (event_mode & WAIT_SLEEP_MODE) && ctrl_toggle;
          if (btndown_flag)
              ret |= keyPressEvent(&keyEvent);
          if (ret) return Return;
      }
      else
      {
          keyEvent.type = SDL_KEYUP;

          keyUpEvent(&keyEvent);
          bool ret = keyPressEvent(&keyEvent);
          if (ret) return Return;
      }
    }

    return Nothing;
}

float ToFloat(Sint16 aValue)
{
  return (static_cast<float>(aValue + 32768.f) / 65535.f);
}


void ONScripterLabel::runEventLoop()
{
    bool started_in_automode = automode_flag;

    //SDL_Event temp_event;
    //for ( SDL_Event& event : m_window->PollEvents() ) {
    //SDL_Event temp_event;
    SDL_Event event;
    while (m_window->WaitEvents(event)) {
        // ignore continous SDL_MOUSEMOTION
        //while (event.type == SDL_MOUSEMOTION) {
        //    if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
        //    if (temp_event.type != SDL_MOUSEMOTION) break;
        //    SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
        //    event = temp_event;
        //}

        //fprintf(stderr, "SDLEvent: %d\n", event.type);

        bool ret = false;
        bool ctrl_toggle = (ctrl_pressed_status != 0);
        bool voice_just_ended = false;
        bool had_automode = automode_flag;

        switch (event.type) {
            // Joypad Events
          case SDL_CONTROLLERDEVICEADDED:
          {
            // We don't care which controller, we're taking input from all of them.
            SDL_GameController* pad = SDL_GameControllerOpen(event.cdevice.which);

            if (pad)
            {
              const char* name = SDL_GameControllerName(pad);
              SDL_Joystick* joystick = SDL_GameControllerGetJoystick(pad);
              SDL_JoystickID instanceId = SDL_JoystickInstanceID(joystick);

              printf("Added %s, %d\n", name, instanceId);
            }
            break;
          }
          case SDL_CONTROLLERDEVICEREMOVED:
            printf("Removed %d\n", event.cdevice.which);
            break;

          case SDL_CONTROLLERBUTTONDOWN:
          case SDL_CONTROLLERBUTTONUP:
          {
            int ret = HandleGamepadEvent(event, had_automode, ctrl_toggle);
            if (ret == WhatToDo::Break) break;
            else if (ret == WhatToDo::Return) return;
            break;
          }

          case SDL_CONTROLLERAXISMOTION:
          {
            static std::pair<float, float> left;
            static std::pair<float, float> right;
            switch (event.caxis.axis)
            {
                case SDL_CONTROLLER_AXIS_LEFTX: left.first = 2.f * (ToFloat(event.caxis.value) - .5f); break;
                case SDL_CONTROLLER_AXIS_LEFTY: left.second = 2.f * (ToFloat(event.caxis.value) - .5f); break;
                case SDL_CONTROLLER_AXIS_RIGHTX: right.first = 2.f * (ToFloat(event.caxis.value) - .5f); break;
                case SDL_CONTROLLER_AXIS_RIGHTY: right.second = 2.f * (ToFloat(event.caxis.value) - .5f); break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                break;
            }

            if (sqrt((right.first * right.first) + (right.second * right.second)) > 0.5)
                m_window->WarpMouse(screen_surface->w, screen_surface->h);
            else if (sqrt((left.first * left.first) + (left.second * left.second)) > 0.5)
                m_window->WarpMouse(screen_surface->w, screen_surface->h);
            break;
          }
          case SDL_MOUSEMOTION:
            m_window->TranslateMouse(event.motion);
            ret = mouseMoveEvent( (SDL_MouseMotionEvent*)&event );
            if (ret) return;
            break;

          case SDL_MOUSEBUTTONDOWN:
            if ( !btndown_flag ) break;
          case SDL_MOUSEBUTTONUP:
            if (!m_window->TranslateMouse(event.button)) break;
            ret = mousePressEvent( (SDL_MouseButtonEvent*)&event );
            if (ret) return;
            if (!(event_mode & WAIT_TEXTOUT_MODE) && had_automode && !automode_flag){
                clearTimer(break_id);
            }
            break;

          case SDL_MOUSEWHEEL:
            if (!m_window->TranslateMouse(event.wheel)) break;
            ret = mouseWheelEvent(&event.wheel);
            if (ret) return;
            if (!(event_mode & WAIT_TEXTOUT_MODE) && had_automode && !automode_flag) {
              clearTimer(break_id);
            }
            break;

          case SDL_JOYBUTTONDOWN:
            event.key.type = SDL_KEYDOWN;
            event.key.keysym.sym = transJoystickButton(event.jbutton.button);
            if(event.key.keysym.sym == SDLK_UNKNOWN)
                break;

          case SDL_KEYDOWN:
            event.key.keysym = transKey(event.key.keysym, true);
            ret = keyDownEvent( (SDL_KeyboardEvent*)&event );
            ctrl_toggle ^= (ctrl_pressed_status != 0);
            //allow skipping sleep waits with start of ctrl keydown
            ret |= (event_mode & WAIT_SLEEP_MODE) && ctrl_toggle;
            if ( btndown_flag )
                ret |= keyPressEvent( (SDL_KeyboardEvent*)&event );
            if (ret) return;
            break;

          case SDL_JOYBUTTONUP:
            event.key.type = SDL_KEYUP;
            event.key.keysym.sym = transJoystickButton(event.jbutton.button);
            if(event.key.keysym.sym == SDLK_UNKNOWN)
                break;

          case SDL_KEYUP:
            event.key.keysym = transKey(event.key.keysym, false);
            keyUpEvent( (SDL_KeyboardEvent*)&event );
            ret = keyPressEvent( (SDL_KeyboardEvent*)&event );
            if (ret) return;
            break;

          case SDL_JOYAXISMOTION:
          {
              SDL_KeyboardEvent ke = transJoystickAxis(event.jaxis);
              if (ke.keysym.sym != SDLK_UNKNOWN){
                  if (ke.type == SDL_KEYDOWN){
                      keyDownEvent( &ke );
                      if (btndown_flag)
                          keyPressEvent( &ke );
                  }
                  else if (ke.type == SDL_KEYUP){
                      keyUpEvent( &ke );
                      keyPressEvent( &ke );
                  }
              }
              break;
          }

          case ONS_TIMER_EVENT:
            timerEvent();
            break;

          case ONS_ANIM_EVENT:
            animEvent();
            break;

          case ONS_SOUND_EVENT:
          case ONS_CDAUDIO_EVENT:

          case ONS_BGMFADE_EVENT:
          case ONS_SEQMUSIC_EVENT:
          case ONS_MUSIC_EVENT:
            flushEventSub( event );
            break;

          case ONS_WAVE_EVENT:
            flushEventSub( event );
            //printf("ONS_WAVE_EVENT %d: %x %d %x\n", event.user.code, wave_sample[0], automode_flag, event_mode);
            if ( (event.user.code != 0) ||
                 !(event_mode & WAIT_VOICE_MODE) ) break;
            if (event_mode & WAIT_VOICE_MODE)
                voice_just_ended = true;
            event_mode &= ~WAIT_VOICE_MODE;
          case ONS_BREAK_EVENT:
            if ((event_mode & WAIT_VOICE_MODE) && wave_sample[0]){
                break;
            }
            if (voice_just_ended) {
                clearTimer(break_id);
                if (automode_flag && (automode_time > 0)) {
                    break_id = SDL_AddTimer(automode_time, breakCallback, NULL);
                    break;
                } else if (autoclick_time > 0) {
                    break_id = SDL_AddTimer(autoclick_time, breakCallback, NULL);
                    break;
                }
            }
            if (!automode_flag && started_in_automode &&
                (clickstr_state != CLICK_NONE)) {
                started_in_automode = false;
                break;
            }
            if (automode_flag || (autoclick_time > 0))
                current_button_state.set(0);
            else if (btntime_value > 0){
                if ( usewheel_flag )
                    current_button_state.set(-5);
                else
                    current_button_state.set(-2);
            } else
                current_button_state.set(0);
            if ((event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE)) && 
                ( (clickstr_state == CLICK_WAIT) || 
                  (clickstr_state == CLICK_NEWPAGE) )){
                playClickVoice(); 
                stopCursorAnimation( clickstr_state );
            }
            return;

          case SDL_WINDOWEVENT:
          {
            SDL_Rect rect = { 0, 0, screen_width, screen_height };
            switch (event.window.event)
            {
              case (SDL_WINDOWEVENT_RESIZED):
              case (SDL_WINDOWEVENT_MOVED):
                refreshSurface(accumulation_surface, &rect, refreshMode());
                SDL_BlitSurface(accumulation_surface, &rect, screen_surface, &rect);

              case (SDL_WINDOWEVENT_ENTER):
              case (SDL_WINDOWEVENT_FOCUS_GAINED):
                UpdateScreen(rect);
                break;
            }
            break;
          }

          case SDL_QUIT:
            endCommand();
            break;
#if 0
          case SDL_VIDEORESIZE:
            //Mion: beginning stab at handling resizable windows; tends to crash
            if (async_movie) SMPEG_pause( async_movie );
            SDL_FreeSurface(screen_surface);
            screen_ratio1 = event.resize.w;
            screen_ratio2 = script_width;
            screen_width  = ExpandPos(script_width);
            screen_height = ExpandPos(script_height);
            screen_surface = SetVideoMode( screen_width, screen_height, screen_bpp, DEFAULT_VIDEO_SURFACE_FLAG | SDL_RESIZABLE );
            {
                SDL_Rect rect = {0, 0, screen_width, screen_height};
                flushDirect( rect, refreshMode() );
            }
            if (async_movie){
                SMPEG_setdisplay( async_movie, screen_surface, NULL, NULL );
                SMPEG_play( async_movie );
            }
            break;
#endif
          default:
            break;
        }
    }
}


