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
            "SlateCore",
            "SlateIconReference"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
	        "CoreUObject",
	        "Slate",
            "Engine",
            "PropertyEditor",
            "DeveloperSettings",
            "EditorStyle",
            "EditorWidgets",
            "InputCore",
            "ApplicationCore"
        });

        if (Target.Version.MajorVersion == 4 && CppStandard < CppStandardVersion.Cpp17)
        { // required for private access in 4.27
	        CppStandard = CppStandardVersion.Cpp17;
        }

        if (Target.Version.MajorVersion >= 5)
        {
            PrivateDependencyModuleNames.Add("ToolWidgets");
        }
    }


}