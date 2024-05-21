#include "ONScripterLabel.h"
#include "Win32Window.h"


Window* CreateWin32Window(ONScripterLabel* onscripter, int w, int h, int x, int y)
{
    return new Win32Window(onscripter, w, h, x, y);
}


void SDLCALL Win32Window::MessageHook(void* userdata, void* hWnd, unsigned int message, Uint64 wParam, Sint64 lParam)
{
    Win32Window* self = static_cast<Win32Window*>(userdata);

    if (message != WM_COMMAND)
    {
        return;
    }

    self->m_actions[wParam]();
}

Win32Window::Win32Window(ONScripterLabel* onscripter, int w, int h, int x, int y)
    : BasicWindow(onscripter, w, h, x, y)
{
    SDL_SetWindowsMessageHook(MessageHook, this);
}

static void setCheckedItem(HMENU menu, UINT_PTR identifier, bool checked)
{
    MENUITEMINFOA menuInfo;
    memset(&menuInfo, 0, sizeof(MENUITEMINFOA));

    menuInfo.cbSize = sizeof(MENUITEMINFO);
    menuInfo.fMask = MIIM_STATE;
    menuInfo.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;

    if (0 == SetMenuItemInfoA(menu, static_cast<UINT>(identifier), FALSE, &menuInfo))
    {
        DWORD err = GetLastError();

        printf("Couldn't make checked: %d\n", err);
    }
}

