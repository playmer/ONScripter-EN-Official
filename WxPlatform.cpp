#include <map>

#include "WxPlatform.h"
#include "ONScripterLabel.h"

#include "wx/wx.h"
#include "wx/dialog.h"
#include "wx/modalhook.h"

#include "wx/evtloop.h"

/////////////////////
// Forward Declarations
class MyFrame;


/////////////////////
// Dialogues

class wxDialogModalData_OnScripter
{
public:
    wxDialogModalData_OnScripter(wxDialog* dialog) : m_evtLoop(dialog) { }

    void RunLoop()
    {
        m_evtLoop.Run();
    }

    void ExitLoop()
    {
        m_evtLoop.ScheduleExit();
        //m_evtLoop.Exit();
    }

private:
    wxModalEventLoop m_evtLoop;
};

wxDEFINE_TIED_SCOPED_PTR_TYPE(wxDialogModalData_OnScripter)

// This is used for emulating buttons on the menubar. By creating and closing a Dialog, we return focus to the
// window rather than opening the empty menu.
class InvisibleDialog : public wxDialog
{
public:

    InvisibleDialog(wxWindow* parent, wxWindowID id)
        : wxDialog(parent, id, "InvisibleDialoge", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
    {
    }
    bool Show(bool show = true) override
    {
        if (show)
            EndModal(0);
        return true;
    }

    int ShowModal() override
    {
        WX_HOOK_MODAL_DIALOG();
    
        wxASSERT_MSG(!IsModal(), wxT("ShowModal() can't be called twice"));
    
        wxDialogModalData_OnScripterTiedPtr modalData(&m_modalData,
            new wxDialogModalData_OnScripter(this));
    
        Show();
    
        // EndModal may have been called from InitDialog handler (called from
        // inside Show()) and hidden the dialog back again
        if (IsShown()) {
            modalData->RunLoop();
        }
        else {
            m_modalData->ExitLoop();
        }
    }

    void OnIdle(wxIdleEvent& event)
    {
        //EndModal(0);
    }

    wxTextCtrl* dialogText;

private:
    void OnOk(wxCommandEvent& event);

    wxDialogModalData_OnScripter* m_modalData;

    DECLARE_EVENT_TABLE()
};


wxBEGIN_EVENT_TABLE(InvisibleDialog, wxDialog)
    EVT_IDLE(InvisibleDialog::OnIdle)
wxEND_EVENT_TABLE()


class ExitDialog : public wxDialog
{
public:

    ExitDialog(wxWindow* parent, wxWindowID id, const std::string& title, const std::string& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE)
        : wxDialog(parent, id, title, pos, size, style)
    {
        //wxString dimensions = "", s;
        //wxPoint p;
        //wxSize  sz;
        //
        //sz.SetWidth(size.GetWidth() - 20);
        //sz.SetHeight(size.GetHeight() - 70);
        //
        //p.x = 6; p.y = 2;
        //s.Printf(_(" x = %d y = %d\n"), p.x, p.y);
        //dimensions.append(s);
        //s.Printf(_(" width = %d height = %d\n"), sz.GetWidth(), sz.GetHeight());
        //dimensions.append(s);
        //dimensions.append(AUTHOR);
        //
        //dialogText = new wxTextCtrl(this, -1, dimensions, p, sz, wxTE_MULTILINE);
        //

        SetTitle("Exit Dialog");
        //SetIcon()

        wxBoxSizer* buttonLayout = new wxBoxSizer(wxHORIZONTAL);
        wxButton* yesButton = new wxButton(this, wxID_OK, _("Yes"));
        wxButton* noButton = new wxButton(this, wxID_CANCEL, _("No"));
        buttonLayout->Add(yesButton, wxEXPAND);
        buttonLayout->Add(noButton, wxEXPAND);

        wxBoxSizer* mainLayout = new wxBoxSizer(wxVERTICAL);
        mainLayout->SetMinSize(wxSize(300, 100));

        wxStaticText* labelText = new wxStaticText(this, wxID_ANY, label);

        mainLayout->Add(labelText, 1, wxALIGN_CENTER | wxALIGN_CENTRE_VERTICAL | wxALIGN_CENTRE_HORIZONTAL, FromDIP(10));
        mainLayout->Add(buttonLayout, 0, wxALIGN_CENTER, FromDIP(10));
        SetSizerAndFit(mainLayout);
    }

    wxTextCtrl* dialogText;

private:
    void OnOk(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};


wxBEGIN_EVENT_TABLE(ExitDialog, wxDialog)
wxEND_EVENT_TABLE()


/////////////////////
// App
class onscripter_en_app : public wxApp
{
public:

    void preinit(WxWindow* m_onscripterWindow, int xpos, int ypos, int width, int height)
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
    std::map<MenuBarFunction, std::vector<wxMenuItem*>> m_actionsMap;
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
    void OnMenuOpen(wxMenuEvent& event);

    wxDECLARE_EVENT_TABLE();

