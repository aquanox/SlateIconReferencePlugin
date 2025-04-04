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

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
// Use engine private access
#include "Misc/DefinePrivateMemberPtr.h"
UE_DEFINE_PRIVATE_MEMBER_PTR(FName, GOverlayIconName, FSlateIcon, StatusOverlayStyleName);

static FName Internal_ReadOverlayFromIcon(const FSlateIcon& InIcon)
{
	return InIcon.*GOverlayIconName;
}

#elif !UE_VERSION_OLDER_THAN(5, 0, 0)
// Use memory layout matching
static FName Internal_ReadOverlayFromIcon(const FSlateIcon& InIcon)
{
	struct FIconAccessor { FName StyleSet, Icon, SmallIcon, OverlayIcon; };
	static_assert(sizeof(FIconAccessor) == sizeof(FSlateIcon), "FIconAccessor does not match FSlateIcon layout");
	return reinterpret_cast<const FIconAccessor*>(&InIcon)->OverlayIcon;
}

#else
// UE4 has no such property, leave it none
static FName Internal_ReadOverlayFromIcon(const FSlateIcon& InIcon) { return FName(); }
#endif

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
	// do some magic as engine did not expose accessor for 4th member
	OverlayIconName = Internal_ReadOverlayFromIcon(Other);
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
