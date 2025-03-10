#ifdef MODAPI_DLL_EXPORT
#include "stdafx.h"
#include <Spore\UI\LayerManager.h>
#include <Spore\UI\SpaceGameUI.h>
#include <Spore\UI\HintManager.h>
#include <Spore\UI\cSPAssetView.h>
#include <Spore\UI\ScrollFrameVertical.h>
#include <Spore\UI\cSPUITextZoom.h>
#include <Spore\UI\GameSettings.h>
#include <Spore\UI\EditorNamePanel.h>
#include <Spore\UI\SimulatorRollovers.h>

namespace UI
{
	namespace Addresses(cLayerManager) 
	{
		DefineAddress(Get, SelectAddress(0x67CC20, 0x67CA90));
	}

	namespace Addresses(cSPAssetView) {
		DefineAddress(_ctor, SelectAddress(0x657DA0, 0x657F30));
		DefineAddress(sub_6590A0, SelectAddress(0x6590A0, 0x659230));
	}

	namespace Addresses(SpaceGameUI)
	{
		DefineAddress(Load, SelectAddress(0x10743F0, 0x1073730));
		DefineAddress(SetActivePalette, SelectAddress(0x106F060, 0x106E410));
		DefineAddress(FillStarTooltipPlanetInfo, SelectAddress(0x1069520, 0x1068980));
	}

	namespace Addresses(cHintManager)
	{
		DefineAddress(Get, SelectAddress(0x67CC10, 0x67CA80));
		DefineAddress(ShowHint, SelectAddress(0x67C980, 0x67C7F0));
	}

	namespace Addresses(ScrollFrameVertical)
	{
		DefineAddress(Create, SelectAddress(0x807C40, 0x8076F0));
	}

	namespace Addresses(cSPUITextZoom)
	{
		DefineAddress(ctor, SelectAddress(0x834E10, 0x8346A0));
		DefineAddress(Initialize, SelectAddress(0x835800, 0x835080));
	}

	namespace Addresses(GameSettings)
	{
		DefineAddress(Show, SelectAddress(0x603430, 0x603670));
	}

	namespace Addresses(EditorNamePanel)
	{
		DefineAddress(Initialize, SelectAddress(0x5BFC70, 0x5BFE40));
		DefineAddress(Dispose, SelectAddress(0x5BFAC0, 0x5BFC90));
		DefineAddress(HandleUIMessage, SelectAddress(0x5C0020, 0x5C01F0));
		DefineAddress(SetExtended, SelectAddress(0x5C02A0, 0x5C0470));
		DefineAddress(HandleMessage, SelectAddress(0x5BF900, 0x5BFAD0));
	}

	namespace Addresses(SimulatorRollover)
	{
		DefineAddress(GetRolloverIdForObject, SelectAddress(0xB67960, 0xB67D70));
		DefineAddress(ShowRollover, SelectAddress(0xB69680, 0xB69A90));
		DefineAddress(ShowRolloverInternal, SelectAddress(0xB68790, 0xB68BA0));
	}
}
#endif
