#pragma once

#include "SettingsIni.hpp"

#include "Core/JSONHandler.hpp"

#include "Utils/MiscUtils.hpp"

namespace Serialization
{
    constexpr std::uint32_t coSaveId = std::byteswap('MCMU');
    constexpr std::uint32_t recordId = std::byteswap('MMAP');
    constexpr std::uint32_t mappingVersion = 3;

    struct MCMSerializationEntry
    {
        RE::VMHandle markerHandle = 0;
		std::string  modID = "";
    };

    inline std::vector<MCMSerializationEntry> g_configEntries;
    inline std::shared_mutex g_configMapMutex;

    class Functions
    {
    public:
        static void RegisterSerializationCallbacks()
        {
            auto* s = SKSE::GetSerializationInterface();
            s->SetUniqueID(coSaveId);
            s->SetSaveCallback(OnSave);
            s->SetLoadCallback(OnLoad);
            s->SetRevertCallback(OnRevert);
        }

        // ---------------------------------------------------------------------
        // Data accessors
        // ---------------------------------------------------------------------
       
		static int GetConfigCount()
        {
            std::shared_lock lock(g_configMapMutex);
            return static_cast<int>(g_configEntries.size());
        }

		static MCMSerializationEntry* GetModEntryFromIndex(std::uint32_t index)
		{
			std::shared_lock lock(g_configMapMutex);
			
			if (index >= g_configEntries.size()) return nullptr;

			return &g_configEntries[index];
		}
		
		static MCMSerializationEntry* GetModEntryFromModID(const std::string& modID)
		{
			std::shared_lock lock(g_configMapMutex);
			if (modID.empty()) return nullptr;

			const bool hasColon = modID.contains("::");

			for (auto& entry : g_configEntries) {
				const std::string& entryID = hasColon ? entry.modID : entry.modID.substr(entry.modID.rfind("::") + 2);
				if (entryID == modID) return &entry;
			}

			return nullptr;
		}

        static bool RegisterMarker(RE::VMHandle handle, const std::string& modID)
        {
            if (!handle || modID.empty()) return false;

			if (GetModEntryFromModID(modID) != nullptr) return false;

			std::unique_lock lock(g_configMapMutex);
            g_configEntries.push_back({ handle, modID });
           
			return true;
        }

		static bool UnregisterMarker(const std::string& modID)
		{
			std::unique_lock lock(g_configMapMutex);
			if (modID.empty()) return false;

			for (auto it = g_configEntries.begin(); it != g_configEntries.end(); ++it) {
				if (it->modID != modID) continue;

				if (auto* ref = MiscUtils::ResolveVMHandle(it->markerHandle)) {
					RE::GarbageCollector::GetSingleton()->Add(ref, true);
				}

				g_configEntries.erase(it);
				return true;
			}

			return false;
		}

		static void UnregisterAllMarkers()
		{
			std::unique_lock lock(g_configMapMutex);

			for (auto& entry : g_configEntries) {
				if (auto* ref = MiscUtils::ResolveVMHandle(entry.markerHandle)) {
					RE::GarbageCollector::GetSingleton()->Add(ref, true);
				}
			}

			TRACE("UnregisterAllMarkers: removed {} marker(s)", g_configEntries.size());

			g_configEntries.clear();
		}

    private:
       
		// ---------------------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------------------
        
		static void WriteString(SKSE::SerializationInterface* intfc, const std::string& str)
        {
            const std::uint32_t len = static_cast<std::uint32_t>(str.size());
            intfc->WriteRecordData(&len, sizeof(len));
            if (len > 0) intfc->WriteRecordData(str.data(), len);
        }

        static bool ReadString(SKSE::SerializationInterface* intfc, std::string& out)
        {
            std::uint32_t len = 0;
            if (!intfc->ReadRecordData(&len, sizeof(len))) return false;
            if (len == 0) { out.clear(); return true; }
            out.resize(len);
            return intfc->ReadRecordData(out.data(), len);
        }

        // ---------------------------------------------------------------------
        // SKSE callbacks
        // ---------------------------------------------------------------------
       
		static void OnSave(SKSE::SerializationInterface* intfc)
        {
            std::shared_lock lock(g_configMapMutex);
			if (!intfc->OpenRecord(recordId, mappingVersion)) {
                TRACE("OnSave: failed to open record");
                return;
            }

            const std::uint32_t count = static_cast<std::uint32_t>(g_configEntries.size());
            intfc->WriteRecordData(&count, sizeof(count));
            for (const auto& entry : g_configEntries) {
                intfc->WriteRecordData(&entry.markerHandle, sizeof(entry.markerHandle));
                WriteString(intfc, entry.modID);
            }

            TRACE("OnSave: {} entries saved", count);
        }

        static void OnLoad(SKSE::SerializationInterface* intfc)
        {
            std::unique_lock lock(g_configMapMutex);
            g_configEntries.clear();

            std::uint32_t type, version, length;
            while (intfc->GetNextRecordInfo(type, version, length)) {
                if (type != recordId) continue;

                std::uint32_t count = 0;
                if (!intfc->ReadRecordData(&count, sizeof(count))) {
                    TRACE("OnLoad: failed to read entry count");
                    return;
                }

                g_configEntries.reserve(count);
                for (std::uint32_t i = 0; i < count; ++i) {
                    RE::VMHandle rawHandle = 0;
                    if (!intfc->ReadRecordData(&rawHandle, sizeof(rawHandle))) {
                        TRACE("OnLoad: failed to read handle for entry {}", i);
                        continue;
                    }

					if (version != mappingVersion) continue;

					std::string modName;
                    if (!ReadString(intfc, modName)) {
                        TRACE("OnLoad: failed to read modName for entry {}", i);
                        continue;
                    }

                    RE::VMHandle newHandle = 0;
                    if (!intfc->ResolveHandle(rawHandle, newHandle)) {
                        TRACE("OnLoad: failed to resolve handle for entry {}", i);
                        continue;
                    }

                    g_configEntries.push_back({ newHandle, modName });
                    
					TRACE("OnLoad: entry {} \"{}\" restored with handle {:016X}", i, modName, static_cast<std::uint64_t>(newHandle));
                }
                TRACE("OnLoad: {} / {} entries restored", g_configEntries.size(), count);
            }

            if (SettingsIni::bGeneral_ResetMCMForOlderSaves && g_configEntries.empty()) MiscUtils::ResetMCMQuest();
			ModCore::JSONHandler::ValidateMCMStructure(GetRegisteredModIDs());
        }

        static void OnRevert(SKSE::SerializationInterface*)
        {
            std::unique_lock lock(g_configMapMutex);
            g_configEntries.clear();
        }

		static std::vector<std::string> GetRegisteredModIDs()
		{
			std::vector<std::string> modIDs;
			modIDs.reserve(g_configEntries.size());

			std::ranges::transform(g_configEntries, std::back_inserter(modIDs), [](const MCMSerializationEntry& entry) {
				return entry.modID;
			});

			return modIDs;
		}
    };
}
