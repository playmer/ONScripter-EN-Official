##########
# SDL

if(ONS_USE_SDLCOMPAT AND NOT ONS_USE_NATIVESDL2)
    add_subdirectory(SDL2)
    set(SDL2_LIBRARY SDL2)
    set(SDL2_INCLUDE_DIR SDL2/include)
elseif(ONS_USE_NATIVESDL12)
    #if (ONS_USE_OLD_DEPS)
    #    set(sdl_source_prefix src/old/sdl-1.2.13)
    #else()
    #    set(sdl_source_prefix src/sdl-1.2.15)
    #endif()
    #
    #add_library(sdl STATIC "")
    #
    #file(GLOB SDL_SOURCE
    #    ${SDL}/src/*.c
    #    ${SDL}/src/audio/*.c
    #    ${SDL}/src/audio/disk/*.c
    #    ${SDL}/src/audio/dummy/*.c
    #    ${SDL}/src/cdrom/*.c
    #    ${SDL}/src/cdrom/dummy/*.c
    #    ${SDL}/src/cpuinfo/*.c
    #    ${SDL}/src/events/*.c
    #    ${SDL}/src/file/*.c
    #    ${SDL}/src/joystick/*.c
    #    ${SDL}/src/stdlib/*.c
    #    ${SDL}/src/thread/*.c
    #    ${SDL}/src/thread/generic/*.c
    #    ${SDL}/src/timer/*.c
    #    ${SDL}/src/video/*.c
    #    ${SDL}/src/video/dummy/*.c
    #)
    #
    #file(
    #    GLOB SDL_MACOS_SOURCES
    #    ${SDL}/src/audio/macosx/*.c
    #    ${SDL}/src/cdrom/macosx/*.c
    #    ${SDL}/src/joystick/darwin/*.c
    #    ${SDL}/src/loadso/dlopen/*.c
    #    ${SDL}/src/loadso/macosx/*.c
    #    # ${SDL}/src/thread/pthread/*.c
    #    ${SDL}/src/timer/unix/*.c
    #    ${SDL}/src/video/quartz/*.m
    #)
    #
    #if( UNIX AND APPLE )
    #	set( LIBSDL_SOURCES ${LIBSDL_SOURCES} ${SDL_MACOS_SOURCES} )
    #	set( LIBSDLMAIN_SOURCES ${SDL}/src/main/macosx/SDLMain.m )
    #
    #	## copy over a pre-created macOS config
    #	set( CFLAGS "-Wall -Wextra -O2 -g -fomit-frame-pointer -fPIC" )
    #	set( CXXFLAGS "${CFLAGS} -fno-exceptions -fno-rtti" )
    #
    #	file( STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/SDL_config.h.macOS SDL_CONFIG_HEADER )
    #	message( AUTHOR_WARNING "+ Building for macOS.")
    #else()
    #	message( FATAL_ERROR "Unknown operating system-- aborting." )
    #endif()
endif()

if(ONS_USE_SDLCOMPAT)
    option(SDL12TESTS "Enable to build SDL-1.2 test programs" OFF)
    option(SDL12DEVEL "Enable installing SDL-1.2 development headers" OFF)
    option(STATICDEVEL "Enable installing static link library" OFF)
    if(NINTENDO_WIIU
        OR NINTENDO_SWITCH
        OR NXDK)
        option(SDL12_FULLY_STATIC "Make SDL12_compat capable of being statically compiled alongside SDL2" ON)
    endif()

    add_subdirectory(sdl12-compat)

    target_include_directories(SDL
    PRIVATE 
        ${SDL2_INCLUDE_DIR}
    )

    target_include_directories(SDL
    INTERFACE 
        sdl12-compat/include
        sdl12-compat/include/SDL
    )
    
    if(NINTENDO_WIIU
       OR NINTENDO_SWITCH
       OR NXDK)
        target_compile_definitions(SDL
        PUBLIC 
            __SDL12_COMPAT_FULLY_STATIC__
        )
    endif()
endif()

##########
# bzip2
if (ONS_USE_OLD_DEPS)
    set(bzip2_source_prefix src/old/bzip2-1.0.4)
else()
    set(bzip2_source_prefix src/bzip2-1.0.8)
endif()

