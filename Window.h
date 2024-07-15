#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <string>
#include <tuple>
#include <vector>

#include "SDL2/SDL.h"

class ONScripterLabel;


enum ONScripterCustomEvent
{
    ONS_USEREVENT_START = SDL_USEREVENT,
    ONS_TIMER_EVENT = SDL_USEREVENT,
    ONS_SOUND_EVENT,
    ONS_CDAUDIO_EVENT,
    ONS_SEQMUSIC_EVENT,
    ONS_WAVE_EVENT,
    ONS_MUSIC_EVENT,
    ONS_BREAK_EVENT,
    ONS_ANIM_EVENT,
    ONS_BGMFADE_EVENT,
    ONS_USEREVENT_END,
};

enum class MenuBarFunction
{
    AUTO,
    CLICKDEF,
    CLICKPAGE,
    DWAVEVOLUME,
    END,
    FONT,
    FULL,
    kidokuoff,
    kidokuon,
    SKIP,
    SUB,
    TEXTFAST,
    TEXTMIDDLE,
    TEXTSLOW,
    VERSION,
    WAVEOFF,
    WAVEON,
    WINDOW,
    UNKNOWN,
};


MenuBarFunction functionNameToMenuBarFunction(const char* functionName);
bool IsCheckable(MenuBarFunction function);

class Window
{
public:
    Window(ONScripterLabel* onscripter);

    virtual int WaitEvents(SDL_Event& event) = 0;
    virtual int PollEvents(SDL_Event& event) = 0;
    virtual void WarpMouse(int x, int y) = 0;
    virtual void SetWindowCaption(const char* title, const char* icon_name) = 0;
    virtual SDL_Surface* SetVideoMode(int width, int height, int bpp, bool fullscreen) = 0;
    virtual void* GetWindowHandle() = 0;
    virtual void Repaint() {};


    virtual std::string Dialog_InputStr(std::string& /*display*/, int /*maximumInputLength*/, bool /*forceDoubleByte*/, const int* /*w*/, const int* /*h*/, const int* /*input_w*/, const int* /*input_h*/)
    {
        return std::string();
    };

    SDL_Window* GetWindow()
    {
        return m_window;
    }

    SDL_Renderer* GetRenderer()
    {
        return m_renderer;
    }

    // Only implemented for current Onscripter custom events.
    virtual void SendCustomEvent(ONScripterCustomEvent event, int value = 0) = 0;

    static void SendCustomEventStatic(ONScripterCustomEvent event, int value = 0)
    {
        s_window->SendCustomEvent(event, value);
    }

    void DeleteMenu()
    {
        m_menuBarEntries.clear();
        CreateMenuBar();
    }

    void ResetMenu()
    {
        m_menuBarEntries.clear();
        CreateMenuBar();
    }

    void KillMenu()
    {
        m_menuBarEntries.pop_back();
        CreateMenuBar();
    }

    void InsertMenu(MenuBarFunction function, const char* label, int depth);

    template <typename SDLEvent>
    bool TranslateMouse(SDLEvent& event)
    {
        return TranslateMouse(event.x, event.y, true);
    }

    bool IgnoreContinuousMouseMove = true;

    void InitMenuIfGameDidNot();


    static SDL_Rect CenterDialog(SDL_Rect dialog_rect, SDL_Rect window_rect)
    {
        SDL_Rect new_dialog_rect = dialog_rect;
        new_dialog_rect.x = window_rect.x + ((window_rect.w - new_dialog_rect.w) / 2);
        new_dialog_rect.y = window_rect.y + ((window_rect.h - new_dialog_rect.h) / 2);
        return new_dialog_rect;
    }

    static SDL_Rect ScaleRectToPixels(SDL_Surface* src_surface, SDL_Surface* dst_surface, SDL_Rect dst_rect);

protected:
    virtual void CreateMenuBar() = 0;
    static void ScalePositionToPixels(int w_1, int h_1, int w_2, int h_2, int& x_m, int& y_m);

    struct MenuBarInput
    {
        MenuBarInput(MenuBarFunction function, const char* label, int depth)
            : m_function(function)
            , m_label(label)
            , m_depth(depth)
        {
        }

        MenuBarInput(MenuBarFunction function, std::string label, int depth)
            : m_function(function)
            , m_label(label)
            , m_depth(depth)
        {
        }

        MenuBarInput() {}

        MenuBarFunction m_function = MenuBarFunction::UNKNOWN;
        std::string m_label;
        int m_depth = 0;
        std::vector<MenuBarInput> m_children; // Unused unless returned from ParseMenuBarTree.
    };


    bool TranslateMouse(int& x, int& y, bool toScreenSize = false);

    static void ReverseChildren(MenuBarInput& input);
    static MenuBarInput* GetCurrentParent(MenuBarInput& input, std::vector<size_t>& depthTracker);
    static MenuBarInput ParseMenuBarTree(std::vector<MenuBarInput>& menuBarEntries);

    bool IsChecked(MenuBarFunction function);
    

    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;
    ONScripterLabel* m_onscripterLabel = NULL;
    std::vector<SDL_Event> m_events;
    std::vector<MenuBarInput> m_menuBarEntries;
    bool m_menuWasInitByGame = false;

    static Window* s_window;
};

// We want the alternate Window implementations to be visible to other files, but their headers
// should remain private to not poison other files with their implementation details. So we 
// provide this factory function in the main Window header.

class BasicWindow;
Window* CreateBasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);

#ifdef USE_QT_WINDOW
class QtWindow;
Window* CreateQtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);

class QtBasicWindow;
Window* CreateQtBasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
#endif

#ifdef USE_WX_WINDOW
class WxWindow;
Window* CreateWxWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
#endif

#ifdef WIN32
class Win32Window;
Window* CreateWin32Window(ONScripterLabel* onscripter, int w, int h, int x, int y);
#endif

#ifdef USE_IMGUIWINDOW
class ImguiWindow;
Window* CreateImGuiWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
#endif

#endif
