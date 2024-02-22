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

help:
	@echo "SporeModLoader Makefile."
	@echo "  Targets:"
	@echo "    all                 == Build SporeModLoader, SporeModManager and SporeModManager.exe"
	@echo "    clean               == remove object files"
	@echo "    SporeModLoader      == Build SporeModLoader"
	@echo "    SporeModManager     == Build SporeModManager"
	@echo "    SporeModManager.exe == Build SporeModManager.exe"
	@echo "  Build Options:"
	@echo "    OPTFLAGS=flag       == compiler optimization (default: -Os -flto)"
	@echo "    WARNFLAGS=flag      == compiler warning levels (default: -Wall -Wextra -Wpedantic)"
	@echo "  Debugging Options:"
	@echo "    DEBUG=1             == add debugging symbols to binaries"
	@echo "    VERBOSE=1           == show verbose compiler output"

all: SporeModLoader SporeModManager.exe SporeModManager

clean:
	$(MAKE) -C SporeModLoader clean BINARY_DIR=$(BINARY_DIR)/SporebinEP1
	$(MAKE) -C SporeModManager clean BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager
	$(MAKE) -C SporeModManager clean MINGW=1 BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

check:
	$(MAKE) -C SporeModManager check BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

SporeModLoader:
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporebinEP1

SporeModManager: $(THIRDPARTY_DIR)/zlib/zconf.h
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

SporeModManager.exe: $(THIRDPARTY_DIR)/zlib/zconf.h
	$(MAKE) -C $(basename $@) BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager MINGW=1

.PHONY: SporeModLoader SporeModManager SporeModManager.exe all clean check help
