#ifndef __WX_WINDOW_H__
#define __WX_WINDOW_H__

#include "Window.h"
#include "wx/wx.h"

class onscripter_en_app;
class WxMainWindow;

class WxWindow : public Window
{
public:
    WxWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
    virtual int WaitEvents(SDL_Event& event);
    virtual int PollEvents(SDL_Event& event);
    virtual void WarpMouse(int x, int y);
    virtual void SetWindowCaption(const char* title, const char* icon_name);
    virtual SDL_Surface* SetVideoMode(int width, int height, int bpp, bool fullscreen);
    virtual void* GetWindowHandle();
    virtual void SendCustomEvent(ONScripterCustomEvent event, int value);
    virtual void CreateMenuBar();

private:
    onscripter_en_app* m_app;
    WxMainWindow* m_mainWindow;
    wxPoint m_originalPosition;
    bool m_wasMaximized = false;
};

#endif
