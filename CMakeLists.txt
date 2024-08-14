cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(onscripter_en LANGUAGES C CXX)

option(ONS_USE_EXT_LIBS "Choose whether to use our own libraries or to use system packages" ON)

function(ons_create_ext_target libname libfilename)
    add_library(${libname} STATIC IMPORTED GLOBAL)
    set_target_properties(
        ${libname}
        PROPERTIES
            IMPORTED_LOCATION 
                ${libfilename}
    )
endfunction()

function(run_library_config library_name include_directories)
    set(${library_name}_lib_cmd ${library_name}-config)
    set(lib_args --libs --cflags)

    find_program(${library_name}_lib_command ${${library_name}_lib_cmd})

    if (${${library_name}_lib_command} MATCHES "-NOTFOUND")
        return()
    endif()

    execute_process(
        COMMAND bash ${${library_name}_lib_command} ${lib_args}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE config_result
        OUTPUT_VARIABLE config_stdout_results
        ERROR_VARIABLE config_stderr_results
    )

    message(STATUS "execute_process command: ${${library_name}_lib_command}")
    message(STATUS "\targs: ${lib_args}")
    message(STATUS "\tresults: ${config_stdout_results}")
    message(STATUS "\terr: ${config_stderr_results}")
    message(STATUS "\tcode: ${config_result}")

    string(REPLACE " " ";" flags ${config_stdout_results})
    foreach(flag ${flags})
        string(FIND "${flag}" "-I" out)
        if(NOT "${out}" EQUAL 0)
            continue()
        endif()

        string(SUBSTRING ${flag} 2 -1 include_directory)

        if (EXISTS ${include_directory})
            set(processed_include_directory ${include_directory})
        else()
            cmake_path(CONVERT ${${library_name}_lib_command} TO_CMAKE_PATH_LIST path_list)
            string(REPLACE "/" ";" path_list ${path_list})
            message(STATUS "\tPathList: ${path_list}")
            
            foreach(component ${path_list})
                message(STATUS "\tcomparing: ${include_directory}, ${component}")
                string(FIND "${include_directory}" "/${component}" out)
                if("${out}" EQUAL 0)
                    set(processed_include_directory ${processed_include_directory}${include_directory})
                    break()
                endif()

                if (component MATCHES ":$")
                    set(component ${component}/)
                endif()
                
                cmake_path(APPEND processed_include_directory ${component})
                message(STATUS "\tAdded: ${processed_include_directory}")
            endforeach()
        endif()

        list(APPEND include_directories_list ${processed_include_directory})
    endforeach()

    message(STATUS "\tinclude_directories [${include_directories}]: ${include_directories_list}")
    set(${include_directories} ${include_directories_list} PARENT_SCOPE)
endfunction()

#if (PLATFORM_PSP)
#
#endif()


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


add_executable(onscripter_en)


if(ONS_USE_EXT_LIBS)
    include(cmake/PkgConfigWrapper.cmake)

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
    PkgConfig_Find_Module(harfbuzz)
    PkgConfig_Find_Module(bzip2)

    run_library_config(sdl sdl_include_directories)
    run_library_config(smpeg smpeg_include_directories)
endif()

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

target_link_libraries(onscripter_en
PUBLIC
    PkgConfigWrapper::SDL
    PkgConfigWrapper::SDL_image
    PkgConfigWrapper::SDL_mixer
    PkgConfigWrapper::SDL_ttf
    PkgConfigWrapper::freetype2
    PkgConfigWrapper::ogg
    PkgConfigWrapper::zlib
    PkgConfigWrapper::vorbis
    PkgConfigWrapper::vorbisfile
    PkgConfigWrapper::libpng
    PkgConfigWrapper::libjpeg
    PkgConfigWrapper::harfbuzz
    PkgConfigWrapper::bzip2
    
    mad
    smpeg
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
    #winres.h
)

set(ONS_COMPILER_FLAG_STYLE "GNU")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
endif()


if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(onscripter_en
    PRIVATE
        -fdiagnostics-color=always
        -static
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
    target_compile_options(onscripter_en
    PRIVATE
        -fpermissive
        -mmmx
    )

    target_compile_definitions(onscripter_en
    PRIVATE
        WIN32
        USE_MESSAGEBOX
        USE_OGG_VORBIS
        USE_X86_GFX
    )
endif()