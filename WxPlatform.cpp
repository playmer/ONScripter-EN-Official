#include "WxPlatform.h"
#include "ONScripterLabel.h"

#include "wx/wx.h"

#include "wx/evtloop.h"








/*



class MyApp: public wxApp
{
public:
    virtual bool OnInit();
};



*/

class MyFrame;

class onscripter_en_app : public wxApp
{
public:

    void preinit(int xpos, int ypos, int width, int height)
    {
        x = xpos;
        y = ypos;
        w = width;
        h = height;
    }

    bool OnInit() override;

    void DrainSdlEvents()
    {
        for (SDL_Event& local_event : m_events)
        {
            if (local_event.type == SDL_WINDOWEVENT)
                local_event.window.windowID = SDL_GetWindowID(m_sdlWindow);
            SDL_PushEvent(&local_event);
        }
        m_events.clear();
    }

    int w, h, x, y;
    //WxMainWindow* m_mainWindow;
    MyFrame* m_mainWindow = NULL;
    SDL_Window* m_sdlWindow = NULL;;
    std::vector<SDL_Event> m_events;
};

wxIMPLEMENT_APP_NO_MAIN(onscripter_en_app);



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

wxDEFINE_EVENT(ONSCRIPTER_CUSTOM_EVENT_SENT, ONScripterCustomWxEvent);


#define MY_EVT_ONSCRIPTER_EVENT_SENT(id, func) \
    wx__DECLARE_EVT1(ONSCRIPTER_CUSTOM_EVENT_SENT, id, (&func))



class MyFrame: public wxFrame
{
public:
    MyFrame(onscripter_en_app* app, const wxString& title, const wxPoint& pos, const wxSize& size);

    onscripter_en_app* m_app;
private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnCustomEvent(ONScripterCustomWxEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    wxDECLARE_EVENT_TABLE();
};

enum
{
    ID_Hello = 1
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Hello,   MyFrame::OnHello)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    MY_EVT_ONSCRIPTER_EVENT_SENT(wxID_ANY, MyFrame::OnCustomEvent)
    EVT_IDLE(MyFrame::OnIdle)
    EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()
//wxIMPLEMENT_APP(MyApp);


//bool MyApp::OnInit()
//{
//    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
//    frame->Show( true );
//    return true;
//}

MyFrame::MyFrame(onscripter_en_app* app, const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size), m_app(app)
{
    //wxMenu *menuFile = new wxMenu;
    //menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
    //                 "Help string shown in status bar for this menu item");
    //menuFile->AppendSeparator();
    //menuFile->Append(wxID_EXIT);
    //wxMenu *menuHelp = new wxMenu;
    //menuHelp->Append(wxID_ABOUT);
    //wxMenuBar *menuBar = new wxMenuBar;
    //menuBar->Append( menuFile, "&File" );
    //menuBar->Append( menuHelp, "&Help" );
    //SetMenuBar( menuBar );
    //CreateStatusBar();
    //SetStatusText( "Welcome to wxWidgets!" );

    app->Bind(ONSCRIPTER_CUSTOM_EVENT_SENT, &MyFrame::OnCustomEvent, this);
}

void MyFrame::OnIdle(wxIdleEvent& event)
{
    //printf("idle\n");
    //printf("\t IdleEvent\n");
    wxTheApp->GetMainLoop()->Exit();
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "This is a wxWidgets' Hello world sample",
                  "About Hello World", wxOK | wxICON_INFORMATION );
}
void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}


void MyFrame::OnCustomEvent(ONScripterCustomWxEvent& event)
{
    SDL_Event sdl_event;
    sdl_event.type = event.m_customEvent;
    sdl_event.user.code = event.m_value;

    m_app->m_events.push_back(sdl_event);

    //printf("\t Custom Event Received\n");
    wxTheApp->GetMainLoop()->Exit();
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    SDL_Event sdl_event;
    sdl_event.type = SDL_QUIT;
    m_app->m_events.push_back(sdl_event);
    event.Skip();  // you may also do:  event.Skip();
    // since the default event handler does call Destroy(), too
}













bool onscripter_en_app::OnInit()
{
    //m_mainWindow = new WxMainWindow(x, y, w, h);
    wxSize size(w, h);
    wxPoint pos(x, y);

    m_mainWindow = new MyFrame(this ,"Hello World", pos, size);
    m_mainWindow->Show();
    return true;
}




















Window* CreateWxWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new WxWindow(onscripter, w, h, x, y);
}

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
    }

private:

    wxTextCtrl* m_pTextCtrl;
    wxMenuBar* m_pMenuBar;
    wxMenu* m_pFileMenu;
    wxMenu* m_pHelpMenu;
};


