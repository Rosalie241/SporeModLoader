/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2026 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_GITHUBAPI_HPP
#define SPOREMODMANAGERHELPERS_GITHUBAPI_HPP

#include <string>

namespace SporeModManagerHelpers
{
	namespace GithubApi
	{
		/// <summary>
        ///     Checks whether an update is available
        /// </summary>>
		bool IsUpdateAvailable(const std::string& revision);

		//bool GetLatestReleaseAssetUrl(const std::string& json, std::string& url);
	}
}

#endif