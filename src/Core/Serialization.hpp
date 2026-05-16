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

		static bool RegisterMarker(RE::VMHandle handle)
		{
			if (!handle) return false;

			std::unique_lock lock(g_configMapMutex);

			g_configEntries.push_back({ handle });

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

    private:

        // ---------------------------------------------------------------------
        // SKSE callbacks
        // ---------------------------------------------------------------------

		static void OnSave(SKSE::SerializationInterface* intfc)
		{
			std::shared_lock lock(g_configMapMutex);
			if (!intfc->OpenRecord(recordId, 1)) {
				TRACE("OnSave: failed to open record");
				return;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(g_configEntries.size());
			intfc->WriteRecordData(&count, sizeof(count));
			for (const auto& entry : g_configEntries) {
				intfc->WriteRecordData(&entry, sizeof(MCMEntry));
			}

			TRACE("OnSave: {} entries saved", count);
		}

		static void OnLoad(SKSE::SerializationInterface* intfc)
		{
			std::unique_lock lock(g_configMapMutex);
			
			g_configEntries.clear();
			
			std::uint32_t type, version, length;
			while (intfc->GetNextRecordInfo(type, version, length)) {
				if (type != recordId || version != 1) continue;

				std::uint32_t count = 0;
				if (!intfc->ReadRecordData(&count, sizeof(count))) {
					TRACE("OnLoad: failed to read entry count");
					return;
				}

				g_configEntries.reserve(count);
				for (std::uint32_t i = 0; i < count; ++i) {
					MCMEntry entry;
					if (!intfc->ReadRecordData(&entry, sizeof(MCMEntry))) {
						TRACE("OnLoad: failed to read entry {}", i);
						continue;
					}

					RE::VMHandle newHandle = 0;
					if (!intfc->ResolveHandle(entry.markerHandle, newHandle)) {
						TRACE("OnLoad: failed to resolve handle for entry {}", i);
						continue;
					}
					entry.markerHandle = newHandle;
					g_configEntries.push_back(entry);
					
					TRACE("OnLoad: entry {} restored with handle {:016X}", i, static_cast<std::uint64_t>(newHandle));
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
