#pragma once

#include "DataHandler.hpp"

#include "Core/JSONHandler.hpp"
#include "Core/Serialization.hpp"

#include "Utils/MiscUtils.hpp"

namespace ModCore
{
	class Main
	{
	public:
		static int GetConfigCount()
		{
			return Serialization::Functions::GetConfigCount();
		}

		static RE::TESObjectREFR* RegisterMarker(std::string modID)
		{
			if (modID.empty()) return nullptr;

			if (IsModNameRegistered(modID)) {
				TRACE("RegisterMarker: mod \"{}\" already registered", modID);
				return nullptr;
			}

			auto* marker = CreateMCMMarker();
			if (!marker) {
				TRACE("RegisterMarker: failed to create marker");
				return nullptr;
			}

			auto handle = MiscUtils::GetVMHandle(marker);
			if (!handle) {
				TRACE("RegisterMarker: failed to get VM handle for marker \"{:08X}\"", marker->formID);
				return nullptr;
			}

			if (!Serialization::Functions::RegisterMarker(handle, modID)) {
				TRACE("RegisterMarker: serialization rejected marker \"{:08X}\" (race condition or duplicate)", marker->formID);
				RE::GarbageCollector::GetSingleton()->Add(marker, true);
				return nullptr;
			}

			TRACE("RegisterMarker: marker \"{:08X}\" registered successfully for mod \"{}\"", marker->formID, modID);

			JSONHandler::CreateNewEntry(modID);

			return marker;
		}

		static bool UnregisterMarker(std::string modID)
		{
			const bool result = Serialization::Functions::UnregisterMarker(modID);
			
			TRACE("UnregisterMarker: modID {} -> {}", modID, result ? "success" : "failed");
			
			return result;
		}

		static bool UnregisterAllMarkers()
		{
			Serialization::Functions::UnregisterAllMarkers();

			return true;
		}
		
		static std::string GetModIDFromConfigID(int configID)
		{
			auto modEntry = Serialization::Functions::GetModEntryFromIndex(static_cast<std::uint32_t>(configID));
			if (!modEntry) return "";

			TRACE("GetModIDFromConfigID for {} is {}", configID, modEntry->modID);

			return modEntry->modID;
		}

		static std::string GetModIDFromSelectedEntry()
		{
			auto modInfo = GetHoveredModInfo();
			if (!modInfo) return "";

			return GetModIDFromModName(modInfo->modID);
		}

		static void GetModIDFromSelectedEntryAsync(std::function<void(std::string)> callback)
		{
			SKSE::GetTaskInterface()->AddUITask([callback]() {
				const auto modID = GetModIDFromSelectedEntry();
				callback(std::move(modID));
			});
		}

		static std::string GetModIDFromModName(std::string modName)
		{
			auto* jsonEntry = JSONHandler::GetModEntryFromModID(modName, true);

			return (jsonEntry && !jsonEntry->modID.empty()) ? jsonEntry->modID : modName;
		}

		static std::string GetModNameFromModID(std::string modID)
		{
			auto* jsonEntry = JSONHandler::GetModEntryFromModID(modID, true);

			return (jsonEntry && !jsonEntry->modName.empty()) ? jsonEntry->modName : modID;
		}

		static RE::TESObjectREFR* GetMarkerFromModID(std::string modID)
		{
			auto* entry = Serialization::Functions::GetModEntryFromModID(modID);
			if (!entry) return nullptr;

			return MiscUtils::ResolveVMHandle(entry->markerHandle);
		}

		static bool SetModNameFromModID(std::string modID, std::string newName)
		{
			return JSONHandler::SetModName(modID, newName);
		}

