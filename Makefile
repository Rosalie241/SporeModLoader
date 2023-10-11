#
# SporeModManager Makefile
#
THIRDPARTY_DIR = 3rdParty
BINARY_DIR     = Bin
SOURCE_DIR     = SporeModManager

PKG_CONFIG := pkg-config
CXX        := g++
CXXFLAGS   := -std=c++20                     \
				-O2 -flto                    \
				-I$(SOURCE_DIR)              \
				-I$(THIRDPARTY_DIR)/tinyxml2 \
				$(shell $(PKG_CONFIG) --cflags minizip)

LDFLAGS    := $(shell $(PKG_CONFIG) --libs minizip)

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
	$(THIRDPARTY_DIR)/tinyxml2/tinyxml2.o

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

all: Bin/SporeModManager

$(BINARY_DIR):
	mkdir -p $@

$(BINARY_DIR)/SporeModManager: $(BINARY_DIR) $(OBJECT_FILES)
	$(CXX) $(OBJECT_FILES) -s -o $@ $(LDFLAGS)

clean:
	rm -rf $(BINARY_DIR) $(OBJECT_FILES)
