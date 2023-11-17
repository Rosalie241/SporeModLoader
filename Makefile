#
# SporeModLoader & SporeModManager Makefile
#

# relative to project directories
BINARY_DIR := ../bin

CC         := gcc
CXX        := g++
MINGW_CC   := i686-w64-mingw32-gcc
MINGW_CXX  := i686-w64-mingw32-g++

export CC CXX MINGW_CC MINGW_CXX

all: SporeModLoader SporeModManager.exe SporeModManager

clean:
	$(MAKE) -C SporeModLoader clean BINARY_DIR=$(BINARY_DIR)/SporebinEP1
	$(MAKE) -C SporeModManager clean BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager
	$(MAKE) -C SporeModManager clean MINGW=1 BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

SporeModLoader:
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporebinEP1

SporeModManager:
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

SporeModManager.exe:
	$(MAKE) -C $(basename $@) BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager MINGW=1

.PHONY: SporeModLoader SporeModManager SporeModManager.exe all clean
