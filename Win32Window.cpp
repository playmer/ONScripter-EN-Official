#ifndef __WIN32_WINDOW_H__
#define __WIN32_WINDOW_H__

#include "Window.h"


class Win32Window : public Window
{
public:
    Win32Window(ONScripterLabel* onscripter, int w, int h, int x, int y);
    virtual int WaitEvents(SDL_Event& event);
    virtual int PollEvents(SDL_Event& event);
    virtual void WarpMouse(int x, int y);
    virtual void SetWindowCaption(const char* title, const char* icon_name);
    virtual SDL_Surface* SetVideoMode(int width, int height, int bpp, bool fullscreen);
    virtual void* GetWindowHandle();
    virtual void SendCustomEvent(ONScripterCustomEvent event, int value);
    virtual void CreateMenuBar();

private:
};

#endif
