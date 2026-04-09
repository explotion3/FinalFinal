using UnrealBuildTool;

public class FinalCore : ModuleRules
{
	public FinalCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"GameplayTags"
		});
	}
}
