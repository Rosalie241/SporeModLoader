#
# SporeModManager Makefile
#
THIRDPARTY_DIR := ../3rdParty
BINARY_DIR     := bin
SOURCE_DIR     := .
EXE_FILE       := SporeModManager
VERBOSE        := 0
MINGW          := 0

CC            ?= gcc
CXX           ?= g++
MINGW_CC      ?= i686-w64-mingw32-gcc
MINGW_CXX     ?= i686-w64-mingw32-g++
MINGW_WINDRES ?= i686-w64-mingw32-windres

ifeq ($(MINGW), 0)
LDFLAGS    := -flto -s
OPTFLAGS   := -Os -flto -Wall -Wextra -Wpedantic
OBJ        := o
EXE_EXT    :=
else
# mingw doesn't like LTO
# and we need to statically link
LDFLAGS    := -s -municode -static
OPTFLAGS   := -Os -Wall -Wextra -Wpedantic
OBJ        := obj
EXE_EXT    := .exe
endif
CFLAGS     := \
				-I$(THIRDPARTY_DIR)/zlib     \
				-I$(THIRDPARTY_DIR)/zlib/contrib/minizip
CXXFLAGS   := -std=c++17                     \
				-I$(SOURCE_DIR)              \
				-I$(THIRDPARTY_DIR)/tinyxml2 \
				-I$(THIRDPARTY_DIR)/zlib     \
				-I$(THIRDPARTY_DIR)/zlib/contrib/minizip

OBJECT_FILES := \
	$(SOURCE_DIR)/SporeModManagerHelpers/FileVersion.$(OBJ) \
	$(SOURCE_DIR)/SporeModManagerHelpers/Path.$(OBJ)        \
	$(SOURCE_DIR)/SporeModManagerHelpers/SporeMod.$(OBJ)    \
	$(SOURCE_DIR)/SporeModManagerHelpers/SporeModXml.$(OBJ) \
	$(SOURCE_DIR)/SporeModManagerHelpers/String.$(OBJ)      \
	$(SOURCE_DIR)/SporeModManagerHelpers/UI.$(OBJ)          \
	$(SOURCE_DIR)/SporeModManagerHelpers/Zip.$(OBJ)         \
	$(SOURCE_DIR)/SporeModManager.$(OBJ)                    \
	$(SOURCE_DIR)/main.$(OBJ)

ifneq ($(MINGW), 0)
	OBJECT_FILES += \
		$(SOURCE_DIR)/version.$(OBJ)
endif

HEADER_FILES := \
	$(SOURCE_DIR)/SporeModManager.hpp \
	$(SOURCE_DIR)/SporeModManagerHelpers.hpp \
	$(SOURCE_DIR)/../version.h


THIRDPARTY_OBJECT_FILES := \
	$(THIRDPARTY_DIR)/tinyxml2/tinyxml2.$(OBJ) \
	$(THIRDPARTY_DIR)/zlib/adler32.$(OBJ)  \
	$(THIRDPARTY_DIR)/zlib/crc32.$(OBJ)    \
	$(THIRDPARTY_DIR)/zlib/inflate.$(OBJ)  \
	$(THIRDPARTY_DIR)/zlib/inffast.$(OBJ)  \
	$(THIRDPARTY_DIR)/zlib/inftrees.$(OBJ) \
	$(THIRDPARTY_DIR)/zlib/zutil.$(OBJ)    \
	$(THIRDPARTY_DIR)/zlib/contrib/minizip/unzip.$(OBJ) \
	$(THIRDPARTY_DIR)/zlib/contrib/minizip/ioapi.$(OBJ)

THIRDPARTY_HEADER_FILES := \
	$(THIRDPARTY_DIR)/zlib/zconf.h

ifeq ($(VERBOSE), 0)
	QUIET     := @
	TEST_ARGS :=
else
	QUIET     :=
	TEST_ARGS := --verbose
endif

%.o: %.c $(THIRDPARTY_HEADER_FILES)
	@echo "CC  $<"
	$(QUIET)$(CC) -c $< -o $@ $(OPTFLAGS) $(CFLAGS)

%.o: %.cpp $(HEADER_FILES) $(THIRDPARTY_HEADER_FILES)
	@echo "CXX $<"
	$(QUIET)$(CXX) -c $< -o $@ $(OPTFLAGS) $(CXXFLAGS)

%.obj: %.c $(THIRDPARTY_HEADER_FILES)
	@echo "MINGW_CC  $<"
	$(QUIET)$(MINGW_CC) -c $< -o $@ $(OPTFLAGS) $(CFLAGS)

%.obj: %.cpp $(HEADER_FILES) $(THIRDPARTY_HEADER_FILES)
	@echo "MINGW_CXX $<"
	$(QUIET)$(MINGW_CXX) -c $< -o $@ $(OPTFLAGS) $(CXXFLAGS)

%.obj: %.rc $(HEADER_FILES)
	@echo "MINGW_RES $<"
	$(QUIET)$(MINGW_WINDRES) -i $< -o $@

all: $(BINARY_DIR)/$(EXE_FILE)$(EXE_EXT)

$(THIRDPARTY_DIR)/zlib/zconf.h: $(THIRDPARTY_DIR)/zlib/zconf.h.in
	@echo "GEN $@"
	$(QUIET)cp -f $@.in $@

$(BINARY_DIR)/$(EXE_FILE): $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES)
	@echo "LD  $@"
	$(QUIET)mkdir -p $(BINARY_DIR)
	$(QUIET)$(CXX) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) -o $@ $(LDFLAGS)

$(BINARY_DIR)/$(EXE_FILE).exe: $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES)
	@echo "MINGW_LD  $@"
	$(QUIET)mkdir -p $(BINARY_DIR)
	$(QUIET)$(MINGW_CXX) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARY_DIR)/$(EXE_FILE)$(EXE_EXT) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) $(THIRDPARTY_HEADER_FILES)

check: $(BINARY_DIR)/$(EXE_FILE)$(EXE_EXT)
	python3 $(SOURCE_DIR)/test.py $(BINARY_DIR)/$(EXE_FILE)$(EXE_EXT) $(TEST_ARGS)

.PHONY: all clean check
