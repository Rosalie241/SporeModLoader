#
# Spore-ModAPI Makefile
#
THIRDPARTY_DIR ?= .
BINARY_DIR     ?= Spore-ModAPI/bin
SOURCE_DIR     ?= Spore-ModAPI/Spore\ ModAPI
DLL_FILE       ?= SporeModAPI.dll
VERBOSE        ?= 0
DEBUG          ?= 0

MINGW_CXX     ?= i686-w64-mingw32-g++
MINGW_WINDRES ?= i686-w64-mingw32-windres


CXXFLAGS   := -std=c++17 \
				-I$(SOURCE_DIR) \
				-I$(THIRDPARTY_DIR)/Detours/src \
				-I$(SOURCE_DIR)/../EASTL-3.02.01/include \
				-I$(SOURCE_DIR)/../EASTL-3.02.01/test/packages/EAStdC/include        \
				-I$(SOURCE_DIR)/../EASTL-3.02.01/test/packages/EAAssert/include      \
				-I$(SOURCE_DIR)/../EASTL-3.02.01/test/packages/EABase/include/Common \
				-Wno-attributes -Wp,-w \
				-DEXECUTABLE_TYPE=2 -DMODAPI_DLL_EXPORT=1 -DSDK_BUILD_VER=320
OPTFLAGS   := -Os
WARNFLAGS  := -Wall -Wextra -Wpedantic
LDFLAGS    := -Wl,--enable-stdcall-fixup

ifeq ($(DEBUG), 1)
OPTFLAGS   += -g3
else
LDFLAGS    +=
endif