Win32Window::ActionOrMenu Win32Window::CreateMenuBarInternal(MenuBarInput& input, UINT_PTR& index)
{
    ActionOrMenu toReturn;
    toReturn.display = input.m_label;
    toReturn.index = ++index;
    toReturn.m_function = input.m_function;
    toReturn.isAction = true;

    switch (input.m_function)
    {
        case MenuBarFunction::SUB:
        {
            HMENU menu = CreateMenu();
            toReturn.index = reinterpret_cast<UINT_PTR>(menu);
            toReturn.isAction = false;

            for (auto& menuBarEntry : input.m_children)
            {
                ActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry, index);

                if (actionOrMenu.isAction) {
                    AppendMenuA(menu, MF_STRING, actionOrMenu.index, actionOrMenu.display.c_str());
                    m_functionsToMenuItems[actionOrMenu.m_function].emplace_back(menu, actionOrMenu.index);

                    if (IsCheckable(actionOrMenu.m_function)) {
                        setCheckedItem(menu, actionOrMenu.index, IsChecked(actionOrMenu.m_function));
                    }
                }
                else {
                    AppendMenuA(menu, MF_POPUP, actionOrMenu.index, actionOrMenu.display.c_str());
                }
            }

            break;
        }
        case MenuBarFunction::FONT:
        {
            HMENU menu = CreateMenu();
            toReturn.index = reinterpret_cast<UINT_PTR>(menu);
            toReturn.isAction = false;

            for (auto& font : m_onscripterLabel->GetFonts())
            {
                ++index;

                if (font.m_fontToUse)
                {
                    m_actions.emplace(index, [this, font = font.m_fontToUse]() {
                            m_onscripterLabel->ChangeFont(font);
                            for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::FONT]) {
                                setCheckedItem(menu, action, true);
                            }

                            //thisAction->setChecked(true);
                        });
                    AppendMenuA(menu, MF_STRING, index, font.m_fontToUse->m_name.c_str());
                    bool checked = m_onscripterLabel->font_file ? font.m_fontToUse->m_availiblePath == m_onscripterLabel->font_file : false;
                    setCheckedItem(menu, index, checked);
                    UINT disabled = !font.m_fontToUse->m_availiblePath.empty() ? MF_ENABLED : MF_GRAYED;
                    EnableMenuItem(menu, index, MF_BYCOMMAND | disabled);
                }
                else {
                    AppendMenuA(menu, MF_STRING, index, font.m_name.c_str());
                    setCheckedItem(menu, index, false);
                    EnableMenuItem(menu, index, MF_BYCOMMAND | MF_GRAYED);
                }

                m_functionsToMenuItems[input.m_function].emplace_back(menu, index);
            }

            break;
        }
        case MenuBarFunction::AUTO:
        {
            m_actions.emplace(index, [this]() {
                if (m_onscripterLabel->mode_ext_flag && !m_onscripterLabel->automode_flag) {
                    m_onscripterLabel->automode_flag = true;
                    m_onscripterLabel->skip_mode &= ~ONScripterLabel::SKIP_NORMAL;
                    printf("change to automode\n");
                    m_onscripterLabel->key_pressed_flag = true;
                    m_onscripterLabel->current_button_state.set(0);
                    m_onscripterLabel->volatile_button_state.set(0);
                    m_onscripterLabel->stopCursorAnimation(ONScripterLabel::CLICK_WAIT);
                    m_onscripterLabel->return_from_event = true;

                    return true;
                }
                });

            break;
        }
        case MenuBarFunction::CLICKDEF:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::CLICKPAGE])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);
                m_onscripterLabel->skip_mode &= ~ONScripterLabel::SKIP_TO_EOP;
                //printf("menu_click_def: disabling page-at-once mode\n");
                });
            break;
        }
        case MenuBarFunction::CLICKPAGE:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::CLICKDEF])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);
                m_onscripterLabel->skip_mode |= ONScripterLabel::SKIP_TO_EOP;
                //printf("menu_click_page: enabling page-at-once mode\n");
                });
            break;
        }
        case MenuBarFunction::DWAVEVOLUME:
        {
            m_actions.emplace(index, [this]() {
                });
            //VolumeDialog::adjustVolumeSliders(RectFromWidget(m_mainWindow), m_onscripterLabel, m_onscripterLabel->voice_volume, m_onscripterLabel->se_volume, m_onscripterLabel->music_volume, m_sdlWidget);
            break;
        }
        case MenuBarFunction::END:
        {
            m_actions.emplace(index, [this]() {
                });
            //std::string output = "Are you sure you want to quit?";
            //if (ExitDialog::shouldExit(RectFromWidget(m_mainWindow), output, m_sdlWidget))
            //{
            //    SendCustomEvent(static_cast<ONScripterCustomEvent>(SDL_QUIT), 0);
            //}
            break;
        }
        case MenuBarFunction::kidokuoff:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::kidokuon])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->kidokumode_flag = false;
                });
            break;
        }
        case MenuBarFunction::kidokuon:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::kidokuoff])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->kidokumode_flag = true;
                });
            break;
        }
        case MenuBarFunction::SKIP:
        {
            m_actions.emplace(index, [this]() {
                m_onscripterLabel->skip_mode |= ONScripterLabel::SKIP_NORMAL;
                m_onscripterLabel->return_from_event = true;
                });
            break;
        }
        case MenuBarFunction::TEXTFAST:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::TEXTMIDDLE])
                    setCheckedItem(menu, action, false);

                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::TEXTSLOW])
                    setCheckedItem(menu, action, false);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->text_speed_no = 2;
                m_onscripterLabel->sentence_font.wait_time = -1;
                });
            break;
        }
        case MenuBarFunction::TEXTMIDDLE:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::TEXTFAST])
                    setCheckedItem(menu, action, false);

                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::TEXTSLOW])
                    setCheckedItem(menu, action, false);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->text_speed_no = 1;
                m_onscripterLabel->sentence_font.wait_time = -1;
                });
            break;
        }
        case MenuBarFunction::TEXTSLOW:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::TEXTFAST])
                    setCheckedItem(menu, action, false);

                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::TEXTMIDDLE])
                    setCheckedItem(menu, action, false);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->text_speed_no = 0;
                m_onscripterLabel->sentence_font.wait_time = -1;
                });
            break;
        }
        case MenuBarFunction::VERSION:
        {
            m_actions.emplace(index, [this]() {
                });
            //VersionDialog::showVersion(RectFromWidget(m_mainWindow), m_onscripterLabel->version_str, m_sdlWidget);
            break;
        }
        case MenuBarFunction::WAVEOFF:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::WAVEON])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->volume_on_flag = false;
                });
            break;
        }
        case MenuBarFunction::WAVEON:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::WAVEOFF])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                m_onscripterLabel->volume_on_flag = true;
                });
            break;
        }
        case MenuBarFunction::FULL:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::WINDOW])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                //SetVideoMode(m_sdlWidget->geometry().width(), m_sdlWidget->geometry().height(), 0, true);
                });
            break;
        }
        case MenuBarFunction::WINDOW:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                for (auto& [menu, action] : m_functionsToMenuItems[MenuBarFunction::FULL])
                    setCheckedItem(menu, action, true);

                for (auto& [menu, action] : m_functionsToMenuItems[function])
                    setCheckedItem(menu, action, true);

                //SetVideoMode(m_originalGeometry.width(), m_originalGeometry.height(), 0, false);
            });

            break;
        }
        default:
        {
            m_actions.emplace(index, [this, function = input.m_function]() {
                });
            //QAction* action = new QAction(QString::fromStdString(input.m_label));
            //m_functionsToMenuItems[input.m_function].emplace_back(action);
            //
            //if (IsCheckable(input.m_function))
            //{
            //    action->setCheckable(true);
            //    action->setChecked(IsChecked(input.m_function));
            //}
            //
            //QObject::connect(action, &QAction::triggered, [this, function = input.m_function]() {
            //    switch (function)
            //    {
            //       
            //    }
            //});
            //
            //toReturn.isAction = true;
            //toReturn.m_actionOrMenu.m_action = action;
            //break;
        }
    }

    return toReturn;
}

void Win32Window::CreateMenuBar()
{
   std::vector<MenuBarInput> menuBarEntriesUtf8 = m_menuBarEntries;
   MenuBarInput menuBarTree = ParseMenuBarTree(menuBarEntriesUtf8);

   if (m_menubar != NULL) {
       m_actions.clear();
       m_functionsToMenuItems.clear();
       DestroyMenu(m_menubar);
       m_menubar = NULL;
   }

   m_menubar = CreateMenu();

   UINT_PTR index = 0;

   for (auto& menuBarEntry : menuBarTree.m_children)
   {
       ActionOrMenu actionOrMenu = CreateMenuBarInternal(menuBarEntry, index);

       if (actionOrMenu.isAction) {
           AppendMenuA(m_menubar, MF_STRING, actionOrMenu.index, actionOrMenu.display.c_str());
           m_functionsToMenuItems[actionOrMenu.m_function].emplace_back(m_menubar, index);
       }
       else
           AppendMenuA(m_menubar, MF_POPUP, actionOrMenu.index, actionOrMenu.display.c_str());
   }

   SetMenu(static_cast<HWND>(GetWindowHandle()), m_menubar);
}
