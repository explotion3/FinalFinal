// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FinalFinalEditorTarget : TargetRules
{
	public FinalFinalEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.AddRange(new string[]
		{
			"FinalApp",
			"FinalCore",
			"FinalData",
			"FinalBattle",
			"FinalRun"
		});
	}
}
