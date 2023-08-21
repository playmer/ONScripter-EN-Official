#include "QWindow"
#include "QResizeEvent"
#include "QEventLoop"

#include "Window.h"


class QSdlWindow;

class QtWindow : public Window
{
public: 
    QtWindow(int w, int h, int x, int y);
    virtual std::vector<SDL_Event>& PollEvents();
private:
    QSdlWindow* m_qtwindow;
    QEventLoop m_eventLoop;
};
