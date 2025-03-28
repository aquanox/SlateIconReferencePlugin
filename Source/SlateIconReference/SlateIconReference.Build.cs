//  Copyright 2025, Aquanox.

using UnrealBuildTool;

public class SlateIconReference : ModuleRules
{
	public bool bStrictIncludesCheck = false;

	public SlateIconReference(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		if (bStrictIncludesCheck)
		{
			bUseUnity = false;
			PCHUsage = PCHUsageMode.NoPCHs;
			bTreatAsEngineModule = true;
		}

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Slate",
			"SlateCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Engine",
			"InputCore"
		});

	}
}
