cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR) 
project(onscripter_en C CXX)

if (APPLE)
    enable_language(OBJCXX)
endif()

option(ONS_USE_AVIFILE "Choose whether to enable Lua support or not" ON)
option(ONS_USE_LUA "Choose whether to enable Lua support or not" ON)
option(ONS_USE_MESSAGEBOX "Determines whether Message Boxes are create on Windows" ON)
option(ONS_USE_OGG_VORBIS "Choose whether to enable Lua support or not" ON)

if (MSVC)
    add_compile_options(/MP)
endif()

###############
# onscripter-en required Libraries/Packages

find_library(LIBMAD_LIBRARY mad.lib libmad.a PATHS ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/ NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX REQUIRED)
find_library(LIBMAD_DEBUG_LIBRARY mad.lib libmad.a PATHS ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/lib/ NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX REQUIRED)

add_library(libmad STATIC IMPORTED [GLOBAL])
# set the location that we are going to import the library from
set_target_properties(
    libmad 
    PROPERTIES
        IMPORTED_LOCATION_DEBUG ${LIBMAD_DEBUG_LIBRARY}
        IMPORTED_LOCATION ${LIBMAD_LIBRARY}
)

find_library(SMPEG2_LIBRARY smpeg2.lib libsmpeg2.a PATHS ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/ NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX REQUIRED)
find_library(SMPEG2_DEBUG_LIBRARY smpeg2.lib libsmpeg2.a PATHS ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/lib/ NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX REQUIRED)
#find_file(SMPEG2_SHARED_LIBRARY smpeg2.dll PATHS ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/ NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX)
#find_file(SMPEG2_SHARED_DEBUG_LIBRARY smpeg2.dll PATHS ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/bin/ NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX)

add_library(smpeg2 STATIC IMPORTED [GLOBAL])
# set the location that we are going to import the library from
set_target_properties(
    smpeg2 
    PROPERTIES
        IMPORTED_LOCATION_DEBUG ${SMPEG2_DEBUG_LIBRARY}
        IMPORTED_LOCATION_RELEASE ${SMPEG2_LIBRARY}
        IMPORTED_LOCATION ${SMPEG2_LIBRARY}
)

find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(Freetype REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS SMPEG2_SHARED_DEBUG_LIBRARY: ${SMPEG2_SHARED_DEBUG_LIBRARY})
message(STATUS SMPEG2_DEBUG_LIBRARY: ${SMPEG2_DEBUG_LIBRARY})
message(STATUS SMPEG2_SHARED_LIBRARY: ${SMPEG2_SHARED_LIBRARY})
message(STATUS SMPEG2_LIBRARY: ${SMPEG2_LIBRARY})



###############
# onscripter-en Build Tools

if(NOT WIN32)
    # embed
    add_executable(onscripter_embed embed.cpp)

    add_custom_command(
        OUTPUT resources.cpp
        COMMAND onscripter_embed ons-en.ico > resources.cpp
        COMMENT "This command generates resources.cpp to embed files into onscripter_en, such as the icon."
        VERBATIM
    )
endif()

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


###############
#SDL CD Rom library adapted from original SDL1 CD Module
add_library(SDLHelper_CD)

target_sources(SDLHelper_CD 
PUBLIC
    SDLHelper_CD/SDL_cdrom.h
    SDLHelper_CD/cdrom/SDL_syscdrom.h
    SDLHelper_CD/cdrom/SDL_cdrom.c
)

target_include_directories(SDLHelper_CD PUBLIC SDLHelper_CD/)

target_link_libraries(SDLHelper_CD
    PRIVATE
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

if (WIN32)
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_WIN32)

    target_sources(SDLHelper_CD 
    PUBLIC
        SDLHelper_CD/cdrom/win32/SDL_syscdrom.c
    )
elseif (APPLE)
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_MACOSX)

    target_sources(SDLHelper_CD 
    PUBLIC
        SDLHelper_CD/cdrom/macosx/AudioFilePlayer.c
        SDLHelper_CD/cdrom/macosx/AudioFilePlayer.h
        SDLHelper_CD/cdrom/macosx/AudioFileReaderThread.c
        SDLHelper_CD/cdrom/macosx/CDPlayer.c
        SDLHelper_CD/cdrom/macosx/CDPlayer.h
        SDLHelper_CD/cdrom/macosx/SDL_syscdrom.c
        SDLHelper_CD/cdrom/macosx/SDL_syscdrom_c.h
        SDLHelper_CD/cdrom/macosx/SDLOSXCAGuard.c
        SDLHelper_CD/cdrom/macosx/SDLOSXCAGuard.h
    )
else()
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_LINUX)

    target_sources(SDLHelper_CD 
    PUBLIC
        SDLHelper_CD/cdrom/linux/SDL_syscdrom.c
    )
endif()

###############
# onscripter-en Tools
add_subdirectory(tools)

###############
# onscripter-en
add_executable(onscripter_en)

#########
# AVI Support
if (ONS_USE_AVIFILE)
    target_compile_definitions(onscripter_en PRIVATE -DUSE_AVIFILE)
endif()

#########
# Lua Support
if (ONS_USE_LUA)
    find_package(Lua REQUIRED)
    target_include_directories(onscripter_en PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(onscripter_en PRIVATE ${LUA_LIBRARIES})
    target_compile_definitions(onscripter_en PRIVATE -DUSE_LUA)

    target_sources(onscripter_en
    PRIVATE
        LUAHandler.cpp
        LUAHandler.h
    )
endif()

#########
# MessageBox Support (Windows)
if (ONS_USE_MESSAGEBOX)
    target_compile_definitions(onscripter_en PRIVATE -DUSE_MESSAGEBOX)
endif()

#########
# Ogg Vorbis Support
if (ONS_USE_OGG_VORBIS)
    target_compile_definitions(onscripter_en PRIVATE -DUSE_OGG_VORBIS)
endif()

target_include_directories(onscripter_en 
PRIVATE 
    .
    ${CMAKE_CURRENT_BINARY_DIR}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(onscripter_en PRIVATE ${FFMPEG_LIBRARY_DIRS})

target_link_libraries(onscripter_en PRIVATE BZip2::BZip2
PRIVATE
    ZLIB::ZLIB
    PNG::PNG
    ${FFMPEG_LIBRARIES}
    Freetype::Freetype
    libmad
    Ogg::ogg
    smpeg2
    Vorbis::vorbis
    Vorbis::vorbisfile
    Vorbis::vorbisenc
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    SDLHelper_CD
)

if (WIN32)
    target_link_libraries(onscripter_en PRIVATE winmm)
endif()

target_compile_features(onscripter_en PUBLIC cxx_std_11)

target_sources(onscripter_en
PRIVATE
    AnimationInfo.cpp
    AnimationInfo.h
    AVIWrapper.cpp
    AVIWrapper.h
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
    FFMpegWrapper.cpp
    FFMpegWrapper.h
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
    MadWrapper.cpp
    MadWrapper.h
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
    "winres.h.in"
)

if(WIN32)
    target_sources(onscripter_en
    PRIVATE
        onscripter.rc
    )
endif()

if(APPLE)
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
endif()

if(NOT WIN32)
    target_sources(onscripter_en
    PRIVATE
        resources.cpp
        resources.h
    )
endif()

# Sets onscripter_en as the startup project in VS, doesn't affect non-VS generators.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT onscripter_en)
