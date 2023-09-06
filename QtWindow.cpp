#include "QInputDialog"

#include "QLabel"
#include "QMenuBar"
#include "QPushButton"
#include "QSlider"
#include "QStyle"

#include "QBoxLayout"

#include "ONScripterLabel.h"
#include "QtWindow.h"


Window* CreateQtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new QtWindow(onscripter, w, h, x, y);
}


class VolumeDialog : public QDialog
{
    Q_OBJECT
public:
    static void adjustVolumeSliders(ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent)
    {
        VolumeDialog* dialog = new VolumeDialog(onscripter, voice_volume, se_volume, music_volume, parent);
        dialog->setModal(true);
        dialog->show();
        dialog->setWindowFlag(Qt::WindowType::Dialog, true);
        dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
        dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
        dialog->setWindowFlag(Qt::WindowType::WindowSystemMenuHint, false);
        dialog->exec();

        delete dialog;
    }



private:
    explicit VolumeDialog(ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(QString::fromUtf8("Volume Dialog"));
        setWindowIcon(this->style()->standardIcon(QStyle::SP_MediaVolume));

        QLabel* slider1Label = new QLabel(QString::fromUtf8("Voice")); // voice_volume
        QSlider* slider1 = new QSlider(Qt::Vertical, parent);
        slider1->setMinimum(0);
        slider1->setMaximum(100);
        slider1->setValue(voice_volume);
        QVBoxLayout* slider1Layout = new QVBoxLayout;
        slider1Layout->addWidget(slider1Label);
        slider1Layout->addWidget(slider1);
        QObject::connect(slider1, &QSlider::sliderMoved, [=](int position) {
            onscripter->SetVoiceVolume(position);
        });

        QLabel* slider2Label = new QLabel(QString::fromUtf8("SFX")); // se_volume
        QSlider* slider2 = new QSlider(Qt::Vertical, parent);
        slider2->setMinimum(0);
        slider2->setMaximum(100);
        slider2->setValue(se_volume);
        QVBoxLayout* slider2Layout = new QVBoxLayout;
        slider2Layout->addWidget(slider2Label);
        slider2Layout->addWidget(slider2);
        QObject::connect(slider2, &QSlider::sliderMoved, [=](int position) {
            onscripter->SetSfxVolume(position);
        });

        QLabel* slider3Label = new QLabel(QString::fromUtf8("BGM")); //music_volume
        QSlider* slider3 = new QSlider(Qt::Vertical, parent);
        slider3->setMinimum(0);
        slider3->setMaximum(100);
        slider3->setValue(music_volume);
        QVBoxLayout* slider3Layout = new QVBoxLayout;
        slider3Layout->addWidget(slider3Label);
        slider3Layout->addWidget(slider3);
        QObject::connect(slider3, &QSlider::sliderMoved, [=](int position) {
            onscripter->SetMusicVolume(position);
        });

        QHBoxLayout* mainLayout = new QHBoxLayout;

        mainLayout->addLayout(slider1Layout);
        mainLayout->addLayout(slider2Layout);
        mainLayout->addLayout(slider3Layout);

        setLayout(mainLayout);

        resize(200, 200);
    }

    QLineEdit* m_lineEdit;
    QString m_textValue;
    ONScripterLabel* onscripter;
};






