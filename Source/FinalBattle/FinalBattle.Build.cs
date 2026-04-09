using UnrealBuildTool;

public class FinalBattle : ModuleRules
{
	public FinalBattle(ReadOnlyTargetRules Target) : base(Target)
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
