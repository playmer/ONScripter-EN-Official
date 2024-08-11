cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(onscripter_en LANGUAGES C CXX)

option(ONS_USE_EXT_LIBS "Choose whether to use our own libraries or to use system packages" OFF)

#find_package(SDL REQUIRED)
include(FindPkgConfig)

pkg_check_modules(SDL IMPORTED_TARGET sdl)
pkg_check_modules(SDL_image IMPORTED_TARGET sdl_image)
pkg_check_modules(SDL_mixer IMPORTED_TARGET sdl_mixer)
pkg_check_modules(SDL_ttf IMPORTED_TARGET sdl_ttf)
pkg_check_modules(smpeg IMPORTED_TARGET smpeg)
pkg_check_modules(freetype2 IMPORTED_TARGET freetype2) # freetype2?
pkg_check_modules(mad IMPORTED_TARGET mad)
pkg_check_modules(ogg IMPORTED_TARGET ogg)
pkg_check_modules(zlib IMPORTED_TARGET zlib)
pkg_check_modules(vorbis IMPORTED_TARGET vorbis)
pkg_check_modules(vorbisfile IMPORTED_TARGET vorbisfile)
pkg_check_modules(libpng IMPORTED_TARGET libpng)
pkg_check_modules(libjpeg IMPORTED_TARGET libjpeg)
pkg_check_modules(harfbuzz IMPORTED_TARGET harfbuzz)
pkg_check_modules(bzip2 IMPORTED_TARGET bzip2)

#-lbrotlidec
#-lbrotlicommon
#-ltiff
#-lwebp
#-lgraphite2
#-ljbig
#-lsharpyuv
#-lzstd
#-ldeflate
#-lLerc
#-llzma
#-liconv
#-lrpcrt4
#-Wl

# Windows stuff
#-ldinput
#-lwinmm
#-ldxguid

target_sources(onscripter_en
PRIVATE
    AnimationInfo.cpp
    AnimationInfo.h
    AVIWrapper.cpp
    AVIWrapper.h
    BaseReader.h
    BasicWindow.cpp
    BasicWindow.h
    CMakeLists.txt
    DirectReader.cpp
    DirectReader.h
    DirPaths.cpp
    DirPaths.h
    DirtyRect.cpp
    DirtyRect.h
    Encoding.cpp
    Encoding.h
    FontInfo.cpp
    FontInfo.h
    graphics_altivec.cpp
    graphics_altivec.h
    graphics_blend.h
    graphics_common.h
    graphics_cpu.h
    graphics_mmx.cpp
    graphics_mmx.h
    graphics_resize.h
    graphics_routines.cpp
    graphics_sse2.cpp
    graphics_sse2.h
    graphics_sum.h
    Layer.cpp
    Layer.h
    #nscriptdecode.cpp
    NsaReader.cpp
    NsaReader.h
    onscripter.cpp
    ONScripterLabel_animation.cpp
    ONScripterLabel_command.cpp
    ONScripterLabel_effect_breakup.cpp
    ONScripterLabel_effect_cascade.cpp
    ONScripterLabel_effect_trig.cpp
    ONScripterLabel_effect.cpp
    ONScripterLabel_event.cpp
    ONScripterLabel_file.cpp
    ONScripterLabel_file2.cpp
    ONScripterLabel_image.cpp
    ONScripterLabel_rmenu.cpp
    ONScripterLabel_sound.cpp
    ONScripterLabel_text.cpp
    ONScripterLabel.cpp
    ONScripterLabel.h
    resize_image.cpp
    resize_image.h
    resources.cpp
    resources.h
    SarReader.cpp
    SarReader.h
    ScriptHandler.cpp
    ScriptHandler.h
    ScriptParser_command.cpp
    ScriptParser.cpp
    ScriptParser.h
    ScriptParser_command.cpp
    sjis2utf16.cpp
    version.h
    Window.cpp
    Window.h
    winres.h
)

set(ONS_COMPILER_FLAG_STYLE "GNU")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
endif()


if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")

elseif(ONS_COMPILER_FLAG_STYLE STREQUAL "MSVC")
    target_compile_definitions(onscripter_en
        PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )

    target_compile_options(onscripter_en 
    PRIVATE
        /W4
    )
endif()


if(WIN32 OR MINGW)
    target_sources(onscripter_en
    PRIVATE
        onscripter.rc
        Win32Window.cpp
        Win32Window.h
    )

    target_link_options(onscripter_en 
    PRIVATE
        $<IF:$<CONFIG:DEBUG>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS>
    )
endif()

if(APPLE)
    if (IOS)
    else() # Technically, WatchOS, TvOS, and iPadOS might fall into this to...
        target_compile_definitions(onscripter_en PUBLIC MACOSX)
        target_include_directories(onscripter_en PUBLIC macosx)
    endif()


    target_sources(onscripter_en
    PRIVATE
        macosx/cocoa_alertbox.h
        macosx/cocoa_alertbox.mm
        macosx/cocoa_alertbox_result.h
        macosx/cocoa_bundle.h
        macosx/cocoa_bundle.mm
        macosx/cocoa_encoding.h
        macosx/cocoa_encoding.mm
        macosx/cocoa_modal_alert.h
        macosx/cocoa_modal_alert.mm
        macosx/cocoa_url.h
        macosx/cocoa_url.mm
    )
#endif()
