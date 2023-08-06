find_package(JPEG REQUIRED)
find_path(DIRENT_INCLUDE_DIRS "dirent.h")

##########
# nscdec
add_executable(nscdec)

target_include_directories(nscdec PRIVATE ../)

target_compile_features(nscdec PUBLIC cxx_std_11)
target_sources(nscdec 
PRIVATE 
    nscdec.cpp
)

target_compile_definitions(nscdec PUBLIC -DTOOLS_BUILD -DSAR)

##########
# nscmake
add_executable(nscmake)

target_include_directories(nscmake PRIVATE ../)

target_compile_features(nscmake PUBLIC cxx_std_11)
target_sources(nscmake 
PRIVATE 
    nscmake.cpp
)

target_compile_definitions(nscmake PUBLIC -DTOOLS_BUILD -DSAR)



####################
# Archive Tools

###############
# SAR Tools

##########
# sarconv
add_executable(sarconv)

target_include_directories(sarconv PRIVATE ../)

target_compile_features(sarconv PUBLIC cxx_std_11)
target_sources(sarconv 
PRIVATE 
    arcconv.cpp
    conv_shared.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../resize_image.cpp
    ../resize_image.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(sarconv
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(sarconv PUBLIC -DTOOLS_BUILD -DSAR)

##########
# sardec
add_executable(sardec)

target_include_directories(sardec PRIVATE ../)

target_compile_features(sardec PUBLIC cxx_std_11)
target_sources(sardec 
PRIVATE 
    arcdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(sardec
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(sardec PUBLIC -DTOOLS_BUILD -DSAR)

##########
# sarmake
add_executable(sarmake)

target_include_directories(sarmake PRIVATE ../ ${DIRENT_INCLUDE_DIRS})

target_compile_features(sarmake PUBLIC cxx_std_11)
target_sources(sarmake 
PRIVATE 
    arcmake.cpp
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(sarmake
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(sarmake PUBLIC -DTOOLS_BUILD -DSAR)


###############
# NS2 Tools

##########
# ns2conv
add_executable(ns2conv)

target_include_directories(ns2conv PRIVATE ../)

target_compile_features(ns2conv PUBLIC cxx_std_11)
target_sources(ns2conv 
PRIVATE 
    arcconv.cpp
    conv_shared.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../resize_image.cpp
    ../resize_image.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(ns2conv
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(ns2conv PUBLIC -DTOOLS_BUILD -DNS2)

##########
# ns2dec
add_executable(ns2dec)

target_include_directories(ns2dec PRIVATE ../)

target_compile_features(ns2dec PUBLIC cxx_std_11)
target_sources(ns2dec 
PRIVATE 
    arcdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(ns2dec
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(ns2dec PUBLIC -DTOOLS_BUILD -DNS2)

##########
# ns2make
add_executable(ns2make)

target_include_directories(ns2make PRIVATE ../ ${DIRENT_INCLUDE_DIRS})

target_compile_features(ns2make PUBLIC cxx_std_11)
target_sources(ns2make 
PRIVATE 
    arcmake.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(ns2make
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(ns2make PUBLIC -DTOOLS_BUILD -DNS2)

###############
# NSA Tools

##########
# nsaconv
add_executable(nsaconv)

target_include_directories(nsaconv PRIVATE ../)

target_compile_features(nsaconv PUBLIC cxx_std_11)
target_sources(nsaconv 
PRIVATE 
    arcconv.cpp
    conv_shared.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../resize_image.cpp
    ../resize_image.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(nsaconv
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(nsaconv PUBLIC -DTOOLS_BUILD -DNSA)

##########
# nsadec
add_executable(nsadec)

target_include_directories(nsadec PRIVATE ../)

target_compile_features(nsadec PUBLIC cxx_std_11)
target_sources(nsadec 
PRIVATE 
    arcdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(nsadec
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(nsadec PUBLIC -DTOOLS_BUILD -DNSA)

##########
# nsamake
add_executable(nsamake)

target_include_directories(nsamake PRIVATE ../ ${DIRENT_INCLUDE_DIRS})

target_compile_features(nsamake PUBLIC cxx_std_11)
target_sources(nsamake 
PRIVATE 
    arcmake.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(nsamake
PRIVATE
    BZip2::BZip2
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

target_compile_definitions(nsamake PUBLIC -DTOOLS_BUILD -DNSA)