OBJECT_FILES := \
	$(SOURCE_DIR)/SourceCode/Allocator.obj \
    $(SOURCE_DIR)/SourceCode/Anim/Animations.obj \
    $(SOURCE_DIR)/SourceCode/App/App.obj \
    $(SOURCE_DIR)/SourceCode/App/Cameras.obj \
    $(SOURCE_DIR)/SourceCode/App/CheatManager.obj \
    $(SOURCE_DIR)/SourceCode/App/GameModes.obj \
    $(SOURCE_DIR)/SourceCode/App/LocaleManager.obj \
    $(SOURCE_DIR)/SourceCode/App/Messages.obj \
    $(SOURCE_DIR)/SourceCode/App/PropertyList.obj \
    $(SOURCE_DIR)/SourceCode/App/PropManager.obj \
    $(SOURCE_DIR)/SourceCode/App/ScenarioMode.obj \
    $(SOURCE_DIR)/SourceCode/App/Viewer.obj \
    $(SOURCE_DIR)/SourceCode/ArgScript/ClassManager.obj \
    $(SOURCE_DIR)/SourceCode/Audio/AudioSystem.obj \
    $(SOURCE_DIR)/SourceCode/Debugging.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesAnim.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesApp.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesArgScript.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesAudio.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesEditors.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesGalaxyGameEntry.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesGraphics.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesIO.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesMisc.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesPalettes.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesPollinator.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesRenderWare.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesResource.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesSimulator.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesSkinner.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesSporepedia.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesSwarm.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesTerrain.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesUI.obj \
    $(SOURCE_DIR)/SourceCode/DLL/AddressesUTFWin.obj \
    $(SOURCE_DIR)/SourceCode/DLL/Application.obj \
    $(SOURCE_DIR)/SourceCode/DLL/dllmain.obj \
    $(SOURCE_DIR)/SourceCode/DLL/DllModAPI.obj \
    $(SOURCE_DIR)/SourceCode/DLL/SpaceToolIconOverride.obj \
    $(SOURCE_DIR)/SourceCode/DLL/stdafx.obj \
    $(SOURCE_DIR)/SourceCode/DLL/TextureOverride.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/allocator_eastl.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/assert.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/EAMemory.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/EASprintf.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/fixed_pool.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/hashtable.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/intrusive_list.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/numeric_limits.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/red_black_tree.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/string.obj \
    $(SOURCE_DIR)/SourceCode/EASTL/thread_support.obj \
    $(SOURCE_DIR)/SourceCode/Editors/BakeManager.obj \
    $(SOURCE_DIR)/SourceCode/Editors/Colorpickers.obj \
    $(SOURCE_DIR)/SourceCode/Editors/Editor.obj \
    $(SOURCE_DIR)/SourceCode/Editors/EditorCamera.obj \
    $(SOURCE_DIR)/SourceCode/Editors/EditorPlayMode.obj \
    $(SOURCE_DIR)/SourceCode/Editors/EditorRequest.obj \
    $(SOURCE_DIR)/SourceCode/Editors/PaletteItemFrame.obj \
    $(SOURCE_DIR)/SourceCode/Editors/SpeciesManager.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/GraphicsManager.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/Shadows.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/ThumbnailManager.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/RenderTargets.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/GeneratedMesh.obj \
    $(SOURCE_DIR)/SourceCode/Pollinator/Pollinator.obj \
    $(SOURCE_DIR)/SourceCode/RenderWare/Raster.obj \
    $(SOURCE_DIR)/SourceCode/RenderWare/RenderWare.obj \
    $(SOURCE_DIR)/SourceCode/RenderWare/RWMesh.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cCity.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cCreatureAbility.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cCreatureAnimal.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cCultureSet.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cEmpire.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cGameData.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cGameDataUFO.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cGameTerrainCursor.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/CommManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/CreatureGame.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cSimulatorPlayerUFO.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cSpatialObject.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cTribe.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cVehicle.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/GameBehaviorManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/GameInputManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/GameModeManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/GameTimeManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/GameViewManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/MissionManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/PlanetModel.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/Planets.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/RelationshipManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/Scenario.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/Serialization.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SimulatorCell.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SimulatorClock.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SimulatorMessages.obj \
    $(SOURCE_DIR)/SourceCode/Editors/EditorPalettes.obj \
    $(SOURCE_DIR)/SourceCode/FixedPoolAllocator.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/Lighting.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/Materials.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/Models.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/Rendering.obj \
    $(SOURCE_DIR)/SourceCode/Graphics/Textures.obj \
    $(SOURCE_DIR)/SourceCode/IO/IODefinitions.obj \
    $(SOURCE_DIR)/SourceCode/IO/ResourceManager.obj \
    $(SOURCE_DIR)/SourceCode/IO/StreamDefinitions.obj \
    $(SOURCE_DIR)/SourceCode/IO/ThreadedObject.obj \
    $(SOURCE_DIR)/SourceCode/IO/XML.obj \
    $(SOURCE_DIR)/SourceCode/LocalizedString.obj \
    $(SOURCE_DIR)/SourceCode/Mutex.obj \
    $(SOURCE_DIR)/SourceCode/RenderWare/RWBuffers.obj \
    $(SOURCE_DIR)/SourceCode/RenderWare/RWSkeleton.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/GameNounManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SimulatorMisc.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SimulatorSpaceGame.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SimulatorSystem.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/cStarRecord.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SolarSystem.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SpaceInventory.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/SpacePlayerData.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/StarManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/Terraforming.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/ToolManager.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/Tools.obj \
    $(SOURCE_DIR)/SourceCode/Simulator/UIEventLog.obj \
    $(SOURCE_DIR)/SourceCode/Skinner.obj \
    $(SOURCE_DIR)/SourceCode/Sporepedia/Assetbrowser.obj \
    $(SOURCE_DIR)/SourceCode/Sporepedia/AssetData.obj \
    $(SOURCE_DIR)/SourceCode/Sporepedia/Shopper.obj \
    $(SOURCE_DIR)/SourceCode/Swarm/Effects.obj \
    $(SOURCE_DIR)/SourceCode/Swarm/SwarmManager.obj \
    $(SOURCE_DIR)/SourceCode/Clock.obj \
    $(SOURCE_DIR)/SourceCode/Terrain/Terrain.obj \
    $(SOURCE_DIR)/SourceCode/UI/HintManager.obj \
    $(SOURCE_DIR)/SourceCode/UI/LayerManager.obj \
    $(SOURCE_DIR)/SourceCode/UI/ScrollFrame.obj \
    $(SOURCE_DIR)/SourceCode/UI/cSPUITextZoom.obj \
    $(SOURCE_DIR)/SourceCode/UI/SpaceGameUI.obj \
    $(SOURCE_DIR)/SourceCode/UI/UI.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Cursors.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Defaults.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Drawables.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Events.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Graphics2D.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Image.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Interactives.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/OutlineFormat.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Tooltips.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/UIEffects.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/UILayout.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/UTFWinAllocator.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/LayoutElements.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/UISerialization.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/Window.obj \
    $(SOURCE_DIR)/SourceCode/ArgScript/ArgScript.obj \
    $(SOURCE_DIR)/SourceCode/Math.obj \
    $(SOURCE_DIR)/SourceCode/Object.obj \
    $(SOURCE_DIR)/SourceCode/ResourceKey.obj \
    $(SOURCE_DIR)/SourceCode/UTFWin/WindowManager.obj

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

%.obj: %.rc $(HEADER_FILES)
	@echo "MINGW_RES $<"
	$(QUIET)$(MINGW_WINDRES) -i "$<" -o "$@"

%.obj: %.cpp $(HEADER_FILES)
	@echo "MINGW_CXX $<"
	$(QUIET)$(MINGW_CXX) -c "$<" -o "$@" $(OPTFLAGS) $(WARNFLAGS) $(CXXFLAGS)

all: $(BINARY_DIR)/$(DLL_FILE)

$(BINARY_DIR)/$(DLL_FILE): $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES)
	@echo "MINGW_LD  $@"
	$(QUIET)mkdir -p $(BINARY_DIR)
	$(QUIET)$(MINGW_CXX) $(THIRDPARTY_OBJECT_FILES) $(OBJECT_FILES) -static -shared $(THIRDPARTY_DIR)/SporeModAPI.def -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARY_DIR)/$(DLL_FILE) $(OBJECT_FILES) $(THIRDPARTY_OBJECT_FILES) $(THIRDPARTY_HEADER_FILES)

.PHONY: all clean
