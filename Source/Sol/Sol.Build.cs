// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Sol : ModuleRules
{
	public Sol(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "Public/Sol.h";
        PublicDependencyModuleNames.AddRange(
            new string[] { 
                "Core", 
                "CoreUObject", 
                "Engine",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "InputCore",
                "AIModule",
                "NavigationSystem",
                "PhysicsCore"
            }
        );
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "UMG",
				"Slate",
				"SlateCore",
			}
        );
        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "OnlineSubsystemNull"
            }
        );
        if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        }
    }
}
