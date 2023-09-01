#include "BasicWindow.h"
#include "ONScripterLabel.h"
#include "SDL_image.h"

#include "SDL_syswm.h"

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
    SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI, &m_window, &m_renderer);
    SDL_SetWindowPosition(m_window, x, y);
}

int BasicWindow::PollEvents(SDL_Event& event)
{
    return SDL_PollEvent(&event);
}
int BasicWindow::WaitEvents(SDL_Event& event)
{
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




static void ScaleMouseToPixels(int w_1, int h_1, int w_2, int h_2, int& x_m, int& y_m)
{
    // Scale the mouse to Window (pixel) coordinates
    float scaleWidth = w_1 / (float)w_2;
    float scaleHeight = h_1 / (float)h_2;
    float scale = std::min(scaleHeight, scaleWidth);

    SDL_Rect dstRect = {};
    dstRect.w = scale * w_2;
    dstRect.h = scale * h_2;
    dstRect.x = (w_1 - dstRect.w) / 2;
    dstRect.y = (h_1 - dstRect.h) / 2;

    x_m = (x_m * scale) + dstRect.x;
    y_m = (y_m * scale) + dstRect.y;

}

void BasicWindow::WarpMouse(int x, int y)
{
    int windowResolutionX, windowResolutionY;
    SDL_GetWindowSizeInPixels(m_window, &windowResolutionX, &windowResolutionY);

    ScaleMouseToPixels(windowResolutionX, windowResolutionY, m_onscripterLabel->screen_surface->w, m_onscripterLabel->screen_surface->h, x, y);
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

    return m_onscripterLabel->screen_surface;
}

void* BasicWindow::GetWindowHandle()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_window, &info);
#ifdef WIN32
    return info.info.win.window;
#elsif defined(APPLE)
    return info.info.cocoa.window;
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
