#pragma once

#include "DataHandler.hpp"

#include "Core/Structure.h"
#include "Core/Main.hpp"
#include "Core/JSONHandler.hpp"

#include "Utils/MiscUtils.hpp"

namespace ModCore
{
	class Reset
	{
	public:

		static void ResetStart()
		{
			std::string resetMessage = SettingsIni::SettingsManager().GetLanguageValue("ResetWarning", ModData::gameLanguage, "Reset all MCMs?");
			std::string resetMessageAll = SettingsIni::SettingsManager().GetLanguageValue("ResetWarningAll", ModData::gameLanguage, "Reset All");
			std::string resetMessageMarkers = SettingsIni::SettingsManager().GetLanguageValue("ResetWarningMarkers", ModData::gameLanguage, "Reset Instances Only");
			MiscUtils::ShowMessageBox(resetMessage, { resetMessageAll, resetMessageMarkers, "$Cancel" }, [](unsigned int result) {
				if (result > 1) return;

				Main::UnregisterAllMarkers();
				PurgeAllMCMMarkers();
				if (result == 0) JSONHandler::ResetToDefault();
				MiscUtils::ResetMCMQuest();

				RE::UIMessageQueue::GetSingleton()->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
			});
		}

	private:
		static void PurgeAllMCMMarkers()
		{
			if (!ModData::MCMMarkerCell) return;

			auto* garbageCollector = RE::GarbageCollector::GetSingleton();
			if (!garbageCollector) return;

			std::vector<RE::TESObjectREFR*> toDelete;

			ModData::MCMMarkerCell->ForEachReference([&](RE::TESObjectREFR* ref) {
				if (!ref) return RE::BSContainer::ForEachResult::kContinue;
				
				toDelete.push_back(ref);
				
				return RE::BSContainer::ForEachResult::kContinue;
			});
			if (toDelete.empty()) return;

			TRACE("Purging {} orphan MCM markers from cell \"{:08X}\"", toDelete.size(), ModData::MCMMarkerCell->formID);

			for (auto* marker : toDelete) {
				TRACE("  -> Deleting marker \"{:08X}\"", marker->formID);
				garbageCollector->Add(marker, true);
			}
		}
	};
}
