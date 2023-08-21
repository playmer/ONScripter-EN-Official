#include "SDL2/SDL.h"

#include "CsvHandler.h"

CsvHandler::CsvHandler(const char* filename)
{
    SDL_RWFromFile(filename, "r");
}
