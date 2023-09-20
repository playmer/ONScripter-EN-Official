#include <unordered_map>

#include "Window.h"
#include "ONScripterLabel.h"


Window* Window::s_window = NULL;

static const std::unordered_map <std::string, MenuBarFunction> stringsToFunction = {
    { std::string("AUTO"), MenuBarFunction::AUTO },
    { std::string("CLICKDEF"), MenuBarFunction::CLICKDEF },
    { std::string("CLICKPAGE"), MenuBarFunction::CLICKPAGE },
    { std::string("DWAVEVOLUME"), MenuBarFunction::DWAVEVOLUME },
    { std::string("END"), MenuBarFunction::END },
    { std::string("FONT"), MenuBarFunction::FONT },
    { std::string("FULL"), MenuBarFunction::FULL },
    { std::string("KIDOKUOFF"), MenuBarFunction::kidokuoff },
    { std::string("KIDOKUON"), MenuBarFunction::kidokuon },
    { std::string("SKIP"), MenuBarFunction::SKIP },
    { std::string("SUB"), MenuBarFunction::SUB },
    { std::string("TEXTFAST"), MenuBarFunction::TEXTFAST },
    { std::string("TEXTMIDDLE"), MenuBarFunction::TEXTMIDDLE },
    { std::string("TEXTSLOW"), MenuBarFunction::TEXTSLOW },
    { std::string("VERSION"), MenuBarFunction::VERSION },
    { std::string("WAVEOFF"), MenuBarFunction::WAVEOFF },
    { std::string("WAVEON"), MenuBarFunction::WAVEON },
    { std::string("WINDOW"), MenuBarFunction::WINDOW },
    { std::string("UNKNOWN"), MenuBarFunction::UNKNOWN },
};


MenuBarFunction functionNameToMenuBarFunction(const char* functionName)
{
    std::string functionNameUpper = functionName;

    for (size_t i = 0; i < functionNameUpper.size(); ++i)
        functionNameUpper[i] = std::toupper(functionNameUpper[i]);

    auto it = stringsToFunction.find(functionNameUpper);

    if (it != stringsToFunction.end())
        return it->second;

    return MenuBarFunction::UNKNOWN;
}


bool IsCheckable(MenuBarFunction function)
{
    switch (function)
    {
    case MenuBarFunction::WAVEOFF:
    case MenuBarFunction::WAVEON:
    case MenuBarFunction::CLICKDEF:
    case MenuBarFunction::CLICKPAGE:
    case MenuBarFunction::TEXTFAST:
    case MenuBarFunction::TEXTMIDDLE:
    case MenuBarFunction::TEXTSLOW:
    case MenuBarFunction::kidokuoff:
    case MenuBarFunction::kidokuon:
    case MenuBarFunction::FULL:
    case MenuBarFunction::WINDOW:
        return true;
    default: 
        return false;
    }
}

bool Window::IsChecked(MenuBarFunction function)
{
    switch (function)
    {
        case MenuBarFunction::WAVEOFF:
        {
            return !m_onscripterLabel->volume_on_flag;
        }
        case MenuBarFunction::WAVEON:
        {
            return m_onscripterLabel->volume_on_flag;
        }
        case MenuBarFunction::CLICKDEF:
        {
            return !(m_onscripterLabel->skip_mode & ONScripterLabel::SKIP_TO_EOP);
        }
        case MenuBarFunction::CLICKPAGE:
        {
            return m_onscripterLabel->skip_mode & ONScripterLabel::SKIP_TO_EOP;
        }
        case MenuBarFunction::TEXTFAST:
        {
            return m_onscripterLabel->text_speed_no == 2;
        }
        case MenuBarFunction::TEXTMIDDLE:
        {
            return m_onscripterLabel->text_speed_no == 1;
        }
        case MenuBarFunction::TEXTSLOW:
        {
            return m_onscripterLabel->text_speed_no == 0;
        }
        case MenuBarFunction::kidokuoff:
        {
            return m_onscripterLabel->skip_mode & ONScripterLabel::SKIP_TO_WAIT;
        }
        case MenuBarFunction::kidokuon:
        {
            return m_onscripterLabel->skip_mode & ONScripterLabel::SKIP_NORMAL;
        }
        case MenuBarFunction::FULL:
        {
            return m_onscripterLabel->fullscreen_mode;
        }
        case MenuBarFunction::WINDOW:
        {
            return !m_onscripterLabel->fullscreen_mode;
        }
    }
}


Window::Window(ONScripterLabel* onscripterLabel)
    : m_onscripterLabel(onscripterLabel)
{
    m_menuBarEntries.emplace_back(MenuBarFunction::END, "Exit", 0);
    m_menuBarEntries.emplace_back(MenuBarFunction::VERSION, "Version Information", 0);
    m_menuBarEntries.emplace_back(MenuBarFunction::DWAVEVOLUME, "Volume", 0);
    m_menuBarEntries.emplace_back(MenuBarFunction::SKIP, "Skip to Choice", 0);
    m_menuBarEntries.emplace_back(MenuBarFunction::AUTO, "Auto mode", 0);
    m_menuBarEntries.emplace_back(MenuBarFunction::SUB, "Settings", 0);
    m_menuBarEntries.emplace_back(MenuBarFunction::SUB, "Sound effects", 1);
    m_menuBarEntries.emplace_back(MenuBarFunction::WAVEON, "On", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::WAVEOFF, "Off", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::SUB, "Click settings", 1);
    m_menuBarEntries.emplace_back(MenuBarFunction::CLICKDEF, "Per line", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::CLICKPAGE, "Per page", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::FONT, "Font", 1);
    m_menuBarEntries.emplace_back(MenuBarFunction::SUB, "Text speed", 1);
    m_menuBarEntries.emplace_back(MenuBarFunction::TEXTSLOW, "Slow", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::TEXTMIDDLE, "Default", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::TEXTFAST, "Fast", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::SUB, "Skip settings", 1);
    m_menuBarEntries.emplace_back(MenuBarFunction::kidokuoff, "Skip all messages", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::kidokuon, "Skip read text", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::SUB, "Screen settings", 1);
    m_menuBarEntries.emplace_back(MenuBarFunction::FULL, "Fullscreen", 2);
    m_menuBarEntries.emplace_back(MenuBarFunction::WINDOW, "Windowed", 2);
}


