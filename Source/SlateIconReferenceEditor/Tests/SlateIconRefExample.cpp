// Copyright 2025, Aquanox.

#include "SlateIconRefExample.h"
#include "SlateIconReferenceEditorModule.h"
#include "Internal/SlateIconRefDataHelper.h"

USlateIconRefExample::USlateIconRefExample()
{
	IconBasic.StyleSetName = "EditorStyle";
	IconBasic.IconName = "PlacementBrowser.Icons.Basic";
	IconBasicRO = IconBasic;

	IconBasicNC = IconBasic;
	IconBasicNC.StyleSetName = "BadStyle";

	IconDefault.StyleSetName = "EditorStyle";
	IconDefault.IconName = "Persona.ToggleReferencePose";
	IconDefault.SmallIconName = "Persona.ToggleReferencePose.Small";
	IconDefaultRO = IconDefault;
	IconDefaultNC = IconDefault;

	IconCustom.StyleSetName = "EditorStyle";
	IconCustom.IconName = "LevelEditor.OpenAddContent.Background";
	IconCustom.SmallIconName = NAME_None;
	IconCustom.OverlayIconName = "LevelEditor.OpenAddContent.Overlay";
	IconCustomRO = IconCustom;
	IconCustomCompact3 = IconCustom;
	IconCustomCompact2 = IconCustom;
	IconCustomCompact1 = IconCustom;
}

bool USlateIconRefExample::SupportsAutoRegistration() const
{
#ifdef WITH_SB_HOST_PROJECT
	return true;
#else
	return false;
#endif
}

USlateIconRefExampleSmall::USlateIconRefExampleSmall()
{
	IconBasic.StyleSetName = "EditorStyle";
	IconBasic.IconName = "PlacementBrowser.Icons.Testing";
	IconBasic.SmallIconName = "PlacementBrowser.BadIcon";
	IconBasic.OverlayIconName = NAME_None;

	IconCompact.StyleSetName = "EditorStyle";
	IconCompact.IconName = "PlacementBrowser.Icons.Basic";

	IconComplexN1.StyleSetName = "EditorStyle";
	IconComplexN1.IconName = "LevelEditor.OpenAddContent.Background";
	IconComplexN1.SmallIconName = NAME_None;
	IconComplexN1.OverlayIconName = "LevelEditor.OpenAddContent.Overlay";

	IconComplexN2.StyleSetName = "EditorStyle";
	IconComplexN2.IconName = "LevelEditor.OpenAddContent.Background";
	IconComplexN2.SmallIconName = NAME_None;
	IconComplexN2.OverlayIconName = "LevelEditor.OpenAddContent.Overlay";
}

bool USlateIconRefExampleSmall::SupportsAutoRegistration() const
{
#ifdef WITH_SB_HOST_PROJECT
	return true;
#else
	return false;
#endif
}


const TArray<FName>& USlateIconRefExample::GetStyleSetNames() const
{
	static TArray<FName>  Result;
	if (Result.Num() == 0)
	{
		Result.Add(FName());
		for (auto& Desc : FSlateIconRefDataHelper::GetDataSource().GetStyleSets())
		{
			Result.Add(Desc->Name);
		}
	}
	return Result;
}