add_library(bzip2 STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(bzip2
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(bzip2
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_sources(bzip2
PRIVATE
    ${bzip2_source_prefix}/blocksort.c
    ${bzip2_source_prefix}/bzlib_private.h
    ${bzip2_source_prefix}/bzlib.c
    ${bzip2_source_prefix}/bzlib.h
    ${bzip2_source_prefix}/compress.c
    ${bzip2_source_prefix}/crctable.c
    ${bzip2_source_prefix}/decompress.c
    ${bzip2_source_prefix}/huffman.c
    ${bzip2_source_prefix}/randtable.c
)

target_include_directories(bzip2
PUBLIC
    ${bzip2_source_prefix}/
)

##########
# libogg
if (ONS_USE_OLD_DEPS)
    set(ogg_source_prefix src/old/libogg-1.1.3)
else()
    set(ogg_source_prefix src/libogg-1.3.5)
endif()

add_library(ogg STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(ogg
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(ogg
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_sources(ogg
PRIVATE
    ${ogg_source_prefix}/src/bitwise.c
    ${ogg_source_prefix}/src/framing.c
)

if (UNIX
    OR NINTENDO_SWITCH
    OR NXDK
    OR ${ONS_TARGET_ARCH} STREQUAL "ppc" 
    OR ${ONS_TARGET_ARCH} STREQUAL "ppc")
    set(SIZE16 "short")
    set(USIZE16 "unsigned short")
    set(SIZE32 "int")
    set(USIZE32 "unsigned int")
    set(SIZE64 "long long")

    configure_file(${ogg_source_prefix}/include/ogg/config_types.h.in ogg/config_types.h
        @ONLY
    )
endif()

target_include_directories(ogg
PUBLIC
    ${ogg_source_prefix}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

##########
# libvorbis
if (ONS_USE_OLD_DEPS)
    set(vorbis_source_prefix src/old/libvorbis-1.2.0)
else()
    set(vorbis_source_prefix src/libvorbis-1.3.7)
endif()

add_library(vorbis STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(vorbis
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(vorbis
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(vorbis
PUBLIC
    ogg
)


if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(vorbis 
    PRIVATE
        -Wno-int-conversion
        -Wno-overflow
        -Wno-incompatible-pointer-types
        -Wno-implicit-function-declaration
    )
endif()

target_sources(vorbis
PRIVATE
    ${vorbis_source_prefix}/lib/analysis.c
    ${vorbis_source_prefix}/lib/backends.h
    #${vorbis_source_prefix}/lib/barkmel.c
    ${vorbis_source_prefix}/lib/bitrate.c
    ${vorbis_source_prefix}/lib/bitrate.h
    ${vorbis_source_prefix}/lib/block.c
    ${vorbis_source_prefix}/lib/codebook.c
    ${vorbis_source_prefix}/lib/codebook.h
    ${vorbis_source_prefix}/lib/codec_internal.h
    ${vorbis_source_prefix}/lib/envelope.c
    ${vorbis_source_prefix}/lib/envelope.h
    ${vorbis_source_prefix}/lib/floor0.c
    ${vorbis_source_prefix}/lib/floor1.c
    ${vorbis_source_prefix}/lib/highlevel.h
    ${vorbis_source_prefix}/lib/info.c
    ${vorbis_source_prefix}/lib/lookup.c
    ${vorbis_source_prefix}/lib/lookup.h
    ${vorbis_source_prefix}/lib/lookups.pl
    ${vorbis_source_prefix}/lib/lookup_data.h
    ${vorbis_source_prefix}/lib/lpc.c
    ${vorbis_source_prefix}/lib/lpc.h
    ${vorbis_source_prefix}/lib/lsp.c
    ${vorbis_source_prefix}/lib/lsp.h
    ${vorbis_source_prefix}/lib/Makefile.am
    ${vorbis_source_prefix}/lib/Makefile.in
    ${vorbis_source_prefix}/lib/mapping0.c
    ${vorbis_source_prefix}/lib/masking.h
    ${vorbis_source_prefix}/lib/mdct.c
    ${vorbis_source_prefix}/lib/mdct.h
    ${vorbis_source_prefix}/lib/misc.h
    ${vorbis_source_prefix}/lib/os.h
    ${vorbis_source_prefix}/lib/psy.c
    ${vorbis_source_prefix}/lib/psy.h
    #${vorbis_source_prefix}/lib/psytune.c
    ${vorbis_source_prefix}/lib/registry.c
    ${vorbis_source_prefix}/lib/registry.h
    ${vorbis_source_prefix}/lib/res0.c
    ${vorbis_source_prefix}/lib/scales.h
    ${vorbis_source_prefix}/lib/sharedbook.c
    ${vorbis_source_prefix}/lib/smallft.c
    ${vorbis_source_prefix}/lib/smallft.h
    ${vorbis_source_prefix}/lib/synthesis.c
    #${vorbis_source_prefix}/lib/tone.c
    ${vorbis_source_prefix}/lib/vorbisenc.c
    ${vorbis_source_prefix}/lib/vorbisfile.c
    ${vorbis_source_prefix}/lib/window.c
    ${vorbis_source_prefix}/lib/window.h
    #${vorbis_source_prefix}/vq/bookutil.c
    #${vorbis_source_prefix}/vq/bookutil.h
    #${vorbis_source_prefix}/vq/build.c
    #${vorbis_source_prefix}/vq/cascade.c
    #${vorbis_source_prefix}/vq/distribution.c
    #${vorbis_source_prefix}/vq/genericdata.c
    #${vorbis_source_prefix}/vq/huffbuild.c
    #${vorbis_source_prefix}/vq/latticebuild.c
    #${vorbis_source_prefix}/vq/latticehint.c
    #${vorbis_source_prefix}/vq/latticepare.c
    #${vorbis_source_prefix}/vq/latticetune.c
    #${vorbis_source_prefix}/vq/localcodebook.h
    #${vorbis_source_prefix}/vq/lspdata.c
    #${vorbis_source_prefix}/vq/metrics.c
    #${vorbis_source_prefix}/vq/residuedata.c
    #${vorbis_source_prefix}/vq/residuesplit.c
    #${vorbis_source_prefix}/vq/run.c
    #${vorbis_source_prefix}/vq/train.c
    #${vorbis_source_prefix}/vq/vqext.h
    #${vorbis_source_prefix}/vq/vqgen.c
    #${vorbis_source_prefix}/vq/vqgen.h
    #${vorbis_source_prefix}/vq/vqsplit.c
    #${vorbis_source_prefix}/vq/vqsplit.h
)

target_include_directories(vorbis
PUBLIC
    ${vorbis_source_prefix}/include
    ${vorbis_source_prefix}/lib
    ${vorbis_source_prefix}/vq
)

##########
# libjpeg
#if (ONS_USE_OLD_DEPS)
#    set(jpeg_source_prefix src/old/jpeg-6b)
#else()
#    set(jpeg_source_prefix src/jpeg-9f)
#endif()

set(jpeg_source_prefix src/jpeg-9f)

add_library(jpeg STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(jpeg
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(jpeg
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(jpeg 
    PRIVATE
        -Wno-implicit-function-declaration
        -Wno-builtin-declaration-mismatch
    )
endif()

target_sources(jpeg
PRIVATE
    ${jpeg_source_prefix}/jaricom.c
    ${jpeg_source_prefix}/jcapimin.c
    ${jpeg_source_prefix}/jcapistd.c
    ${jpeg_source_prefix}/jcarith.c
    ${jpeg_source_prefix}/jccoefct.c
    ${jpeg_source_prefix}/jccolor.c
    ${jpeg_source_prefix}/jcdctmgr.c
    ${jpeg_source_prefix}/jchuff.c
    ${jpeg_source_prefix}/jcinit.c
    ${jpeg_source_prefix}/jcmainct.c
    ${jpeg_source_prefix}/jcmarker.c
    ${jpeg_source_prefix}/jcmaster.c
    ${jpeg_source_prefix}/jcomapi.c
    ${jpeg_source_prefix}/jcparam.c
    ${jpeg_source_prefix}/jcprepct.c
    ${jpeg_source_prefix}/jcsample.c
    ${jpeg_source_prefix}/jctrans.c
    ${jpeg_source_prefix}/jdapimin.c
    ${jpeg_source_prefix}/jdapistd.c
    ${jpeg_source_prefix}/jdarith.c
    ${jpeg_source_prefix}/jdatadst.c
    ${jpeg_source_prefix}/jdatasrc.c
    ${jpeg_source_prefix}/jdcoefct.c
    ${jpeg_source_prefix}/jdcolor.c
    ${jpeg_source_prefix}/jddctmgr.c
    ${jpeg_source_prefix}/jdhuff.c
    ${jpeg_source_prefix}/jdinput.c
    ${jpeg_source_prefix}/jdmainct.c
    ${jpeg_source_prefix}/jdmarker.c
    ${jpeg_source_prefix}/jdmaster.c
    ${jpeg_source_prefix}/jdmerge.c
    ${jpeg_source_prefix}/jdpostct.c
    ${jpeg_source_prefix}/jdsample.c
    ${jpeg_source_prefix}/jdtrans.c
    ${jpeg_source_prefix}/jerror.c
    ${jpeg_source_prefix}/jfdctflt.c
    ${jpeg_source_prefix}/jfdctfst.c
    ${jpeg_source_prefix}/jfdctint.c
    ${jpeg_source_prefix}/jidctflt.c
    ${jpeg_source_prefix}/jidctfst.c
    ${jpeg_source_prefix}/jidctint.c
    ${jpeg_source_prefix}/jquant1.c
    ${jpeg_source_prefix}/jquant2.c
    ${jpeg_source_prefix}/jutils.c
    ${jpeg_source_prefix}/jmemmgr.c
    ${jpeg_source_prefix}/jmemnobs.c
)

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCSourceCompiles)

check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_symbol_exists(size_t stddef.h stdlib.h stdio.h HAVE_ANSI_SIZE_T)
check_symbol_exists(setmode io.h USE_SETMODE)
check_c_source_compiles(
"
void f()
{
    char x[ (char)0xff ]; /* x[-1] if signed */
}
"
CHAR_IS_UNSIGNED)

check_c_source_compiles(
"
void f()
{
    char x[ ((signed char)0xff) >> 1 ]; /* x[-1] if signed */
}
"
RIGHT_SHIFT_IS_UNSIGNED)

if(NOT HAVE_STRING_H)
    set(NEED_BSD_STRINGS TRUE)
else()
    set(NEED_BSD_STRINGS FALSE)
endif()

if(NOT HAVE_ANSI_SIZE_T)
    set(NEED_SYS_TYPES_H TRUE)
else()
    set(NEED_SYS_TYPES_H FALSE)
endif()

configure_file(${jpeg_source_prefix}/jconfig.txt jconfig.h)

target_include_directories(jpeg
PUBLIC
    ${jpeg_source_prefix}/
    ${CMAKE_CURRENT_BINARY_DIR}
)

##########
# freetype
if (ONS_USE_OLD_DEPS)
    set(freetype_source_prefix src/old/freetype-2.3.5)
else()
    set(freetype_source_prefix src/freetype-2.13.0)
endif()

add_library(freetype STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(freetype
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(freetype
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()


if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(freetype 
    PRIVATE
        -Wno-implicit-function-declaration
    )
endif()

target_compile_definitions(freetype 
PUBLIC 
    FT2_BUILD_LIBRARY
    FT_OPTION_AUTOFIT2
)

target_sources(freetype
PRIVATE
    ${freetype_source_prefix}/src/autofit/afangles.h
    ${freetype_source_prefix}/src/autofit/afcjk.h
    ${freetype_source_prefix}/src/autofit/afdummy.h
    ${freetype_source_prefix}/src/autofit/aferrors.h
    ${freetype_source_prefix}/src/autofit/afglobal.h
    ${freetype_source_prefix}/src/autofit/afhints.h
    ${freetype_source_prefix}/src/autofit/afindic.h
    ${freetype_source_prefix}/src/autofit/aflatin.h
    ${freetype_source_prefix}/src/autofit/aflatin2.h
    ${freetype_source_prefix}/src/autofit/afloader.h
    ${freetype_source_prefix}/src/autofit/afmodule.h
    ${freetype_source_prefix}/src/autofit/aftypes.h
    ${freetype_source_prefix}/src/autofit/afwarp.h
    ${freetype_source_prefix}/src/autofit/autofit.c
    ${freetype_source_prefix}/src/autofit/Jamfile
    ${freetype_source_prefix}/src/autofit/module.mk
    ${freetype_source_prefix}/src/autofit/rules.mk
    ${freetype_source_prefix}/src/base/ftapi.c
    ${freetype_source_prefix}/src/base/ftbase.c
    ${freetype_source_prefix}/src/base/ftbbox.c
    ${freetype_source_prefix}/src/base/ftbitmap.c
    ${freetype_source_prefix}/src/base/ftdebug.c
    ${freetype_source_prefix}/src/base/ftgasp.c
    ${freetype_source_prefix}/src/base/ftglyph.c
    ${freetype_source_prefix}/src/base/ftgxval.c
    ${freetype_source_prefix}/src/base/ftinit.c
    ${freetype_source_prefix}/src/base/ftlcdfil.c
    ${freetype_source_prefix}/src/base/ftotval.c
    ${freetype_source_prefix}/src/base/ftstroke.c
    ${freetype_source_prefix}/src/base/ftsystem.c
    ${freetype_source_prefix}/src/base/fttype1.c
    ${freetype_source_prefix}/src/base/Jamfile
    ${freetype_source_prefix}/src/base/rules.mk
    ${freetype_source_prefix}/src/cache/ftcache.c
    ${freetype_source_prefix}/src/cache/ftccache.h
    ${freetype_source_prefix}/src/cache/ftccback.h
    ${freetype_source_prefix}/src/cache/ftcerror.h
    ${freetype_source_prefix}/src/cache/ftcglyph.h
    ${freetype_source_prefix}/src/cache/ftcimage.h
    ${freetype_source_prefix}/src/cache/ftcmanag.h
    ${freetype_source_prefix}/src/cache/ftcmru.h
    ${freetype_source_prefix}/src/cache/ftcsbits.h
    ${freetype_source_prefix}/src/cache/Jamfile
    ${freetype_source_prefix}/src/cache/rules.mk
    ${freetype_source_prefix}/src/cff/cff.c
    ${freetype_source_prefix}/src/cff/cffcmap.h
    ${freetype_source_prefix}/src/cff/cffdrivr.h
    ${freetype_source_prefix}/src/cff/cfferrs.h
    ${freetype_source_prefix}/src/cff/cffgload.h
    ${freetype_source_prefix}/src/cff/cffload.h
    ${freetype_source_prefix}/src/cff/cffobjs.h
    ${freetype_source_prefix}/src/cff/cffparse.h
    ${freetype_source_prefix}/src/cff/cfftoken.h
    ${freetype_source_prefix}/src/cff/cfftypes.h
    ${freetype_source_prefix}/src/cff/Jamfile
    ${freetype_source_prefix}/src/cff/module.mk
    ${freetype_source_prefix}/src/cff/rules.mk
    ${freetype_source_prefix}/src/cid/ciderrs.h
    ${freetype_source_prefix}/src/cid/cidgload.h
    ${freetype_source_prefix}/src/cid/cidload.h
    ${freetype_source_prefix}/src/cid/cidobjs.h
    ${freetype_source_prefix}/src/cid/cidparse.h
    ${freetype_source_prefix}/src/cid/cidriver.h
    ${freetype_source_prefix}/src/cid/cidtoken.h
    ${freetype_source_prefix}/src/cid/Jamfile
    ${freetype_source_prefix}/src/cid/module.mk
    ${freetype_source_prefix}/src/cid/rules.mk
    ${freetype_source_prefix}/src/cid/type1cid.c
    ${freetype_source_prefix}/src/otvalid/Jamfile
    ${freetype_source_prefix}/src/otvalid/module.mk
    ${freetype_source_prefix}/src/otvalid/otvalid.c
    ${freetype_source_prefix}/src/otvalid/otvalid.h
    ${freetype_source_prefix}/src/otvalid/otvcommn.h
    ${freetype_source_prefix}/src/otvalid/otverror.h
    ${freetype_source_prefix}/src/otvalid/otvgpos.h
    ${freetype_source_prefix}/src/otvalid/otvmod.h
    ${freetype_source_prefix}/src/otvalid/rules.mk
    ${freetype_source_prefix}/src/psaux/afmparse.h
    ${freetype_source_prefix}/src/psaux/Jamfile
    ${freetype_source_prefix}/src/psaux/module.mk
    ${freetype_source_prefix}/src/psaux/psaux.c
    ${freetype_source_prefix}/src/psaux/psauxerr.h
    ${freetype_source_prefix}/src/psaux/psauxmod.h
    ${freetype_source_prefix}/src/psaux/psconv.h
    ${freetype_source_prefix}/src/psaux/psobjs.h
    ${freetype_source_prefix}/src/psaux/rules.mk
    ${freetype_source_prefix}/src/psaux/t1cmap.h
    ${freetype_source_prefix}/src/psaux/t1decode.h
    ${freetype_source_prefix}/src/pshinter/Jamfile
    ${freetype_source_prefix}/src/pshinter/module.mk
    ${freetype_source_prefix}/src/pshinter/pshalgo.h
    ${freetype_source_prefix}/src/pshinter/pshglob.h
    ${freetype_source_prefix}/src/pshinter/pshinter.c
    ${freetype_source_prefix}/src/pshinter/pshmod.h
    ${freetype_source_prefix}/src/pshinter/pshnterr.h
    ${freetype_source_prefix}/src/pshinter/pshrec.h
    ${freetype_source_prefix}/src/pshinter/rules.mk
    ${freetype_source_prefix}/src/psnames/Jamfile
    ${freetype_source_prefix}/src/psnames/module.mk
    ${freetype_source_prefix}/src/psnames/psmodule.h
    ${freetype_source_prefix}/src/psnames/psnamerr.h
    ${freetype_source_prefix}/src/psnames/psnames.c
    ${freetype_source_prefix}/src/psnames/pstables.h
    ${freetype_source_prefix}/src/psnames/rules.mk
    ${freetype_source_prefix}/src/sfnt/Jamfile
    ${freetype_source_prefix}/src/sfnt/module.mk
    ${freetype_source_prefix}/src/sfnt/rules.mk
    ${freetype_source_prefix}/src/sfnt/sfdriver.h
    ${freetype_source_prefix}/src/sfnt/sferrors.h
    ${freetype_source_prefix}/src/sfnt/sfnt.c
    ${freetype_source_prefix}/src/sfnt/sfobjs.h
    ${freetype_source_prefix}/src/sfnt/ttbdf.h
    ${freetype_source_prefix}/src/sfnt/ttcmap.h
    ${freetype_source_prefix}/src/sfnt/ttkern.h
    ${freetype_source_prefix}/src/sfnt/ttload.h
    ${freetype_source_prefix}/src/sfnt/ttmtx.h
    ${freetype_source_prefix}/src/sfnt/ttpost.h
    ${freetype_source_prefix}/src/sfnt/ttsbit.h
    ${freetype_source_prefix}/src/sfnt/ttsbit0.c
    ${freetype_source_prefix}/src/smooth/ftgrays.h
    ${freetype_source_prefix}/src/smooth/ftsmerrs.h
    ${freetype_source_prefix}/src/smooth/ftsmooth.h
    ${freetype_source_prefix}/src/smooth/Jamfile
    ${freetype_source_prefix}/src/smooth/module.mk
    ${freetype_source_prefix}/src/smooth/rules.mk
    ${freetype_source_prefix}/src/smooth/smooth.c
    ${freetype_source_prefix}/src/truetype/Jamfile
    ${freetype_source_prefix}/src/truetype/module.mk
    ${freetype_source_prefix}/src/truetype/rules.mk
    ${freetype_source_prefix}/src/truetype/truetype.c
    ${freetype_source_prefix}/src/truetype/ttdriver.h
    ${freetype_source_prefix}/src/truetype/tterrors.h
    ${freetype_source_prefix}/src/truetype/ttgload.h
    ${freetype_source_prefix}/src/truetype/ttgxvar.h
    ${freetype_source_prefix}/src/truetype/ttinterp.h
    ${freetype_source_prefix}/src/truetype/ttobjs.h
    ${freetype_source_prefix}/src/truetype/ttpload.h
    ${freetype_source_prefix}/src/type1/Jamfile
    ${freetype_source_prefix}/src/type1/module.mk
    ${freetype_source_prefix}/src/type1/rules.mk
    ${freetype_source_prefix}/src/type1/t1afm.h
    ${freetype_source_prefix}/src/type1/t1driver.h
    ${freetype_source_prefix}/src/type1/t1errors.h
    ${freetype_source_prefix}/src/type1/t1gload.h
    ${freetype_source_prefix}/src/type1/t1load.h
    ${freetype_source_prefix}/src/type1/t1objs.h
    ${freetype_source_prefix}/src/type1/t1parse.h
    ${freetype_source_prefix}/src/type1/t1tokens.h
    ${freetype_source_prefix}/src/type1/type1.c
)

target_include_directories(freetype
PUBLIC
    ${freetype_source_prefix}/include
)


##########
# SDL_mixer
#if (ONS_USE_OLD_DEPS)
#    set(sdl_mixer_source_prefix src/old/SDL_mixer-1.2.8)
#else()
#    set(sdl_mixer_source_prefix src/SDL_mixer-1.2.12)
#endif()

set(sdl_mixer_source_prefix src/SDL_mixer-1.2.12)

add_library(SDL_mixer STATIC)

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_mixer
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(SDL_mixer
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(SDL_mixer
PUBLIC
    freetype
    ${ONS_SDL_TARGET}
)

if(WIN32)
    target_compile_definitions(SDL_mixer
    PUBLIC
        USE_NATIVE_MIDI
        MID_MUSIC
    )

    target_sources(SDL_mixer
    PRIVATE
        ${sdl_mixer_source_prefix}/native_midi/native_midi_common.c
        ${sdl_mixer_source_prefix}/native_midi/native_midi_win32.c
    )
elseif(APPLE)
    target_compile_definitions(SDL_mixer
    PUBLIC
        USE_NATIVE_MIDI
        MID_MUSIC
    )

    target_sources(SDL_mixer
    PRIVATE
        ${sdl_mixer_source_prefix}/native_midi/native_midi_common.c
        ${sdl_mixer_source_prefix}/native_midi/native_midi_mac.c
    )
endif()

target_include_directories(SDL_mixer
PRIVATE
    ${sdl_mixer_source_prefix}/native_midi/
PUBLIC
    ${sdl_mixer_source_prefix}/
)

target_sources(SDL_mixer
PRIVATE
    ${sdl_mixer_source_prefix}/dynamic_mp3.c
    ${sdl_mixer_source_prefix}/dynamic_mp3.h
    ${sdl_mixer_source_prefix}/dynamic_ogg.c
    ${sdl_mixer_source_prefix}/dynamic_ogg.h
    ${sdl_mixer_source_prefix}/effects_internal.c
    ${sdl_mixer_source_prefix}/effects_internal.h
    ${sdl_mixer_source_prefix}/effect_position.c
    ${sdl_mixer_source_prefix}/effect_stereoreverse.c
    ${sdl_mixer_source_prefix}/load_aiff.c
    ${sdl_mixer_source_prefix}/load_aiff.h
    ${sdl_mixer_source_prefix}/load_ogg.c
    ${sdl_mixer_source_prefix}/load_ogg.h
    ${sdl_mixer_source_prefix}/load_voc.c
    ${sdl_mixer_source_prefix}/load_voc.h
    ${sdl_mixer_source_prefix}/Makefile.in
    ${sdl_mixer_source_prefix}/mixer.c
    ${sdl_mixer_source_prefix}/music.c
    ${sdl_mixer_source_prefix}/music_cmd.c
    ${sdl_mixer_source_prefix}/music_cmd.h
    ${sdl_mixer_source_prefix}/music_ogg.c
    ${sdl_mixer_source_prefix}/music_ogg.h
    ${sdl_mixer_source_prefix}/SDL_mixer.h
    #${sdl_mixer_source_prefix}/SDL_mixer.qpg.in
    #${sdl_mixer_source_prefix}/SDL_mixer.spec
    #${sdl_mixer_source_prefix}/SDL_mixer.spec.in
    ${sdl_mixer_source_prefix}/wavestream.c
    ${sdl_mixer_source_prefix}/wavestream.h
)


##########
# SDL_ttf-2.0.11
if (ONS_USE_OLD_DEPS)
    set(sdl_ttf_source_prefix src/old/SDL_ttf-2.0.8)
else()
    set(sdl_ttf_source_prefix src/SDL_ttf-2.0.11)
endif()

add_library(SDL_ttf STATIC)

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_ttf
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(SDL_ttf
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(SDL_ttf
PUBLIC
    freetype
    ${ONS_SDL_TARGET}
)

target_sources(SDL_ttf
PRIVATE
    ${sdl_ttf_source_prefix}/SDL_ttf.c
    ${sdl_ttf_source_prefix}/SDL_ttf.h
)

target_include_directories(SDL_ttf
PUBLIC
    ${sdl_ttf_source_prefix}/
)

##########
# zlib-1.2.3
set(zlib_source_prefix src/zlib-1.2.3)

add_library(zlib STATIC)

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(zlib
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(zlib
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_sources(zlib
PRIVATE
    ${zlib_source_prefix}/adler32.c
    ${zlib_source_prefix}/compress.c
    ${zlib_source_prefix}/crc32.c
    ${zlib_source_prefix}/crc32.h
    ${zlib_source_prefix}/deflate.c
    ${zlib_source_prefix}/deflate.h
    ${zlib_source_prefix}/gzio.c
    ${zlib_source_prefix}/infback.c
    ${zlib_source_prefix}/inffast.c
    ${zlib_source_prefix}/inffast.h
    ${zlib_source_prefix}/inffixed.h
    ${zlib_source_prefix}/inflate.c
    ${zlib_source_prefix}/inflate.h
    ${zlib_source_prefix}/inftrees.c
    ${zlib_source_prefix}/inftrees.h
    ${zlib_source_prefix}/trees.c
    ${zlib_source_prefix}/trees.h
    ${zlib_source_prefix}/uncompr.c
    ${zlib_source_prefix}/zconf.h
    ${zlib_source_prefix}/zconf.in.h
    ${zlib_source_prefix}/zlib.h
    ${zlib_source_prefix}/zutil.c
    ${zlib_source_prefix}/zutil.h
)

target_include_directories(zlib
PUBLIC
    ${zlib_source_prefix}/
)

##########
# libpng
if (ONS_USE_OLD_DEPS)
    set(libpng_source_prefix src/old/libpng-1.2.24)
else()
    set(libpng_source_prefix src/libpng-1.6.9)
endif()

add_library(libpng STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(libpng
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(libpng
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(libpng
PUBLIC
    zlib
)

target_sources(libpng
PUBLIC
    ${libpng_source_prefix}/png.c
    ${libpng_source_prefix}/png.h
    ${libpng_source_prefix}/pngconf.h
    ${libpng_source_prefix}/pngerror.c
    ${libpng_source_prefix}/pnggccrd.c
    ${libpng_source_prefix}/pngget.c
    ${libpng_source_prefix}/pngmem.c
    ${libpng_source_prefix}/pngpread.c
    ${libpng_source_prefix}/pngread.c
    ${libpng_source_prefix}/pngrio.c
    ${libpng_source_prefix}/pngrtran.c
    ${libpng_source_prefix}/pngrutil.c
    ${libpng_source_prefix}/pngset.c
    ${libpng_source_prefix}/pngtrans.c
    ${libpng_source_prefix}/pngvcrd.c
    ${libpng_source_prefix}/pngwio.c
    ${libpng_source_prefix}/pngwrite.c
    ${libpng_source_prefix}/pngwtran.c
    ${libpng_source_prefix}/pngwutil.c
)

target_include_directories(libpng
PUBLIC
    ${libpng_source_prefix}/
)


##########
# sdl_image
if (ONS_USE_OLD_DEPS)
    set(sdl_image_source_prefix src/old/SDL_image-1.2.6)
else()
    set(sdl_image_source_prefix src/SDL_image-1.2.12)
endif()

add_library(SDL_image STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_image
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(SDL_image
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_compile_definitions(SDL_image 
PUBLIC 
    LOAD_JPG
    LOAD_PNG
    LOAD_BMP
    LOAD_GIF
    LOAD_LBM
    LOAD_PCX
    LOAD_PNM
    LOAD_TGA
)

if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_image 
    PRIVATE
        -Wno-incompatible-pointer-types
    )
endif()

target_link_libraries(SDL_image
PUBLIC
    ${ONS_SDL_TARGET}
    #PkgConfigWrapper::libjpeg
    jpeg
    libpng
)

target_sources(SDL_image
PRIVATE
    ${sdl_image_source_prefix}/IMG.c
    ${sdl_image_source_prefix}/IMG_bmp.c
    ${sdl_image_source_prefix}/IMG_gif.c
    ${sdl_image_source_prefix}/IMG_jpg.c
    ${sdl_image_source_prefix}/IMG_lbm.c
    ${sdl_image_source_prefix}/IMG_pcx.c
    ${sdl_image_source_prefix}/IMG_png.c
    ${sdl_image_source_prefix}/IMG_pnm.c
    ${sdl_image_source_prefix}/IMG_tga.c
    ${sdl_image_source_prefix}/IMG_tif.c
    ${sdl_image_source_prefix}/IMG_xcf.c
    ${sdl_image_source_prefix}/IMG_xpm.c
    ${sdl_image_source_prefix}/IMG_xv.c
    ${sdl_image_source_prefix}/IMG_xxx.c
    ${sdl_image_source_prefix}/SDL_image.h
)

target_include_directories(SDL_image
PUBLIC
    ${sdl_image_source_prefix}/
)

##########
# smpeg
set(smpeg_source_prefix src/smpeg)

add_library(smpeg STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(smpeg
    PRIVATE
        $<$<CONFIG:DEBUG>:-g>
        -Wno-narrowing
    )
    
    target_link_options(smpeg
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_compile_definitions(smpeg
PRIVATE
    NOCONTROLS
)

if (NINTENDO_SWITCH)
    target_compile_definitions(smpeg
    PUBLIC
        ENDIAN_IN_SYS
    )
endif()

target_compile_features(smpeg
PUBLIC 
    cxx_std_98
)

target_sources(smpeg
PRIVATE
    ${smpeg_source_prefix}/MPEG.cpp
    ${smpeg_source_prefix}/MPEG.h
    ${smpeg_source_prefix}/MPEGaction.h
    ${smpeg_source_prefix}/MPEGaudio.h
    ${smpeg_source_prefix}/MPEGerror.h
    ${smpeg_source_prefix}/MPEGfilter.c
    ${smpeg_source_prefix}/MPEGfilter.h
    ${smpeg_source_prefix}/MPEGlist.cpp
    ${smpeg_source_prefix}/MPEGlist.h
    ${smpeg_source_prefix}/MPEGring.cpp
    ${smpeg_source_prefix}/MPEGring.h
    ${smpeg_source_prefix}/MPEGstream.cpp
    ${smpeg_source_prefix}/MPEGstream.h
    ${smpeg_source_prefix}/MPEGsystem.cpp
    ${smpeg_source_prefix}/MPEGsystem.h
    ${smpeg_source_prefix}/MPEGvideo.h
    ${smpeg_source_prefix}/smpeg.cpp
    ${smpeg_source_prefix}/smpeg.h
    ${smpeg_source_prefix}/audio/bitwindow.cpp
    ${smpeg_source_prefix}/audio/COPYING.LIB
    ${smpeg_source_prefix}/audio/filter.cpp
    ${smpeg_source_prefix}/audio/filter_2.cpp
    ${smpeg_source_prefix}/audio/hufftable.cpp
    ${smpeg_source_prefix}/audio/MPEGaudio.cpp
    ${smpeg_source_prefix}/audio/mpeglayer1.cpp
    ${smpeg_source_prefix}/audio/mpeglayer2.cpp
    ${smpeg_source_prefix}/audio/mpeglayer3.cpp
    ${smpeg_source_prefix}/audio/mpegtable.cpp
    ${smpeg_source_prefix}/audio/mpegtoraw.cpp
    ${smpeg_source_prefix}/video/decoders.cpp
    ${smpeg_source_prefix}/video/decoders.h
    ${smpeg_source_prefix}/video/dither.h
    ${smpeg_source_prefix}/video/floatdct.cpp
    ${smpeg_source_prefix}/video/gdith.cpp
    ${smpeg_source_prefix}/video/jrevdct.cpp
    #${smpeg_source_prefix}/video/mmxflags_asm.S
    #${smpeg_source_prefix}/video/mmxidct_asm.S
    ${smpeg_source_prefix}/video/motionvec.cpp
    ${smpeg_source_prefix}/video/MPEGvideo.cpp
    ${smpeg_source_prefix}/video/parseblock.cpp
    ${smpeg_source_prefix}/video/proto.h
    ${smpeg_source_prefix}/video/readfile.cpp
    ${smpeg_source_prefix}/video/util.cpp
    ${smpeg_source_prefix}/video/util.h
    ${smpeg_source_prefix}/video/vhar128.cpp
    ${smpeg_source_prefix}/video/vhar128.h
    ${smpeg_source_prefix}/video/video.cpp
    ${smpeg_source_prefix}/video/video.h
)

target_link_libraries(smpeg
PUBLIC
    ${ONS_SDL_TARGET}
)

#smpeg-config.in
target_include_directories(smpeg
PUBLIC
    ${smpeg_source_prefix}/
)
