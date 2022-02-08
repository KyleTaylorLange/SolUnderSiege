// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SolUnderSiegeTarget : TargetRules
{
	public SolUnderSiegeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        ExtraModuleNames.Add("SolCore");
        ExtraModuleNames.Add("Lastim");
	ExtraModuleNames.Add("SolGame");
        //bUsesSteam = true;
    }
}
