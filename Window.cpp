#include "Window.h"

Window* Window::s_window = NULL;

bool Window::TranslateMouse(int& x, int& y)
{
    int windowResolutionX, windowResolutionY;
    SDL_GetWindowSizeInPixels(m_window, &windowResolutionX, &windowResolutionY);
    //SDL_GetRendererOutputSize(m_window->GetRenderer(), &windowResolutionX, &windowResolutionY);
    int windowPointsW, windowPointsH;
    SDL_GetWindowSize(m_window, &windowPointsW, &windowPointsH);

    if (windowResolutionX == windowPointsW && windowResolutionY == windowPointsH) {
        printf("same, skipping transform\n");
        return false;
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
