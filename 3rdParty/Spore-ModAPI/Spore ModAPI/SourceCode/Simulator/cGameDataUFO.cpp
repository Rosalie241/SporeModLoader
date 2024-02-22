#ifndef MODAPI_DLL_EXPORT
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
#include <Spore\Simulator\cGameDataUFO.h>

namespace Simulator
{
	auto_METHOD(cGameDataUFO, float, ChangeEnergy, Args(float delta), Args(delta));

	auto_METHOD_VOID(cGameDataUFO, Initialize, Args(UfoType type, cEmpire* empire), Args(type, empire));

	auto_STATIC_METHOD(Simulator, cGameDataUFO*, CreateUFO, Args(UfoType type, cEmpire* empire), Args(type, empire));
}
#endif
