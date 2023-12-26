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

#define cCropCirclesToolStrategyPtr eastl::intrusive_ptr<Simulator::cCropCirclesToolStrategy>

namespace Simulator
{
	class cCropCirclesToolStrategy
		: public cDefaultProjectileWeapon
	{
	public:
		static const uint32_t STRATEGY_ID = 0xC0B1AE6A;

		virtual bool OnHit(cSpaceToolData* pTool, const Vector3& position, SpaceToolHit hitType, int) override;
	};

	namespace Addresses(cCropCirclesToolStrategy)
	{
		DeclareAddress(OnHit);
	}
}