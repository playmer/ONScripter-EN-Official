#include "QInputDialog"

#include "QLabel"
#include "QMenuBar"
#include "QPushButton"

#include "QBoxLayout"

#include "ONScripterLabel.h"
#include "QtWindow.h"


Window* CreateQtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new QtWindow(onscripter, w, h, x, y);
}

enum Renderer
{
    Dx11,
    Metal,
    OpenGl
};

//Render Driver: direct3d
//Render Driver: direct3d11
//Render Driver: direct3d12
//Render Driver: opengl
//Render Driver: opengles2
//Render Driver: software
const char* GetRendererName(Renderer renderer)
{
    switch (renderer)
    {
        case Renderer::Dx11: return "direct3d11";
        case Renderer::Metal: return "metal";
        case Renderer::OpenGl: return "opengl";
        default: return "";
    }
}

class ONScripterCustomQtEvent : public QEvent
{
public:
    ONScripterCustomQtEvent(ONScripterCustomEvent customEvent, int value)
        : QEvent(QEvent::User), m_customEvent(customEvent), m_value(value) {};
    ONScripterCustomEvent m_customEvent;
    int m_value;
};

class QSdlWindow : public QWindow
{
public:
    QSdlWindow(QtWindow* owner, Renderer renderer)
        : m_owner(owner)
    {
        switch (renderer)
        {
            case Renderer::Dx11:
                setSurfaceType(QSurface::Direct3DSurface);
                break;
            case Renderer::Metal: 
                setSurfaceType(QSurface::MetalSurface);
                break;
            case Renderer::OpenGl:
                SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");
                setSurfaceType(QSurface::OpenGLSurface);
                break;
            default: 
                break;
        }
    }

    virtual ~QSdlWindow() = default;

    SDL_Window* Initialize(int w, int h, int x, int y)
    {
        void*  windowId = reinterpret_cast<void*>(winId());
        m_window = SDL_CreateWindowFrom(windowId);

        setBaseSize(QSize(w, h));
        setPosition(QPoint(x, y));
        //requestUpdate();

        return m_window;
    }

    void Update()
    {
        //requestUpdate();
    }

    virtual bool event(QEvent* event)
    {
        static_assert((QEvent::User < ONS_USEREVENT_START) && (ONS_USEREVENT_END < QEvent::MaxUser));

        //if (m_window == NULL)
        //    return QWindow::event(event);

        fprintf(stderr, "QtEvent %d\n", event->type());
        switch (event->type())
        {
            case QEvent::User:
            {
                ONScripterCustomQtEvent* qtEvent = static_cast<ONScripterCustomQtEvent*>(event);
                SDL_Event event;
                event.type = qtEvent->m_customEvent;
                event.user.code = qtEvent->m_value;

                m_events.push_back(event);
                return false;
            }
            case QEvent::Move:
            {
                QMoveEvent* qtEvent = static_cast<QMoveEvent*>(event);
                SDL_Event sdlEvent;
                sdlEvent.type = SDL_WINDOWEVENT;
                sdlEvent.window.event = SDL_WINDOWEVENT_MOVED;
                sdlEvent.window.data1 = qtEvent->pos().x();
                sdlEvent.window.data2 = qtEvent->pos().y();

                m_events.push_back(sdlEvent);
                return false;
            }
            case QEvent::Resize:
            {
                QResizeEvent* qtEvent = static_cast<QResizeEvent*>(event);

                SDL_Event sdlEvent;
                sdlEvent.type = SDL_WINDOWEVENT;

                sdlEvent.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
                sdlEvent.window.data1 = qtEvent->size().width();
                sdlEvent.window.data2 = qtEvent->size().height();

                m_events.push_back(sdlEvent);

                sdlEvent.window.event = SDL_WINDOWEVENT_RESIZED;
                sdlEvent.window.data1 = qtEvent->size().width();
                sdlEvent.window.data2 = qtEvent->size().height();

                m_events.push_back(sdlEvent);//SDL_PushEvent(&sdlEvent);
                return false;
            }
            default:
            {
                return QWindow::event(event);
            }
        }
        //if (event->type() == QEvent::UpdateRequest)
        //{
        //    Update();
        //    return false;
        //}
        //else 
    }

    virtual void exposeEvent(QExposeEvent*)
    {
        //requestUpdate();
    }

    //virtual void resizeEvent(QResizeEvent* aEvent)
    //{
    //    printf("ResizeEvent: {%d, %d}\n", aEvent->size().width(), aEvent->size().height());
    //    aEvent->accept();
    //}