    bool m_modalOpen = false;
};

enum
{
    ID_Hello = 1
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Hello,   MyFrame::OnHello)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    EVT_MENU_OPEN(MyFrame::OnMenuOpen)
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

void MyFrame::OnMenuOpen(wxMenuEvent& event)
{
    wxMenu* menu = event.GetMenu();
    if (event.GetMenu()->GetMenuItems().size() == 0) {
        // 
        //wxMessageBox("Testing", "woo");
        
        //auto exit = new ExitDialog(this, wxNewEventType(), "Exit Dialog", "Would you like to exit the game?");
        //exit->ShowModal();
        
        //m_modalOpen = true;
        //auto invisible = new InvisibleDialog(this, wxNewEventType());
        //
        //invisible->ShowModal();
        //{
        //}
        //m_modalOpen = false;

        //wxTheApp->GetMainLoop()->Exit();
    }
    //if (event.GetMenu() == helpMenu)
    //{
    //    wxLogDebug("help menu opened");
    //}
}

void MyFrame::OnIdle(wxIdleEvent& event)
{
    //printf("idle\n");
    //printf("\t IdleEvent\n");
    if (!m_modalOpen)
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
    if (!m_modalOpen)
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

    m_app->preinit(this, x, y, w, h);

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
    auto processWxEvents = [this]() {
        m_app->OnRun();
        for (SDL_Event& event : m_app->m_events)
        {
            if (event.type == SDL_WINDOWEVENT)
                event.window.windowID = SDL_GetWindowID(m_window);
            SDL_PushEvent(&event);
        }

        m_app->m_events.clear();
    };

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

        // ignore continuous SDL_MOUSEMOTION
        while (IgnoreContinuousMouseMove && event.type == SDL_MOUSEMOTION) {
            processWxEvents();
            if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
            if (temp_event.type != SDL_MOUSEMOTION) break;
            SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            event = temp_event;
        }

        if (ret)
        {
            return 1;
        }

        SDL_Delay(6);
    }
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

WxActionOrMenu WxWindow::CreateMenuBarInternal(MenuBarInput& input, bool topLevel)
{
    WxActionOrMenu toReturn;
    switch (input.m_function)
    {
        case MenuBarFunction::SUB:
        {
            wxMenu* menu = new wxMenu(input.m_label);

            for (auto& menuBarEntry : input.m_children)
            {
                WxActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry);

                if (actionOrMenu.isAction)
                    menu->Append(actionOrMenu.m_actionOrMenu.m_action);
                else
                    menu->AppendSubMenu(actionOrMenu.m_actionOrMenu.m_menu, actionOrMenu.m_actionOrMenu.m_menu->GetTitle());
            }

            toReturn.isAction = false;
            toReturn.m_actionOrMenu.m_menu = menu;
            break;
        }
        case MenuBarFunction::FONT:
        {
            wxMenu* menu = new wxMenu(input.m_label);

            for (auto& font : m_onscripterLabel->GetFonts())
            {
                wxMenuItem* action = NULL;

                if (font.m_fontToUse) {
                    //QObject::connect(action, &QAction::triggered, [this, thisAction = action, font = font.m_fontToUse]() {
                    //    m_onscripterLabel->ChangeFont(font);
                    //    for (auto& action : m_actionsMap[MenuBarFunction::FONT]) {
                    //        action->setChecked(false);
                    //    }
                    //
                    //    thisAction->setChecked(true);
                    //    });
                    action = new wxMenuItem(NULL, wxNewId(), font.m_fontToUse->m_name, "", wxITEM_RADIO, NULL);
                    action->Check(m_onscripterLabel->font_file ? font.m_fontToUse->m_availiblePath == m_onscripterLabel->font_file : false);
                    action->Enable(font.m_fontToUse->m_availiblePath.empty());
                }
                else {
                    action = new wxMenuItem(NULL, wxNewId(), font.m_name, "", wxITEM_RADIO, NULL);
                    action->Check(false);
                    action->Enable(false);
                }

                menu->Append(action);
                m_app->m_actionsMap[input.m_function].emplace_back(action);
            }

            toReturn.isAction = false;
            toReturn.m_actionOrMenu.m_menu = menu;
            break;
        }
        default:
        {
            //QAction* action = new QAction(QString::fromStdString(input.m_label));
            //m_actionsMap[input.m_function].emplace_back(action);
            //
            //if (IsCheckable(input.m_function))
            //{
            //    action->setCheckable(true);
            //    action->setChecked(IsChecked(input.m_function));
            //}

            //m_app->m_mainWindow->SetFocus();
            wxItemKind kind = wxITEM_NORMAL;

            if (IsCheckable(input.m_function))
            {
                kind = wxITEM_CHECK;
            }

            switch (input.m_function)
            {
            case MenuBarFunction::AUTO:
            case MenuBarFunction::CLICKDEF:
            case MenuBarFunction::CLICKPAGE:
            case MenuBarFunction::DWAVEVOLUME:
            case MenuBarFunction::END:
            case MenuBarFunction::kidokuoff:
            case MenuBarFunction::kidokuon:
            case MenuBarFunction::SKIP:
            case MenuBarFunction::TEXTFAST:
            case MenuBarFunction::TEXTMIDDLE:
            case MenuBarFunction::TEXTSLOW:
            case MenuBarFunction::VERSION:
            case MenuBarFunction::WAVEOFF:
            case MenuBarFunction::WAVEON:
            case MenuBarFunction::FULL:
            case MenuBarFunction::WINDOW:
                if (topLevel) {
                    toReturn.m_actionOrMenu.m_menu = new wxMenu(input.m_label);
                    //toReturn.m_actionOrMenu.m_menu
                }
                else {
                    toReturn.m_actionOrMenu.m_action = new wxMenuItem(NULL, wxNewId(), input.m_label, "", kind, NULL);
                }
                break;
            default:
                break;
            }

            toReturn.isAction = true;

            break;
        }
    }

    return toReturn;
}

void WxWindow::CreateMenuBar()
{
    MenuBarInput menuBarTree = ParseMenuBarTree(m_menuBarEntries);

    wxMenuBar* menuBar = new wxMenuBar();

    for (auto& menuBarEntry : menuBarTree.m_children)
    {
        WxActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry, true);
        menuBar->Append(actionOrMenu.m_actionOrMenu.m_menu, actionOrMenu.m_actionOrMenu.m_menu->GetTitle());
    }
    m_app->m_mainWindow->SetMenuBar(menuBar);
}

