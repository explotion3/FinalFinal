using UnrealBuildTool;

public class FinalApp : ModuleRules
{
	public FinalApp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"PaperZD",
			"Paper2D",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"FinalCore",
			"FinalData",
			"FinalBattle",
			"FinalRun"
		});
	}
}
