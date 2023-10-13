#
# SporeModManager Makefile
#
THIRDPARTY_DIR = 3rdParty
BINARY_DIR     = bin
SOURCE_DIR     = SporeModManager

PKG_CONFIG := pkg-config
CC         := gcc
CXX        := g++
CFLAGS     := -O2 -flto                      \
				-I$(THIRDPARTY_DIR)/zlib     \
				-I$(THIRDPARTY_DIR)/zlib/contrib/minizip
CXXFLAGS   := -std=c++17                     \
				-O2 -flto                    \
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
	$(SOURCE_DIR)/SporeModManagerHelpers/Zip.o \
	$(THIRDPARTY_DIR)/tinyxml2/tinyxml2.o \
	$(THIRDPARTY_DIR)/zlib/contrib/minizip/unzip.o \
	$(THIRDPARTY_DIR)/zlib/contrib/minizip/ioapi.o

THIRDPARTY_LIBRARIES = \
	$(THIRDPARTY_DIR)/zlib/build/libz.a

THIRDPARTY_BUILD_DIRS = \
	$(THIRDPARTY_DIR)/zlib/build

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

all: $(BINARY_DIR)/SporeModManager

$(BINARY_DIR):
	mkdir -p $@

$(THIRDPARTY_DIR)/zlib/build/libz.a:
	cmake -S $(THIRDPARTY_DIR)/zlib -B $(THIRDPARTY_DIR)/zlib/build \
    	-DCMAKE_BUILD_TYPE=Release                  \
    	-DCMAKE_C_COMPILER=$(CC)                    \
    	-G "Unix Makefiles"
	$(MAKE) zlibstatic -C $(THIRDPARTY_DIR)/zlib/build

$(BINARY_DIR)/SporeModManager: $(THIRDPARTY_LIBRARIES) $(BINARY_DIR) $(OBJECT_FILES)
	$(CXX) $(OBJECT_FILES) -o $@ $(THIRDPARTY_LIBRARIES) $(LDFLAGS)

clean:
	rm -rf $(BINARY_DIR) $(OBJECT_FILES) $(THIRDPARTY_BUILD_DIRS)
