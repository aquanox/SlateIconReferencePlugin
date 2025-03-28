// Copyright 2025, Aquanox.

#include "SlateIconReference.h"
#include "Modules/ModuleManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Slate/SlateBrushAsset.h"
#include "Styling/SlateColor.h"
#include "Styling/StyleDefaults.h"

#include "UObject/Package.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, SlateIconReference);

FSlateIconReference::FSlateIconReference(const FName& InStyleSetName)
	: StyleSetName(InStyleSetName)
{

}

FSlateIconReference::FSlateIconReference(const FSlateIcon& InIcon)
{
	operator=(InIcon);
}

FSlateIconReference::FSlateIconReference(const FName InStyleSetName, const FName InStyleName, const FName InSmallStyleName, const FName InStatusOverlayStyleName)
	: StyleSetName(InStyleSetName), IconName(InStyleName), SmallIconName(InSmallStyleName), OverlayIconName(InStatusOverlayStyleName)
{
}

FSlateIconReference& FSlateIconReference::operator=(const FSlateIcon& Other)
{
	StyleSetName = Other.GetStyleSetName();
	IconName = Other.GetStyleName();
	SmallIconName = Other.GetSmallStyleName();

	//there is no getter, but in memory layout it is next after small icon
	// so um ... try this instead of making dependency on private_access
	const uint8* pStyle = reinterpret_cast<const uint8*>(&Other.GetStyleName());
	const uint8* pSmall = reinterpret_cast<const uint8*>(&Other.GetSmallStyleName());
	const uint8* pOverlay = pSmall + ( reinterpret_cast<intptr_t>(pSmall) - reinterpret_cast<intptr_t>(pStyle) );
	OverlayIconName = * reinterpret_cast<const FName*>(pOverlay);

	return *this;
}

FSlateIcon FSlateIconReference::ToSlateIcon() const
{
	return FSlateIcon(StyleSetName, IconName, SmallIconName, OverlayIconName);
}

// USlateBrushAsset* FSlateIconReference::ToSlateBrushAsset()
// {
// 	FName CombinedName = *FNameBuilder(StyleSetName).Append(TEXT("_")).Append(IconName.ToString());
// 	USlateBrushAsset* Asset = NewObject<USlateBrushAsset>(GetTransientPackage(), CombinedName, RF_Transient);
// 	Asset->Brush = *GetIcon();
// 	return Asset;
// }

const ISlateStyle* FSlateIconReference::GetStyleSet() const
{
	return ToSlateIcon().GetStyleSet();
}

const FSlateBrush* FSlateIconReference::GetIcon() const
{
	return ToSlateIcon().GetIcon();
}

const FSlateBrush* FSlateIconReference::GetOptionalIcon() const
{
	return ToSlateIcon().GetOptionalIcon();
}

const FSlateBrush* FSlateIconReference::GetOptionalSmallIcon() const
{
	return ToSlateIcon().GetOptionalSmallIcon();
}

const FSlateBrush* FSlateIconReference::GetOverlayIcon() const
{
	const FSlateBrush* Brush = GetOptionalOverlayIcon();
	return Brush ? Brush : FStyleDefaults::GetNoBrush();
}

const FSlateBrush* FSlateIconReference::GetOptionalOverlayIcon() const
{
	return ToSlateIcon().GetOverlayIcon();
}
