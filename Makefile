#
# SporeModLoader & SporeModManager Makefile
#
SOURCE_DIR     := $(CURDIR)
THIRDPARTY_DIR := $(SOURCE_DIR)/3rdParty

CC         := gcc
CXX        := g++
MINGW_CC   := i686-w64-mingw32-gcc
MINGW_CXX  := i686-w64-mingw32-g++

export THIRDPARTY_DIR CC CXX MINGW_CC MINGW_CXX

all: SporeModLoader SporeModManager SporeModManager.exe

clean:
	$(MAKE) -C SporeModLoader clean BINARY_DIR=$(SOURCE_DIR)/bin/SporebinEP1
	$(MAKE) -C SporeModManager clean BINARY_DIR=$(SOURCE_DIR)/bin/SporeModLoader/SporeModManager
	$(MAKE) -C SporeModManager clean MINGW=1 BINARY_DIR=$(SOURCE_DIR)/bin/SporeModLoader/SporeModManager

SporeModLoader:
	$(MAKE) -C $@ SOURCE_DIR=$(SOURCE_DIR)/$@ BINARY_DIR=$(SOURCE_DIR)/bin/SporebinEP1

SporeModManager:
	$(MAKE) -C $@ SOURCE_DIR=$(SOURCE_DIR)/$@ BINARY_DIR=$(SOURCE_DIR)/bin/SporeModLoader/SporeModManager

SporeModManager.exe:
	$(MAKE) -C $(basename $@) SOURCE_DIR=$(SOURCE_DIR)/$(basename $@) BINARY_DIR=$(SOURCE_DIR)/bin/SporeModLoader/SporeModManager MINGW=1

.PHONY: SporeModLoader SporeModManager SporeModManager.exe all clean
