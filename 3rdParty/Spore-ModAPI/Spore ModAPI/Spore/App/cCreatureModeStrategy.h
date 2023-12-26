#pragma once

#include <Spore\App\IMessageListener.h>
#include <Spore\App\IGameMode.h>
#include <Spore\Input.h>
#include <Spore\Clock.h>
#include <Spore\Simulator\SubSystem\cStrategy.h>
#include <Spore\Simulator\cCreatureDisplayStrategy.h>

#define cCreatureModeStrategyPtr eastl::intrusive_ptr<App::cCreatureModeStrategy>
#define CreatureModeStrategy (*App::cCreatureModeStrategy::Get())

namespace App
{
	class cCreatureModeStrategy
		/* 00h */	: public App::IGameMode
		/* 04h */	, public App::IMessageListener
		/* 08h */	, public Simulator::cStrategy
	{
	public:
		static cCreatureModeStrategy* Get();

		//TODO check sub_D38770, execute action?

	public:
		/* 24h */	int field_24;
		/* 28h */	Simulator::cGonzagoTimer field_28;
		/* 48h */	bool field_48;
		/* 49h */	bool field_49;
		/* 4Ch */	eastl::vector<int> field_4C;
		/* 60h */	int field_60;
		/* 64h */	int field_64;
		/* 68h */	cCreatureDisplayStrategyPtr mpDisplayStrategy;
		/* 6Ch */	bool field_6C;
		/* 70h */	int field_70;
		/* 74h */	int field_74;
		/* 78h */	int field_78;
		/* 7Ch */	int field_7C;
		/* 80h */	int field_80;
		/* 88h */	Clock field_88;
		/* A0h */	int field_A0;
		/* A4h */	int field_A4;  // -1
		/* A8h */	int field_A8;  // -1
		/* ACh */	int field_AC;
		/* B0h */	int field_B0;
		/* B4h */	int field_B4;
		/* B8h */	int field_B8;
		/* BCh */	eastl::vector<int> field_BC;
		/* D0h */	int field_D0;
		/* D4h */	int field_D4;
		/* D8h */	int field_D8;
		/* DCh */	bool mbDisplayingEvolutionButton;
		/* DDh */	bool field_DD;
		/* DEh */	bool field_DE;
		/* E0h */	int mAllPartsUnlockedColumn;  // 1
		/* E4h */	bool field_E4;
		/* E5h */	bool field_E5;
		/* E6h */	bool field_E6;
	};
	ASSERT_SIZE(cCreatureModeStrategy, 0xE8);

	namespace Addresses(cCreatureModeStrategy)
	{
		DeclareAddress(Get);  // 0xD2D640 0xD38840
	}

#ifdef SDK_TO_GHIDRA
	cCreatureModeStrategy* sCreatureModeStrategy;
#endif
}
