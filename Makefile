# -*- makefile-gmake -*-
#
# THIS IS A GENERATED FILE - changes will not be kept if configure is
# run again.  If you wish to customise it, please be sure to give your
# version a different filename.
#
# Makefile for ONScripter-EN

TOPSRC=/mnt/c/msys64/home/playmer/repos/ONScripter-EN-Official

WIN32=
OBJSUFFIX=.o
LIBSUFFIX=.a
EXESUFFIX=

PREFIX ?= /usr/local

# Extra handling for internal libraries.
EXTRADEPS= $(internal_png) $(internal_jpeg) $(internal_sdl_image) $(internal_ogglibs) $(internal_sdl_mixer) $(internal_bzip2) $(internal_freetype) $(internal_sdl_ttf)
INTERNAL_SDL=$(findstring true,false)
INTERNAL_SMPEG=$(findstring true,false)
INTERNAL_LIBPNG=$(findstring true,true)
INTERNAL_LIBJPEG=$(findstring true,true)
INTERNAL_OGGLIBS=$(findstring true,true)
SDL_CONFIG=sdl-config

export PATH     :=   $(shell pwd)/extlib/bin:$(PATH)
export CFLAGS   := -I$(shell pwd)/extlib/include $(CFLAGS) 
export CPPFLAGS := -I$(shell pwd)/extlib/include $(CPPFLAGS) 
export LDFLAGS  := -L$(shell pwd)/extlib/lib $(LDFLAGS) 
export CSTD     := -std=c99
export CXXSTD   := -std=c++98

export CC      := gcc
export CXX     := g++
export MAKE    := make
export GNUMAKE := make
export AR      := ar
export RANLIB  := ranlib

# ONScripter variables
DEPFLAGS = -MMD -MF $(@:.o=.d) -MT $@
OSCFLAGSEXTRA = -Wall  -DHAVE_LOCALTIME_R $(OSCTMPFLAGS)
SANFLAGS = 
INCS =  -Iextlib/include $(shell $(SDL_CONFIG) --cflags)      \
                        $(shell smpeg-config --cflags)    \
                        $(shell ./extlib/bin/freetype-config --cflags) \
       $(shell [ -d extlib/include/SDL ] && echo -Iextlib/include/SDL)

TOOL_LIBS = -Lextlib/lib \
            extlib/lib/libjpeg$(LIBSUFFIX) extlib/lib/libpng$(LIBSUFFIX) extlib/lib/libz$(LIBSUFFIX) \
            extlib/lib/libbz2$(LIBSUFFIX)
LIBS = -lX11 -Lextlib/lib \
       extlib/lib/libSDL_image$(LIBSUFFIX) $(if $(findstring true,true),extlib/lib/libjpeg$(LIBSUFFIX) extlib/lib/libpng$(LIBSUFFIX) extlib/lib/libz$(LIBSUFFIX)) \
       extlib/lib/libSDL_mixer$(LIBSUFFIX) $(if $(or $(findstring true,true),$(findstring true,false)),extlib/lib/libvorbisfile$(LIBSUFFIX) extlib/lib/libvorbis$(LIBSUFFIX) extlib/lib/libogg$(LIBSUFFIX)) \
       $(shell $(SDL_CONFIG) --libs)      \
       $(shell smpeg-config --libs)    \
       extlib/lib/libSDL_ttf$(LIBSUFFIX) $(shell ./extlib/bin/freetype-config --libs) \
       extlib/lib/libbz2$(LIBSUFFIX) $(if $(findstring true,false),-ldl -lasound -lX11 -lXext -lXrandr -pthread -lpulse-simple -lpulse)

DEFS = -DLINUX -DUSE_OGG_VORBIS -D_FILE_OFFSET_BITS=64 -D_TIME_BITS=64 
EXT_OBJS = 

SDL_MIXER_FLAGS = --enable-music-native-midi-gpl

.SUFFIXES:
.SUFFIXES: .o .cpp .h .c

ifdef DEBUG
OSCFLAGS = -O0 -g -pg -ggdb -pipe -Wpointer-arith   $(OSCFLAGSEXTRA) $(SANFLAGS)
export LDFLAGS  := -pg $(LDFLAGS)
else
  ifdef PROF
  OSCFLAGS = -O2 -pg -pipe -Wpointer-arith   $(OSCFLAGSEXTRA)
  export LDFLAGS  := -pg $(LDFLAGS)
  else
  OSCFLAGS = -O2 -fomit-frame-pointer -pipe -Wpointer-arith   $(OSCFLAGSEXTRA) $(SANFLAGS)
  endif
endif

TARGET ?= onscripter-en

binary: $(TARGET)

.PHONY: all clean distclean tools binary
all: $(TARGET) tools

SDLOTHERCONFIG := 
OTHERCONFIG := 

OTHER_OBJS =
RC_HDRS =

ICONFILE ?= ons-en.png
RESOURCES ?= $(ICONFILE) =icon.png
OTHER_OBJS = resources$(OBJSUFFIX) X11_messagebox$(OBJSUFFIX)
RC_HDRS = resources.h
RCCLEAN = resources.cpp embed $(OTHER_OBJS)

resources$(OBJSUFFIX): $(RC_HDRS)

resources.cpp: embed $(filter-out =%,$(RESOURCES))
	./embed $(patsubst =%,%,$(RESOURCES)) > $@

embed: embed.cpp
	$(CXX) $(CXXSTD) $< -o $@

include Makefile.extlibs
include Makefile.onscripter
include Makefile.unittest

.PHONY: libtoolreplace
libtoolreplace: | $(EB)
	@cp $(ES)/required/freetype-config $(EB)

clean: pclean $(CLEAN_TARGETS) libtoolreplace
distclean: clean pdistclean $(DISTCLEAN_TARGETS)
	rm -r -f extlib/bin extlib/lib extlib/include \
	         extlib/share extlib/man
	rm -f Makefile 

.PHONY: install* uninstall*
install-bin:
	./install-sh -c -s $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
install: install-bin install-tools
install-tools: install-tools-yes
install-tools-yes: $(TOOLS)
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	$(foreach file, $(TOOLS), (./install-sh -c -s $(file) $(DESTDIR)$(PREFIX)/bin/ || true);)
uninstall:
	$(RM) $(addprefix $(DESTDIR)$(PREFIX)/bin/, $(TARGET) $(TOOLBINS))

graphics_sse2.o: graphics_sse2.cpp graphics_sse2.h graphics_common.h graphics_sum.h graphics_blend.h
	$(CXX) $(CXXSTD) $(OSCFLAGS) $(INCS) $(DEPFLAGS) $(DEFS) -msse2 -c $< -o $@

graphics_mmx.o: graphics_mmx.cpp graphics_mmx.h graphics_common.h graphics_sum.h
	$(CXX) $(CXXSTD) $(OSCFLAGS) $(INCS) $(DEPFLAGS) $(DEFS) -mmmx -c $< -o $@

.PHONY: dist
dist:
	git archive --prefix=onscripter-en-20240721/ HEAD
	tar cf onscripter-en-20240721-fullsrc.tar onscripter-en-20240721
	rm -rf onscripter-en-20240721/extlib onscripter-en-20240721/win_dll \
			onscripter-en-20240721/tools/libgnurx
	tar cf onscripter-en-20240721-src.tar onscripter-en-20240721
	bzip2 -9 onscripter-en-20240721-*src.tar
	rm -rf onscripter-en-20240721