		static void UpdateMenuModNames()
		{
			SKSE::GetTaskInterface()->AddUITask([]() {
				auto* movie = MiscUtils::GetJournalMovie();
				if (!movie) return;

				std::vector<std::string> strings;
				std::vector<RE::GFxValue> args;
				{
					std::shared_lock lock(Serialization::g_configMapMutex);
					const std::size_t count = Serialization::g_configEntries.size();
				
					strings.reserve(count);
					args.reserve(count);
				
					for (std::size_t i = 0; i < count; ++i) {
						if (auto* runtimeEntry = Serialization::Functions::GetModEntryFromIndex(static_cast<std::uint32_t>(i))) {
							strings.push_back(runtimeEntry->modID);
						} else {
							strings.push_back("Unknown Mod");
						}
						args.emplace_back(strings.back().c_str());
					}
				}

				movie->Invoke("_root.ConfigPanelFader.configPanel.setModNames", nullptr, args.data(), static_cast<int>(args.size()));

				auto panel = MiscUtils::GetConfigPanelGfx();
				if (!panel.IsObject()) return;

				RE::GFxValue modList;
				panel.GetMember("_modList", &modList);
				if (!modList.IsObject()) return;

				RE::GFxValue entryList;
				modList.GetMember("entryList", &entryList);
				if (!entryList.IsArray()) return;

				const uint32_t size = entryList.GetArraySize();
				if (size == 0) return;

				for (uint32_t i = 0; i < size; ++i) {
					RE::GFxValue entry;
					entryList.GetElement(i, &entry);
					if (!entry.IsObject()) continue;

					RE::GFxValue modNameVal;
					entry.GetMember("modName", &modNameVal);
					if (!modNameVal.IsString()) continue;

					const std::string modID = modNameVal.GetString();
					auto* jsonEntry = JSONHandler::GetModEntryFromModID(modID);
					if (!jsonEntry) continue;

					std::string displayText = !jsonEntry->modName.empty() ? jsonEntry->modName : MiscUtils::GetSKSETranslation(modID);

					RE::GFxValue textVal(displayText.c_str());
					entry.SetMember("text", textVal);
					entryList.SetElement(i, entry);

					TRACE("UpdateMenuModNames: [{}] modID={} -> \"{}\"", i, modID, displayText);
				}

				modList.Invoke("InvalidateData", nullptr, nullptr, 0);

				RebuildMCMStructurePositions();
				ApplyCustomSort();
			});
		}

	private:

		static bool IsModNameRegistered(const std::string& modName)
		{
			return Serialization::Functions::GetModEntryFromModID(modName) != nullptr;
		}

		static RE::TESObjectREFR* CreateMCMMarker(RE::NiPoint3 a_position = {}, RE::NiPoint3 a_angle = {})
		{
			if (!ModData::MCMMarkerCell) return nullptr;

			const auto handle = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(
				ModData::MCMMarkerBase->As<RE::TESBoundObject>(), a_position, a_angle, ModData::MCMMarkerCell,
				nullptr, nullptr, nullptr, RE::ObjectRefHandle(), true, true);

			const auto handlePtr = handle.get();
			auto* ref = (handlePtr && handlePtr.get() ? handlePtr.get() : nullptr);

			TRACE("MCM Marker created in cell \"{:08X}\" with formID \"{:08X}\"", ModData::MCMMarkerCell->formID, ref ? ref->formID : 0x0);

			return ref;
		}

		struct HoveredModInfo
		{
			int index;
			std::string modID;
			std::string modName;
		};

		static std::optional<HoveredModInfo> GetHoveredModInfo()
		{
			auto panel = MiscUtils::GetConfigPanelGfx();
			if (!panel.IsObject()) return std::nullopt;

			RE::GFxValue modList;
			panel.GetMember("_modList", &modList);
			if (!modList.IsObject()) return std::nullopt;

			RE::GFxValue entryList;
			modList.GetMember("entryList", &entryList);
			if (!entryList.IsArray()) return std::nullopt;

			// Method 1 : selectedIndex
			RE::GFxValue indexVal;
			modList.GetMember("selectedIndex", &indexVal);
			if (indexVal.IsNumber()) {
				int index = static_cast<int>(indexVal.GetNumber());
				if (index >= 0) {
					RE::GFxValue entry;
					entryList.GetElement(static_cast<uint32_t>(index), &entry);
					if (entry.IsObject()) {
						RE::GFxValue modIDVal;
						entry.GetMember("modName", &modIDVal);
						if (modIDVal.IsString()) {
							RE::GFxValue modNameVal;
							entry.GetMember("text", &modNameVal);
							
							HoveredModInfo result;
							result.index = index;
							result.modID = modIDVal.GetString();
							result.modName = modNameVal.IsString() ? modNameVal.GetString() : result.modID;
							return result;
						}
					}
				}
			}

			// Method 2 : hitTest with cursor position
			RE::GFxValue root;
			panel.GetMember("_root", &root);
			RE::GFxValue xMouse, yMouse;
			if (root.IsObject()) {
				root.GetMember("_xmouse", &xMouse);
				root.GetMember("_ymouse", &yMouse);
			}
			if (!xMouse.IsNumber() || !yMouse.IsNumber()) return std::nullopt;

			RE::GFxValue hitArgs[3];
			hitArgs[0] = RE::GFxValue(xMouse.GetNumber());
			hitArgs[1] = RE::GFxValue(yMouse.GetNumber());
			hitArgs[2] = RE::GFxValue(true);

			const uint32_t size = entryList.GetArraySize();
			for (uint32_t i = 0; i < size; ++i)
			{
				RE::GFxValue entry;
				entryList.GetElement(i, &entry);
				if (!entry.IsObject()) continue;

				RE::GFxValue clipIndexVal;
				entry.GetMember("clipIndex", &clipIndexVal);
				if (clipIndexVal.GetType() != RE::GFxValue::ValueType::kNumber) continue;

				RE::GFxValue clipArgs[1];
				clipArgs[0] = RE::GFxValue(clipIndexVal.GetNumber());
				RE::GFxValue clip;
				modList.Invoke("getClipByIndex", &clip, clipArgs, 1);
				if (!clip.IsObject()) continue;

				RE::GFxValue hitResult;
				clip.Invoke("hitTest", &hitResult, hitArgs, 3);
				if (!hitResult.IsBool() || !hitResult.GetBool()) continue;

				RE::GFxValue modIDVal;
				entry.GetMember("modName", &modIDVal);
				if (!modIDVal.IsString()) continue;

				RE::GFxValue modNameVal;
				entry.GetMember("text", &modNameVal);
				
				HoveredModInfo result;
				result.index = static_cast<int>(i);
				result.modID = modIDVal.GetString();
				result.modName = modNameVal.IsString() ? modNameVal.GetString() : result.modID;
				return result;
			}

			return std::nullopt;
		}

