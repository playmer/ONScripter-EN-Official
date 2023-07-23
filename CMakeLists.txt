cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR) 


# we should not do it this way, I'm being lazy for testing/ initial development of this cmake, we should just have the user pass in the toolchain
# file on windows/mac, and then the find_packages should get intercepted and these libraries installed, linux can use their built in package managers.
include(FetchContent)
FetchContent_Declare(
    vcpkg
    GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    FetchContent_Populate(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()

project(onscripter_en C CXX)


option(ONS_USE_LUA "Choose whether to enable Lua support or not" ON)

find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(Freetype REQUIRED)
find_library(libmad_path libmad)
message(STATUS libmad: ${libmad_path})
find_package(Ogg CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)









#Theoretical CD Rom library
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

if (MSVC)
    target_compile_definitions(SDLHelper_CD PRIVATE -DSDL_CDROM_WIN32)

    target_sources(SDLHelper_CD 
    PUBLIC
        SDLHelper_CD/cdrom/win32/SDL_syscdrom.c
    )
endif()



if (MSVC)
    add_compile_options(/MP)
endif()


add_executable(onscripter_en)

if (ONS_USE_LUA)
    find_package(Lua REQUIRED)
    target_include_directories(onscripter_en PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(onscripter_en PRIVATE ${LUA_LIBRARIES})
    target_compile_definitions(onscripter_en PRIVATE -DUSE_LUA -DUSE_AVIFILE)

    target_sources(onscripter_en
    PRIVATE
        LUAHandler.cpp
        LUAHandler.h
    )
endif()

target_link_libraries(onscripter_en PRIVATE BZip2::BZip2)
target_link_libraries(onscripter_en PRIVATE ZLIB::ZLIB)
target_link_libraries(onscripter_en PRIVATE PNG::PNG)
target_include_directories(onscripter_en PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(onscripter_en PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(onscripter_en PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(onscripter_en PRIVATE Freetype::Freetype)
target_link_libraries(onscripter_en PRIVATE Ogg::ogg)
target_link_libraries(onscripter_en PRIVATE Vorbis::vorbis)
target_link_libraries(onscripter_en PRIVATE Vorbis::vorbisfile)
target_link_libraries(onscripter_en PRIVATE Vorbis::vorbisenc)
target_link_libraries(onscripter_en
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(onscripter_en PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
target_link_libraries(onscripter_en PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(onscripter_en PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)

target_link_libraries(onscripter_en PRIVATE SDLHelper_CD)






#target_compile_definitions(onscripter_en PUBLIC -DUSE_OGG_VORBIS -DINTEGER_OGG_VORBIS)

target_compile_features(onscripter_en PUBLIC cxx_std_17)

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
    #embed.cpp
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
    onscripter.rc.in
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
    resources.h
    SarReader.cpp
    SarReader.h
    ScriptHandler.cpp
    ScriptHandler.h
    ScriptParser_command.cpp
    ScriptParser.cpp
    ScriptParser.h
    ScriptParser_command.cpp
    SDL_win32_main.c
    sjis2utf16.cpp
    version.h
    winres.h
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT onscripter_en)