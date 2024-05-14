# This is mostly for CI, might want to detect that scenario before setting this at all times.
option(ONSCRIPTER_EN_CI "Choose whether to accept arguments for vcpkg." OFF)
if (ONSCRIPTER_EN_CI)
    set(ONSCRIPTER_EN_VCPKG_PASS_ARGS OFF)
    if (ONSCRIPTER_EN_VCPKG_BUILDTREE)
        set(ONSCRIPTER_EN_VCPKG_PASS_ARGS ON)
        list(APPEND ONSCRIPTER_EN_VCPKG_ARGS "--x-buildtrees-root=${ONSCRIPTER_EN_VCPKG_BUILDTREE}")
    endif()
    
    if (ONSCRIPTER_EN_VCPKG_PACKAGES)
        set(ONSCRIPTER_EN_VCPKG_PASS_ARGS ON)
        list(APPEND ONSCRIPTER_EN_VCPKG_ARGS "--x-packages-root=${ONSCRIPTER_EN_VCPKG_PACKAGES}")
    endif()

    message(STATUS Args: ${ONSCRIPTER_EN_VCPKG_ARGS})

    if (ONSCRIPTER_EN_VCPKG_PASS_ARGS)
        list(JOIN ONSCRIPTER_EN_VCPKG_ARGS " " ONSCRIPTER_EN_VCPKG_ARGS_OUTPUT)
        set(VCPKG_INSTALL_OPTIONS ${ONSCRIPTER_EN_VCPKG_ARGS_OUTPUT})
    endif()
endif()

if(VCPKG_TARGET_TRIPLET STREQUAL arm64-android)
    set(ANDROID_ABI arm64-v8a)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
elseif(VCPKG_TARGET_TRIPLET STREQUAL arm-android)
    set(ANDROID_ABI armeabi-v7a)
    set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
elseif(VCPKG_TARGET_TRIPLET STREQUAL arm-neon-android)
    set(ANDROID_ABI armeabi-v7a)
    set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
elseif(VCPKG_TARGET_TRIPLET STREQUAL x64-android)
    set(ANDROID_ABI x86_64)
    set(CMAKE_ANDROID_ARCH_ABI x86_64)
elseif(VCPKG_TARGET_TRIPLET STREQUAL x86-android)
    set(ANDROID_ABI x86)
    set(CMAKE_ANDROID_ARCH_ABI x86)
elseif(VCPKG_TARGET_TRIPLET STREQUAL armv6-android)
    set(ANDROID_ABI armeabi)
    set(CMAKE_ANDROID_ARCH_ABI armeabi-v6)
else()
    if(CMAKE_GENERATOR STREQUAL "Visual Studio 14 2015 Win64")
        set(VCPKG_TARGET_TRIPLET x64-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 14 2015 ARM")
        set(VCPKG_TARGET_TRIPLET arm-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 14 2015")
        set(VCPKG_TARGET_TRIPLET x86-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 15 2017 Win64")
        set(VCPKG_TARGET_TRIPLET x64-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 15 2017 ARM")
        set(VCPKG_TARGET_TRIPLET arm-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 15 2017")
        set(VCPKG_TARGET_TRIPLET x86-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019" AND CMAKE_VS_PLATFORM_NAME_DEFAULT STREQUAL "ARM64")
        set(VCPKG_TARGET_TRIPLET arm64-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019")
        set(VCPKG_TARGET_TRIPLET x64-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022" AND CMAKE_VS_PLATFORM_NAME_DEFAULT STREQUAL "ARM64")
        set(VCPKG_TARGET_TRIPLET arm64-windows-static)
    elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
        set(VCPKG_TARGET_TRIPLET x64-windows-static)
    else()
        find_program(CL_FOUND cl)

        if(CL_FOUND MATCHES "amd64/cl.exe$" OR CL_FOUND MATCHES "x64/cl.exe$")
            set(VCPKG_TARGET_TRIPLET x64-windows-static)
            #set(VCPKG_CRT_LINKAGE dynamic)
        elseif(CL_FOUND MATCHES "arm/cl.exe$")
            set(VCPKG_TARGET_TRIPLET arm-windows-static)
            #set(VCPKG_CRT_LINKAGE dynamic)
        elseif(CL_FOUND MATCHES "arm64/cl.exe$")
            set(VCPKG_TARGET_TRIPLET arm64-windows-static)
            #set(VCPKG_CRT_LINKAGE dynamic)
        elseif(CL_FOUND MATCHES "bin/cl.exe$" OR CL_FOUND MATCHES "x86/cl.exe$")
            set(VCPKG_TARGET_TRIPLET x86-windows-static)
            #set(VCPKG_CRT_LINKAGE dynamic)
        endif()
    endif()
endif()
