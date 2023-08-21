

#include "QtWindow.h"


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
        SDL_Window* window = SDL_CreateWindowFrom(windowId);

        setBaseSize(QSize(w, h));
        setPosition(QPoint(x, y));
        requestUpdate();

        return window;
    }

    void Update()
    {
        requestUpdate();
    }

    virtual bool event(QEvent* event)
    {
        if (event->type() == QEvent::UpdateRequest)
        {
            Update();
            return false;
        }
        else
        {
            return QWindow::event(event);
        }
    }

    virtual void exposeEvent(QExposeEvent*)
    {
        requestUpdate();
    }

    virtual void resizeEvent(QResizeEvent* aEvent)
    {
        printf("ResizeEvent: {%d, %d}\n", aEvent->size().width(), aEvent->size().height());
        aEvent->accept();
    }

    virtual void keyPressEvent(QKeyEvent* aEvent)
    {

    }

    virtual void focusInEvent(QFocusEvent*)
    {

    }

    virtual void focusOutEvent(QFocusEvent*)
    {

    }

private:
    QtWindow* m_owner;
};



QtWindow::QtWindow(int w, int h, int x, int y)
    : Window(w, h, x, y)
{
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

    m_qtwindow = new QSdlWindow(this, );
    m_window = m_qtwindow->Initialize(w, h, x, y);
    m_renderer = SDL_CreateRenderer(m_window, preferredRendererIndex, 0);
}

std::vector<SDL_Event>& QtWindow::PollEvents()
{
    m_eventLoop.processEvents();

    m_events.clear();
    SDL_Event polledEvent;
    while (SDL_PollEvent(&polledEvent))
    {
        // ignore continous SDL_MOUSEMOTION
        SDL_Event temp_event;
        while (IgnoreContinuousMouseMove && polledEvent.type == SDL_MOUSEMOTION) {
            if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
            if (temp_event.type != SDL_MOUSEMOTION) break;
            SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            polledEvent = temp_event;
        }

        m_events.emplace_back(polledEvent);
    }
    return m_events;
}

Window* CreateQtWindow(int w, int h, int x, int y)
{
    return new QtWindow(w, h, x, y);
}
