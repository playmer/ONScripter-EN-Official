##########
# bzip2
if (ONS_USE_OLD_DEPS)
    set(bzip2_source_prefix src/old/bzip2-1.0.4)
else()
    set(bzip2_source_prefix src/bzip2-1.0.8)
endif()

add_library(bzip2 STATIC "")

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
# libjpeg
if (ONS_USE_OLD_DEPS)
    set(jpeg_source_prefix src/old/jpeg-6b)
else()
    set(jpeg_source_prefix src/jpeg-6b)
endif()

add_library(jpeg STATIC "")

if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(jpeg 
    PRIVATE
        -Wno-implicit-function-declaration
        -Wno-builtin-declaration-mismatch
    )
endif()

target_sources(jpeg
PRIVATE
    #${jpeg_source_prefix}/ansi2knr.1
    #${jpeg_source_prefix}/ansi2knr.c
    #${jpeg_source_prefix}/cderror.h
    #${jpeg_source_prefix}/cdjpeg.c
    #${jpeg_source_prefix}/cdjpeg.h
    #${jpeg_source_prefix}/change.log
    #${jpeg_source_prefix}/cjpeg.1
    #${jpeg_source_prefix}/cjpeg.c
    #${jpeg_source_prefix}/ckconfig.c
    #${jpeg_source_prefix}/config.guess
    #${jpeg_source_prefix}/config.sub
    #${jpeg_source_prefix}/configure
    #${jpeg_source_prefix}/djpeg.1
    #${jpeg_source_prefix}/djpeg.c
    #${jpeg_source_prefix}/example.c
    #${jpeg_source_prefix}/install-sh
    ${jpeg_source_prefix}/jcapimin.c
    ${jpeg_source_prefix}/jcapistd.c
    ${jpeg_source_prefix}/jccoefct.c
    ${jpeg_source_prefix}/jccolor.c
    ${jpeg_source_prefix}/jcdctmgr.c
    ${jpeg_source_prefix}/jchuff.c
    ${jpeg_source_prefix}/jchuff.h
    ${jpeg_source_prefix}/jcinit.c
    ${jpeg_source_prefix}/jcmainct.c
    ${jpeg_source_prefix}/jcmarker.c
    ${jpeg_source_prefix}/jcmaster.c
    ${jpeg_source_prefix}/jcomapi.c
    #${jpeg_source_prefix}/jconfig.bcc
    #${jpeg_source_prefix}/jconfig.cfg
    #${jpeg_source_prefix}/jconfig.dj
    #${jpeg_source_prefix}/jconfig.mac
    #${jpeg_source_prefix}/jconfig.manx
    #${jpeg_source_prefix}/jconfig.mc6
    #${jpeg_source_prefix}/jconfig.sas
    #${jpeg_source_prefix}/jconfig.st
    #${jpeg_source_prefix}/jconfig.vc
    #${jpeg_source_prefix}/jconfig.vms
    #${jpeg_source_prefix}/jconfig.wat
    ${jpeg_source_prefix}/jcparam.c
    ${jpeg_source_prefix}/jcphuff.c
    ${jpeg_source_prefix}/jcprepct.c
    ${jpeg_source_prefix}/jcsample.c
    ${jpeg_source_prefix}/jctrans.c
    ${jpeg_source_prefix}/jdapimin.c
    ${jpeg_source_prefix}/jdapistd.c
    ${jpeg_source_prefix}/jdatadst.c
    ${jpeg_source_prefix}/jdatasrc.c
    ${jpeg_source_prefix}/jdcoefct.c
    ${jpeg_source_prefix}/jdcolor.c
    ${jpeg_source_prefix}/jdct.h
    ${jpeg_source_prefix}/jddctmgr.c
    ${jpeg_source_prefix}/jdhuff.c
    ${jpeg_source_prefix}/jdhuff.h
    ${jpeg_source_prefix}/jdinput.c
    ${jpeg_source_prefix}/jdmainct.c
    ${jpeg_source_prefix}/jdmarker.c
    ${jpeg_source_prefix}/jdmaster.c
    ${jpeg_source_prefix}/jdmerge.c
    ${jpeg_source_prefix}/jdphuff.c
    ${jpeg_source_prefix}/jdpostct.c
    ${jpeg_source_prefix}/jdsample.c
    ${jpeg_source_prefix}/jdtrans.c
    ${jpeg_source_prefix}/jerror.c
    ${jpeg_source_prefix}/jerror.h
    ${jpeg_source_prefix}/jfdctflt.c
    ${jpeg_source_prefix}/jfdctfst.c
    ${jpeg_source_prefix}/jfdctint.c
    ${jpeg_source_prefix}/jidctflt.c
    ${jpeg_source_prefix}/jidctfst.c
    ${jpeg_source_prefix}/jidctint.c
    ${jpeg_source_prefix}/jidctred.c
    ${jpeg_source_prefix}/jinclude.h
    ${jpeg_source_prefix}/jmemansi.c
    #${jpeg_source_prefix}/jmemdos.c
    #${jpeg_source_prefix}/jmemdosa.asm
    #${jpeg_source_prefix}/jmemmac.c
    ${jpeg_source_prefix}/jmemmgr.c
    ${jpeg_source_prefix}/jmemname.c
    ${jpeg_source_prefix}/jmemnobs.c
    ${jpeg_source_prefix}/jmemsys.h
    ${jpeg_source_prefix}/jmorecfg.h
    ${jpeg_source_prefix}/jpegint.h
    ${jpeg_source_prefix}/jpeglib.h
    ${jpeg_source_prefix}/jpegtran.1
    ${jpeg_source_prefix}/jpegtran.c
    ${jpeg_source_prefix}/jquant1.c
    ${jpeg_source_prefix}/jquant2.c
    ${jpeg_source_prefix}/jutils.c
    ${jpeg_source_prefix}/jversion.h
    #${jpeg_source_prefix}/ltconfig
    #${jpeg_source_prefix}/ltmain.sh
    #${jpeg_source_prefix}/makcjpeg.st
    #${jpeg_source_prefix}/makdjpeg.st
    #${jpeg_source_prefix}/makeapps.ds
    #${jpeg_source_prefix}/makelib.ds
    #${jpeg_source_prefix}/makeproj.mac
    #${jpeg_source_prefix}/makljpeg.st
    #${jpeg_source_prefix}/maktjpeg.st
    #${jpeg_source_prefix}/makvms.opt
    #${jpeg_source_prefix}/rdcolmap.c
    #${jpeg_source_prefix}/rdbmp.c
    #${jpeg_source_prefix}/rdgif.c
    #${jpeg_source_prefix}/rdjpgcom.1
    #${jpeg_source_prefix}/rdjpgcom.c
    #${jpeg_source_prefix}/rdppm.c
    #${jpeg_source_prefix}/rdrle.c
    #${jpeg_source_prefix}/rdswitch.c
    #${jpeg_source_prefix}/rdtarga.c
    #${jpeg_source_prefix}/README
    #${jpeg_source_prefix}/transupp.c
    #${jpeg_source_prefix}/transupp.h
    #${jpeg_source_prefix}/wrbmp.c
    #${jpeg_source_prefix}/wrgif.c
    #${jpeg_source_prefix}/wrjpgcom.1
    #${jpeg_source_prefix}/wrjpgcom.c
    #${jpeg_source_prefix}/wrppm.c
    #${jpeg_source_prefix}/wrrle.c
    #${jpeg_source_prefix}/wrtarga.c

)

