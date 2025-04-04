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
	/**
	 * Default constructor (empty icon).
	 */
	FSlateIconReference() = default;

	/**
	 * Creates and initializes a new icon from a style set and style name
	 *
	 * @param InStyleSetName The name of the style set the icon can be found in.
	 * @param InStyleName The name of the style for the icon
	 * @param InSmallStyleName The name of the style for the small icon
	 * @param InStatusOverlayStyleName The name of the style for a status icon to overlay on the base image
	 */
	FSlateIconReference(FName InStyleSetName, FName InStyleName, FName InSmallStyleName = NAME_None, FName InStatusOverlayStyleName = NAME_None);

	/**
	 * Default constructor (provided icon).
	 */
	explicit FSlateIconReference(const FSlateIcon& InIcon);

	explicit operator FSlateIcon() const
	{
		return ToSlateIcon();
	}

	FSlateIconReference& operator=(const FSlateIcon& Other);

	/**
	 * Convert icon reference into FSlateIcon
	 */
	FSlateIcon ToSlateIcon() const;

	/**
	 * Gets the resolved style set.
	 *
	 * @return Style set, or nullptr if the style set wasn't found.
	 * @see GetSmallStyleName, GetStyleName, GetStyleSetName
	 */
	const ISlateStyle* GetStyleSet() const;

	/**
	 * Gets the resolved icon.
	 *
	 * @return Icon brush, or FStyleDefaults::GetNoBrush() if the icon wasn't found.
	 * @see GetSmallIcon
	 */
	const FSlateBrush* GetIcon() const;

	/**
	 * Optionally gets the resolved icon, returning nullptr if it's not defined
	 *
	 * @return Icon brush, or nullptr if the icon wasn't found.
	 */
	const FSlateBrush* GetOptionalIcon() const;

	/**
	 * Gets the resolved small icon.
	 *
	 * @return Icon brush, or FStyleDefaults::GetNoBrush() if the icon wasn't found.
	 * @see GetIcon
	 */
	const FSlateBrush* GetSmallIcon() const;

	/**
	 * Optionally gets the resolved small icon, returning nullptr if it's not defined
	 *
	 * @return Icon brush, or nullptr if the icon wasn't found.
	 */
	const FSlateBrush* GetOptionalSmallIcon() const;

	/**
	 * Gets the resolved overlay icon.
	 *
	 * @return Icon brush, or FStyleDefaults::GetNoBrush() if the icon wasn't found.
	 * @see GetIcon
	 */
	const FSlateBrush* GetOverlayIcon() const;

	/**
	 * Optionally gets the resolved overlay icon, returning nullptr if it's not defined
	 *
	 * @return Icon brush, or nullptr if the icon wasn't found.
	 */
	const FSlateBrush* GetOptionalOverlayIcon() const;

	/**
	 * Gets the name of the style set the icon can be found in.
	 *
	 * @return Style Set name.
	 */
	const FName& GetStyleSetName() const {  return StyleSetName; }
	/**
	 * Gets the name of the style for the icon.
	 *
	 * @return Style name.
	 */
	const FName& GetStyleName() const { return IconName; }
	/**
	 * Gets the name of the style for the small icon.
	 *
	 * @return Style name.
	 */
	const FName& GetSmallStyleName() const { return SmallIconName; }
	/**
	 * Gets the name of the style for the status icon.
	 *
	 * @return Style name.
	 */
	const FName& GetOverlayStyleName() const { return OverlayIconName; }

	/**
	 * Checks whether the icon is set to something.
	 *
	 * @return true if the icon is set, false otherwise.
	 */
	bool IsSet() const
	{
		return StyleSetName != NAME_None && IconName != NAME_None;
	}
};
