#pragma once

#include "DataHandler.hpp"

namespace SettingsIni
{
	// General
	inline int  iGeneral_VerboseMode = 1;
	inline bool bGeneral_ResetMCMForOlderSaves = true;
	inline bool bGeneral_OverwriteInvalidScripts = true;
	inline bool bGeneral_ExtractScriptSources = false;

	// Input bindings (DIK codes)
	inline int iInput_ResetKey = 19; // R
	inline int iInput_RenameKey = 1; // Mouse right
	inline int iInput_ReorderKey = 2; // Mouse Middle

	// Language
	using LanguageMap = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;
	static inline LanguageMap languageSettingsDefault = {
		{ "RenameMenu", {
			{ "english",  "Rename: %s" },
			{ "french",   "Renommer : %s" },
			{ "german",   "Umbenennen: %s" },
			{ "spanish",  "Renombrar: %s" },
			{ "italian",  "Rinomina: %s" },
			{ "polish",   "Zmień nazwę: %s" },
			{ "russian",  "Переименовать: %s" },
			{ "japanese", "名前変更: %s" },
			{ "chinese",  "重命名：%s" }
		}},

		{ "AutoSort", {
			{ "english",  "+ Auto sort" },
			{ "french",   "+ Trier automatiquement" },
			{ "german",   "+ Automatisch sortieren" },
			{ "spanish",  "+ Ordenar automáticamente" },
			{ "italian",  "+ Ordina automaticamente" },
			{ "polish",   "+ Sortuj automatycznie" },
			{ "russian",  "+ Автоматическая сортировка" },
			{ "japanese", "+ 自動並び替え" },
			{ "chinese",  "+ 自动排序" }
		}},

		{ "DisableMenu", {
			{ "english",  "+ Disable menu" },
			{ "french",   "+ Désactiver le menu" },
			{ "german",   "+ Menü deaktivieren" },
			{ "spanish",  "+ Desactivar menú" },
			{ "italian",  "+ Disattiva menu" },
			{ "polish",   "+ Wyłącz menu" },
			{ "russian",  "+ Отключить меню" },
			{ "japanese", "+ メニューを無効化" },
			{ "chinese",  "+ 禁用菜单" }
		}},

		{ "PlaceFirst", {
			{ "english",  "> Place first" },
			{ "french",   "> Placer en premier" },
			{ "german",   "> An erste Stelle setzen" },
			{ "spanish",  "> Colocar primero" },
			{ "italian",  "> Metti per primo" },
			{ "polish",   "> Umieść na początku" },
			{ "russian",  "> Поместить первым" },
			{ "japanese", "> 最初に配置" },
			{ "chinese",  "> 放在最前面" }
		}},

		{ "PlaceAfter", {
			{ "english",  "> Place after: %s" },
			{ "french",   "> Placer après : %s" },
			{ "german",   "> Platzieren nach: %s" },
			{ "spanish",  "> Colocar después de: %s" },
			{ "italian",  "> Posiziona dopo: %s" },
			{ "polish",   "> Umieść po: %s" },
			{ "russian",  "> Поместить после: %s" },
			{ "japanese", "> 次の後に配置: %s" },
			{ "chinese",  "> 放在之后：%s" }
		}},

		{ "ResetWarning", {
			{ "english",  "Choose whether to fully reset all MCM settings or simply rebuild menu instance indexing while preserving your personal settings." },
			{ "french",   "Choisissez si vous souhaitez réinitialiser entièrement tous les paramètres du MCM ou simplement reconstruire l'indexation des instances tout en conservant vos réglages personnels." },
			{ "german",   "Wählen Sie, ob Sie alle MCM-Einstellungen vollständig zurücksetzen oder nur die Instanzindizierung neu erstellen und dabei Ihre persönlichen Einstellungen beibehalten möchten." },
			{ "spanish",  "Elija si desea restablecer completamente todos los ajustes del MCM o simplemente reconstruir la indexación de instancias conservando sus ajustes personales." },
			{ "italian",  "Scegli se reimpostare completamente tutte le impostazioni MCM oppure ricostruire solo l'indicizzazione delle istanze mantenendo le impostazioni personali." },
			{ "polish",   "Wybierz, czy chcesz całkowicie zresetować wszystkie ustawienia MCM, czy tylko odbudować indeksowanie instancji z zachowaniem ustawień osobistych." },
			{ "russian",  "Выберите, хотите ли вы полностью сбросить все настройки MCM или только перестроить индексацию экземпляров, сохранив личные настройки." },
			{ "japanese", "すべてのMCM設定を完全にリセットするか、個人設定を保持したままインスタンスのインデックスのみを再構築するか選択してください。" },
			{ "chinese",  "请选择是完全重置所有 MCM 设置，还是仅重建实例索引并保留个人设置。" }
		}},

		{ "ResetWarningAll", {
			{ "english",  "Full reset" },
			{ "french",   "Réinitialisation complète" },
			{ "german",   "Vollständiger Reset" },
			{ "spanish",  "Restablecimiento completo" },
			{ "italian",  "Reimpostazione completa" },
			{ "polish",   "Pełny reset" },
			{ "russian",  "Полный сброс" },
			{ "japanese", "完全リセット" },
			{ "chinese",  "完全重置" }
		}},

		{ "ResetWarningMarkers", {
			{ "english",  "Rebuild indexing" },
			{ "french",   "Reconstruire l'indexation" },
			{ "german",   "Indizierung neu erstellen" },
			{ "spanish",  "Reconstruir indexación" },
			{ "italian",  "Ricostruisci indicizzazione" },
			{ "polish",   "Odbuduj indeksowanie" },
			{ "russian",  "Перестроить индексацию" },
			{ "japanese", "インデックスを再構築" },
			{ "chinese",  "重建索引" }
		}}
	};

