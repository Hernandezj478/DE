// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DETarget : TargetRules
{
	public DETarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("DE");
		RegisterModules();
	}

	private void RegisterModules()
	{
		ExtraModuleNames.AddRange(new string[]
		{
			"Characters", 
			"CoreData", 
			"CoreSystems", 
			"Gameplay", 
			"Items", 
			"Inventory", 
			"Interface", 
			"UtilityFeatures", 
			"MessageHandler", 
			"Environment",
			"Statline",
            "CharacterMovement"
        });
	}
}
