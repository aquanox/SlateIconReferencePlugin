// Copyright 2025, Aquanox.

#pragma once

#include "Styling/SlateBrush.h"
#include "Textures/SlateIcon.h"
#include "SlateIconReference.generated.h"

class USlateBrushAsset;

/**
 * Wrapper around FSlateIcon used to represent an icon in Slate
 *
 * FSlateIcon can contain up to three different brushes:
 * - Icon brush
 * - SmallIcon brush
 * - StatusOverlay brush
 *
 * Editor display can be changed with DisplayMode metadata property.
 * @code
 * UPROPERTY(EditAnywhere, Config, Category="Demo")
 * FSlateIconReference DefaultDisplayMode;
 *
 * UPROPERTY(EditAnywhere, Config, Category="Demo", meta=(DisplayMode=Compact))
 * FSlateIconReference CompactDisplayMode;
 * @endcode
 */
USTRUCT()
struct SLATEICONREFERENCE_API FSlateIconReference
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category=SlateIcon)
	FName StyleSetName;

	UPROPERTY(EditAnywhere, Category=SlateIcon)
	FName IconName;

	UPROPERTY(EditAnywhere, Category=SlateIcon)
	FName SmallIconName;

	UPROPERTY(EditAnywhere, Category=SlateIcon)
	FName OverlayIconName;
public:
	FSlateIconReference() = default;
	explicit FSlateIconReference(const FName& InStyleSetName);
	explicit FSlateIconReference(const FSlateIcon& InIcon);
	FSlateIconReference(const FName InStyleSetName, const FName InStyleName, const FName InSmallStyleName = NAME_None, const FName InStatusOverlayStyleName = NAME_None);

	operator FSlateIcon() const
	{
		return FSlateIcon(StyleSetName, IconName, SmallIconName, OverlayIconName);
	}

	FSlateIconReference& operator=(const FSlateIcon& Other);

	FSlateIcon ToSlateIcon() const;

	const ISlateStyle* GetStyleSet() const; // nullable
	const FSlateBrush* GetIcon() const; // null-safe
	const FSlateBrush* GetOptionalIcon() const; // nullable GetIcon
	const FSlateBrush* GetOptionalSmallIcon() const; // nullable GetSmallIcon
	const FSlateBrush* GetOverlayIcon() const; // engine:nullablem, here:nullsafe
	const FSlateBrush* GetOptionalOverlayIcon() const; // nullable

	// FSlateIcon mimic

	const FName& GetStyleSetName() const {  return StyleSetName; }
	const FName& GetStyleName() const { return IconName; }
	const FName& GetSmallStyleName() const { return SmallIconName; }
	const FName& GetOverlayStyleName() const { return OverlayIconName; }

	bool IsSet() const { return StyleSetName != NAME_None && IconName != NAME_None; }
};