	inline LanguageMap languageSettings = languageSettingsDefault;

	class SettingsManager
	{
	public:
		SettingsManager()
		{
			bindings = {
				// General
				{ "General", "iVerboseMode", &iGeneral_VerboseMode },
				{ "General", "bResetMCMForOlderSaves", &bGeneral_ResetMCMForOlderSaves },
				{ "General", "bOverwriteInvalidScripts", &bGeneral_OverwriteInvalidScripts },
				{ "General", "bExtractScriptSources", &bGeneral_ExtractScriptSources },

				// Input
				{ "Input", "iResetKey", &iInput_ResetKey },
				{ "Input", "iRenameKey", &iInput_RenameKey },
				{ "Input", "iReorderKey", &iInput_ReorderKey }
			};
		}

		bool ReadSettings()
		{
			std::wstring   wpath_str(path.begin(), path.end());
			const wchar_t* wpath = wpath_str.c_str();

			bool readStatus = false;

			logger::info("Trying to read INI file at path: {}", path);

			if (std::filesystem::exists(path)) {
				CSimpleIniA ini;
				ini.SetUnicode();

				if (ini.LoadFile(wpath) >= 0) {
					for (const auto& bind : bindings) {
						std::visit([&](auto* ptr) {
							using T = std::decay_t<decltype(*ptr)>;
							if constexpr (std::is_same_v<T, bool>) {
								*ptr = ini.GetBoolValue(bind.section, bind.key, *ptr);
							} else if constexpr (std::is_same_v<T, int>) {
								*ptr = static_cast<int>(ini.GetLongValue(bind.section, bind.key, *ptr));
							} else if constexpr (std::is_same_v<T, float>) {
								*ptr = static_cast<float>(ini.GetDoubleValue(bind.section, bind.key, *ptr));
							} else if constexpr (std::is_same_v<T, std::string>) {
								*ptr = ini.GetValue(bind.section, bind.key, ptr->c_str());
							}
						},
							bind.var);
					}
					LoadLanguageSettings(ini);
					readStatus = true;
				} else {
					logger::error("Failed to load INI file at {}", path);
				}
			} else {
				logger::warn("INI file does not exist at {}", path);
			}

			// Clamping logic
			iGeneral_VerboseMode = std::clamp(iGeneral_VerboseMode, 0, 2);

			// External data
			[&]() {
				using namespace ModData;

				debugVerboseMode = iGeneral_VerboseMode;
			}();

			return readStatus;
		}

		void LoadLanguageSettings(const CSimpleIniA& ini)
		{
			languageSettings = languageSettingsDefault;

			CSimpleIniA::TNamesDepend keys;
			ini.GetAllKeys("Language", keys);

			for (auto& key : keys) {
				std::string keyStr = key.pItem;

				size_t pos = keyStr.rfind('_');
				if (pos == std::string::npos) continue;

				std::string variable = keyStr.substr(0, pos);
				std::string lang = keyStr.substr(pos + 1);
				std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

				const char* value = ini.GetValue("Language", keyStr.c_str(), nullptr);
				if (value && *value) languageSettings[variable][lang] = value;
			}
		}

		std::string GetLanguageValue(const std::string& variable, const std::string& lang, const std::string& defaultValue = "") const
		{
			std::string langLower = lang;
			std::transform(langLower.begin(), langLower.end(), langLower.begin(), ::tolower);

			auto varIt = languageSettings.find(variable);
			if (varIt == languageSettings.end()) return defaultValue;

			auto langIt = varIt->second.find(langLower);
			if (langIt == varIt->second.end()) return defaultValue;

			return langIt->second;
		}

