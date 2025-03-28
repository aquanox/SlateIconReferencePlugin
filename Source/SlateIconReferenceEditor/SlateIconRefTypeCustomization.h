// Copyright 2025, Aquanox.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
#include "Math/Vector2D.h"
#include "Templates/SharedPointer.h"
#include "Internal/SlateIconRefAccessor.h"

class IPropertyHandle;
class IDetailChildrenBuilder;
class FDetailWidgetRow;

class FSlateIconRefTypeCustomization : public IPropertyTypeCustomization
{
	using ThisClass = FSlateIconRefTypeCustomization;
public:
	inline static const FName TypeName = "SlateIconReference";

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	virtual ~FSlateIconRefTypeCustomization();

	virtual void CustomizeHeader( TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InUtils ) override;
	virtual void CustomizeChildren( TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& InBuilder, IPropertyTypeCustomizationUtils& InUtils ) override;

	bool CanEdit() const;

	void OnTryGuessSmallImage() const;

	void OnReset(FName InMember) const;

	void OnCopyAsBrush(FName InMember) const;
	void OnCopyAsBrushArray() const;
	void OnExportBrushAsset(FName InHandle) const;
private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	ESlateIconDisplayMode DisplayMode;
	bool bNoClear;
};

