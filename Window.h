#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <string>
#include <tuple>
#include <vector>

#include "SDL2/SDL.h"

class ONScripterLabel;

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

enum ONScripterCustomEvent;

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

    void InsertMenu(MenuBarFunction function, const char* label, int depth)
    {
        m_menuBarEntries.emplace_back(function, label, depth);
        CreateMenuBar();
    }

    bool IgnoreContinuousMouseMove = true;
protected:
    virtual void CreateMenuBar() = 0;

    struct MenuBarInput
    {
        MenuBarInput(MenuBarFunction function, const char* label, int depth)
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


    bool TranslateMouse(int& x, int& y);

    static void ReverseChildren(MenuBarInput& input);
    static MenuBarInput* GetCurrentParent(MenuBarInput& input, std::vector<size_t>& depthTracker);
    MenuBarInput ParseMenuBarTree();

    bool IsChecked(MenuBarFunction function);
    

    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;
    ONScripterLabel* m_onscripterLabel = NULL;
    std::vector<SDL_Event> m_events;

    std::vector<MenuBarInput> m_menuBarEntries;

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
#endif

#ifdef USE_IMGUIWINDOW
class ImguiWindow;
Window* CreateImGuiWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
#endif

#endif
