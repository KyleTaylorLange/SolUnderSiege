// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SolUnderSiegeServerTarget : TargetRules
{
	public SolUnderSiegeServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		ExtraModuleNames.Add("SolCore");
		ExtraModuleNames.Add("Lastim");
		ExtraModuleNames.Add("SolGame");
    }
}