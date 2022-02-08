// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SolUnderSiegeEditorTarget : TargetRules
{
	public SolUnderSiegeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("SolCore");
		ExtraModuleNames.Add("Lastim");
		ExtraModuleNames.Add("SolGame");
    }
}