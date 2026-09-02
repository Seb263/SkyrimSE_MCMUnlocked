#pragma once

#include "DataHandler.hpp"

#include "Core/Structure.h"
#include "Core/Main.hpp"

#include "Utils/MiscUtils.hpp"

namespace ModCore
{
	class Reorder
	{
	public:

		static inline std::vector<std::string> g_reorderIndexMapping;
		static inline std::string latestModID = "";

		static void ReorderStart()
		{
			Main::GetModIDFromSelectedEntryAsync([](std::string modID) {
				if (modID.empty()) return;
				latestModID = modID;

				auto it = std::find_if(mcmStructure.begin(), mcmStructure.end(), [](const MCMEntry& e) { return e.modID == latestModID; });
				if (it == mcmStructure.end()) return;
				MCMEntry& currentEntry = *it;

				MiscUtils::SetSearchWidgetDisabled(true);

				std::vector<MCMEntry> snapshot;
				{
					std::shared_lock lock(g_mcmStructureMutex);
					snapshot = mcmStructure;
				}
				std::sort(snapshot.begin(), snapshot.end(), [](const MCMEntry& a, const MCMEntry& b) {
					return a.modPosition < b.modPosition;
				});

				g_reorderIndexMapping.clear();
				g_reorderIndexMapping.push_back("");
				g_reorderIndexMapping.push_back("");
				g_reorderIndexMapping.push_back("");

				const std::string title = (!currentEntry.modName.empty()) ? currentEntry.modName : latestModID;
				const bool isDisabled = currentEntry.disabled;

				if (isDisabled) {
					currentEntry.disabled = false;
					Main::UpdateMenuModNames();
					return;
				}

				std::string autoSortText = SettingsIni::SettingsManager().GetLanguageValue("AutoSort", ModData::gameLanguage, "+ Auto sort");
				std::string disableText = SettingsIni::SettingsManager().GetLanguageValue("DisableMenu", ModData::gameLanguage, "+ Disable menu");
				std::string enableText = SettingsIni::SettingsManager().GetLanguageValue("EnableMenu", ModData::gameLanguage, "+ Enable menu");
				std::string placeFirstText = SettingsIni::SettingsManager().GetLanguageValue("PlaceFirst", ModData::gameLanguage, "> Place first");
				std::string placeAfterFormat = SettingsIni::SettingsManager().GetLanguageValue("PlaceAfter", ModData::gameLanguage, "> Place after: %s");
			
				std::vector<std::string> options;
				options.push_back(autoSortText);
				options.push_back(disableText);
				options.push_back(placeFirstText);
				for (const auto& entry : snapshot) {
					if (entry.modID == latestModID) continue;
				
					const std::string& displayName = MiscUtils::GetSKSETranslation(entry.modName.empty() ? entry.modID : entry.modName);
					std::string placeAfterText = placeAfterFormat;
					if (size_t pos = placeAfterText.find("%s"); pos != std::string::npos) placeAfterText.replace(pos, 2, displayName);
					options.push_back(std::move(placeAfterText));
				
					g_reorderIndexMapping.push_back(entry.modID);
				}

				SKSE::GetTaskInterface()->AddUITask([title, options]() {
					std::vector<RE::GFxValue> labelArgs;
					labelArgs.reserve(options.size());
					for (const auto& opt : options) {
						RE::GFxValue v;
						v.SetString(opt.c_str());
						labelArgs.push_back(v);
					}

					auto panel = MiscUtils::GetConfigPanelGfx();
					if (!panel.IsObject()) return;

					RE::GFxValue titleVal;
					titleVal.SetString(title.c_str());
					panel.SetMember("_dialogTitleText", titleVal);

					panel.Invoke("setMenuDialogOptions", nullptr, labelArgs.data(), (unsigned int)labelArgs.size());

					RE::GFxValue dialogArgs[2];
					dialogArgs[0].SetNumber(0);
					dialogArgs[1].SetNumber(0);
					panel.Invoke("setMenuDialogParams", nullptr, dialogArgs, 2);
				});
			});
		}

		static void ReorderDone(int selectedIndex)
		{
			MiscUtils::SetSearchWidgetDisabled(false);

			{
				std::unique_lock lock(g_mcmStructureMutex);

				auto it = std::find_if(mcmStructure.begin(), mcmStructure.end(), [](const MCMEntry& e) { return e.modID == latestModID; });
				if (it == mcmStructure.end()) return;
				MCMEntry& currentEntry = *it;

				if (selectedIndex == 0) { // Alphabetical sort: reset the selected mod to -1

					currentEntry.modPosition = -1;

				} else if (selectedIndex == 1) { // Enable / Disable
				
					currentEntry.disabled = !currentEntry.disabled;

				} else if (selectedIndex == 2) { // Move first
				
					for (auto& e : mcmStructure) {
						if (e.modID != latestModID && e.modPosition >= 0) e.modPosition++;
					}

					auto targetIt = std::find_if(mcmStructure.begin(), mcmStructure.end(), [](const MCMEntry& e){ return e.modID == latestModID; });
					if (targetIt != mcmStructure.end()) targetIt->modPosition = 0;

				} else if (selectedIndex >= 3 && selectedIndex < static_cast<int>(g_reorderIndexMapping.size())) {

					const std::string& afterModID = g_reorderIndexMapping[selectedIndex];

					auto refIt = std::find_if(mcmStructure.begin(), mcmStructure.end(), [&](const MCMEntry& e) { return e.modID == afterModID; });
					if (refIt == mcmStructure.end()) return;

					const int afterPosition = refIt->modPosition;

					for (auto& e : mcmStructure) {
						if (e.modID != latestModID && e.modPosition > afterPosition) e.modPosition++;
					}

					auto targetIt = std::find_if(mcmStructure.begin(), mcmStructure.end(), [](const MCMEntry& e) { return e.modID == latestModID; });
					if (targetIt != mcmStructure.end()) targetIt->modPosition = afterPosition + 1;
				}
			}

			Main::UpdateMenuModNames();
		}
	};
}
