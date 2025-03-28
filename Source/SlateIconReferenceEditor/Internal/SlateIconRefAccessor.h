// Copyright 2025, Aquanox.

#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Templates/SharedPointer.h"
#include "Misc/EnumClassFlags.h"
#include "Templates/UnrealTemplate.h"

class IPropertyHandle;
struct FSlateIconReference;
struct FSlateIconDescriptor;
struct FSlateStyleSetDescriptor;

namespace Switches
{
	// dev switch to update on tick instead of on event
	constexpr bool bRealtimeUpdates = true;
}

/**
 * Display modes of slate icon customization
 */
enum class ESlateIconDisplayMode : uint32
{
	Standard		= 0x00,
	Compact			= 0x01,

	WithIcon		= 0x10,
	WithSmallIcon   = 0x20,
	WithOverlayIcon = 0x40,
	WithAll			= WithIcon|WithSmallIcon|WithOverlayIcon,

	//Default			= Standard|WithIcon|WithSmallIcon,
	Default			= Standard|WithIcon|WithSmallIcon|WithOverlayIcon,

	// Default value for Standard display mode
	DefaultStandard	= Standard|WithIcon|WithSmallIcon|WithOverlayIcon,
	// Default value for Compact display mode
	DefaultCompact  = Compact|WithIcon,
};
ENUM_CLASS_FLAGS(ESlateIconDisplayMode);

/**
 * Various accessors around property handle that used across entire customization
 *
 * Wraps primary struct property handle and optional name for the member property that is being displayed
 */
class FSlateIconRefAccessor
{
	TSharedPtr<IPropertyHandle> PropertyHandle;
public:
	static FName Member_StyleSetName();
	static FName Member_IconName();
	static FName Member_SmallIconName();
	static FName Member_OverlayIconName();

	// Easy list of icon members
	static TConstArrayView<FName> IconMembers();

	// Name to Mask matching
	static TConstArrayView<TTuple<FName, ESlateIconDisplayMode>> IconMemberMasks();

	FSlateIconRefAccessor() = default;
	FSlateIconRefAccessor(const TSharedPtr<IPropertyHandle>& PropertyHandle) : PropertyHandle(PropertyHandle) { }

	/**
	 *
	 */
	TSharedPtr<IPropertyHandle> GetHandle() const { return PropertyHandle; }

	/**
	 * Can property be edited
	 */
	bool IsEditable() const;

	/**
	 * Can property be cleared (Having Clear button or None options in menu)
	 */
	bool AllowClearingValue() const;

	/**
	 * Read property value from underlying handle
	 */
	bool ReadPropertyValue(FSlateIconReference& OutData) const;

	/**
	 * Read named property value from underlying handle
	 */
	bool ReadPropertyValueByName(const FName& InName, FName& OutData) const;

	/**
	 * Select icon descriptor for named member property or default if read failed
	 */
	TSharedPtr<FSlateIconDescriptor> SelectDescriptorByName(FName InName) const;

	/**
	 * Select style set descriptor for named member property or default if read failed
	 */
	TSharedPtr<FSlateStyleSetDescriptor> GetStyleDescriptor() const;

	/**
	 * Set style set value for property and reset other members
	 */
	void SetPropertyStyleSetValue(const FName& InValue) const;

	/**
	 * Set named member property value
	 */
	void SetPropertyIconValue(FName InName, const FName& InValue) const;
};

DECLARE_LOG_CATEGORY_EXTERN(LogSlateIcon, Log, All);

