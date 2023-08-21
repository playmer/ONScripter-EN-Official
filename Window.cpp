#include "Window.h"

Window::Window(int w, int h, int x, int y)
{

}

std::vector<SDL_Event>& Window::PollEvents()
{
    return m_events;
}
