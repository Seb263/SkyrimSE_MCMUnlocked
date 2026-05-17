#pragma once

#include "DataHandler.hpp"

#include "Core/Main.hpp"

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
		return ModCore::Main::GetConfigCount();
	}

	RE::TESObjectREFR* RegisterMarker(RE::StaticFunctionTag*, RE::BSFixedString a_modName)
	{
		return ModCore::Main::RegisterMarker(a_modName.c_str());
	}

	bool UnregisterMarker(RE::StaticFunctionTag*, int a_configID)
	{
		return ModCore::Main::UnregisterMarker(a_configID);
	}

	RE::TESObjectREFR* GetMarkerFromIndex(RE::StaticFunctionTag*, int a_configID)
	{
		return ModCore::Main::GetMarkerFromIndex(a_configID);
	}
	
	void UpdateMenuModNames(RE::StaticFunctionTag*, RE::BSFixedString a_menuName, RE::BSFixedString a_target)
	{
		ModCore::Main::UpdateMenuModNames(a_menuName.c_str(), a_target.c_str());
	}

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm)
	{
		vm->RegisterFunction("GetVersion", "MCMUnlocked", GetVersion);
		vm->RegisterFunction("GetConfigCount", "MCMUnlocked", GetConfigCount);
		vm->RegisterFunction("RegisterMarker", "MCMUnlocked", RegisterMarker);
		vm->RegisterFunction("UnregisterMarker", "MCMUnlocked", UnregisterMarker);
		vm->RegisterFunction("GetMarkerFromIndex", "MCMUnlocked", GetMarkerFromIndex);
		vm->RegisterFunction("UpdateMenuModNames", "MCMUnlocked", UpdateMenuModNames);
		return true;
	}
}
