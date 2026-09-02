#include "Events.h"

namespace Events
{
	RE::BSEventNotifyControl Events::ModEventSink::ProcessEvent(const SKSE::ModCallbackEvent* event, RE::BSTEventSource<SKSE::ModCallbackEvent>*)
	{
		if (!event || !MiscUtils::IsJournalMenuOpen()) return continueEvent;

		if (event->eventName == "SKICP_inputAccepted") {
			std::string result = event->strArg.c_str();
			ModCore::Rename::RenameDone(result);
		} else if (event->eventName == "SKICP_menuAccepted") {
			int selectedIndex = static_cast<int>(event->numArg);
			ModCore::Reorder::ReorderDone(selectedIndex);
		}

		return continueEvent;
	}

	void Events::ModEventSink::ProcessInputHookTemplate(RE::BSTEventSource<RE::InputEvent*>* dispatcher, RE::InputEvent* const* eventList)
	{
		if (!dispatcher || !eventList || !*eventList || !MiscUtils::IsJournalMenuOpen() || !MiscUtils::IsConfigPanelActive()) {
			return _processInputHook(dispatcher, eventList);
		}

		bool ctrlHeld = false;
		for (auto event = *eventList; event; event = event->next) {
			auto buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || !buttonEvent->IsHeld() && !buttonEvent->IsDown()) continue;
			if (buttonEvent->GetDevice() != RE::INPUT_DEVICE::kKeyboard) continue;
			const auto id = buttonEvent->GetIDCode();
			if (id == 29 || id == 157) { // 29 = Left Ctrl, 157 = Right Ctrl (DIK codes)
				ctrlHeld = true;
				break;
			}
		}

		for (auto event = *eventList; event; event = event->next) {
			auto buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || !buttonEvent->IsDown()) continue;

			if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kKeyboard) {
				const auto id = buttonEvent->GetIDCode();
				if (ctrlHeld && id == SettingsIni::iInput_ResetKey) {
					ModCore::Reset::ResetStart();
					constexpr RE::InputEvent* const dummy[] = { nullptr };
					return _processInputHook(dispatcher, dummy);
				}
				continue;
			}

			if (buttonEvent->GetDevice() != RE::INPUT_DEVICE::kMouse) continue;
			const auto id = buttonEvent->GetIDCode();
			
			if (id == SettingsIni::iInput_RenameKey) ModCore::Rename::RenameStart();
			else if (id == SettingsIni::iInput_ReorderKey) ModCore::Reorder::ReorderStart();
			else continue;
			
			constexpr RE::InputEvent* const dummy[] = { nullptr };
			return _processInputHook(dispatcher, dummy);
		}

		_processInputHook(dispatcher, eventList);
	}
}