class InputStrDialog : public QDialog
{
    Q_OBJECT
public:
    static std::string getInputStr(const std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr)
    {
        std::string toReturn;
        int err = 0;
        while (err == 0)
        {
            InputStrDialog* dialog = new InputStrDialog(display, maximumInputLength, forceDoubleByte, w, h, input_w, input_h, parent);
            dialog->setModal(true);
            dialog->show();
            dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
            dialog->setWindowFlag(Qt::WindowType::Dialog, true);
            dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
            dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
            err = dialog->exec();

            toReturn = dialog->m_lineEdit->text().toStdString();

            delete dialog;
        }

        return toReturn;
    }



private:
    explicit InputStrDialog(const std::string& label, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr)
        : QDialog(parent)
    {
        QString display = "Input String Dialog";
        this->setWindowTitle(display);

        QString labelText = QString::fromStdString(label);

        QHBoxLayout* inputLayout = new QHBoxLayout;

        QPushButton* okButton = new QPushButton("Ok", this);
        m_lineEdit = new QLineEdit(this);
        m_lineEdit->setMaxLength(maximumInputLength);

        QObject::connect(okButton, &QPushButton::clicked, [=]() {
            this->accept();
            this->done(1);
            });

        inputLayout->addWidget(m_lineEdit);
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

    QLineEdit* m_lineEdit;

    QString m_textValue;
};

class VersionDialog : public QDialog
{
    Q_OBJECT
public:
    static void showVersion(const std::string& display, QWidget* parent = nullptr)
    {
        VersionDialog* dialog = new VersionDialog(display);
        dialog->setModal(true);
        dialog->show();
        //dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
        dialog->setWindowFlag(Qt::WindowType::Dialog, true);
        dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
        dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
        dialog->exec();

        delete dialog;
    }

private:
    explicit VersionDialog(const std::string& label, QWidget* parent = nullptr)
        : QDialog(parent)
    {
        QIcon messageInfoIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);

        QString display = "Version Dialog";
        setWindowTitle(display);
        setWindowIcon(messageInfoIcon);

        QHBoxLayout* inputLayout = new QHBoxLayout;

        QString labelText = QString::fromStdString(label);
        inputLayout->addWidget(new QLabel(labelText));

        QPushButton* okButton = new QPushButton("Ok", this);

        QObject::connect(okButton, &QPushButton::clicked, [=]() {
            this->accept();
            this->done(1);
        });

        QVBoxLayout* mainLayout = new QVBoxLayout;

        mainLayout->addLayout(inputLayout);
        mainLayout->addWidget(okButton);
        setLayout(mainLayout);
    }

    QLineEdit* m_lineEdit;

    QString m_textValue;
};



class ExitDialog : public QDialog
{
    Q_OBJECT
public:
    static bool shouldExit(const std::string& display, QWidget* parent = nullptr)
    {
        ExitDialog* dialog = new ExitDialog(display);
        dialog->setModal(true);
        dialog->show();
        //dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
        dialog->setWindowFlag(Qt::WindowType::Dialog, true);
        dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
        dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
        bool ret = !!dialog->exec();

        delete dialog;

        return ret;
    }

private:
    explicit ExitDialog(const std::string& label, QWidget* parent = nullptr)
        : QDialog(parent)
    {
        QIcon messageInfoIcon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);

        QString display = "Exit Dialog";
        setWindowTitle(display);
        setWindowIcon(messageInfoIcon);

        QPushButton* yesButton = new QPushButton("Yes", this);

        QObject::connect(yesButton, &QPushButton::clicked, [=]() {
            this->accept();
            this->done(1);
        });
        QPushButton* noButton = new QPushButton("No", this);

        QObject::connect(noButton, &QPushButton::clicked, [=]() {
            this->reject();
            this->done(0);
        });

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(yesButton);
        buttonLayout->addWidget(noButton);

        QVBoxLayout* mainLayout = new QVBoxLayout;

        QString labelText = QString::fromStdString(label);
        mainLayout->addWidget(new QLabel(labelText));
        mainLayout->addLayout(buttonLayout);