bool Window::TranslateMouse(int& x, int& y, bool toScreenSize)
{
    int windowResolutionX, windowResolutionY;
    SDL_GetWindowSizeInPixels(m_window, &windowResolutionX, &windowResolutionY);
    //SDL_GetRendererOutputSize(m_window->GetRenderer(), &windowResolutionX, &windowResolutionY);
    int windowPointsW, windowPointsH;

    if (toScreenSize)
    {
        windowPointsW = m_onscripterLabel->screen_surface->w;
        windowPointsH = m_onscripterLabel->screen_surface->h;
    }
    else
    {
        SDL_GetWindowSize(m_window, &windowPointsW, &windowPointsH);
    }

    if (windowResolutionX == windowPointsW && windowResolutionY == windowPointsH) {
        printf("same, skipping transform\n");
        return true;
    }

    float scaleHeight = windowResolutionY / (float)windowPointsH;
    float scaleWidth = windowResolutionX / (float)windowPointsW;
    float scale = std::min(scaleHeight, scaleWidth);

    SDL_Rect dstRect = {};
    dstRect.w = scale * windowPointsW;
    dstRect.h = scale * windowPointsH;
    dstRect.x = (windowResolutionX - dstRect.w) / 2;
    dstRect.y = (windowResolutionY - dstRect.h) / 2;

    int new_x = (x / scale) - (dstRect.x / scale);
    int new_y = (y / scale) - (dstRect.y / scale);

    if (new_x < 0 || new_x > windowPointsW || new_y < 0 || new_y > windowPointsH)
    {
        // clip the mouse to the window
        if (new_x < 0)
            x = 0;

        if (new_x > windowPointsW)
            x = windowPointsW;

        if (new_y < 0)
            y = 0;

        if (new_y > windowPointsH)
            y = windowPointsH;

        return false;
    }

    fprintf(stderr, "Warping Orig: {%d, %d}; New: {%d, %d}\n", x, y, new_x, new_y);

    x = new_x;
    y = new_y;
    return true;
}

Window::MenuBarInput* Window::GetCurrentParent(MenuBarInput& input, std::vector<size_t>& depthTracker)
{
    MenuBarInput* toReturn = &input;
    
    for (size_t i = 0; i < depthTracker.size(); ++i)
        toReturn = &toReturn->m_children[depthTracker[i]];

    return toReturn;
}


void Window::ReverseChildren(MenuBarInput& input)
{
    for (size_t i = 0; i < input.m_children.size(); ++i)
    {
        ReverseChildren(input.m_children[i]);
    }

    std::reverse(input.m_children.begin(), input.m_children.end());
}


Window::MenuBarInput Window::ParseMenuBarTree(std::vector<MenuBarInput>& menuBarEntries)
{
    MenuBarInput toReturn;
    std::vector<size_t> depthTracker; // the size is how many nodes deep we are, the values are how we get there.

    for (size_t i = 0; i < menuBarEntries.size(); ++i)
    {
        const MenuBarInput& currentEntry = menuBarEntries[i];
        if (depthTracker.size() > currentEntry.m_depth)//depthTracker.back()
        {
            depthTracker.pop_back();
        }

        MenuBarInput* parent = GetCurrentParent(toReturn, depthTracker);


        if (currentEntry.m_function == MenuBarFunction::SUB)
        {
            depthTracker.push_back(parent->m_children.size());
            parent->m_children.push_back(currentEntry);
        }
        else
        {
            parent->m_children.push_back(currentEntry);
        }
    }

    ReverseChildren(toReturn);

    return toReturn;
}

void Window::ScaleMouseToPixels(int w_1, int h_1, int w_2, int h_2, int& x_m, int& y_m)
{
    // Scale the mouse to Window (pixel) coordinates
    float scaleWidth = w_1 / (float)w_2;
    float scaleHeight = h_1 / (float)h_2;
    float scale = std::min(scaleHeight, scaleWidth);

    SDL_Rect dstRect = {};
    dstRect.w = scale * w_2;
    dstRect.h = scale * h_2;
    dstRect.x = (w_1 - dstRect.w) / 2;
    dstRect.y = (h_1 - dstRect.h) / 2;

    x_m = (x_m * scale) + dstRect.x;
    y_m = (y_m * scale) + dstRect.y;
}

void Window::InsertMenu(MenuBarFunction function, const char* label, int depth)
{
    if (m_onscripterLabel->script_h.enc.getEncoding() == Encoding::CODE_CP932) {
        std::string shiftJisLabel = label;
        std::string transcodedLabel;
        transcodedLabel.resize(shiftJisLabel.length() * 2 + 3);
        DirectReader::convertFromSJISToUTF8(&transcodedLabel[0], shiftJisLabel.c_str());

        m_menuBarEntries.emplace_back(function, transcodedLabel, depth);
    }
    else {
        m_menuBarEntries.emplace_back(function, label, depth);
    }

    CreateMenuBar();
}

