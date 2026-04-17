using UnrealBuildTool;

public class FinalEditor : ModuleRules
{
	public FinalEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"UnrealEd",
			"DataValidation",
			"AssetRegistry",
			"FinalCore",
			"FinalData",
			"FinalBattle",
			"FinalRun",
			"FinalApp"
		});
	}
}