        setLayout(mainLayout);
    }

    QLineEdit* m_lineEdit;

    QString m_textValue;
};




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

        //fprintf(stderr, "QtEvent %d\n", event->type());
        switch (event->type())
        {
            case QEvent::User:
            {
                ONScripterCustomQtEvent* qtEvent = static_cast<ONScripterCustomQtEvent*>(event);
                SDL_Event event;
                event.type = qtEvent->m_customEvent;
                event.user.code = qtEvent->m_value;

                m_events.push_back(event);
                return true;
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
                return true;
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
                return true;
            }
            case QEvent::Close:
            {
                QCloseEvent* qtEvent = static_cast<QCloseEvent*>(event);
                SDL_Event sdlEvent;
                sdlEvent.type = SDL_QUIT;

                m_events.push_back(sdlEvent);
                return QWindow::event(event);
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

    virtual void closeEvent(QCloseEvent*)
    {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_QUIT;

        m_events.push_back(sdlEvent);
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

class SdlMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    SdlMainWindow(QtWindow* window)
        : m_window(window)
    {

    }

protected:
    void closeEvent(QCloseEvent* event)
    {
        Window::SendCustomEventStatic(static_cast<ONScripterCustomEvent>(SDL_QUIT), 0);
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        if (m_window->m_qtapplication.activePopupWidget() == NULL && m_window->m_mainWindow->isFullScreen())
        {
            if (event->type() == QEvent::MouseMove)
            {
                QMouseEvent* mouseMoveEvent = static_cast<QMouseEvent*>(event);
                if (menuBar()->isHidden())
                {
                    QRect rect = geometry();
                    rect.setHeight(25);

                    if (rect.contains(mouseMoveEvent->globalPos()))
                    {
                        menuBar()->show();
                    }
                }
                else
                {
                    QRect rect = QRect(menuBar()->mapToGlobal(QPoint(0, 0)), menuBar()->size());

                    if (!rect.contains(mouseMoveEvent->globalPos()))
                    {
                        menuBar()->hide();
                    }
                }
            }
            else if (event->type() == QEvent::Leave && (object == this))
            {
                menuBar()->hide();
            }
        }

        return QMainWindow::eventFilter(object, event);
    }

    QtWindow* m_window;
};



QtWindow::QtWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
    : Window(m_onscripterLabel)
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

    m_mainWindow = new SdlMainWindow(this);
    m_sdlWindow = new QSdlWindow(this, preferredRenderer);
    m_sdlWidget = QWidget::createWindowContainer(m_sdlWindow);
    m_sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_window = m_sdlWindow->Initialize(w, h, x, y);
    m_mainWindow->setCentralWidget(m_sdlWidget);
    m_mainWindow->move(x, y);
    
    m_sdlWindow->resize(w, h);
    m_renderer = SDL_CreateRenderer(m_window, preferredRendererIndex, 0);

    CreateMenuBar();

    m_mainWindow->show();

    m_qtapplication.installEventFilter(m_mainWindow);
}

int QtWindow::WaitEvents(SDL_Event& event)
{
    auto processQtEvents = [this]() {
        m_eventLoop.processEvents(QEventLoop::AllEvents);
        for (SDL_Event& event : m_sdlWindow->m_events)
        {
            if (event.type == SDL_WINDOWEVENT)
                event.window.windowID = SDL_GetWindowID(m_window);
            SDL_PushEvent(&event);
        }

        m_sdlWindow->m_events.clear();
    };

    while (true)
    {
        processQtEvents();
        bool ret = SDL_PollEvent(&event) == 1;

        SDL_Event temp_event;

        // ignore continous SDL_MOUSEMOTION
        while (IgnoreContinuousMouseMove && event.type == SDL_MOUSEMOTION) {
            processQtEvents();
            if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
            if (temp_event.type != SDL_MOUSEMOTION) break;
            SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            event = temp_event;
        }

        if (ret)
        {
            fprintf(stderr, "SDLEvent: %d\n", event.type);
            return 1;
        }
        SDL_Delay(1);
    }
}

int QtWindow::PollEvents(SDL_Event& event)
{
    m_eventLoop.processEvents(QEventLoop::AllEvents);
    for (SDL_Event& event : m_sdlWindow->m_events)
    {
        if (event.type == SDL_WINDOWEVENT)
            event.window.windowID = SDL_GetWindowID(m_window);
        SDL_PushEvent(&event);
    }

    m_sdlWindow->m_events.clear();

    return SDL_PollEvent(&event);
}

void QtWindow::WarpMouse(int x, int y)
{
    int windowResolutionX, windowResolutionY;
    SDL_GetWindowSizeInPixels(m_window, &windowResolutionX, &windowResolutionY);

    ScaleMouseToPixels(windowResolutionX, windowResolutionY, m_onscripterLabel->screen_surface->w, m_onscripterLabel->screen_surface->h, x, y);
    TranslateMouse(x, y);

    SDL_WarpMouseInWindow(m_window, x, y);
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
    // In both paths here we're doing some additional event processing in between focus requests.
    // This is a best effort attempt to maintain window _and_ widget focus after a fullscreen transition.
    auto focusSwap = [this]()
        {
            m_eventLoop.processEvents(QEventLoop::AllEvents);
            m_mainWindow->setFocus();
            m_eventLoop.processEvents(QEventLoop::AllEvents);
            m_sdlWidget->setFocus();
            m_eventLoop.processEvents(QEventLoop::AllEvents);
        };

    if (fullscreen)
    {
        m_originalPosition = m_mainWindow->pos();
        m_originalGeometry = m_sdlWidget->geometry();
        m_wasMaximized = m_mainWindow->isMaximized();
        m_mainWindow->menuBar()->hide();
        m_mainWindow->showFullScreen();

        focusSwap();
    }
    else if (m_wasMaximized)
    {
        m_mainWindow->menuBar()->show();
        m_mainWindow->showMaximized();

        focusSwap();

        m_wasMaximized = false;
    }
    else
    {
        m_mainWindow->menuBar()->show();
        m_mainWindow->resize(width, height);
        m_mainWindow->showNormal();

        focusSwap();

        m_mainWindow->move(m_originalPosition);

        // Adjust the total height of the window so that the widget we're rendering into is the requested
        // size, otherwise the menubar will take some height from it.
        int menubarHeight = m_mainWindow->menuBar()->geometry().height();
        m_mainWindow->resize(width, height + menubarHeight);

        m_wasMaximized = false;
    }

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


std::string QtWindow::Command_InputStr(std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h)
{
    return InputStrDialog::getInputStr(display, maximumInputLength, forceDoubleByte, NULL, NULL, NULL, NULL, m_sdlWidget);
}


ActionOrMenu QtWindow::CreateMenuBarInternal(MenuBarInput& input)
{
    ActionOrMenu toReturn;

    switch (input.m_function)
    {
        case MenuBarFunction::SUB:
        {
            QString display = QString::fromStdString(input.m_label);
            QMenu* menu = new QMenu(display);

            for (auto& menuBarEntry : input.m_children)
            {
                ActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry);

                if (actionOrMenu.isAction)
                    menu->addAction(actionOrMenu.m_actionOrMenu.m_action);
                else
                    menu->addMenu(actionOrMenu.m_actionOrMenu.m_menu);
            }

            toReturn.isAction = false;
            toReturn.m_actionOrMenu.m_menu = menu;
            break;
        }
        default:
        {
            QAction* action = new QAction(QString::fromStdString(input.m_label));
            m_actionsMap[input.m_function].emplace_back(action);

            if (IsCheckable(input.m_function))
            {
                action->setCheckable(true);
                action->setChecked(IsChecked(input.m_function));
            }

            QObject::connect(action, &QAction::triggered, [this, function = input.m_function]() {
                switch (function)
                {
                    case MenuBarFunction::AUTO:
                    {
                        m_onscripterLabel->automode_flag = true;
                        m_onscripterLabel->skip_mode &= ~ONScripterLabel::SKIP_NORMAL;
                        break;
                    }
                    case MenuBarFunction::CLICKDEF:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::CLICKPAGE])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);
                        m_onscripterLabel->skip_mode &= ~ONScripterLabel::SKIP_TO_EOP;
                        //printf("menu_click_def: disabling page-at-once mode\n");
                        break;
                    }
                    case MenuBarFunction::CLICKPAGE:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::CLICKDEF])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);
                        m_onscripterLabel->skip_mode |= ONScripterLabel::SKIP_TO_EOP;
                        //printf("menu_click_page: enabling page-at-once mode\n");
                        break;
                    }
                    case MenuBarFunction::DWAVEVOLUME:
                    {
                        VolumeDialog::adjustVolumeSliders(m_onscripterLabel, m_onscripterLabel->voice_volume, m_onscripterLabel->se_volume, m_onscripterLabel->music_volume, m_sdlWidget);
                        break;
                    }
                    case MenuBarFunction::END:
                    {
                        std::string output = "Are you sure you want to quit?";
                        if (ExitDialog::shouldExit(output, m_sdlWidget))
                        {
                            SendCustomEvent(static_cast<ONScripterCustomEvent>(SDL_QUIT), 0);
                        }
                        break;
                    }
                    case MenuBarFunction::FONT:
                    {
                        break;
                    }
                    case MenuBarFunction::kidokuoff:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::kidokuon])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->skip_mode |= ONScripterLabel::SKIP_TO_WAIT;
                        break;
                    }
                    case MenuBarFunction::kidokuon:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::kidokuoff])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->skip_mode |= ONScripterLabel::SKIP_NORMAL;
                        break;
                    }
                    case MenuBarFunction::SKIP:
                    {
                        m_onscripterLabel->skip_mode |= ONScripterLabel::SKIP_NORMAL;
                        break;
                    }
                    case MenuBarFunction::TEXTFAST:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::TEXTMIDDLE])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[MenuBarFunction::TEXTSLOW])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->text_speed_no = 2;
                        m_onscripterLabel->sentence_font.wait_time = -1;
                        break;
                    }
                    case MenuBarFunction::TEXTMIDDLE:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::TEXTFAST])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[MenuBarFunction::TEXTSLOW])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->text_speed_no = 1;
                        m_onscripterLabel->sentence_font.wait_time = -1;
                        break;
                    }
                    case MenuBarFunction::TEXTSLOW:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::TEXTFAST])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[MenuBarFunction::TEXTMIDDLE])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->text_speed_no = 0;
                        m_onscripterLabel->sentence_font.wait_time = -1;
                        break;
                    }
                    case MenuBarFunction::VERSION:
                    {
                        VersionDialog::showVersion("NETANNAD\nCopyright 2004.Team Netannad", m_sdlWidget);
                        break;
                    }
                    case MenuBarFunction::WAVEOFF:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::WAVEON])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->volume_on_flag = false;
                        break;
                    }
                    case MenuBarFunction::WAVEON:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::WAVEOFF])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        m_onscripterLabel->volume_on_flag = true;
                        break;
                    }
                    case MenuBarFunction::FULL:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::WINDOW])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        SetVideoMode(m_sdlWidget->geometry().width(), m_sdlWidget->geometry().height(), 0, true);
                        break;
                    }
                    case MenuBarFunction::WINDOW:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::FULL])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);

                        
                        SetVideoMode(m_originalGeometry.width(), m_originalGeometry.height(), 0, false);

                        break;
                    }
                }
            });

            toReturn.isAction = true;
            toReturn.m_actionOrMenu.m_action = action;
            break;
        }
    }

    return toReturn;
}

