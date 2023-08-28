#ifndef __QT_WINDOW_H__
#define __QT_WINDOW_H__

#include "QWindow"
#include "QResizeEvent"
#include "QEventLoop"
#include "QApplication"
#include "QMainWindow"
#include "QToolBar"

#include "Window.h"


class QSdlWindow;

class QtWindow : public Window
{
public: 
    QtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y);
    virtual std::vector<SDL_Event>& PollEvents();
    virtual void WarpMouse(int x, int y);
    virtual void SetWindowCaption(const char* title, const char* icon_name);
    virtual SDL_Surface* SetVideoMode(int width, int height, int bpp, bool fullscreen);
    virtual void* GetWindowHandle();
    virtual void Repaint();
    virtual void SendCustomEvent(ONScripterCustomEvent event, int value);

    std::string Command_InputStr(std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h);

private:
    void MenuImpl_VolumeSlider();
    void MenuImpl_Version();
    void MenuImpl_Exit();

    virtual void CreateToolbar();

    QMainWindow* m_mainWindow = NULL;
    QToolBar* m_toolbar = NULL;
    QSdlWindow* m_sdlWindow = NULL;
    QWidget* m_sdlWidget = NULL;
    SDL_Event m_temp_event;

    // The order of the following members matters.
    char* argv = NULL; // Should consider fulfilling this correctly.
    int argc = 0;
    QApplication m_qtapplication;
    QEventLoop m_eventLoop;
};

#endif
