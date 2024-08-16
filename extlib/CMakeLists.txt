
if (ONS_USE_OLD_DEPS)
    
endif()


##########
# bzip2
add_library(bzip2 STATIC)

target_sources(bzip2
PRIVATE
    src/bzip2-1.0.8/blocksort.c
    src/bzip2-1.0.8/bzlib_private.h
    src/bzip2-1.0.8/bzlib.c
    src/bzip2-1.0.8/bzlib.h
    src/bzip2-1.0.8/compress.c
    src/bzip2-1.0.8/crctable.c
    src/bzip2-1.0.8/decompress.c
    src/bzip2-1.0.8/huffman.c
    src/bzip2-1.0.8/randtable.c
)

target_include_directories(bzip2
PUBLIC
    src/bzip2-1.0.8/
)

##########
# zlib-1.2.3
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