void QtWindow::CreateMenuBar()
{
    std::vector<MenuBarInput> menuBarEntriesUtf8 = m_menuBarEntries;


    // We should do this at a higher level, but we don't have a cross platform decoder
    // so briefly we'll do this only in Qt. Actually never mind Qt6 killed proper text 
    // encoding support, but keeping this in here for later.
    //QStringDecoder decoder{ "SHIFT-JIS" };
    //
    //if (m_onscripterLabel->script_h.enc.getEncoding() == Encoding::CODE_CP932)
    //{
    //    for (auto& entry : menuBarEntriesUtf8)
    //    {
    //        QString decodedString = decoder.decode(entry.m_label.c_str());
    //        entry.m_label = decodedString.toStdString();            
    //    }
    //}

    MenuBarInput menuBarTree = ParseMenuBarTree(menuBarEntriesUtf8);



    // FIXME: Memory leak? Or will deleting the existing menuBar clean them up?
    for (auto& actionsEntry : m_actionsMap)
        actionsEntry.second.clear();

    QMenuBar* menuBar = new QMenuBar();

    for (auto& menuBarEntry : menuBarTree.m_children)
    {
        ActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry);

        if (actionOrMenu.isAction)
            menuBar->addAction(actionOrMenu.m_actionOrMenu.m_action);
        else
            menuBar->addMenu(actionOrMenu.m_actionOrMenu.m_menu);
    }

    // Apple has a global menubar, were we to call `setMenuBar` here, it would disappear
    // when clicking on one of our dialogs, for now, do not set it, so that it's the same
    // across all Windows (the central window, and dialogs).
#ifndef APPLE
    m_mainWindow->setMenuBar(menuBar);
#endif
}


#include "QtWindow.moc"
