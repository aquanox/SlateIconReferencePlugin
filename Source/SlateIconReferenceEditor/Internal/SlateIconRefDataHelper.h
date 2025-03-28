// Copyright 2025, Aquanox.

#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTemplate.h"
#include "Containers/Array.h"
#include "Templates/UnrealTemplate.h"

class SToolTip;
class IPropertyHandle;
struct FSlateIconReference;
class FSlateIconRefDataHelper;

/**
 * Represents information about slate icon
 */
struct FSlateIconDescriptor
{
	FName				StyleSetName;
	FName				Name;
	TOptional<FText>	DisplayTextOverride;
	bool				bUnknown = false;

	const FName& GetID() const { return Name; }
	// null-safe getbrush
	const FSlateBrush* GetBrushSafe() const;
	// nullable getbrush
	const FSlateBrush* GetBrush() const;
	FText GetDisplayText() const;
	bool IsNone() const { return Name.IsNone(); }
	bool IsUnknown() const;

	bool operator<(const FSlateIconDescriptor& Other) const { return Name.Compare(Other.Name) < 0; }
	bool operator==(const FSlateIconDescriptor& Other) const { return StyleSetName == Other.StyleSetName && Name == Other.Name; }
	bool operator==(const FName& Other) const { return Name == Other; }
};

/**
 * Represents information about slate style set
 */
struct FSlateStyleSetDescriptor
{
	FName				Name;
	TOptional<FText>	DisplayTextOverride;
	bool				bUnknown = false;

	TArray<TSharedPtr<FSlateIconDescriptor>> Icons;

	const FName& GetID() const { return Name; }
	const ISlateStyle* GetStyleSet() const;
	FText GetDisplayText() const;
	bool IsNone() const { return Name.IsNone(); }
	bool IsUnknown() const;
	const TArray<TSharedPtr<FSlateIconDescriptor>>& GetRegisteredIcons() const { return Icons; }
	int32 GetNumRegisteredIcons() const { return Icons.Num(); }

	bool operator< (const FSlateStyleSetDescriptor& Other) const { return Name.Compare(Other.Name) < 0; }
	bool operator==(const FSlateStyleSetDescriptor& Other) const { return Name == Other.Name; }
	bool operator==(const FName& Other) const { return Name == Other; }
};

/**
 * Style data storage
 */
class FSlateIconRefDataHelper : public FNoncopyable
{
public:
	static FSlateIconRefDataHelper& GetDataSource();

	void SetupStyleData();
	void ClearStyleData();

	void ForceRescan() { bInitialized = false; }

	void GatherStyleData(bool bAllowNone, TArray<TSharedPtr<FSlateStyleSetDescriptor>>& OutArray);
	void GatherIconData(bool bAllowNone, TArray<TSharedPtr<FSlateIconDescriptor>>& OutArray);

	TSharedPtr<FSlateStyleSetDescriptor> FindStyleSet(FName StyleSetName);
	TSharedPtr<FSlateIconDescriptor> FindIcon(FName StyleSetName, FName IconName);

	TArray<TSharedPtr<FSlateStyleSetDescriptor>> const& GetStyleSets() const { return KnownStyleSets; }

public:
	bool bInitialized = false;

	TSharedPtr<FSlateStyleSetDescriptor> EmptyStyleSet;
	TSharedPtr<FSlateStyleSetDescriptor> AutoStyleSet;
	TSharedPtr<FSlateIconDescriptor> EmptyImage;

	// all discovered stylesets
	TArray<TSharedPtr<FSlateStyleSetDescriptor>> KnownStyleSets;
	// all discovered icons
	TArray<TSharedPtr<FSlateIconDescriptor>> KnownIcons;
	// searchable icon map
	using FImageKey = TPair<FName, FName>;
	TMap<FImageKey, TSharedPtr<FSlateIconDescriptor>> KnownIconsMap;
};
