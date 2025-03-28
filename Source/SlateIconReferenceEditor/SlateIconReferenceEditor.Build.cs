// Copyright 2025, Aquanox.

using UnrealBuildTool;

public class SlateIconReferenceEditor : ModuleRules
{
    public bool bStrictIncludesCheck = false;


    public SlateIconReferenceEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "SlateIconReference"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Engine",
            "PropertyEditor",
            "DeveloperSettings",
            "EditorStyle",
            "EditorWidgets",
            "InputCore",
            "ToolWidgets"
        });
    }


}