    virtual void keyPressEvent(QKeyEvent* aEvent)
    {

    }

    virtual void focusInEvent(QFocusEvent*)
    {

    }

    virtual void focusOutEvent(QFocusEvent*)
    {

    }

    std::vector<SDL_Event> m_events;

private:
    QtWindow* m_owner = NULL;
    SDL_Window* m_window = NULL;
};



QtWindow::QtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
    : Window()
    , m_qtapplication(argc, &argv)
{
    if (s_window != NULL)
        fprintf(stderr, "Game has created two instances of Window, there should only be one.");
    s_window = this;
    m_onscripterLabel = onscripter;

#if WIN32
    const Renderer preferredRenderer = Renderer::Dx11;

    // Unsure if we can use metal yet, I believe there's a bug in SDL that doesn't permit
    // Metal renderer creation on a Qt window.
//#else APPLE
//  const Renderer preferredRenderer = Renderer::Metal;
#else
    const Renderer preferredRenderer = Renderer::OpenGl;
#endif
    const char* preferredRendererName = GetRendererName(preferredRenderer);

    const int numDrivers = SDL_GetNumRenderDrivers();
    int preferredRendererIndex = 0;

    for (int i = 0; i < numDrivers; ++i)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);
        if (strcmp(info.name, preferredRendererName) == 0)
        {
            preferredRendererIndex = i;
        }
    }

    m_mainWindow = new QMainWindow();
    m_sdlWindow = new QSdlWindow(this, preferredRenderer);
    m_sdlWidget = QWidget::createWindowContainer(m_sdlWindow);
    m_sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_window = m_sdlWindow->Initialize(w, h, x, y);
    m_mainWindow->setCentralWidget(m_sdlWidget);
    m_mainWindow->move(x, y);
    
    m_sdlWindow->resize(w, h);
    m_renderer = SDL_CreateRenderer(m_window, preferredRendererIndex, 0);

    m_mainWindow->show();

    //std::string test = "show this text";
    //Command_InputStr(test, 10, false, NULL, NULL, NULL, NULL);
}

