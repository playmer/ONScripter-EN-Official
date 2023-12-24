#include "WxPlatform.h"
#include "ONScripterLabel.h"

#include "wx/wx.h"


Window* CreateWxWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new WxWindow(onscripter, w, h, x, y);
}

class onscripter_en_app : public wxApp
{
};

wxIMPLEMENT_APP_NO_MAIN(onscripter_en_app);

class WxMainWindow : public wxFrame
{
public:

    /** Constructor. Creates a new TextFrame */
    WxMainWindow(int xpos, int ypos, int width, int height)
        : wxFrame()
    {
        wxPoint position(xpos, ypos);
        SetPosition(position);
        wxSize size(width, height);
        SetSize(size);


        Show();
    }

private:

    wxTextCtrl* m_pTextCtrl;
    wxMenuBar* m_pMenuBar;
    wxMenu* m_pFileMenu;
    wxMenu* m_pHelpMenu;
};

class ONScripterCustomWxEvent : public wxEvent
{
public:
    ONScripterCustomWxEvent(wxEventType eventType, int winid, ONScripterCustomEvent customEvent, int value)
        : wxEvent(winid, eventType), m_customEvent(customEvent), m_value(value) 
    {
    };

    // implement the base class pure virtual
    virtual wxEvent* Clone() const { return new ONScripterCustomWxEvent(*this); }

    ONScripterCustomEvent m_customEvent;
    int m_value;
};

wxDEFINE_EVENT(ONSCRIPTER_CUSTOM_EVENT, ONScripterCustomWxEvent);


WxWindow::WxWindow(ONScripterLabel* onscripter, int w, int h, int x, int y) 
    : Window(onscripter)
{
    if (s_window != NULL)
        fprintf(stderr, "Game has created two instances of Window, there should only be one.");
    s_window = this;

    wxInitialize(onscripter->argc, onscripter->argv);
    //if (wxEntryStart(onscripter->argc, onscripter->argv))
    //    wxEntryCleanup();
    //
    //m_app = new onscripter_en_app();
    m_mainWindow = new WxMainWindow(x, y, w, h);

    

    //while (m_app->Pending())
    //    m_app->Dispatch();

    m_window = SDL_CreateWindowFrom(m_mainWindow->GetHWND());
    m_renderer = SDL_CreateRenderer(m_window, 0, 0);
}

int WxWindow::WaitEvents(SDL_Event& event)
{
    //while (!m_app->Pending()) {
    //    SDL_Delay(1);
    //}
    //
    //while (m_app->Pending())
    //    m_app->Dispatch();

    return 1;
}

int WxWindow::PollEvents(SDL_Event& event)
{
    //while (m_app->Pending())
    //    m_app->Dispatch();
    //m_app->Dispatch();

    return SDL_PollEvent(&event);
}


void WxWindow::WarpMouse(int x, int y)
{

}

void WxWindow::SetWindowCaption(const char* title, const char* icon_name)
{
    wxString wxTitle = title;
    m_mainWindow->SetTitle(wxTitle);
}

SDL_Surface* WxWindow::SetVideoMode(int width, int height, int bpp, bool fullscreen)
{
    // In both paths here we're doing some additional event processing in between focus requests.
    // This is a best effort attempt to maintain window _and_ widget focus after a fullscreen transition.
//    auto focusSwap = [this]()
//        {
//            m_eventLoop.processEvents(QEventLoop::AllEvents);
//            m_mainWindow->setFocus();
//            m_eventLoop.processEvents(QEventLoop::AllEvents);
//            m_sdlWidget->setFocus();
//            m_eventLoop.processEvents(QEventLoop::AllEvents);
//        };
//
    //if (fullscreen)
    //{
    //    m_originalPosition = m_mainWindow->pos();
    //    m_originalGeometry = m_sdlWidget->geometry();
    //    m_wasMaximized = m_mainWindow->isMaximized();
    //    m_mainWindow->menuBar()->hide();
    //    m_mainWindow->showFullScreen();
    //
    //    focusSwap();
    //}
    //else if (m_wasMaximized)
    //{
    //    m_mainWindow->menuBar()->show();
    //    m_mainWindow->showMaximized();
    //
    //    focusSwap();
    //
    //    m_wasMaximized = false;
    //}
    //else
    //{
    //    m_mainWindow->menuBar()->show();
    //    m_mainWindow->resize(width, height);
    //    m_mainWindow->showNormal();
    //
    //    focusSwap();
    //
    //    m_mainWindow->move(m_originalPosition);
    //
    //    // Adjust the total height of the window so that the widget we're rendering into is the requested
    //    // size, otherwise the menubar will take some height from it.
    //    int menubarHeight = m_mainWindow->menuBar()->geometry().height();
    //    m_mainWindow->resize(width, height + menubarHeight);
    //
    //    m_wasMaximized = false;
    //}

    if (fullscreen) {
        m_originalPosition = m_mainWindow->GetPosition();
        m_wasMaximized = m_mainWindow->IsMaximized();
        m_mainWindow->ShowFullScreen(fullscreen);
        m_mainWindow->Show();
    }
    else if (m_wasMaximized) {
        m_mainWindow->Maximize();
        m_mainWindow->Show();

        m_wasMaximized = false;
    }
    else {
        wxPoint position(m_originalPosition);
        m_mainWindow->SetPosition(position);
        wxSize size(width, height);
        m_mainWindow->SetSize(size);
        m_mainWindow->Show();

        m_wasMaximized = false;
    }

    return m_onscripterLabel->screen_surface;
}

void* WxWindow::GetWindowHandle()
{
    return m_mainWindow->GetHandle();
}

void WxWindow::SendCustomEvent(ONScripterCustomEvent event, int value)
{
    m_app->QueueEvent(new ONScripterCustomWxEvent(ONSCRIPTER_CUSTOM_EVENT, m_mainWindow->GetId(), event, value));
}

void WxWindow::CreateMenuBar()
{

}

