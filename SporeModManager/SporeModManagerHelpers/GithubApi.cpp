/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2026 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "GithubApi.hpp"
#include "Download.hpp"
#include "MallocWrapper.hpp"

#include "tiny-json.h"

#include <iostream>
#include <cstring>

using namespace SporeModManagerHelpers;

//
// Exported Functions
//


bool GithubApi::IsUpdateAvailable(const std::string& revision)
{
	const std::string url = "https://api.github.com/repos/Rosalie241/SporeModLoader/git/refs/tags";
	json_t pool[1024];
	std::vector<char> downloadBuffer;
	MallocWrapper<char> jsonBuffer;

	if (!Download::DownloadBuffer(url, downloadBuffer))
	{
		return false;
	}
	
	// allocate memory and copy the downloaded
	// buffer to a writeable buffer
	jsonBuffer.AllocateOnce(downloadBuffer.size() + 1);
	std::memset(jsonBuffer.data(), 0, jsonBuffer.size());
	std::memcpy(jsonBuffer.data(), downloadBuffer.data(), downloadBuffer.size());

	const json_t* parent = json_create(jsonBuffer.data(), pool, 1024);
	if (parent == nullptr)
	{
		std::cerr << "Error: failed to create JSON context!" << std::endl;
		return false;
	}

	if (json_getType(parent) != JSON_ARRAY)
	{
		std::cerr << "Error: JSON root object isn't an array!" << std::endl;
		return false;
	}

	const json_t* firstChild = json_getChild(parent);
	if (json_getType(firstChild) != JSON_OBJ)
	{
		std::cerr << "Error: JSON child object isn't an expected type!" << std::endl;
		return false;
	}

	const json_t* objectField = json_getProperty(firstChild, "object");
	if (objectField == nullptr)
	{
		std::cerr << "Error: failed to find object field in JSON!" << std::endl;
		return false;
	}

	const json_t* childObjectField = json_getChild(objectField);
	if (childObjectField == nullptr)
	{
		std::cerr << "Error: failed to retrieve child of object field in JSON!" << std::endl;
		return false;
	}

	const json_t* shaField = json_getProperty(objectField, "sha");
	if (shaField == nullptr || json_getType(shaField) != JSON_TEXT)
	{
		std::cerr << "Error: failed to find sha field in JSON!" << std::endl;
		return false;
	}

	std::cout << "github: " << json_getValue(shaField) << std::endl;
	std::cout << "current: " << revision << std::endl;

	return std::string(json_getValue(shaField)).rfind(revision, 0) != 0;
}



