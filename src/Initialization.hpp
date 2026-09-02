#pragma once

#include "DataHandler.hpp"
#include "Events.h"
#include "SettingsIni.hpp"

#include "Core/JSONHandler.hpp"
#include "Core/Serialization.hpp"

#include "Utils/MiscUtils.hpp"

namespace ModData
{
	class DataHandler
	{
	public:
		bool preInitialized = false;
		bool preLoaded = false;

		static DataHandler* GetSingleton()
		{
			static DataHandler singleton;
			return &singleton;
		}

		void PreInitData()
		{
			if (preInitialized) return;
			preInitialized = true;

			ExtractGameAssets();
		}

		void PreLoadData()
		{
			if (preLoaded) return;
			preLoaded = true;

			TESdataHandler = RE::TESDataHandler::GetSingleton();
			if (IsConflictingPluginLoaded()) {
				logger::error("Conflicting plugin found. Abort mod initialization.");
				modStatus = false;
				return;
			}

			gameLanguage = MiscUtils::GetGameINISetting<std::string>("sLanguage:General").value_or("english");
			std::transform(gameLanguage.begin(), gameLanguage.end(), gameLanguage.begin(), ::tolower);

			LoadPluginsForms();
			Serialization::Functions::RegisterSerializationCallbacks();
			Events::ModEventSink::LoadEvents();
			ModCore::JSONHandler::ParseJSONToStructure();
		}

	private:
		static inline void LoadPluginsForms()
		{
			logger::info("Loading Plugins Froms Data...");

			for (const auto& formInfo : pluginForms) {
				*formInfo.formPtr = TESdataHandler->LookupForm(formInfo.formID, formInfo.pluginName.data());
				if (!*formInfo.formPtr && !formInfo.optional) {
					if (formInfo.pluginName == PLUGIN_NAME) {
						REPORT_AND_FAIL(
							"ERROR: The required plugin \"{}\" is missing! This means the mod is either not installed correctly or your mod manager failed to enable it.\n"
							"If you believe you installed the mod properly, please redo the manual installation without using a mod manager.\n\n"
							"This is NOT a bug - DO NOT report it! Instructions for manual installation are available on the mod's page.\n\n"
							"DETAILS: Form \"{}\" not found in \"{}\".",
							formInfo.pluginName, formInfo.name, formInfo.pluginName);
					} else {
						REPORT_AND_FAIL("ERROR: Form \"{}\" not found in \"{}\".", formInfo.pluginName, formInfo.name, formInfo.pluginName);
					}
				}
			}

			logger::info("Loading Plugins Froms Data: DONE");
		}

		static inline bool IsConflictingPluginLoaded()
		{
			constexpr std::array<std::string_view, 2> conflictingPlugins{{
				"MenuMaid2.esp", "MenuMaid.esp"
			}};

			for (const auto& plugin : conflictingPlugins) {
				const bool loaded = ModData::TESdataHandler->GetLoadedModIndex(plugin).has_value()
					|| ModData::TESdataHandler->GetLoadedLightModIndex(plugin).has_value();
				
				if (loaded) {
					RE::DebugMessageBox(std::format("{} is not compatible with {}. The MCM will not be able to function. "
						"Please keep only one mod.", plugin, MOD_NAME
					).c_str());
					return true;
				}
			}

			return false;
		}

