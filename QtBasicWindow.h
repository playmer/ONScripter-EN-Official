#ifndef __QT_BASICWINDOW_H__
#define __QT_BASICWINDOW_H__

#ifdef MACOSX
#include <map>

#include "QWindow"
#include "QResizeEvent"
#include "QEventLoop"
#include "QApplication"
#include "QMainWindow"
#include "QToolBar"

#include "BasicWindow.h"

struct ActionOrMenu
{
    union
    {
        QAction* m_action;
        QMenu* m_menu;
    } m_actionOrMenu;
    bool isAction;
};

class QtBasicWindow : public BasicWindow
{
public:
    QtBasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
    virtual int WaitEvents(SDL_Event& event);
    virtual int PollEvents(SDL_Event& event);
    virtual void CreateMenuBar();

    std::string Command_InputStr(std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h);

private:
    ActionOrMenu CreateMenuBarInternal(MenuBarInput& input);

    std::map<MenuBarFunction, std::vector<QAction*>> m_actionsMap;
    QMenuBar* m_menuBar;

    // The order of the following members matters.
    char* argv = NULL; // Should consider fulfilling this correctly.
    int argc = 0;
    QApplication m_qtapplication;
    QEventLoop m_eventLoop;
};

#endif
#endif
