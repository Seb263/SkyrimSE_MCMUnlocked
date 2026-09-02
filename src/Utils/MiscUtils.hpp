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

	static std::string NormalizeModID(std::string_view modID)
	{
		const auto pos = modID.rfind("::");

		return pos == std::string_view::npos ? std::string(modID) : std::string(modID.substr(pos + 2));
	}
	static std::string GetSKSETranslation(std::string_view key)
	{
		const std::string normalizedKey = NormalizeModID(key);
		
		std::string result;
		SKSE::Translation::Translate(normalizedKey, result);
		
		const auto first = result.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return normalizedKey;

		const auto last = result.find_last_not_of(" \t\r\n");
		result = result.substr(first, last - first + 1);
		
		return result.empty() ? normalizedKey : result;
	}

	static bool IsJournalMenuOpen()
	{
		auto* ui = RE::UI::GetSingleton();
		if (!ui) return false;

		return ui->IsMenuOpen(RE::JournalMenu::MENU_NAME);
	}
	
	static RE::GFxMovieView* GetJournalMovie()
	{
		auto* ui = RE::UI::GetSingleton();
		if (!ui) return nullptr;
		
		auto menu = ui->GetMenu(RE::JournalMenu::MENU_NAME);
		if (!menu) return nullptr;
		
		auto* movie = menu->uiMovie.get();
		return movie;
	}

	static RE::GFxValue GetConfigPanelGfx()
	{
		RE::GFxValue result;

		auto movie = GetJournalMovie();
		if (!movie) return result;

		movie->GetVariable(&result, "_root.ConfigPanelFader.configPanel");

		if (!result.IsObject()) result = RE::GFxValue();

		return result;
	}

	static bool IsConfigPanelActive()
	{
		auto panel = GetConfigPanelGfx();
		if (!panel.IsObject()) return false;

		RE::GFxValue stateVal;
		panel.GetMember("_state", &stateVal);
		if (!stateVal.IsNumber()) return false;
		if (static_cast<int>(stateVal.GetNumber()) != 0) return false;

		RE::GFxValue focusVal;
		panel.GetMember("_focus", &focusVal);
		if (!focusVal.IsNumber()) return false;
		const int focus = static_cast<int>(focusVal.GetNumber());
		if (focus != 0) return false;

		RE::GFxValue optionsList;
		panel.GetMember("_optionsList", &optionsList);
		if (!optionsList.IsObject()) return true;

		RE::GFxValue optionsVisible;
		optionsList.GetMember("_visible", &optionsVisible);
		if (optionsVisible.IsBool() && optionsVisible.GetBool()) return false;

		return true;
	}

	static void SetSearchWidgetDisabled(bool disabled)
	{
		auto panel = GetConfigPanelGfx();
		if (!panel.IsObject()) return;

		RE::GFxValue searchWidget;
		panel.GetMember("searchWidget", &searchWidget);
		if (!searchWidget.IsObject()) return;

		RE::GFxValue val;
		val.SetBoolean(disabled);
		searchWidget.SetMember("isDisabled", val);
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

	static void ShowMessageBox(const std::string& a_message, std::vector<std::string> a_buttons, std::function<void(unsigned int)> a_callback)
	{
		class MessageBoxCallback : public RE::IMessageBoxCallback
		{
			std::function<void(unsigned int)> _callback;
		public:
			~MessageBoxCallback() override {}
			MessageBoxCallback(std::function<void(unsigned int)> callback) : _callback(callback) {}
			void Run(std::uint8_t a_response) override {
				_callback(a_response);
			}
		};

		auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
		auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
		auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);
		auto* messagebox = factory->Create();

		RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback = RE::make_smart<MessageBoxCallback>(std::move(a_callback));
		messagebox->callback = messageCallback;
		messagebox->bodyText = a_message;
		for (auto& text : a_buttons) messagebox->buttonText.push_back(text.c_str());

		RE::MessageBoxMenu::QueueMessage(messagebox);
	}

	template <typename T>
	static std::optional<T> GetGameINISetting(const std::string& name, std::optional<T> defaultValue = std::nullopt)
	{
		auto settings = RE::INISettingCollection::GetSingleton();
		if (!settings) return defaultValue;

		if (auto* setting = settings->GetSetting(name.c_str())) {

			static auto getNative = [](RE::Setting* setting) -> std::optional<std::variant<bool, float, int, unsigned int, std::string>> {
				switch (setting->GetType()) {
					case RE::Setting::Type::kBool: return setting->GetBool();
					case RE::Setting::Type::kFloat: return setting->GetFloat();
					case RE::Setting::Type::kInteger: return setting->GetInteger();
					case RE::Setting::Type::kUnsignedInteger: return setting->GetUnsignedInteger();
					case RE::Setting::Type::kString: return std::string(setting->GetString());
					default: return std::nullopt;
				}
			};

			static auto convertTo = [](auto& nativeValue, auto defaultVal) -> T {
				try {
					if (!nativeValue.has_value()) return defaultVal;

					const auto& v = nativeValue.value();

					if constexpr (std::is_same_v<T, std::string>) {
						if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? "true" : "false";
						if (std::holds_alternative<float>(v)) return std::to_string(std::get<float>(v));
						if (std::holds_alternative<int>(v)) return std::to_string(std::get<int>(v));
						if (std::holds_alternative<unsigned int>(v)) return std::to_string(std::get<unsigned int>(v));
						if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
					}

					if constexpr (std::is_same_v<T, bool>) {
						if (std::holds_alternative<std::string>(v)) {
							std::string s = std::get<std::string>(v);
							std::transform(s.begin(), s.end(), s.begin(), ::tolower);
							if (s == "1" || s == "true" || s == "yes") return true;
							if (s == "0" || s == "false" || s == "no") return false;
							return defaultVal;
						}
						if (std::holds_alternative<float>(v)) return std::get<float>(v) != 0.0f;
						if (std::holds_alternative<int>(v)) return std::get<int>(v) != 0;
						if (std::holds_alternative<unsigned int>(v)) return std::get<unsigned int>(v) != 0;
					}

					if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
						if (std::holds_alternative<std::string>(v)) return static_cast<T>(std::stoi(std::get<std::string>(v)));
						if (std::holds_alternative<float>(v)) return static_cast<T>(std::get<float>(v));
						if (std::holds_alternative<int>(v)) return static_cast<T>(std::get<int>(v));
						if (std::holds_alternative<unsigned int>(v)) return static_cast<T>(std::get<unsigned int>(v));
					}

					if constexpr (std::is_floating_point_v<T>) {
						if (std::holds_alternative<std::string>(v)) return std::stof(std::get<std::string>(v));
						if (std::holds_alternative<float>(v)) return static_cast<T>(std::get<float>(v));
						if (std::holds_alternative<int>(v)) return static_cast<T>(std::get<int>(v));
						if (std::holds_alternative<unsigned int>(v)) return static_cast<T>(std::get<unsigned int>(v));
					}

					return defaultVal;
				} catch (...) {
					return defaultVal;
				}
			};

			auto nativeValue = getNative(setting);
			return convertTo(nativeValue, defaultValue.value_or(T{}));
		}

		return defaultValue;
	}
};
