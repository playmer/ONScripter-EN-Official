#include <vector>

#include "SDL2/SDL.h"

class Window
{
public:
    Window(int w, int h, int x, int y)
    {

    }

    virtual std::vector<SDL_Event>& PollEvents();

    SDL_Window* GetWindow()
    {
        return m_window;
    }

    bool IgnoreContinuousMouseMove = true;
private:
    SDL_Window* m_window;
    std::vector<SDL_Event> m_events;
};

// We want the alternate Window implementations to be visible to other files, but their headers
// should remain private to not poison other files with their implementation details. So we 
// provide this factory function in the main Window header.

#ifdef USE_QTWINDOW
Window* CreateQtWindow();
#endif

#ifdef USE_IMGUIWINDOW
Window* CreateImGuiWindow();
#endif
