#pragma once

#include "DataHandler.hpp"

#include "Core/Main.hpp"

#include "Utils/MiscUtils.hpp"

namespace Papyrus
{
	std::vector<uint32_t> GetVersion(RE::StaticFunctionTag*)
	{
		using namespace SKSE;
		const auto* plugin = PluginDeclaration::GetSingleton();
		std::vector<uint32_t> versionVector;
		
		versionVector.push_back(plugin->GetVersion().major());
		versionVector.push_back(plugin->GetVersion().minor());
		versionVector.push_back(plugin->GetVersion().patch());
		
		return versionVector;
	}

	int GetConfigCount(RE::StaticFunctionTag*)
	{
		if (!ModData::modStatus) return 0;

		return ModCore::Main::GetConfigCount();
	}

	RE::TESObjectREFR* RegisterMarker(RE::BSScript::Internal::VirtualMachine* a_vm, RE::VMStackID a_stackID,
		RE::StaticFunctionTag*, RE::BSFixedString a_modID, RE::TESQuest* a_menu)
	{
		if (!ModData::modStatus || a_modID.empty()) return nullptr;

		std::string scriptName = "Default";

		if (a_menu) {
			if (auto policy = a_vm->GetObjectHandlePolicy1()) {
				RE::BSTSmartPointer<RE::BSScript::Object> scriptObj;
				a_vm->FindBoundObject(policy->GetHandleForObject(a_menu->GetFormType(), a_menu), "SKI_ConfigBase", scriptObj);
				if (scriptObj) scriptName = scriptObj->GetTypeInfo()->GetName();
			}
		}

		std::string finalModID = a_modID.c_str();
		if (!finalModID.contains("::")) finalModID = scriptName + "::" + finalModID;

		return ModCore::Main::RegisterMarker(finalModID);
	}

	bool UnregisterMarker(RE::BSScript::Internal::VirtualMachine* a_vm, RE::VMStackID a_stackID,
		RE::StaticFunctionTag*, RE::BSFixedString a_modID, RE::TESQuest* a_menu)
	{
		if (!ModData::modStatus || a_modID.empty()) return false;

		std::string scriptName = "Default";

		if (a_menu) {
			if (auto policy = a_vm->GetObjectHandlePolicy1()) {
				RE::BSTSmartPointer<RE::BSScript::Object> scriptObj;
				a_vm->FindBoundObject(policy->GetHandleForObject(a_menu->GetFormType(), a_menu), "SKI_ConfigBase", scriptObj);
				if (scriptObj) scriptName = scriptObj->GetTypeInfo()->GetName();
			}
		}

		std::string finalModID = a_modID.c_str();
		if (!finalModID.contains("::")) finalModID = scriptName + "::" + finalModID;

		return ModCore::Main::UnregisterMarker(finalModID);
	}
	
	bool UnregisterAllMarkers(RE::StaticFunctionTag*)
	{
		if (!ModData::modStatus) return false;

		return ModCore::Main::UnregisterAllMarkers();
	}
	
	RE::BSFixedString GetModIDFromConfigID(RE::StaticFunctionTag*, int a_configID)
	{
		if (!ModData::modStatus) return "";

		return RE::BSFixedString(ModCore::Main::GetModIDFromConfigID(a_configID));
	}

	RE::BSScript::LatentStatus GetModIDFromSelectedEntry(RE::BSScript::Internal::VirtualMachine* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*)
	{
		SKSE::GetTaskInterface()->AddUITask([a_vm, a_stackID]() {
			const auto result = RE::BSFixedString(ModCore::Main::GetModIDFromSelectedEntry());

			a_vm->ReturnLatentResult(a_stackID, result);
		});

		return RE::BSScript::LatentStatus::kStarted;
	}

	RE::BSFixedString GetModNameFromModID(RE::StaticFunctionTag*, RE::BSFixedString a_modID)
	{
		if (!ModData::modStatus) return "";

		return RE::BSFixedString(ModCore::Main::GetModNameFromModID(a_modID.c_str()));
	}
	
	RE::TESObjectREFR* GetMarkerFromModID(RE::StaticFunctionTag*, RE::BSFixedString a_modID)
	{
		if (!ModData::modStatus) return nullptr;

		return ModCore::Main::GetMarkerFromModID(a_modID.c_str());
	}
	
	void UpdateMenuModNames(RE::StaticFunctionTag*)
	{
		if (!ModData::modStatus) return;

		ModCore::Main::UpdateMenuModNames();
	}

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm)
	{
		vm->RegisterFunction("GetVersion", "MCMUnlocked", GetVersion);
		vm->RegisterFunction("GetConfigCount", "MCMUnlocked", GetConfigCount);
		vm->RegisterFunction("RegisterMarker", "MCMUnlocked", RegisterMarker, true);
		vm->RegisterFunction("UnregisterMarker", "MCMUnlocked", UnregisterMarker, true);
		vm->RegisterFunction("UnregisterAllMarkers", "MCMUnlocked", UnregisterAllMarkers);
		vm->RegisterFunction("GetModIDFromConfigID", "MCMUnlocked", GetModIDFromConfigID);
		vm->RegisterLatentFunction<RE::BSFixedString>("GetModIDFromSelectedEntry", "MCMUnlocked", GetModIDFromSelectedEntry);
		vm->RegisterFunction("GetModNameFromModID", "MCMUnlocked", GetModNameFromModID);
		vm->RegisterFunction("GetMarkerFromModID", "MCMUnlocked", GetMarkerFromModID);
		vm->RegisterFunction("UpdateMenuModNames", "MCMUnlocked", UpdateMenuModNames);

		// NOTE : NativeLatentFunction.h needs to be patched :
		// From "this->_retType = GetRawType<latentR>()" to "this->_retType = GetRawType<latentR>()()"

		return true;
	}
}
