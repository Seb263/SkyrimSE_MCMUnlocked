#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Core/Serialization.hpp"

namespace ModData
{
	class DataHandler
	{
	public:
		bool preLoaded = false;

		static DataHandler* GetSingleton()
		{
			static DataHandler singleton;
			return &singleton;
		}

		void PreLoadData()
		{
			if (preLoaded) return;
			preLoaded = true;

			TESdataHandler = RE::TESDataHandler::GetSingleton();

			LoadPluginsForms();
			Serialization::Functions::RegisterSerializationCallbacks();
		}

	private:
		static inline void LoadPluginsForms()
		{
			logger::info("Loading Plugins Froms Data...");

			for (const auto& formInfo : pluginForms) {
				*formInfo.formPtr = TESdataHandler->LookupForm(formInfo.formID, formInfo.pluginName.data());
				if (!*formInfo.formPtr && !formInfo.optional) {
					REPORT_AND_FAIL("ERROR: Form \"{}\" not found in \"{}\".", formInfo.pluginName, formInfo.name, formInfo.pluginName);
				}
			}

			logger::info("Loading Plugins Froms Data: DONE");
		}
	};
}