		static void RebuildMCMStructurePositions()
		{
			auto toLower = [](std::string s) {
				std::transform(s.begin(), s.end(), s.begin(), ::tolower);
				return s;
			};

			auto panel = MiscUtils::GetConfigPanelGfx();
			if (!panel.IsObject()) return;

			RE::GFxValue modList;
			panel.GetMember("_modList", &modList);
			if (!modList.IsObject()) return;
			
			RE::GFxValue entryList;
			modList.GetMember("entryList", &entryList);
			if (!entryList.IsArray()) return;

			const uint32_t size = entryList.GetArraySize();

			std::vector<std::string> gfxTexts(size);
			for (uint32_t i = 0; i < size; ++i) {
				RE::GFxValue entry;
				entryList.GetElement(i, &entry);
				
				RE::GFxValue text;
				entry.GetMember("text", &text);

				gfxTexts[i] = text.IsString() ? MiscUtils::GetSKSETranslation(text.GetString()) : "";
			}

			std::vector<MCMEntry> snapshot;
			{
				std::shared_lock lock(g_mcmStructureMutex);
				snapshot = mcmStructure;
			}

			auto findMCMEntry = [&](const std::string& text) -> const MCMEntry* {
				for (const auto& e : snapshot)  {
					const std::string& key = e.modName.empty() ? e.modID : e.modName;
					if (key == text) return &e;
				}
				return nullptr;
			};

			struct SortableText {
				std::string text;
				int position;
			};

			std::vector<SortableText> positioned;
			std::vector<SortableText> floating;

			for (const auto& text : gfxTexts) {
				const MCMEntry* mcm = findMCMEntry(text);
				int pos = (mcm && mcm->modPosition >= 0) ? mcm->modPosition : -1;
				if (pos >= 0) positioned.push_back({ text, pos });
				else floating.push_back({ text, -1 });
			}

			std::sort(positioned.begin(), positioned.end(), [](const SortableText& a, const SortableText& b) {
				return a.position < b.position;
			});
			std::sort(floating.begin(), floating.end(), [&](const SortableText& a, const SortableText& b) {
				return toLower(a.text) < toLower(b.text);
			});

			std::vector<std::string> mergedGFx;
			mergedGFx.reserve(size);

			const uint32_t nPos = static_cast<uint32_t>(positioned.size());
			const uint32_t nFloat = static_cast<uint32_t>(floating.size());

			uint32_t fIdx = 0;
			for (uint32_t p = 0; p < nPos; ++p) {
				while (fIdx < nFloat && toLower(floating[fIdx].text) < toLower(positioned[p].text)) {
					mergedGFx.push_back(floating[fIdx++].text);
				}
				mergedGFx.push_back(positioned[p].text);
			}

			while (fIdx < nFloat) {
				mergedGFx.push_back(floating[fIdx++].text);
			}

			std::vector<MCMEntry> absentEntries;
			for (const auto& e : snapshot) {
				const std::string& key = e.modName.empty() ? e.modID : e.modName;
				bool foundInGFx = std::find(gfxTexts.begin(), gfxTexts.end(), key) != gfxTexts.end();
				if (!foundInGFx) absentEntries.push_back(e);
			}
			std::sort(absentEntries.begin(), absentEntries.end(), [&](const MCMEntry& a, const MCMEntry& b) {
				if (a.modPosition >= 0 && b.modPosition >= 0) return a.modPosition < b.modPosition;
				if (a.modPosition >= 0) return true;
				if (b.modPosition >= 0) return false;
					
				const std::string& ka = a.modName.empty() ? a.modID : a.modName;
				const std::string& kb = b.modName.empty() ? b.modID : b.modName;
				return toLower(ka) < toLower(kb);
			});

			std::vector<std::string> finalOrder;
			finalOrder.reserve(mergedGFx.size() + absentEntries.size());

			uint32_t absentIdx = 0;
			const uint32_t nAbsent = static_cast<uint32_t>(absentEntries.size());

			for (uint32_t i = 0; i < static_cast<uint32_t>(mergedGFx.size()); ++i) {
				while (absentIdx < nAbsent) {
					int absentPos = absentEntries[absentIdx].modPosition;
					if (absentPos < 0 || absentPos <= static_cast<int>(finalOrder.size())) {
						finalOrder.push_back(absentEntries[absentIdx++].modName.empty()
							? absentEntries[absentIdx - 1].modID : absentEntries[absentIdx - 1].modName);
					} else break;
				}
				finalOrder.push_back(mergedGFx[i]);
			}

			while (absentIdx < nAbsent) {
				const auto& e = absentEntries[absentIdx++];
				finalOrder.push_back(e.modName.empty() ? e.modID : e.modName);
			}

			{
				std::unique_lock lock(g_mcmStructureMutex);

				for (uint32_t i = 0; i < static_cast<uint32_t>(finalOrder.size()); ++i) {
					const std::string& text = finalOrder[i];

					auto it = std::find_if(mcmStructure.begin(), mcmStructure.end(),
						[&](const MCMEntry& e) {
							const std::string& key = e.modName.empty() ? e.modID : e.modName;
							return key == text;
						});

					if (it != mcmStructure.end()) {
						it->modPosition = static_cast<int>(i);
					} else {
						MCMEntry newEntry;
						newEntry.modID = text;
						newEntry.modName = text;
						newEntry.modPosition = static_cast<int>(i);
						mcmStructure.push_back(newEntry);
					}
				}
			}

			JSONHandler::SaveStructureToJSON();
		}