		std::optional<std::variant<bool, int, float, std::string>> GetValueVariant(const std::string& key_section)
		{
			auto sep = key_section.rfind(':');
			if (sep == std::string::npos) {
				logger::error("GetValueVariant: Invalid key_section format: '{}'", key_section);
				return std::nullopt;
			}
			std::string section = key_section.substr(0, sep);
			std::string key     = key_section.substr(sep + 1);
			for (const auto& bind : bindings) {
				if (key == bind.key && section == bind.section) {
					if (auto v = std::get_if<bool*>        (&bind.var)) return **v;
					if (auto v = std::get_if<int*>         (&bind.var)) return **v;
					if (auto v = std::get_if<float*>       (&bind.var)) return **v;
					if (auto v = std::get_if<std::string*> (&bind.var)) return **v;
				}
			}
			return std::nullopt;
		}

		template <typename T>
		T GetValue(const std::string& key_section, const T& defaultValue = T{})
		{
			auto opt = GetValueVariant(key_section);
			if (!opt) {
				logger::error("GetValue: No binding found for '{}'", key_section);
				return defaultValue;
			}

			return std::visit([&](auto&& val) -> T {
				using V = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, std::string>) {
					if constexpr (std::is_same_v<V, std::string>) return val;
				} else if constexpr (std::is_same_v<T, bool>) {
					if constexpr (std::is_same_v<V, bool>)  return val;
					if constexpr (std::is_same_v<V, int>)   return val != 0;
					if constexpr (std::is_same_v<V, float>) return val != 0.0f;
				} else if constexpr (std::is_same_v<T, int>) {
					if constexpr (std::is_same_v<V, int>)   return val;
					if constexpr (std::is_same_v<V, float>) return static_cast<int>(val);
					if constexpr (std::is_same_v<V, bool>)  return val ? 1 : 0;
				} else if constexpr (std::is_same_v<T, float>) {
					if constexpr (std::is_same_v<V, float>) return val;
					if constexpr (std::is_same_v<V, int>)   return static_cast<float>(val);
					if constexpr (std::is_same_v<V, bool>)  return val ? 1.0f : 0.0f;
				}
				logger::error("GetValue: Type mismatch for '{}'", key_section);
				return defaultValue;
			}, *opt);
		}

		template <typename T>
		bool SetValue(const std::string& key_section, const T& value)
		{
			auto sep = key_section.rfind(':');
			if (sep == std::string::npos) {
				logger::error("SetValue: Invalid key_section format: '{}'", key_section);
				return false;
			}

			std::string section = key_section.substr(0, sep);
			std::string key = key_section.substr(sep + 1);

			if (section.empty() || key.empty()) {
				logger::error("SetValue: Empty section or key in '{}'", key_section);
				return false;
			}

			for (auto& bind : bindings) {
				if (section == bind.section && key == bind.key) {
					bool matched = std::visit([&](auto* ptr) -> bool {
						using PtrType = std::decay_t<decltype(*ptr)>;
						if constexpr (std::is_same_v<PtrType, T>) {
							*ptr = value;
							return true;
						}
						return false;
					}, bind.var);

					if (!matched) {
						logger::error("SetValue: Type mismatch for '{}:{}'", section, key);
						return false;
					}

					CSimpleIniA ini;
					ini.SetUnicode();
					if (std::filesystem::exists(path)) ini.LoadFile(path.c_str());

					if constexpr (std::is_same_v<T, bool>) {
						ini.SetBoolValue(section.c_str(), key.c_str(), value);
					} else if constexpr (std::is_same_v<T, int>) {
						ini.SetLongValue(section.c_str(), key.c_str(), value);
					} else if constexpr (std::is_same_v<T, float>) {
						ini.SetDoubleValue(section.c_str(), key.c_str(), value);
					} else if constexpr (std::is_same_v<T, std::string>) {
						ini.SetValue(section.c_str(), key.c_str(), value.c_str());
					} else {
						return false;
					}

					if (ini.SaveFile(path.c_str()) < 0) {
						logger::error("SetValue: Failed to save INI file at '{}'", path);
						return false;
					}

					return true;
				}
			}

			logger::error("SetValue: No binding found for '{}:{}'", section, key);
			return false;
		}

	private:
		inline static std::string path = "Data/SKSE/Plugins/MCM-Unlocked.ini";
		inline static std::string prefix = "MCMU";

		using IniValue = std::variant<bool*, int*, float*, std::string*>;

		struct IniBinding
		{
			const char* section;
			const char* key;
			IniValue    var;
			bool        syncGlobal = false;
		};

		std::vector<IniBinding> bindings;
	};
}
