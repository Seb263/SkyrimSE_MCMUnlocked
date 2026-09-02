#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Core/Structure.h"

#include "Utils/MiscUtils.hpp"

namespace ModCore
{
	class JSONHandler
	{
		inline static std::string jsonPath = "Data/SKSE/Plugins/MCM-Unlocked_UserData.json";

	public:


		static void ValidateMCMStructure(const std::vector<std::string>& structure)
		{
			std::unordered_set<std::string> existing;

			{
				std::unique_lock lock(g_mcmStructureMutex);

				existing.reserve(mcmStructure.size());
				for (const auto& entry : mcmStructure) {
					existing.insert(entry.modID);
				}
			}

			for (const auto& modName : structure) {
				if (existing.contains(modName)) continue;
				ModCore::JSONHandler::CreateNewEntry(modName);
			}
		}

		static MCMEntry* GetModEntryFromModID(const std::string& modID, const bool ext = false)
		{
			std::shared_lock lock(g_mcmStructureMutex);
			if (modID.empty()) return nullptr;

			const bool hasColon = modID.contains("::");

			for (auto& entry : mcmStructure) {
				if (entry.modID.empty()) continue;
				
				const std::string& entryID = hasColon ? entry.modID : entry.modID.substr(entry.modID.rfind("::") + 2);
				if (entryID == modID) return &entry;
			}

			if (ext) {
				for (auto& entry : mcmStructure) {
					if (entry.modName.empty()) continue;
					if (entry.modName == modID) return &entry;
				}
			}

			return nullptr;
		}

		static MCMEntry* CreateNewEntry(const std::string& modID)
		{
			if (modID.empty()) return nullptr;

			{
				std::unique_lock lock(g_mcmStructureMutex);

				for (auto& entry : mcmStructure) {
					if (entry.modID != modID) continue;
					
					TRACE("CreateNewEntry: MCM entry already exists for '{}'", modID);
					return &entry;
				}

				MCMEntry entry;
				entry.modID = modID;
				entry.modName = MiscUtils::GetSKSETranslation(modID);

				mcmStructure.push_back(std::move(entry));
				TRACE("CreateNewEntry: Created empty MCM entry for '{}'", modID);
			}

			SaveStructureToJSON();

			return &mcmStructure.back();
		}

		static bool SetModName(const std::string& modID, const std::string& newName)
		{
			if (modID.empty() || newName.empty()) return false;

			bool modified = false;
			{
				std::unique_lock lock(g_mcmStructureMutex);

				for (auto& entry : mcmStructure) {
					if (entry.modID != modID) continue;

					entry.modName = newName;

					TRACE("SetModName: Updated mod name for '{}' => '{}'", modID, newName);
					modified = true;
					break;
				}
			}

			if (modified) SaveStructureToJSON();

			return modified;
		}

		static bool SetModPosition(const std::string& modID, int newPosition)
		{
			if (modID.empty() || newPosition < 0) return false;

			bool modified = false;
			{
				std::unique_lock lock(g_mcmStructureMutex);

				for (auto& entry : mcmStructure) {
					if (entry.modID != modID) continue;

					entry.modPosition = newPosition;

					TRACE("Updated mod position for '{}' -> {}", modID, newPosition);
					modified = true;
					break;
				}
			}

			if (modified) SaveStructureToJSON();

			return modified;
		}

		static void ResetToDefault()
		{
			{
				std::unique_lock lock(g_mcmStructureMutex);
				mcmStructure.clear();
			}

			if (std::filesystem::exists(jsonPath)) {
				std::filesystem::remove(jsonPath);
				logger::info("ResetToDefault: JSON file deleted '{}'", jsonPath);
			}

			ParseJSONToStructure();
			logger::info("ResetToDefault: mcmStructure regenerated");
		}

		static void ParseJSONToStructure()
		{
			mcmStructure.clear();

			if (!std::filesystem::exists(jsonPath)) {
				logger::warn("ParseJSONToStructure: MCM JSON file not found: {}", jsonPath);
				return;
			}

			try {
				std::ifstream fileStream(jsonPath);

				if (!fileStream.is_open()) {
					logger::error("ParseJSONToStructure: Failed to open MCM JSON file: {}", jsonPath);
					return;
				}

				json j = json::parse(fileStream);

				for (const auto& [modID, modData] : j.items()) {
					MCMEntry entry;

					entry.modID = modID;

					if (modData.contains("Name") && modData["Name"].is_string()) {
						entry.modName = modData["Name"].get<std::string>();
					}

					if (modData.contains("Position") && modData["Position"].is_number_integer()) {
						entry.modPosition = modData["Position"].get<int>();
					}

					if (modData.contains("Disabled") && modData["Disabled"].is_boolean()) {
						entry.disabled = modData["Disabled"].get<bool>();
					}

					mcmStructure.push_back(std::move(entry));
				}

				logger::info("ParseJSONToStructure: Loaded {} MCM entries from JSON", mcmStructure.size());

			} catch (const std::exception& e) {
				logger::error("ParseJSONToStructure: Failed to parse MCM JSON '{}': {}", jsonPath, e.what());
			}
		}

		static bool SaveStructureToJSON()
		{
			std::unique_lock lock(g_mcmStructureMutex);

			try {
				json j;

				for (const auto& entry : mcmStructure) {
					if (entry.modID.empty()) continue;

					j[entry.modID] = {
						{ "Name", entry.modName },
						{ "Position", entry.modPosition },
						{ "Disabled", entry.disabled }
					};
				}

				std::ofstream out(jsonPath, std::ios::binary | std::ios::trunc);

				if (!out.is_open()) {
					logger::error("Failed to open JSON file for writing: {}", jsonPath);
					return false;
				}

				out << j.dump(4);

				TRACE("Successfully saved {} MCM entries to JSON", mcmStructure.size());

				return true;

			} catch (const std::exception& e) {
				logger::error("Failed to save JSON '{}': {}", jsonPath, e.what());
			}

			return false;
		}
	};
}