std::vector<SDL_Event>& QtWindow::PollEvents()
{
    m_eventLoop.processEvents(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);

    for (SDL_Event& event : m_sdlWindow->m_events)
    {
        if (event.type == SDL_WINDOWEVENT)
            event.window.windowID = SDL_GetWindowID(m_window);
        SDL_PushEvent(&event);
    }

    m_events.clear();
    SDL_Event polledEvent;
    while (SDL_PollEvent(&polledEvent))
    {
        // ignore continous SDL_MOUSEMOTION
        while (IgnoreContinuousMouseMove && polledEvent.type == SDL_MOUSEMOTION) {
            if (SDL_PeepEvents(&m_temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
            if (m_temp_event.type != SDL_MOUSEMOTION) break;
            SDL_PeepEvents(&m_temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            polledEvent = m_temp_event;
        }

        fprintf(stderr, "SDLEvent: %d\n", polledEvent.type);
        m_events.emplace_back(polledEvent);
    }
    //m_events.insert(m_events.end(), m_sdlWindow->m_events.begin(), m_sdlWindow->m_events.end());
    m_sdlWindow->m_events.clear();

    fprintf(stderr, "Loop\n");

    return m_events;
}


void QtWindow::WarpMouse(int x, int y)
{

}

void QtWindow::SetWindowCaption(const char* title, const char* icon_name)
{
    m_mainWindow->setWindowTitle(title);
    
    SDL_Surface* surface = IMG_Load(icon_name);
    if (surface)
    {
        SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
        SDL_Surface* preparredSurface = SDL_ConvertSurface(surface, format, 0);
        QImage iconQImage(static_cast<uchar*>(surface->pixels), surface->w, surface->h, QImage::Format_RGB32);
        QPixmap iconQPixmap = QPixmap::fromImage(iconQImage);
        QIcon icon(iconQPixmap);
        m_mainWindow->setWindowIcon(icon);

        SDL_FreeSurface(preparredSurface);
        SDL_FreeSurface(surface);
    }
}

SDL_Surface* QtWindow::SetVideoMode(int width, int height, int bpp, bool fullscreen)
{
    SDL_SetWindowSize(m_window, width, height);
    SDL_SetWindowFullscreen(m_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    m_mainWindow->resize(width, height);

    return m_onscripterLabel->screen_surface;
}

void* QtWindow::GetWindowHandle()
{
    return (void*)m_mainWindow->winId();
}


void QtWindow::Repaint()
{
    //m_sdlWindow->requestUpdate();
    //m_mainWindow->repaint();
}


void QtWindow::SendCustomEvent(ONScripterCustomEvent event, int value)
{
    QCoreApplication::postEvent(static_cast<QtWindow*>(s_window)->m_sdlWindow, new ONScripterCustomQtEvent(event, value));
}





void QtWindow::MenuImpl_VolumeSlider()
{

}

void QtWindow::MenuImpl_Version()
{

}

void QtWindow::MenuImpl_Exit()
{

}


class InputStrDialog : public QDialog
{
    Q_OBJECT
public:
    static std::string getInputStr(const std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h)
    {
        InputStrDialog* dialog = new InputStrDialog(display, maximumInputLength, forceDoubleByte, w, h, input_w, input_h);
        dialog->show();

        return std::string();
    }

private:
    explicit InputStrDialog(const std::string& label, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr)
    {
        QString display = "Input String";
        this->setWindowTitle(display);

        QString labelText = QString::fromStdString(label);

        QHBoxLayout* inputLayout = new QHBoxLayout;

        QPushButton* okButton = new QPushButton("Ok", this);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setMaxLength(maximumInputLength);

        inputLayout->addWidget(lineEdit);
        inputLayout->addWidget(okButton);


        QVBoxLayout* mainLayout = new QVBoxLayout;

        mainLayout->addWidget(new QLabel(labelText));
        mainLayout->addLayout(inputLayout);
        setLayout(mainLayout);
    
        // NOTE: These probably need to be relative to the parent window.
        int width = w == NULL ? 0 : *w;
        int height = h == NULL ? 0 : *h;

        move(width, height);
        //setBaseSize()
    }
};


std::string QtWindow::Command_InputStr(std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h)
{
    //QString displayText = QString::fromStdString(display);
    //QString output = QInputDialog::getText(m_sdlWidget, "ONScripter TextModal", displayText);

    return InputStrDialog::getInputStr(display, maximumInputLength, forceDoubleByte, NULL, NULL, NULL, NULL);
}


static QMenu* CreateMenu(std::vector<std::tuple<MenuBarFunction, std::string, int>> m_menuBarEntries, size_t& index)
{

    while (index < m_menuBarEntries.size())
    {
        std::tuple<MenuBarFunction, std::string, int>& menuBarEntry = m_menuBarEntries[index++];
        if (std::get<0>(menuBarEntry) == MenuBarFunction::Submenu)
        {
            QString display = QString::fromStdString(std::get<1>(menuBarEntry));
            QMenu* menu = new QMenu(display);
            menu->addMenu(CreateMenu(m_menuBarEntries, index));
        }


    }
}


static QMenuBar* CreateMenuBar(std::vector<std::tuple<MenuBarFunction, std::string, int>> m_menuBarEntries)
{
    size_t index = 0;

    while (index < m_menuBarEntries.size())
    {
        std::tuple<MenuBarFunction, std::string, int>& menuBarEntry = m_menuBarEntries[index++];
        if (std::get<0>(menuBarEntry) == MenuBarFunction::Submenu)
        {
            QString display = QString::fromStdString(std::get<1>(menuBarEntry));
            QMenu* menu = new QMenu(display);
            CreateMenu(m_menuBarEntries, index);
        }


    }
}

void QtWindow::CreateToolbar()
{
    QMenuBar* menuBar = new QMenuBar();
    int lastNest = 0;

    for (auto& menuBarEntry : m_menuBarEntries)
    {
        if (std::get<0>(menuBarEntry) == MenuBarFunction::Submenu)
        {
            QString display = QString::fromStdString(std::get<1>(menuBarEntry));
            QMenu* menu = new QMenu(display);
        }
    }

    m_mainWindow->setMenuBar(menuBar);
    //m_mainWindow->menuBar()->addMenu()
    //if (m_toolbar)
    //{
    //    m_mainWindow->removeToolBar(m_toolbar);
    //    m_toolbar = NULL;
    //}
    //
    //m_toolbar = new QToolBar(m_sdlWidget);
    //
    //for (auto& toolbarEntry : m_toolbarEntries)
    //{
    //    //m_toolbar->add
    //    //m_toolbar->
    //}
    //
    //m_mainWindow->addToolBar(m_toolbar);
}


#include "QtWindow.moc"
