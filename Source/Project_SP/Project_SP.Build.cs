// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_SP : ModuleRules
{
	public Project_SP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "NavigationSystem",
            "AIModule",
            "Niagara",
            "EnhancedInput",
            "UMG",
            "CinematicCamera",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "Slate",
            "SlateCore",
            "Niagara" 
        });
    }
}
