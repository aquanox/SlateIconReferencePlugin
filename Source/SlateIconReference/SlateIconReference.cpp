// Copyright 2025, Aquanox.

#include "SlateIconReference.h"
#include "Modules/ModuleManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Slate/SlateBrushAsset.h"
#include "Styling/ISlateStyle.h"
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

FSlateIconReference::FSlateIconReference(FName InStyleSetName, FName InStyleName, FName InSmallStyleName, FName InStatusOverlayStyleName)
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
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	return FSlateIcon(StyleSetName, IconName, (SmallIconName == NAME_None) ? ISlateStyle::Join(IconName, ".Small") :  SmallIconName);
#else
	return FSlateIcon(StyleSetName, IconName, SmallIconName, OverlayIconName);
#endif
}

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
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	const ISlateStyle* StyleSet = GetStyleSet();
	return StyleSet ? StyleSet->GetOptionalBrush(OverlayIconName) : nullptr;
#else
	return ToSlateIcon().GetOverlayIcon();
#endif
}
