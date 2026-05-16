Scriptname MCMUnlockedMarkerScript Extends ObjectReference

SKI_ConfigBase Property InstanceScript Auto Hidden
String Property ModName Auto Hidden

Function Initialize(SKI_ConfigBase a_config, String a_modName)
    InstanceScript = a_config
    ModName = a_modName
EndFunction