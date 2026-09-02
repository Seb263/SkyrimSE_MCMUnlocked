#pragma once

#include "DataHandler.hpp"

#include "Core/Structure.h"
#include "Core/Main.hpp"
#include "Core/JSONHandler.hpp"

#include "Utils/MiscUtils.hpp"

namespace ModCore
{
	class Rename
	{
	public:

		static inline std::string latestModID = "";
		static void RenameStart()
		{
			Main::GetModIDFromSelectedEntryAsync([](std::string modID) {
				if (modID.empty()) return;
				latestModID = modID;

				auto* jsonEntry = JSONHandler::GetModEntryFromModID(modID);
				if (!jsonEntry) return;

				MiscUtils::SetSearchWidgetDisabled(true);

				std::string dialogTitle = SettingsIni::SettingsManager().GetLanguageValue("RenameMenu", ModData::gameLanguage, "Rename: %s");
				if (size_t pos = dialogTitle.find("%s"); pos != std::string::npos) dialogTitle.replace(pos, 2, MiscUtils::GetSKSETranslation(jsonEntry->modID));

				OpenTextInputDialog(dialogTitle, (jsonEntry->modName.empty() ? jsonEntry->modID : jsonEntry->modName));
			});
		}
		
		static void RenameDone(std::string newName)
		{
			const auto modID = latestModID;
			if (modID.empty()) return;

			if (newName.empty()) newName = MiscUtils::NormalizeModID(modID);

			Main::SetModNameFromModID(modID, newName);

			MiscUtils::SetSearchWidgetDisabled(false);

			Main::UpdateMenuModNames();
		}

	private:

		static void OpenTextInputDialog(const std::string& titleText, const std::string& initialText = "")
		{
			SKSE::GetTaskInterface()->AddUITask([titleText, initialText]() {
				auto panel = MiscUtils::GetConfigPanelGfx();
				if (!panel.IsObject()) return;

				RE::GFxValue titleVal;
				titleVal.SetString(titleText.c_str());
				panel.SetMember("_dialogTitleText", titleVal);

				RE::GFxValue args[1];
				args[0].SetString(initialText.c_str());
				panel.Invoke("setInputDialogParams", nullptr, args, 1);
			});
		}
	};
}
