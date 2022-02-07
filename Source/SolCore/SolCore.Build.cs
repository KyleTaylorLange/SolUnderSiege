// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SolCore : ModuleRules
{
	public SolCore(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        //PrivatePCHHeaderFile = "Public/SolCore.h";
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
				//"InputCore",
                "UMG",
				"Slate",
				"SlateCore",
			}
        );
        // DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "OnlineSubsystemNull",
                "OnlineSubsystemSteam"
            }
        );
        /*
        if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            if (UEBuildConfiguration.bCompileSteamOSS == true)
            {
                DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
            }
        }
        */
    }

}
