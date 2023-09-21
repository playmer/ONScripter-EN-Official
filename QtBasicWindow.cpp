#include "QInputDialog"

#include "QLabel"
#include "QMenuBar"
#include "QPushButton"
#include "QSlider"
#include "QStyle"

#include "QBoxLayout"

#include "ONScripterLabel.h"
#include "QtBasicWindow.h"

#include "QtDialogs.h"


Window* CreateQtBasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new QtBasicWindow(onscripter, w, h, x, y);
}




QtBasicWindow::QtBasicWindow(ONScripterLabel* onscripter, int w, int h, int x, int y)
        : BasicWindow(onscripter, w, h, x, y)
        , m_qtapplication(argc, &argv)
{
    CreateMenuBar();
}

int QtBasicWindow::WaitEvents(SDL_Event& event)
{
    while (true)
    {
        m_eventLoop.processEvents(QEventLoop::AllEvents);
        bool ret = SDL_PollEvent(&event) == 1;

        SDL_Event temp_event;

        // ignore continuous SDL_MOUSEMOTION
        while (IgnoreContinuousMouseMove && event.type == SDL_MOUSEMOTION) {
            m_eventLoop.processEvents(QEventLoop::AllEvents);
            if (SDL_PeepEvents(&temp_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 0) break;
            if (temp_event.type != SDL_MOUSEMOTION) break;
            SDL_PeepEvents(&temp_event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            event = temp_event;
        }

        if (ret) {
            //fprintf(stderr, "SDLEvent: %d\n", event.type);
            return 1;
        }
        SDL_Delay(1);
    }
}

int QtBasicWindow::PollEvents(SDL_Event& event)
{
    m_eventLoop.processEvents(QEventLoop::AllEvents);

    return SDL_PollEvent(&event);
}

std::string QtBasicWindow::Dialog_InputStr(std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h)
{
    SDL_Rect window_rect;
    SDL_GetWindowSize(m_window, &window_rect.w, &window_rect.h);
    SDL_GetWindowPosition(m_window, &window_rect.x, &window_rect.y);
    return InputStrDialog::getInputStr(window_rect, display, maximumInputLength, forceDoubleByte, w, h, input_w, input_h, NULL);
}


ActionOrMenu QtBasicWindow::CreateMenuBarInternal(MenuBarInput& input)
{
    SDL_Rect window_rect;
    SDL_GetWindowSize(m_window, &window_rect.w, &window_rect.h);
    SDL_GetWindowPosition(m_window, &window_rect.x, &window_rect.y);

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

            QObject::connect(action, &QAction::triggered, [this, window_rect, function = input.m_function]() {
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
                        VolumeDialog::adjustVolumeSliders(window_rect, m_onscripterLabel, m_onscripterLabel->voice_volume, m_onscripterLabel->se_volume, m_onscripterLabel->music_volume, NULL);
                        break;
                    }
                    case MenuBarFunction::END:
                    {
                        std::string output = "Are you sure you want to quit?";
                        if (ExitDialog::shouldExit(window_rect, output, NULL))
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
                        VersionDialog::showVersion(window_rect, m_onscripterLabel->version_str, NULL);
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

                        //SetVideoMode(m_sdlWidget->geometry().width(), m_sdlWidget->geometry().height(), 0, true);
                        break;
                    }
                    case MenuBarFunction::WINDOW:
                    {
                        for (auto& action : m_actionsMap[MenuBarFunction::FULL])
                            action->setChecked(false);

                        for (auto& action : m_actionsMap[function])
                            action->setChecked(true);


                        //SetVideoMode(m_originalGeometry.width(), m_originalGeometry.height(), 0, false);

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

void QtBasicWindow::CreateMenuBar()
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

    if (m_menuBar == NULL)
        m_menuBar = new QMenuBar();

    for (auto& menuBarEntry : menuBarTree.m_children)
    {
        ActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry);

        if (actionOrMenu.isAction)
            m_menuBar->addAction(actionOrMenu.m_actionOrMenu.m_action);
        else
            m_menuBar->addMenu(actionOrMenu.m_actionOrMenu.m_menu);
    }
}

#include "QtBasicWindow.moc"
