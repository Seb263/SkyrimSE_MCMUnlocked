#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

namespace ModCore
{
	struct MCMEntry
	{
		std::string modID = "";
		std::string modName = "";
		int modPosition = -1;
		bool disabled = false;
	};

	using MCMStructure = std::vector<MCMEntry>;

	inline MCMStructure mcmStructure;
	inline std::shared_mutex g_mcmStructureMutex;
}
