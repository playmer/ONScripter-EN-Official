## Windows Build

```batch
git clone --recurse-submodules https://github.com/microsoft/vcpkg.git
git clone https://github.com/playmer/onscriper-en.git
cd onscripter-en
cmake -B build 
```



## Windows (MSVC) Build

Requirements: Visual Studio 2022 Community (or similar, the Build Tools will likely also work), CMake, Ninja, Qt (Some env variables needed, need to fill this out)

```bash
git clone --recurse-submodules https://github.com/playmer/ONScripter-EN-Official.git
cd ONScripter-EN-Official
cmake --preset=ninja .
cmake --build --preset ninja-release
cpack --preset=ninja-package-release
```

## Linux Build

Requirements: Working GCC toolchain, CMake, Ninja, Qt (Some env variables needed, need to fill this out)

```bash
$ sudo apt install apt autoconf automake build-essential build-essential cmake cmake fcitx-libs-dev git gnome-desktop-testing libasound2-dev libaudio-dev libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libbz2-dev libdbus-1-dev libdecor-0-dev libdrm-dev libegl1-mesa-dev libfreetype-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev libibus-1.0-dev libjack-dev libmad0-dev libogg-dev libpipewire-0.3-dev libpng-dev libpulse-dev libsamplerate0-dev libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsmpeg-dev libsndio-dev libtool libudev-dev libvorbis-dev libwayland-dev libx11-dev libxcursor-dev libxext-dev libxfixes-dev libxi-dev libxkbcommon-dev libxrandr-dev libxss-dev lua5.4 make nasm ninja-build pkg-config zlib1g-dev

git clone --recurse-submodules https://github.com/playmer/ONScripter-EN-Official.git
cd ONScripter-EN-Official
cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_MAKE_PROGRAM=ninja --preset=ninja .
cmake --build --preset ninja-release
cpack --preset=ninja-package-release
```

## MacOS Build

Requirements: XCode (with Command Line tools), CMake, Qt (Some env variables needed, need to fill this out)

```bash
brew install nasm
git clone --recurse-submodules https://github.com/playmer/ONScripter-EN-Official.git
cd ONScripter-EN-Official
cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_MAKE_PROGRAM=ninja --preset=ninja .
cmake --build --preset ninja-release
cpack --preset=ninja-package-release
```

## Android Build

Requirements: Working NDK toolchain (Some env variables needed, need to fill this out), CMake, Ninja, Qt (Some env variables needed, need to fill this out) 

```bash
git clone --recurse-submodules https://github.com/playmer/ONScripter-EN-Official.git
cd ONScripter-EN-Official/android-project
./BuildApk.ps1
```
