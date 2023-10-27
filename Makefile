#
# SporeModManager Makefile
#
THIRDPARTY_DIR := 3rdParty
BINARY_DIR     := bin
SOURCE_DIR     := SporeModManager
EXE_FILE       := SporeModManager
VERBOSE        := 0

PKG_CONFIG := pkg-config
CC         := gcc
CXX        := g++
CFLAGS     := -Os -flto                      \
				-I$(THIRDPARTY_DIR)/zlib     \
				-I$(THIRDPARTY_DIR)/zlib/contrib/minizip
CXXFLAGS   := -std=c++17                     \
				-Os -flto                    \
				-I$(SOURCE_DIR)              \
				-I$(THIRDPARTY_DIR)/tinyxml2 \
				-I$(THIRDPARTY_DIR)/zlib     \
				-I$(THIRDPARTY_DIR)/zlib/contrib/minizip

LDFLAGS    := -flto -s

OBJECT_FILES = \
	$(SOURCE_DIR)/main.o \
	$(SOURCE_DIR)/SporeModManager.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/FileVersion.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/Path.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/SporeMod.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/SporeModXml.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/String.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/UI.o \
	$(SOURCE_DIR)/SporeModManagerHelpers/Zip.o

THIRDPARTY_OBJECT_FILES = \
	$(THIRDPARTY_DIR)/tinyxml2/tinyxml2.o \
	$(THIRDPARTY_DIR)/zlib/adler32.o  \
	$(THIRDPARTY_DIR)/zlib/crc32.o    \
	$(THIRDPARTY_DIR)/zlib/inflate.o  \
	$(THIRDPARTY_DIR)/zlib/inffast.o  \
	$(THIRDPARTY_DIR)/zlib/inftrees.o \
	$(THIRDPARTY_DIR)/zlib/zutil.o    \
	$(THIRDPARTY_DIR)/zlib/contrib/minizip/unzip.o \
	$(THIRDPARTY_DIR)/zlib/contrib/minizip/ioapi.o

THIRDPARTY_HEADERS = \
	$(THIRDPARTY_DIR)/zlib/zconf.h

ifeq ($(VERBOSE), 0)
	QUIET := @
else
	QUIET :=
endif

%.o: %.c $(THIRDPARTY_HEADERS)
	@echo "CC  $<"
	$(QUIET)$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.cpp $(THIRDPARTY_HEADERS)
	@echo "CXX $<"
	$(QUIET)$(CXX) -c $< -o $@ $(CXXFLAGS)

all: $(BINARY_DIR)/$(EXE_FILE)

$(BINARY_DIR):
	$(QUIET)mkdir -p $@

$(THIRDPARTY_DIR)/zlib/zconf.h:
	@echo "GEN $@"
	$(QUIET)cp $@.in $@

$(BINARY_DIR)/$(EXE_FILE): $(BINARY_DIR) $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES)
	@echo "LD  $@"
	$(QUIET)$(CXX) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARY_DIR)/$(EXE_FILE) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) $(THIRDPARTY_HEADERS)

.PHONY: all clean
