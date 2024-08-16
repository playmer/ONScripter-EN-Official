cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(onscripter_en LANGUAGES C CXX)

# Determine what architecture we're targeting.
include(cmake/TargetArch.cmake)
target_architecture(ONS_TARGET_ARCH)
message(STATUS "Target Architecture: ${ONS_TARGET_ARCH}")

# Options we can pass into CMake, platform will determine some defaults.
if (PLATFORM_PSP)
    option(ONS_HOST_PLATFORM "Platform for which could be feasibly used to develop an ONScripter title, if off, we won't compile tools" OFF)
else()
    option(ONS_HOST_PLATFORM "Platform for which could be feasibly used to develop an ONScripter title, if off, we won't compile tools" ON)
endif()

option(ONS_USE_EXT_LIBS "Choose whether to use our own libraries or to use system packages" ON)
option(ONS_COMPILE_TOOLS "Choose whether to use our own libraries or to use system packages" ${ONS_ONLY_PLATFORM})

# A CMake implementation of embed.cpp, which we can't rely on due to a limitation in CMake's cross compilation capabilities.
# (Essentially when cross compiling, we'd need to mix compilation target architectures. Embed.cpp would need to be compiled
# for the host platform, while ONScripter (and maybe the tools) would need to target some completely different arch.
# this is something for which CMake isn't designed. There's some hacks around this, but it's easier to just write a CMake 
# script to implement the functionality, even if it's quite a bit slower.)
set(EMBED_COMMAND ${CMAKE_COMMAND} -P cmake/ResourceGenerator.cmake -- ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp ons-en.ico )
set(ADDITIONAL_EMBED_DEPENDENCIES "")

add_custom_command(
    OUTPUT resources.cpp
    COMMAND ${EMBED_COMMAND}
    DEPENDS ${ADDITIONAL_EMBED_DEPENDENCIES} ${CMAKE_CURRENT_SOURCE_DIR}/ons-en.ico
    COMMENT "This command generates resources.cpp to embed files into onscripter_en, such as the icon."
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#############################################
# Dependencies
if(ONS_USE_EXT_LIBS)
    include(cmake/DependencyFinder.cmake)

    PkgConfig_Find_Module(sdl SDL)
    PkgConfig_Find_Module(SDL_image)
    PkgConfig_Find_Module(SDL_mixer)
    PkgConfig_Find_Module(SDL_ttf)
    PkgConfig_Find_Module(freetype2)
    PkgConfig_Find_Module(ogg)
    PkgConfig_Find_Module(zlib)
    PkgConfig_Find_Module(vorbis)
    PkgConfig_Find_Module(vorbisfile)
    PkgConfig_Find_Module(libpng)
    PkgConfig_Find_Module(libjpeg)
    PkgConfig_Find_Module(bzip2)
    PkgConfig_Find_Module(libtiff-4)
    PkgConfig_Find_Module(libwebp)

    set_property(TARGET PkgConfigWrapper::SDL_image APPEND PROPERTY INTERFACE_LINK_LIBRARIES PkgConfigWrapper::libtiff-4)
    set_property(TARGET PkgConfigWrapper::SDL_image APPEND PROPERTY INTERFACE_LINK_LIBRARIES PkgConfigWrapper::libwebp)
    
    target_link_libraries(PkgConfigWrapper::SDL_mixer INTERFACE mad)

    run_library_config(smpeg smpeg_include_directories)
endif()

#############################################
# Tools
add_subdirectory(tools)

#############################################
# Configure version.h

set(I \"ons-en.ico\")

string(TIMESTAMP Y %Y)
string(TIMESTAMP M %m)
string(TIMESTAMP D %d)
string(TIMESTAMP 0M %m)
string(TIMESTAMP 0D %d)

# trim off the leading 0 if the month/day has one.
string(REGEX REPLACE "^0" "" M ${M})
string(REGEX REPLACE "^0" "" D ${D})

configure_file(onscripter.rc.in onscripter.rc
    @ONLY
)

configure_file(winres.h.in winres.h
    @ONLY
)

configure_file(version.h.in version.h
    @ONLY
)

#############################################
# ONScripter-EN
add_executable(onscripter_en)

target_include_directories(onscripter_en 
PRIVATE 
    .
    ${CMAKE_CURRENT_BINARY_DIR}
    ${smpeg_include_directories}
)

target_compile_definitions(onscripter_en 
PRIVATE 
    _GNU_SOURCE=1
)

target_compile_features(onscripter_en 
PUBLIC 
    cxx_std_98
)

target_sources(onscripter_en
PRIVATE
    AnimationInfo.cpp
    AnimationInfo.h
#    AVIWrapper.cpp
#    AVIWrapper.h
    BaseReader.h
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
)

# Determine flag style
set(ONS_COMPILER_FLAG_STYLE "GNU")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
endif()

# Default Compiler Options
if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(onscripter_en
    PRIVATE
        -fdiagnostics-color=always
        -fpermissive
    )
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

target_link_options(onscripter_en
PUBLIC
    -static
)


# Dependencies
target_link_libraries(onscripter_en
PUBLIC
    PkgConfigWrapper::SDL
    PkgConfigWrapper::SDL_ttf
    PkgConfigWrapper::freetype2
    PkgConfigWrapper::SDL_image
    PkgConfigWrapper::libjpeg
    PkgConfigWrapper::libpng
    PkgConfigWrapper::zlib
    PkgConfigWrapper::SDL_mixer
    PkgConfigWrapper::ogg
    PkgConfigWrapper::vorbis
    PkgConfigWrapper::vorbisfile
    
    smpeg
    mad

)


# Architecture Specific Options
if (${ONS_TARGET_ARCH} STREQUAL "x86_64" OR ${ONS_TARGET_ARCH} STREQUAL "i386")
    target_compile_definitions(onscripter_en
    PRIVATE
        USE_X86_GFX
    )

    if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
        set_source_files_properties(
            graphics_sse2.cpp
            COMPILE_OPTIONS -msse2
        )

        set_source_files_properties(
            graphics_mmx.cpp
            COMPILE_OPTIONS -mmmx
        )
    endif()
endif()

# Platform Specific Options
if(PSP)
    create_pbp_file(
        TARGET onscripter_en
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE onscripter_en
        VERSION 01.00
    )
elseif(WIN32)
    target_sources(onscripter_en
    PRIVATE
        onscripter.rc
        winres.h
    )

    if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
        target_compile_options(onscripter_en
        PRIVATE
            -fpermissive
            -mwindows
        )
    endif()

    target_compile_definitions(onscripter_en
    PRIVATE
        WIN32
        USE_MESSAGEBOX
        USE_OGG_VORBIS
    )

    target_link_libraries(onscripter_en
    PRIVATE
        iconv
        m
        user32
        gdi32
        winmm
        dxguid
        rpcrt4
    )
endif()