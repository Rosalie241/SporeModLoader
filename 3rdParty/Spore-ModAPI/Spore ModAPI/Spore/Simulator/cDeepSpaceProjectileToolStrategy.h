/****************************************************************************
* Copyright (C) 2019 Eric Mor
*
* This file is part of Spore ModAPI.
*
* Spore ModAPI is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/
#pragma once

#include <Spore\Simulator\cDefaultProjectileWeapon.h>

#define cDeepSpaceProjectileToolStrategyPtr eastl::intrusive_ptr<Simulator::cDeepSpaceProjectileToolStrategy>

namespace Simulator
{
	class cDeepSpaceProjectileToolStrategy
		: public cDefaultProjectileWeapon
	{
	public:
		static const uint32_t STRATEGY_ID = 0x68ECCB0A;

		virtual bool WhileFiring(cSpaceToolData* pTool, const Vector3& aimPoint, int) override;
	};

	namespace Addresses(cDeepSpaceProjectileToolStrategy)
	{
		DeclareAddress(WhileFiring);
	}
}