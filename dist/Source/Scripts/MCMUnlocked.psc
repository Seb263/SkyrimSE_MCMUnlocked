Scriptname MCMUnlocked Hidden

; Returns version information of the native plugin
Int[] Function GetVersion() global native

; Returns the current number of registered MCM configurations
Int Function GetConfigCount() global native

; Creates a new MCM marker, registers it internally, and returns the associated ObjectReference
ObjectReference Function RegisterMarker() global native

; Unregisters a configuration by index and removes its associated marker
; Returns true if the operation succeeded
Bool Function UnregisterMarker(Int configID) global native

; Retrieves the ObjectReference linked to the given configuration index
; Returns None if the index is invalid or the marker is missing
ObjectReference Function GetMarkerFromIndex(Int configID) global native

; Updates displayed mod names for a given menu
Function UpdateMenuModNames(string menuName, string target) global native


; Helpers

; Retrieves the marker script attached to the configuration index
; Returns None if the marker or script is missing
MCMUnlockedMarkerScript Function GetMarkerScript(Int configID) global
	ObjectReference marker = GetMarkerFromIndex(configID)
	If !marker
		Return None
	EndIf

	MCMUnlockedMarkerScript markerScript = (marker as MCMUnlockedMarkerScript)
	If !markerScript
		Return None
	EndIf

	Return markerScript
EndFunction

; Retrieves the SKI_ConfigBase instance associated with a configuration index
; Returns None if the marker or script is missing
SKI_ConfigBase Function GetConfigBase(Int configID) global
	MCMUnlockedMarkerScript markerScript = GetMarkerScript(configID)
	If !markerScript
		Return None
	EndIf

	Return markerScript.InstanceScript
EndFunction

; Returns the mod name associated with a configuration index
; Returns an empty string if the marker or script is missing
String Function GetModName(Int configID) global
	MCMUnlockedMarkerScript markerScript = GetMarkerScript(configID)
	If !markerScript
		Return ""
	EndIf

	Return markerScript.ModName
EndFunction