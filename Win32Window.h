#ifndef __WIN32_WINDOW_H__
#define __WIN32_WINDOW_H__

#include "Window.h"
#include "BasicWindow.h"

#include "Windows.h"

#include <functional>
#include <map>

class Win32Window : public BasicWindow
{
public:
    Win32Window(ONScripterLabel* onscripter, int w, int h, int x, int y);
    virtual void CreateMenuBar();

private:
    static void SDLCALL MessageHook(void* userdata, void* hWnd, unsigned int message, Uint64 wParam, Sint64 lParam);


    std::map<size_t, std::function<void()>> m_actions;
    std::map<MenuBarFunction, std::vector<std::pair<HMENU, UINT_PTR>>> m_functionsToMenuItems;

    struct ActionOrMenu 
    {
        std::string display;
        UINT_PTR index;
        MenuBarFunction m_function;
        bool isAction;
    };

    ActionOrMenu CreateMenuBarInternal(MenuBarInput& input, UINT_PTR& index);
    HMENU m_menubar = NULL;
    HMENU m_menu = NULL;
};

#endif
