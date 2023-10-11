#
# SporeModManager Makefile
#
THIRDPARTY_DIR = 3rdParty
BINARY_DIR     = bin
SOURCE_DIR     = SporeModManager

PKG_CONFIG := pkg-config
CXX        := g++
CXXFLAGS   := -std=c++17                     \
				-O2 -flto                    \
				-I$(SOURCE_DIR)              \
				-I$(THIRDPARTY_DIR)/tinyxml2 \
				-I$(THIRDPARTY_DIR)/minizip-ng

LDFLAGS    := -s

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

THIRDPARTY_LIBRARIES = \
	$(THIRDPARTY_DIR)/minizip-ng/build/libminizip.a \
	$(THIRDPARTY_DIR)/minizip-ng/build/_deps/zlib-build/libz.a

THIRDPARTY_BUILD_DIRS = \
	$(THIRDPARTY_DIR)/minizip-ng/build

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

all: $(BINARY_DIR)/SporeModManager

$(BINARY_DIR):
	mkdir -p $@

$(THIRDPARTY_DIR)/minizip-ng/build/libminizip.a:
	cmake -S $(THIRDPARTY_DIR)/minizip-ng -B $(THIRDPARTY_DIR)/minizip-ng/build \
		-DMZ_LZMA=OFF  -DMZ_ZSTD=OFF                \
    	-DMZ_BZIP2=OFF -DMZ_PKCRYPT=OFF             \
    	-DMZ_WZAES=OFF -DMZ_SIGNING=OFF             \
    	-DMZ_DECOMPRESS_ONLY=ON -DMZ_ICONV=OFF      \
    	-DMZ_OPENSSL=OFF -DMZ_LIBBSD=OFF            \
    	-DMZ_FORCE_FETCH_LIBS=ON -DMZ_FETCH_LIBS=ON \
    	-DCMAKE_BUILD_TYPE=Release                  \
    	-DCMAKE_C_COMPILER=$(CC)                    \
    	-G "Unix Makefiles"
	$(MAKE) -C $(THIRDPARTY_DIR)/minizip-ng/build

$(THIRDPARTY_DIR)/minizip-ng/build/_deps/zlib-build/libz.a: $(THIRDPARTY_DIR)/minizip-ng/build/libminizip.a

$(BINARY_DIR)/SporeModManager: $(THIRDPARTY_LIBRARIES) $(BINARY_DIR) $(OBJECT_FILES)
	$(CXX) $(OBJECT_FILES) -o $@ $(THIRDPARTY_LIBRARIES) $(LDFLAGS)

clean:
	rm -rf $(BINARY_DIR) $(OBJECT_FILES) $(THIRDPARTY_BUILD_DIRS)