WxWindow::WxWindow(ONScripterLabel* onscripter, int w, int h, int x, int y) 
    : Window(onscripter)
{
    if (s_window != NULL)
        fprintf(stderr, "Game has created two instances of Window, there should only be one.");
    s_window = this;

    m_app = new onscripter_en_app();

    m_app->preinit(x, y, w, h);

    wxApp::SetInstance(m_app);
    wxEntryStart(onscripter->argc, onscripter->argv);
    wxTheApp->OnInit();

    // this performs the wxWidgets event loop, 
    // only returning after the user has 'closed' the GUI
    wxTheApp->OnRun();



//    wxInitialize(onscripter->argc, onscripter->argv);
//    //if (wxEntryStart(onscripter->argc, onscripter->argv))
//    //    wxEntryCleanup();
//    //
//    //m_app = new onscripter_en_app();
//    m_mainWindow = new WxMainWindow(x, y, w, h);
//
//    
//
//    //while (m_app->Pending())
//    //    m_app->Dispatch();
//
//
#ifdef WIN32
    m_window = SDL_CreateWindowFrom(m_app->m_mainWindow->GetHWND());
#elif defined(MACOSX)
#else
    // Maybe needs to be a wxPanel instead of Frame?
    //GtkWidget* widget = m_app->m_mainWindow->GetHandle();
    //gtk_widget_realize(widget);
    //Window xid = GDK_WINDOW_XWINDOW(widget->window);
    //SDL_Window* win = SDL_CreateWindowFrom(xid);
#endif
    m_renderer = SDL_CreateRenderer(m_window, 0, 0);

    m_app->m_sdlWindow = m_window;
}

WxWindow::~WxWindow()
{
    // cleaning up...
    wxTheApp->OnExit();
    wxEntryCleanup();
}

int WxWindow::WaitEvents(SDL_Event& event)
{
    static int i = 0;
    auto processWxEvents = [this]() {
            m_app->OnRun();
            //printf("\t Ran WxEventLoop\n");
            for (SDL_Event& event : m_app->m_events)
            {
                if (event.type == SDL_WINDOWEVENT)
                    event.window.windowID = SDL_GetWindowID(m_window);
                SDL_PushEvent(&event);
                //printf("\t Pushed Event\n");
            }
            m_app->m_events.clear();
        };

    //printf("Wait Call: %d\n", i);

    while (true)
    {
        // Before processing any events from the OS/WxWidgets, drain the SDL queue.
        bool ret = SDL_PollEvent(&event) == 1;

        if (!ret)
        {
            processWxEvents();
            ret = SDL_PollEvent(&event) == 1;
        }

        SDL_Event temp_event;

        // ignore continous SDL_MOUSEMOTION
        while (IgnoreContinuousMouseMove && event.type == SDL_MOUSEMOTION) {
            processWxEvents();
            if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
            if (temp_event.type != SDL_MOUSEMOTION) break;
            SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            event = temp_event;
        }

        if (ret)
        {
            //fprintf(stderr, "SDLEvent: %d\n", event.type);
            ++i;
            return 1;
        }
        SDL_Delay(6);
    }

    //processWxEvents();
    //int ret = SDL_PollEvent(&event);
    //
    //while (ret == 0) {
    //    SDL_Delay(1);
    //    processWxEvents();
    //    ret = SDL_PollEvent(&event);
    //}
    //
    //return ret;
}

int WxWindow::PollEvents(SDL_Event& event)
{
    m_app->DrainSdlEvents();

    // Before processing any events from the OS/WxWidgets, drain the SDL queue.
    int ret = SDL_PollEvent(&event) == 1;

    if (ret == 1) {
        return ret;
    }
    m_app->OnRun();
    m_app->DrainSdlEvents();

    return SDL_PollEvent(&event);
}


void WxWindow::WarpMouse(int x, int y)
{

}

void WxWindow::SetWindowCaption(const char* title, const char* icon_name)
{
    wxString wxTitle = title;
    m_app->m_mainWindow->SetTitle(wxTitle);
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
        m_originalPosition = m_app->m_mainWindow->GetPosition();
        m_wasMaximized = m_app->m_mainWindow->IsMaximized();
        m_app->m_mainWindow->ShowFullScreen(fullscreen);
        m_app->m_mainWindow->Show();
    }
    else if (m_wasMaximized) {
        m_app->m_mainWindow->Maximize();
        m_app->m_mainWindow->Show();

        m_wasMaximized = false;
    }
    else {
        wxPoint position(m_originalPosition);
        m_app->m_mainWindow->SetPosition(position);
        wxSize size(width, height);
        m_app->m_mainWindow->SetSize(size);
        m_app->m_mainWindow->Show();

        m_wasMaximized = false;
    }

    return m_onscripterLabel->screen_surface;
}

void* WxWindow::GetWindowHandle()
{
    return m_app->m_mainWindow->GetHandle();
}

void WxWindow::SendCustomEvent(ONScripterCustomEvent event, int value)
{
    ONScripterCustomWxEvent* event_to_send = new ONScripterCustomWxEvent(ONSCRIPTER_CUSTOM_EVENT_SENT, m_app->m_mainWindow->GetId(), event, value);
    event_to_send->SetEventObject(m_app->m_mainWindow);
    m_app->QueueEvent(event_to_send);

    //ONScripterCustomWxEvent event_to_send(ONSCRIPTER_CUSTOM_EVENT_SENT, m_app->m_mainWindow->GetId(), event, value);
    //event_to_send.SetEventObject(m_app->m_mainWindow);
    //m_app->m_mainWindow->ProcessWindowEvent(event_to_send);

    //m_app->QueueEvent(new ONScripterCustomWxEvent(ONSCRIPTER_CUSTOM_EVENT_SENT, m_app->m_mainWindow->GetId(), event, value));
}

void WxWindow::CreateMenuBar()
{

}

