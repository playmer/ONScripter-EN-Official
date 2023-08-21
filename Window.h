#include <vector>

#include "SDL2/SDL.h"

class Window
{
public:
    virtual std::vector<SDL_Event>& PollEvents();

    SDL_Window* GetWindow()
    {
        return m_window;
    }

    SDL_Renderer* GetRenderer()
    {
        return m_renderer;
    }

    bool IgnoreContinuousMouseMove = true;
protected:
    Window(int w, int h, int x, int y);

    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;
    std::vector<SDL_Event> m_events;
};

// We want the alternate Window implementations to be visible to other files, but their headers
// should remain private to not poison other files with their implementation details. So we 
// provide this factory function in the main Window header.

Window* CreateDummyWindow(int w, int h, int x, int y);

#ifdef USE_QTWINDOW
Window* CreateQtWindow(int w, int h, int x, int y);
#endif

#ifdef USE_IMGUIWINDOW
Window* CreateImGuiWindow(int w, int h, int x, int y);
#endif