target_include_directories(jpeg
PUBLIC
    ${jpeg_source_prefix}/
)

##########
# libpng
if (ONS_USE_OLD_DEPS)
    set(libpng_source_prefix src/old/libpng-1.2.24)
else()
    set(libpng_source_prefix src/libpng-1.6.9)
endif()

add_library(libpng STATIC "")

target_link_libraries(libpng
PUBLIC
    PkgConfigWrapper::zlib
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
    set(sdl_image_source_prefix src/old/sdl_image-1.2.6)
else()
    set(sdl_image_source_prefix src/sdl_image-1.2.12)
endif()

add_library(SDL_image STATIC "")

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
    PkgConfigWrapper::SDL
    jpeg
    libpng
)

target_sources(SDL_image
PRIVATE
    ${sdl_image_source_prefix}/
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
# freetype
if (ONS_USE_OLD_DEPS)
    set(freetype_source_prefix src/old/freetype-2.3.5)
else()
    set(freetype_source_prefix src/freetype-2.13.0)
endif()

add_library(freetype STATIC "")


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
# zlib-1.2.3
if (ONS_USE_OLD_DEPS)
    set(zlib_source_prefix src/zlib-1.2.13)
else()
    set(zlib_source_prefix src/zlib-1.2.3)
endif()

add_library(zlib STATIC)

target_sources(zlib
PRIVATE
    src/zlib-1.2.3/adler32.c
    src/zlib-1.2.3/compress.c
    src/zlib-1.2.3/crc32.c
    src/zlib-1.2.3/crc32.h
    src/zlib-1.2.3/deflate.c
    src/zlib-1.2.3/deflate.h
    src/zlib-1.2.3/gzio.c
    src/zlib-1.2.3/infback.c
    src/zlib-1.2.3/inffast.c
    src/zlib-1.2.3/inffast.h
    src/zlib-1.2.3/inffixed.h
    src/zlib-1.2.3/inflate.c
    src/zlib-1.2.3/inflate.h
    src/zlib-1.2.3/inftrees.c
    src/zlib-1.2.3/inftrees.h
    src/zlib-1.2.3/trees.c
    src/zlib-1.2.3/trees.h
    src/zlib-1.2.3/uncompr.c
    src/zlib-1.2.3/zconf.h
    src/zlib-1.2.3/zconf.in.h
    src/zlib-1.2.3/zlib.h
    src/zlib-1.2.3/zutil.c
    src/zlib-1.2.3/zutil.h
)

target_include_directories(zlib
PUBLIC
    src/zlib-1.2.3/
)