#ifndef __QT_WINDOW_H__
#define __QT_WINDOW_H__

#include <map>

#include "QWindow"
#include "QResizeEvent"
#include "QEventLoop"
#include "QApplication"
#include "QMainWindow"
#include "QToolBar"

#include "Window.h"


class QSdlWindow;
class SdlMainWindow;

struct ActionOrMenu
{
    union
    {
        QAction* m_action;
        QMenu* m_menu;
    } m_actionOrMenu;
    bool isAction;
};

class QtWindow : public Window
{
public:
    QtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
    virtual int WaitEvents(SDL_Event& event);
    virtual int PollEvents(SDL_Event& event);
    virtual void WarpMouse(int x, int y);
    virtual void SetWindowCaption(const char* title, const char* icon_name);
    virtual SDL_Surface* SetVideoMode(int width, int height, int bpp, bool fullscreen);
    virtual void* GetWindowHandle();
    virtual void Repaint();
    virtual void SendCustomEvent(ONScripterCustomEvent event, int value);
    virtual void CreateMenuBar();

    std::string Command_InputStr(std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h);

private:
    ActionOrMenu CreateMenuBarInternal(MenuBarInput& input);

    SdlMainWindow* m_mainWindow = NULL;
    QToolBar* m_toolbar = NULL;
    QSdlWindow* m_sdlWindow = NULL;
    QWidget* m_sdlWidget = NULL;

    QPoint m_originalPosition;
    bool m_wasMaximized = false;

    std::map<MenuBarFunction, std::vector<QAction*>> m_actionsMap;

    // The order of the following members matters.
    char* argv = NULL; // Should consider fulfilling this correctly.
    int argc = 0;
    QApplication m_qtapplication;
    QEventLoop m_eventLoop;

    friend SdlMainWindow;
};

#endif
