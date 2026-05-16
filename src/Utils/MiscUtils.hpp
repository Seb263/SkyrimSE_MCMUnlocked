#pragma once

#include "DataHandler.hpp"

class MiscUtils
{
public:

	static RE::VMHandle GetVMHandle(RE::TESObjectREFR* a_ref)
	{
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto* policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		if (!policy) return 0;

		return policy->GetHandleForObject(RE::FormType::Reference, a_ref);
	}

	static RE::TESObjectREFR* ResolveVMHandle(RE::VMHandle a_handle)
	{
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto* policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		if (!policy) return nullptr;
		
		auto* form = policy->GetObjectForHandle(RE::FormType::Reference, a_handle);
		return skyrim_cast<RE::TESObjectREFR*>(form);
	}

	static void ResetMCMQuest()
	{
		std::jthread([]() {
			std::this_thread::sleep_for(2s);

			SKSE::GetTaskInterface()->AddTask([]() {
				auto* scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
				if (!scriptFactory) return;

				auto* script = scriptFactory->Create();
				if (!script) return;

				script->SetCommand("SetStage SKI_ConfigManagerInstance 1");
				script->CompileAndRun(nullptr);

				delete script;
			});
		}).detach();
	}
};