		static void ApplyCustomSort()
		{
			auto toLower = [](std::string s) {
				std::transform(s.begin(), s.end(), s.begin(), ::tolower);
				return s;
			};

			auto panel = MiscUtils::GetConfigPanelGfx();
			if (!panel.IsObject()) return;
			
			RE::GFxValue modList;
			panel.GetMember("_modList", &modList);
			if (!modList.IsObject()) return;

			RE::GFxValue entryList;
			modList.GetMember("entryList", &entryList);
			if (!entryList.IsArray()) return;
			
			const uint32_t size = entryList.GetArraySize();
			std::vector<RE::GFxValue> entries(size);
			for (uint32_t i = 0; i < size; ++i) {
				entryList.GetElement(i, &entries[i]);
			}

			std::vector<MCMEntry> snapshot;
			{
				std::shared_lock lock(g_mcmStructureMutex);
				snapshot = mcmStructure;
			}

			auto getEntryText = [](RE::GFxValue& gfx) -> std::string {
				RE::GFxValue text;
				gfx.GetMember("text", &text);
				return text.IsString() ? std::string(text.GetString()) : "";
			};

			auto findEntry = [&](const std::string& text) -> const MCMEntry* {
				for (const auto& e : snapshot) {
					const std::string& key = e.modName.empty() ? e.modID : e.modName;
					if (key == text) return &e;
				}
				return nullptr;
			};

			std::sort(entries.begin(), entries.end(), [&](RE::GFxValue& a, RE::GFxValue& b) {
				const MCMEntry* entryA = findEntry(getEntryText(a));
				const MCMEntry* entryB = findEntry(getEntryText(b));

				const bool disabledA = entryA ? entryA->disabled : false;
				const bool disabledB = entryB ? entryB->disabled : false;

				if (disabledA != disabledB) return !disabledA;

				const int posA = entryA ? entryA->modPosition : -1;
				const int posB = entryB ? entryB->modPosition : -1;

				if (posA < 0 && posB < 0) return toLower(getEntryText(a)) < toLower(getEntryText(b));
				if (posA < 0) return false;
				if (posB < 0) return true;
				return posA < posB;
			});

			auto findMCMEntry = [&](const std::string& text) -> const MCMEntry* {
				for (const auto& e : snapshot)  {
					const std::string& key = e.modName.empty() ? e.modID : e.modName;
					if (key == text) return &e;
				}
				return nullptr;
			};

			for (uint32_t i = 0; i < size; ++i) {
				auto text = getEntryText(entries[i]);

				const MCMEntry* mcm = findMCMEntry(text);
				const bool disabled = mcm ? mcm->disabled : false;
				
				entries[i].SetMember("enabled", !disabled);
				entryList.SetElement(i, entries[i]);
			}

			modList.Invoke("InvalidateData", nullptr, nullptr, 0);
		}
	};
}
