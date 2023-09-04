add_library(SDLHelper_CD)

target_sources(SDLHelper_CD 
PUBLIC
    SDL_cdrom.h
    cdrom/SDL_syscdrom.h
    cdrom/SDL_cdrom.c
)

target_include_directories(SDLHelper_CD PUBLIC .)

target_link_libraries(SDLHelper_CD
    PRIVATE
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

if (WIN32)
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_WIN32)

    target_sources(SDLHelper_CD 
    PUBLIC
        cdrom/win32/SDL_syscdrom.c
    )
elseif (APPLE)
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_MACOSX)

    target_sources(SDLHelper_CD 
    PUBLIC
        cdrom/macosx/AudioFilePlayer.c
        cdrom/macosx/AudioFilePlayer.h
        cdrom/macosx/AudioFileReaderThread.c
        cdrom/macosx/CDPlayer.c
        cdrom/macosx/CDPlayer.h
        cdrom/macosx/SDL_syscdrom.c
        cdrom/macosx/SDL_syscdrom_c.h
        cdrom/macosx/SDLOSXCAGuard.c
        cdrom/macosx/SDLOSXCAGuard.h
    )
elseif(Linux)
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_LINUX)

    target_sources(SDLHelper_CD 
    PUBLIC
        cdrom/linux/SDL_syscdrom.c
    )
else() # Android, iOS
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_DUMMY)

    target_sources(SDLHelper_CD 
    PUBLIC
        cdrom/dummy/SDL_syscdrom.c
    )
endif()