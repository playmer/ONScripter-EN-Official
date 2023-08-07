## Windows Build

```batch
git clone https://github.com/microsoft/vcpkg.git
git clone https://github.com/playmer/onscriper-en.git
cd onscripter-en
cmake -B build 
```

## Linux Build

```bash
$ sudo apt install build-essential cmake ninja-build libbz2-dev libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libfreetype-dev libmad0-dev libogg-dev libpng-dev libvorbis-dev lua5.4 libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsmpeg-dev zlib1g-dev


sudo apt install nasm libx11-dev libxft-dev libxext-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev libibus-1.0-dev

git clone https://github.com/microsoft/vcpkg.git
git clone https://github.com/playmer/onscriper-en-official.git
cd onscripter-en-official
cmake -B build 
ninja


```


## MacOS Build

Requirements: XCode (with Command Line tools), CMake

```bash
brew install nasm
git clone https://github.com/microsoft/vcpkg.git
git clone https://github.com/playmer/onscriper-en-official.git
cd onscripter-en-official
cmake -B build 
ninja

```