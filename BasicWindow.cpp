#include "BasicWindow.h"
#include "ONScripterLabel.h"
#include "SDL_image.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#if defined(WIN32) || defined(APPLE)
    #include "SDL_syswm.h"
#endif

Window* CreateBasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new BasicWindow(onscripter, w, h, x, y);
}

BasicWindow::BasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
    : Window(onscripter)
{
    if (s_window != NULL)
        fprintf(stderr, "Game has created two instances of Window, there should only be one.");
    s_window = this;
    //SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI, &m_window, &m_renderer);
    m_window = SDL_CreateWindow("", 0, 0, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    m_renderer = NULL;
    SDL_SetWindowPosition(m_window, x, y);
}

int BasicWindow::PollEvents(SDL_Event& event)
{
    int pending = SDL_PollEvent(&event);

#ifdef __EMSCRIPTEN__
    if (pending)
    {
        emscripten_sleep(1);
    }
#endif
    return pending;
}

int BasicWindow::WaitEvents(SDL_Event& event)
{
#ifdef __EMSCRIPTEN__
    emscripten_sleep(1);
#endif
    auto ret = SDL_WaitEvent(&event);
    SDL_Event temp_event;
    while (IgnoreContinuousMouseMove && event.type == SDL_MOUSEMOTION) {
        if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
        if (temp_event.type != SDL_MOUSEMOTION) break;
        SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
        event = temp_event;
    }

    return ret;
}

void BasicWindow::WarpMouse(int x, int y)
{
    int windowResolutionX, windowResolutionY;
    SDL_GetWindowSizeInPixels(m_window, &windowResolutionX, &windowResolutionY);

    ScalePositionToPixels(windowResolutionX, windowResolutionY, m_onscripterLabel->screen_surface->w, m_onscripterLabel->screen_surface->h, x, y);
    TranslateMouse(x, y);

    SDL_WarpMouseInWindow(m_window, x, y);
}

void BasicWindow::SetWindowCaption(const char* title, const char* icon_name)
{
    SDL_SetWindowTitle(m_window, title);

    SDL_Surface* icon = IMG_Load(icon_name);
    if (icon != NULL) {
        SDL_SetWindowIcon(m_window, icon);
        SDL_FreeSurface(icon);
    }
}

SDL_Surface* BasicWindow::SetVideoMode(int width, int height, int bpp, bool fullscreen)
{
    SDL_SetWindowSize(m_window, width, height);
    SDL_SetWindowFullscreen(m_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

    return SDL_GetWindowSurface(m_window);
}

// Ideally we remove this function completely, once we remove using SendMessage to set the Window Icon
// on Window, hence why this is not fully implemented for Linux.
void* BasicWindow::GetWindowHandle()
{
#ifdef WIN32
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_window, &info);
    return info.info.win.window;
#elif defined(APPLE)
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_window, &info);
    return info.info.cocoa.window;
#else
    return NULL;
#endif
}

void BasicWindow::SendCustomEvent(ONScripterCustomEvent eventType, int value)
{
    SDL_Event event;
    event.type = eventType;
    event.user.code = value;
    SDL_PushEvent(&event);
}

void BasicWindow::CreateMenuBar()
{
}
