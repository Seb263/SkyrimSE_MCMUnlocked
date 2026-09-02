Scriptname MCMUnlocked Hidden

; Returns version information of the native plugin
Int[] Function GetVersion() global native

; Returns the current number of registered MCM configurations
Int Function GetConfigCount() global native

; Creates a new MCM marker, registers it internally, and returns the associated ObjectReference
ObjectReference Function RegisterMarker(String modName) global native

; Unregisters a configuration by mod ID and removes its associated marker
Bool Function UnregisterMarker(String modID) global native

; Removes all registered configurations and markers
Bool Function UnregisterAllMarkers() global native

; Returns the mod ID at the given config index
String Function GetModIDFromConfigID(Int configID) global native

; Returns the mod ID of the currently selected entry
String Function GetModIDFromSelectedEntry() global native

; Returns the display name of a mod by ID
String Function GetModNameFromModID(string modID) global native

; Returns the marker linked to a mod ID
ObjectReference Function GetMarkerFromModID(string modID) global native

; Updates displayed mod names for the left panel
Function UpdateMenuModNames() global native

; Helpers

SKI_ConfigBase Function GetConfigBase(String modID) global
	ObjectReference marker = GetMarkerFromModID(modID)
	If !marker
		Return None
	EndIf

	MCMUnlockedMarkerScript markerScript = (marker as MCMUnlockedMarkerScript)
	If !markerScript
		Return None
	EndIf

	Return markerScript.InstanceScript
EndFunction