#pragma once

namespace ModData
{
	constexpr std::string_view MOD_NAME = "MCM Unlocked";
	constexpr std::string_view PLUGIN_NAME = "MCM Unlocked.esp";

	struct PluginForm
	{
		std::string_view name;
		void**           formPtr;
		uint32_t         formID;
		std::string_view pluginName;
		bool             optional = false;
	};

	// Properties storing game form references
	inline RE::TESObjectACTI* MCMMarkerBase;
	inline RE::TESObjectCELL* MCMMarkerCell;

	static inline const std::vector<PluginForm> pluginForms = {
		{ "MCMMarkerBase", reinterpret_cast<void**>(&MCMMarkerBase), 0x800, PLUGIN_NAME },
		{ "MCMMarkerCell", reinterpret_cast<void**>(&MCMMarkerCell), 0x801, PLUGIN_NAME }
	};

	inline RE::TESDataHandler* TESdataHandler;
}