		static inline void ExtractGameAssets()
		{
			constexpr unsigned char ConfigManagerPscBytes[] = {
				#include "SKI_ConfigManager.psc.h"
			};
			constexpr unsigned char ConfigManagerPexBytes[] = {
				#include "SKI_ConfigManager.pex.h"
			};
			constexpr unsigned char McmRecorderPscBytes[] = {
				#include "McmRecorder.psc.h"
			};
			constexpr unsigned char McmRecorderPexBytes[] = {
				#include "McmRecorder.pex.h"
			};

			const std::string_view ConfigManagerPscData{ reinterpret_cast<const char*>(ConfigManagerPscBytes), sizeof(ConfigManagerPscBytes) - 1 };
			const std::string_view ConfigManagerPexData{ reinterpret_cast<const char*>(ConfigManagerPexBytes), sizeof(ConfigManagerPexBytes) - 1 };
			const std::string_view McmRecorderPscData{ reinterpret_cast<const char*>(McmRecorderPscBytes), sizeof(McmRecorderPscBytes) - 1 };
			const std::string_view McmRecorderPexData{ reinterpret_cast<const char*>(McmRecorderPexBytes), sizeof(McmRecorderPexBytes) - 1 };

			enum class AssetFlags : uint8_t
			{
				kNone = 0,
				kScriptSource = 1 << 0,
				kMcmRecorder = 1 << 1,
				kMcmRecorderSrc = kScriptSource | kMcmRecorder,
			};

			auto has = [](AssetFlags val, AssetFlags flag) {
				return (static_cast<uint8_t>(val) & static_cast<uint8_t>(flag)) != 0;
			};

			struct AssetEntry
			{
				std::string_view data;
				std::string_view dest;
				AssetFlags flags;
			};

			const std::array<AssetEntry, 4> assets{{
				{ ConfigManagerPscData, "Data/Source/Scripts/SKI_ConfigManager.psc", AssetFlags::kScriptSource },
				{ ConfigManagerPexData, "Data/Scripts/SKI_ConfigManager.pex", AssetFlags::kNone },
				{ McmRecorderPscData, "Data/Source/Scripts/McmRecorder.psc", AssetFlags::kMcmRecorderSrc },
				{ McmRecorderPexData, "Data/Scripts/McmRecorder.pex", AssetFlags::kMcmRecorder },
			}};

			const bool extractSources = SettingsIni::bGeneral_ExtractScriptSources;
			const bool overwriteOnDiff = SettingsIni::bGeneral_OverwriteInvalidScripts;
			const bool mcmRecorderLoaded = IsPluginListed("McmRecorder.esp");

			for (const auto& asset : assets) {
				if (has(asset.flags, AssetFlags::kScriptSource) && !extractSources) {
					TRACE("ExtractGameAssets: Skipping source script '{}' (ExtractScriptSources disabled).", asset.dest);
					continue;
				}
				if (has(asset.flags, AssetFlags::kMcmRecorder) && !mcmRecorderLoaded) {
					TRACE("ExtractGameAssets: Skipping '{}' (McmRecorder.esp not loaded).", asset.dest);
					continue;
				}
				try {
					const std::size_t srcHash = std::hash<std::string_view>{}(asset.data);
					const std::filesystem::path destPath(asset.dest);
					if (std::filesystem::exists(destPath)) {
						std::ifstream existing(destPath, std::ios::binary);
						if (existing) {
							const std::string destData{
								std::istreambuf_iterator<char>{existing},
								std::istreambuf_iterator<char>{}
							};
							const std::size_t destHash = std::hash<std::string>{}(destData);
							if (srcHash == destHash) {
								TRACE("ExtractGameAssets: Asset '{}' is up-to-date, skipping.", asset.dest);
								continue;
							}
							if (!overwriteOnDiff) {
								TRACE("ExtractGameAssets: Asset '{}' differs but overwrite is disabled, skipping.", asset.dest);
								continue;
							}
							TRACE("ExtractGameAssets: Asset '{}' differs, replacing.", asset.dest);
						}
					}
					std::filesystem::create_directories(destPath.parent_path());
					std::ofstream out(destPath, std::ios::binary | std::ios::trunc);
					if (!out) {
						logger::error("ExtractGameAssets: Failed to open output stream for '{}'.", asset.dest);
						continue;
					}
					out.write(asset.data.data(), static_cast<std::streamsize>(asset.data.size()));
					if (!out) {
						std::filesystem::remove(destPath);
						logger::error("ExtractGameAssets: Failed to write '{}'.", asset.dest);
						continue;
					}
					TRACE("ExtractGameAssets: Asset '{}' extracted successfully.", asset.dest);
				} catch (const std::exception& e) {
					logger::error("ExtractGameAssets: Exception extracting '{}': {}", asset.dest, e.what());
				}
			}
		}

		static inline bool IsPluginListed(std::string_view pluginName)
		{
			struct Candidate { const char* env; const char* folder; };
			Candidate candidates[3];
			int count = 0;

			if (REL::Module::IsVR()) {
				candidates[count++] = { "LOCALAPPDATA", "Skyrim VR" };
			} else {
				candidates[count++] = { "LOCALAPPDATA", "Skyrim Special Edition" };
				candidates[count++] = { "LOCALAPPDATA", "Skyrim Special Edition GOG" };
			}

			for (int i = 0; i < count; ++i) {
				const char* appData = std::getenv(candidates[i].env);
				if (!appData) continue;

				const std::filesystem::path pluginsFile = std::filesystem::path(appData) / candidates[i].folder / "plugins.txt";

				std::ifstream f(pluginsFile);
				if (!f) continue;

				std::string line;
				while (std::getline(f, line)) {
					if (!line.empty() && line.back() == '\r') line.pop_back();
					if (!line.empty() && line[0] == '*') line.erase(0, 1);
					else continue;

					if (_stricmp(line.c_str(), pluginName.data()) == 0) return true;
				}
			}
			return false;
		}
	};
}
