#
# SporeModLoader Makefile
#
THIRDPARTY_DIR ?= ../3rdParty
BINARY_DIR     ?= bin
SOURCE_DIR     ?= .
DLL_FILE       ?= dinput8.dll
VERBOSE        ?= 0

MINGW_CXX  ?= i686-w64-mingw32-g++
CXXFLAGS   := -std=c++17                     \
				-I$(SOURCE_DIR)              \
				-I$(THIRDPARTY_DIR)/Detours/src \
				-Wno-attributes -Wp,-w
OPTFLAGS   := -Os -flto
LDFLAGS    := -s -flto

OBJECT_FILES := \
	$(SOURCE_DIR)/SporeModLoaderHelpers.obj \
	$(SOURCE_DIR)/SporeModLoader.obj \
	$(SOURCE_DIR)/dllmain.obj

HEADER_FILES := \
	$(SOURCE_DIR)/SporeModLoaderHelpers.hpp \
	$(SOURCE_DIR)/SporeModLoader.hpp

THIRDPARTY_OBJECT_FILES := \
	$(THIRDPARTY_DIR)/Detours/src/detours.obj  \
	$(THIRDPARTY_DIR)/Detours/src/modules.obj  \
	$(THIRDPARTY_DIR)/Detours/src/disasm.obj   \
	$(THIRDPARTY_DIR)/Detours/src/image.obj    \
	$(THIRDPARTY_DIR)/Detours/src/creatwth.obj \
	$(THIRDPARTY_DIR)/Detours/src/disolx86.obj

ifeq ($(VERBOSE), 0)
	QUIET := @
else
	QUIET :=
endif

%.obj: %.cpp $(HEADER_FILES)
	@echo "MINGW_CXX $<"
	$(QUIET)$(MINGW_CXX) -c $< -o $@ $(OPTFLAGS) $(CXXFLAGS)

all: $(BINARY_DIR)/$(DLL_FILE)

$(BINARY_DIR):
	$(QUIET)mkdir -p $@

$(BINARY_DIR)/$(DLL_FILE): $(BINARY_DIR) $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES)
	@echo "MINGW_LD  $@"
	$(QUIET)$(MINGW_CXX) $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES) -static -shared $(SOURCE_DIR)/dllmain.def -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARY_DIR)/$(DLL_FILE) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) $(THIRDPARTY_HEADER_FILES)

.PHONY: all clean