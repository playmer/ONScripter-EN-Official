cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR) 
project(onscripter_en LANGUAGES C CXX)

if (APPLE)
    enable_language(OBJCXX)
endif()

if (NOT ANDROID)
    option(ONS_USE_AVIFILE "Choose whether to enable Lua support or not" ON)
    option(ONS_USE_QT_WINDOW "Choose whether to enable Qt window support or not, this will allow various Window commands to be enabled" ON)
else ()
    option(ONS_USE_AVIFILE "Choose whether to enable Lua support or not" OFF)
    option(ONS_USE_QT_WINDOW "Choose whether to enable Qt window support or not, this will allow various Window commands to be enabled" OFF)
endif()

option(ONS_USE_IMGUI_WINDOW "Choose whether to enable ImGui window support or not, this will allow various Window commands to be enabled" ON)
option(ONS_USE_LIB_MAD "Choose whether to use libmad instead of smpeg for mp3 audio" OFF)
option(ONS_USE_LUA "Choose whether to enable Lua support or not" ON)
option(ONS_USE_MESSAGEBOX "Determines whether Message Boxes are create on Windows" ON)
option(ONS_USE_OGG_VORBIS "Choose whether to enable Lua support or not" ON)

if (MSVC)
    add_compile_options(/MP)
endif()

###############
# onscripter-en required Libraries/Packages
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


#########
# Qt Support
if (ONS_USE_QT_WINDOW)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
    qt_standard_project_setup()
endif()

###############
# onscripter-en Build Tools

# embed
add_executable(onscripter_embed embed.cpp)

target_compile_features(onscripter_embed PUBLIC cxx_std_11)
add_custom_command(
    OUTPUT resources.cpp
    COMMAND onscripter_embed ons-en.ico gamecontrollerdb.txt > ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp
    DEPENDS onscripter_embed ${CMAKE_CURRENT_SOURCE_DIR}/gamecontrollerdb.txt ${CMAKE_CURRENT_SOURCE_DIR}/ons-en.ico
    COMMENT "This command generates resources.cpp to embed files into onscripter_en, such as the icon."
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


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

# Qt is a little weird
if (ANDROID)
    add_library(onscripter_en)
elseif (ONS_USE_QT_WINDOW)
    qt_add_executable(onscripter_en)
    qt_disable_unicode_defines(onscripter_en)
else()    
    add_executable(onscripter_en)
endif()

#########
# AVI Support
if (ONS_USE_AVIFILE)
    find_package(FFMPEG REQUIRED)

    target_compile_definitions(onscripter_en PRIVATE -DUSE_AVIFILE)
    
    target_include_directories(onscripter_en 
    PRIVATE 
        ${FFMPEG_INCLUDE_DIRS}
    )
    
    target_link_directories(onscripter_en PRIVATE ${FFMPEG_LIBRARY_DIRS})

    target_link_libraries(onscripter_en PRIVATE BZip2::BZip2
    PRIVATE
        ${FFMPEG_LIBRARIES}
    )

    target_sources(onscripter_en
    PRIVATE
        FFMpegWrapper.cpp
        FFMpegWrapper.h
    )
endif()

#########
# Lib Mad Support
if (ONS_USE_LIB_MAD)
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

    target_link_libraries(onscripter_en PRIVATE libmad)
    
    target_sources(onscripter_en
    PRIVATE
        MadWrapper.cpp
        MadWrapper.h
    )
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
)

#########
# Qt Window Support
if (ONS_USE_QT_WINDOW)
    target_link_libraries(onscripter_en PRIVATE Qt::Core Qt::Widgets)
    target_compile_definitions(onscripter_en PRIVATE -DUSE_QT_WINDOW)

    target_sources(onscripter_en
    PRIVATE
        QtWindow.cpp
        QtWindow.h
    )

    if(APPLE)
        set(executable_path "$<TARGET_FILE_NAME:onscripter_en>.app")
    else()
        set(executable_path "\${QT_DEPLOY_BIN_DIR}/$<TARGET_FILE_NAME:onscripter_en>")
    endif()

    # Helper app, not necessarily built as part of this project.
    # Generate a deployment script to be executed at install time
    qt_generate_deploy_script(
        TARGET onscripter_en
        OUTPUT_SCRIPT deploy_script
        CONTENT "
    qt_deploy_runtime_dependencies(
        EXECUTABLE \"${executable_path}\"
        GENERATE_QT_CONF
        VERBOSE
    )")
endif()

target_link_libraries(onscripter_en
PRIVATE
    BZip2::BZip2
    ZLIB::ZLIB
    PNG::PNG
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

if(WIN32)
    target_sources(onscripter_en
    PRIVATE
        onscripter.rc
    )

    if (ONS_USE_QT_WINDOW)
        # Qt Deployment stuff
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt PATHS ${_qt_cmake_dir}/../../bin)
        message(STATUS Found Qt tools: ${TOOL_WINDEPLOYQT_DEBUG},${TOOL_WINDEPLOYQT})

        
        add_custom_command(TARGET onscripter_en POST_BUILD
            COMMAND $<IF:$<CONFIG:DEBUG>,${TOOL_WINDEPLOYQT},${TOOL_WINDEPLOYQT}> 
                    $<IF:$<CONFIG:DEBUG>,--debug,--release>
                    $<TARGET_FILE:onscripter_en>
            COMMENT "Running windeployqt..."
        )
    endif()
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

    if (ONS_USE_QT_WINDOW)
        #include(${QT_DEPLOY_SUPPORT})
        # Qt Deployment stuff
        #find_program(TOOL_MACDEPLOYQT_DEBUG NAMES macdeployqt.debug.bat PATHS ${_qt_cmake_dir}/../../bin)
        #find_program(TOOL_MACDEPLOYQT NAMES macdeployqt PATHS ${_qt_cmake_dir}/../../bin)
    endif()
endif()

# Sets onscripter_en as the startup project in VS, doesn't affect non-VS generators.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT onscripter_en)
