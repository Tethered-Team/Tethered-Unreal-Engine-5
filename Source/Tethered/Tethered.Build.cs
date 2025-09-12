// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Tethered : ModuleRules
{
	public Tethered(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Tethered",
			"Tethered/Variant_Platforming",
			"Tethered/Variant_Platforming/Animation",
			"Tethered/Variant_Combat",
			"Tethered/Variant_Combat/AI",
			"Tethered/Variant_Combat/Animation",
			"Tethered/Variant_Combat/Gameplay",
			"Tethered/Variant_Combat/Interfaces",
			"Tethered/Variant_Combat/UI",
			"Tethered/Variant_SideScrolling",
			"Tethered/Variant_SideScrolling/AI",
			"Tethered/Variant_SideScrolling/Gameplay",
			"Tethered/Variant_SideScrolling/Interfaces",
			"Tethered/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
