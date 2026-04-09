using UnrealBuildTool;

public class FinalRun : ModuleRules
{
	public FinalRun(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"FinalCore",
			"FinalData"
		});
	}
}
