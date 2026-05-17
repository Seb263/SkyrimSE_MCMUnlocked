#pragma once

#include "DataHandler.hpp"

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

		static RE::TESObjectREFR* RegisterMarker(std::string modName)
		{
			if (modName.empty()) return nullptr;

			if (Serialization::Functions::IsModNameRegistered(modName)) {
				TRACE("RegisterMarker: mod \"{}\" already registered", modName);
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

			if (!Serialization::Functions::RegisterMarker(handle, modName)) {
				TRACE("RegisterMarker: serialization rejected marker \"{:08X}\" (race condition or duplicate)", marker->formID);
				RE::GarbageCollector::GetSingleton()->Add(marker, true);
				return nullptr;
			}

			TRACE("RegisterMarker: marker \"{:08X}\" registered successfully for mod \"{}\"", marker->formID, modName);
			return marker;
		}

		static std::string GetModNameFromConfigID(int a_configID)
		{
			if (a_configID < 0) return "";
			
			const std::string name = Serialization::Functions::GetModName(static_cast<std::uint32_t>(a_configID));
			TRACE("GetModNameFromConfigID: configID {} -> \"{}\"", a_configID, name);
			
			return name;
		}

		static bool UnregisterMarker(int a_configID)
		{
			const bool result = Serialization::Functions::UnregisterMarker(static_cast<std::uint32_t>(a_configID));
			TRACE("UnregisterMarker: configID {} -> {}", a_configID, result ? "success" : "failed");
			return result;
		}

		static RE::TESObjectREFR* GetMarkerFromIndex(int a_configID)
		{
			return Serialization::Functions::GetMarkerFromIndex(static_cast<std::uint32_t>(a_configID));
		}

		static void UpdateMenuModNames(std::string a_menuName, std::string a_target)
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui) return;

			auto movie = ui->GetMovieView(a_menuName);
			if (!movie) return;

			std::shared_lock lock(Serialization::g_configMapMutex);
			const std::size_t count = Serialization::g_configEntries.size();

			std::vector<std::string> strings(count);
			std::vector<RE::GFxValue> args(count);

			for (std::size_t i = 0; i < count; ++i) {
				strings[i] = GetModNameFromConfigID(static_cast<std::uint32_t>(i));
				args[i].SetString(strings[i].c_str());
			}

			TRACE("UpdateMenuModNames: sending {} name(s) to \"{}\":", count, a_target);
			for (std::size_t i = 0; i < count; ++i) {
				TRACE("  -> [{}] \"{}\"", i, strings[i]);
			}

			RE::GFxValue result;
			movie->Invoke(a_target.c_str(), &result, args.data(), static_cast<int>(count));
		}

	private:

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
	};
}
