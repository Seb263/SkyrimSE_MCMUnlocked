scriptname SKI_ConfigManager extends SKI_QuestBase hidden

; CONSTANTS ---------------------------------------------------------------------------------------

string property		JOURNAL_MENU	= "Journal Menu" autoReadonly
string property		MENU_ROOT		= "_root.ConfigPanelFader.configPanel" autoReadonly


; PRIVATE VARIABLES -------------------------------------------------------------------------------

; -- Version 1 --

SKI_ConfigBase		_activeConfig

; -- Version 2 --

; keep those for now
bool				_lockInit		= false
bool				_locked			= false

; -- Version 4 --

bool				_cleanupFlag	= false
int					_initialCount	= 0
int					_updateCounter	= 0


; INITIALIZATION ----------------------------------------------------------------------------------

event OnInit()
	OnGameReload()
endEvent

; @implements SKI_QuestBase
event OnGameReload()
	GoToState("")
	
	RegisterForModEvent("SKICP_modSelected", "OnModSelect")
	RegisterForModEvent("SKICP_pageSelected", "OnPageSelect")
	RegisterForModEvent("SKICP_optionHighlighted", "OnOptionHighlight")
	RegisterForModEvent("SKICP_optionSelected", "OnOptionSelect")
	RegisterForModEvent("SKICP_optionDefaulted", "OnOptionDefault")
	RegisterForModEvent("SKICP_keymapChanged", "OnKeymapChange")
	RegisterForModEvent("SKICP_sliderSelected", "OnSliderSelect")
	RegisterForModEvent("SKICP_sliderAccepted", "OnSliderAccept")
	RegisterForModEvent("SKICP_menuSelected", "OnMenuSelect")
	RegisterForModEvent("SKICP_menuAccepted", "OnMenuAccept")
	RegisterForModEvent("SKICP_colorSelected", "OnColorSelect")
	RegisterForModEvent("SKICP_colorAccepted", "OnColorAccept")
	RegisterForModEvent("SKICP_inputSelected", "OnInputSelect")
	RegisterForModEvent("SKICP_inputAccepted", "OnInputAccept")
	RegisterForModEvent("SKICP_dialogCanceled", "OnDialogCancel")

	RegisterForMenu(JOURNAL_MENU)

	; no longer used but better safe than sorry
	_lockInit = true

	_cleanupFlag = true

	CleanUp()
	SendModEvent("SKICP_configManagerReady")

	_updateCounter = 0
	RegisterForSingleUpdate(5)
endEvent


; EVENTS ------------------------------------------------------------------------------------------

event OnUpdate()

	if (_cleanupFlag)
		CleanUp()
	endIf

	Int addedCount = MCMUnlocked.GetConfigCount() - _initialCount
	If (addedCount > 0)
		Debug.Notification("MCM: Registered " + addedCount + " new menu(s).")
	EndIf

	_initialCount = MCMUnlocked.GetConfigCount()
	SendModEvent("SKICP_configManagerReady")

	If (_updateCounter < 6)
		_updateCounter += 1
		RegisterForSingleUpdate(5)
	Else
		RegisterForSingleUpdate(30)
	EndIf
endEvent

event OnMenuOpen(string a_menuName)
    GotoState("BUSY")
    _activeConfig = none
	MCMUnlocked.UpdateMenuModNames(JOURNAL_MENU, MENU_ROOT + ".setModNames")
endEvent

event OnMenuClose(string a_menuName)
	GotoState("")
	if (_activeConfig)
		_activeConfig.CloseConfig()
	endIf

	_activeConfig = none
endEvent

event OnModSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int configIndex = a_numArg as int
	if (configIndex > -1)

		; We can clean the buffers of the previous menu now
		if (_activeConfig)
			_activeConfig.CloseConfig()
		endIf

		_activeConfig = MCMUnlocked.GetConfigBase(configIndex)
		If _activeConfig
			_activeConfig.OpenConfig()
		Else
			Debug.Trace("[MCMUnlocked] OnModSelect: GetConfigBase(" + configIndex + ") returned None")
		EndIf
	endIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnPageSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	string page = a_strArg
	int index = a_numArg as int
	If _activeConfig
		_activeConfig.SetPage(page, index)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnOptionHighlight(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.HighlightOption(optionIndex)
	EndIf
endEvent

event OnOptionSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.SelectOption(optionIndex)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnOptionDefault(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.ResetOption(optionIndex)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnKeymapChange(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
    int optionIndex = a_numArg as int
    int keyCode = UI.GetInt(JOURNAL_MENU, MENU_ROOT + ".selectedKeyCode")

    string conflictControl = Input.GetMappedControl(keyCode)
    string conflictName = ""

    Int count = MCMUnlocked.GetConfigCount()
    Int i = 0
    While conflictControl == "" && i < count
        SKI_ConfigBase config = MCMUnlocked.GetConfigBase(i)
        If (config != none)
            conflictControl = config.GetCustomControl(keyCode)
            If conflictControl != ""
                conflictName = MCMUnlocked.GetModName(i)
            EndIf
        EndIf
        i += 1
    EndWhile

    If _activeConfig
        _activeConfig.RemapKey(optionIndex, keyCode, conflictControl, conflictName)
    EndIf
    UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnSliderSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.RequestSliderDialogData(optionIndex)
	Else
		Debug.Trace("[MCMUnlocked] OnSliderSelect: _activeConfig is None, forcing unlock")
		UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
	EndIf
endEvent

event OnSliderAccept(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	float value = a_numArg
	If _activeConfig
		_activeConfig.SetSliderValue(value)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnMenuSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.RequestMenuDialogData(optionIndex)
	Else
		Debug.Trace("[MCMUnlocked] OnMenuSelect: _activeConfig is None, forcing unlock")
		UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
	EndIf
endEvent

event OnMenuAccept(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int value = a_numArg as int
	If _activeConfig
		_activeConfig.SetMenuIndex(value)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnColorSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.RequestColorDialogData(optionIndex)
	Else
		Debug.Trace("[MCMUnlocked] OnColorSelect: _activeConfig is None, forcing unlock")
		UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
	EndIf
endEvent

event OnColorAccept(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int color = a_numArg as int
	If _activeConfig
		_activeConfig.SetColorValue(color)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnInputSelect(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	int optionIndex = a_numArg as int
	If _activeConfig
		_activeConfig.RequestInputDialogData(optionIndex)
	Else
		Debug.Trace("[MCMUnlocked] OnInputSelect: _activeConfig is None, forcing unlock")
		UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
	EndIf
endEvent

event OnInputAccept(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	If _activeConfig
		_activeConfig.SetInputText(a_strArg)
	EndIf
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent

event OnDialogCancel(string a_eventName, string a_strArg, float a_numArg, Form a_sender)
	UI.InvokeBool(JOURNAL_MENU, MENU_ROOT + ".unlock", true)
endEvent


; FUNCTIONS ---------------------------------------------------------------------------------------

int function GetVersion()
	return 4
endFunction

int function RegisterMod(SKI_ConfigBase a_menu, string a_modName)
	ObjectReference marker = MCMUnlocked.RegisterMarker(a_modName)
	If !marker
		Return -1
	EndIf

	MCMUnlockedMarkerScript markerScript = marker as MCMUnlockedMarkerScript
	If !markerScript
		Return -1
	EndIf
	
	markerScript.Initialize(a_menu, a_modName)
    Return 1
endFunction

int function UnregisterMod(SKI_ConfigBase a_menu)
    Int count = MCMUnlocked.GetConfigCount()
    Int i = 0
    While i < count
        SKI_ConfigBase existing = MCMUnlocked.GetConfigBase(i)
        If existing == a_menu
            MCMUnlocked.UnregisterMarker(i)
            Return i
        EndIf
        i += 1
    EndWhile
    Return -1
endFunction

function ForceReset()
    Log("Forcing config manager reset...")
    SendModEvent("SKICP_configManagerReset")

    GotoState("BUSY")

	Int i = MCMUnlocked.GetConfigCount() - 1
	While i >= 0
		MCMUnlocked.UnregisterMarker(i)
		i -= 1
	EndWhile

    GotoState("")

    SendModEvent("SKICP_configManagerReady")
endFunction

function CleanUp()
    GotoState("BUSY")

    _cleanupFlag = false

	Int i = MCMUnlocked.GetConfigCount() - 1

	While i >= 0
		SKI_ConfigBase existing = MCMUnlocked.GetConfigBase(i)
        If (existing == none || existing.GetFormID() == 0)
            MCMUnlocked.UnregisterMarker(i)
        EndIf
		i -= 1
	EndWhile

    GotoState("")
EndFunction

function Log(string a_msg)
	Debug.Trace(self + ": " + a_msg)
endFunction


; STATES ---------------------------------------------------------------------------------------

state BUSY
	int function RegisterMod(SKI_ConfigBase a_menu, string a_modName)
		return -2
	endFunction

	int function UnregisterMod(SKI_ConfigBase a_menu)
		return -2
	endFunction

	function ForceReset()
	endFunction

	function CleanUp()
	endFunction
endState
