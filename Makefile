#
# SporeModLoader & SporeModManager Makefile
#

THIRDPARTY_DIR := 3rdParty
# relative to project directories
BINARY_DIR     := ../bin

CC            := gcc
CXX           := g++

export CC CXX

help:
	@echo "SporeModLoader Makefile."
	@echo "  Targets:"
	@echo "    all             == build SporeModManager"
	@echo "    clean           == remove object files"
	@echo "    SporeModManager == build SporeModManager"
	@echo "  Build Options:"
	@echo "    OPTFLAGS=flag   == compiler optimization (default: -Os -flto)"
	@echo "    WARNFLAGS=flag  == compiler warning levels (default: -Wall -Wextra -Wpedantic)"
	@echo "    WERROR=1        == compiler treats warnings as errors"
	@echo "  Debugging Options:"
	@echo "    DEBUG=1         == add debugging symbols to binaries"
	@echo "    ASAN=1          == add address sanitizer to binaries"
	@echo "    VERBOSE=1       == show verbose compiler output"

all: SporeModManager

clean:
	$(MAKE) -C SporeModManager clean BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

check:
	$(MAKE) -C SporeModManager check BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

SporeModManager: $(THIRDPARTY_DIR)/zlib/zconf.h
	$(MAKE) -C $@ BINARY_DIR=$(BINARY_DIR)/SporeModLoader/SporeModManager

.PHONY: SporeModManager all clean check help
