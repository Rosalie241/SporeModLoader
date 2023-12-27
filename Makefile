#
# SporeModLoader & SporeModManager Makefile
#

THIRDPARTY_DIR := 3rdParty
# relative to project directories
BINARY_DIR     := ../bin

CC            := gcc
CXX           := g++
MINGW_CC      := i686-w64-mingw32-gcc
MINGW_CXX     := i686-w64-mingw32-g++
MINGW_WINDRES := i686-w64-mingw32-windres

export CC CXX MINGW_CC MINGW_CXX MINGW_WINDRES

all: SporeModLoader SporeModManager.exe SporeModManager

clean:
	$(MAKE) -C SporeModLoader clean BINARY_DIR=$(BINARY_DIR)/SporebinEP1
	$(MAKE) -C SporeModManager clean BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager
	$(MAKE) -C SporeModManager clean MINGW=1 BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

# this is needed due to compiling SporeModLoader twice,
# which, when compiling with multiple threads, will cause
# zconf.h to be generated twice, causing a build failure
# due to a race condition (cp will fail due to the file
# already existing, even with -f)
$(THIRDPARTY_DIR)/zlib/zconf.h: $(THIRDPARTY_DIR)/zlib/zconf.h.in
	$(MAKE) -C SporeModManager ../$@

SporeModLoader:
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporebinEP1

SporeModManager: $(THIRDPARTY_DIR)/zlib/zconf.h
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

SporeModManager.exe: $(THIRDPARTY_DIR)/zlib/zconf.h
	$(MAKE) -C $(basename $@) BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager MINGW=1

.PHONY: SporeModLoader SporeModManager SporeModManager.exe all clean
