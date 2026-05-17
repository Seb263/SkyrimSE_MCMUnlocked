#pragma once

#include "SettingsIni.hpp"

#include "Utils/MiscUtils.hpp"

namespace Serialization
{
    constexpr std::uint32_t coSaveId = std::byteswap('MCMU');
    constexpr std::uint32_t recordId = std::byteswap('MMAP');

    struct MCMEntry
    {
        RE::VMHandle markerHandle = 0;
        std::string  modName;
    };

    inline std::vector<MCMEntry> g_configEntries;
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

        static bool IsModNameRegistered(const std::string& modName)
        {
            std::shared_lock lock(g_configMapMutex);
            for (const auto& entry : g_configEntries) {
                if (entry.modName == modName) return true;
            }
            return false;
        }

        static bool RegisterMarker(RE::VMHandle handle, const std::string& modName)
        {
            if (!handle || modName.empty()) return false;
            std::unique_lock lock(g_configMapMutex);

			for (const auto& entry : g_configEntries) {
                if (entry.modName == modName) {
                    TRACE("RegisterMarker: mod \"{}\" already registered (double-check under lock)", modName);
                    return false;
                }
            }

            g_configEntries.push_back({ handle, modName });
            return true;
        }

        static bool UnregisterMarker(std::uint32_t a_index)
        {
            std::unique_lock lock(g_configMapMutex);
            if (a_index >= g_configEntries.size()) return false;
            auto& entry = g_configEntries[a_index];
            auto* ref = MiscUtils::ResolveVMHandle(entry.markerHandle);
            if (ref) RE::GarbageCollector::GetSingleton()->Add(ref, true);
            g_configEntries.erase(g_configEntries.begin() + a_index);
            return true;
        }

        static RE::TESObjectREFR* GetMarkerFromIndex(std::uint32_t a_index)
        {
            std::shared_lock lock(g_configMapMutex);
            if (a_index >= g_configEntries.size()) return nullptr;
            return MiscUtils::ResolveVMHandle(g_configEntries[a_index].markerHandle);
        }

        static std::string GetModName(std::uint32_t a_index)
        {
            std::shared_lock lock(g_configMapMutex);
            if (a_index >= g_configEntries.size()) return "";
            return g_configEntries[a_index].modName;
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
            if (!intfc->OpenRecord(recordId, 2)) {
                TRACE("OnSave: failed to open record");
                return;
            }

            const std::uint32_t count = static_cast<std::uint32_t>(g_configEntries.size());
            intfc->WriteRecordData(&count, sizeof(count));
            for (const auto& entry : g_configEntries) {
                intfc->WriteRecordData(&entry.markerHandle, sizeof(entry.markerHandle));
                WriteString(intfc, entry.modName);
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

                    std::string modName;
                    if (version >= 2) {
                        if (!ReadString(intfc, modName)) {
                            TRACE("OnLoad: failed to read modName for entry {}", i);
                            continue;
                        }
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
        }

        static void OnRevert(SKSE::SerializationInterface*)
        {
            std::unique_lock lock(g_configMapMutex);
            g_configEntries.clear();
        }
    };
}
