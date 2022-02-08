// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SolGame : ModuleRules
{
	public SolGame(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
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
                "PhysicsCore",
                "SolCore"
            }
        );
    }
}
