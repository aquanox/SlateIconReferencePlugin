// Copyright 2025, Aquanox.

#pragma once

#include "Styling/SlateBrush.h"
#include "Templates/UnrealTemplate.h"
#include "Types/SlateStructs.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Internal/SlateIconRefAccessor.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

/**
 * Icon preview widget that supports layered icons
 */
class SSlateIconStaticPreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSlateIconStaticPreview)
		: _AutoRefresh(true) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, SourceProperty)
		SLATE_ARGUMENT(TSharedPtr<FSlateIconDescriptor>, SourceDescriptor)
		SLATE_ATTRIBUTE(float, TargetHeight)
		SLATE_ATTRIBUTE(float, MinWidth)
		SLATE_ATTRIBUTE(float, MaxWidth)
		SLATE_ARGUMENT(bool, AutoRefresh)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

private:
	const FSlateBrush* GetPropertyBrush() const;
	const FSlateBrush* GetPropertyOverlayBrush() const;

	FOptionalSize GetScaledImageBrushWidth() const;
	FOptionalSize GetScaledImageBrushHeight() const;

	EVisibility GetVisibilityForPreviewBorder() const;
	EVisibility GetVisibilityForPreviewImage() const;

	EVisibility GetVisibilityForPreviewIcon() const;
protected:
	using FUpdateVisualsFunc = TDelegate<void()>;
	FUpdateVisualsFunc UpdateVisualsFunc;

	void UpdateVisuals(TSharedPtr<IPropertyHandle> IconPropertyHandle);
	void UpdateVisuals(TSharedPtr<FSlateIconDescriptor> IconDescriptor);
private:
	TSharedPtr<SImage> Image;

	FSlateBrush TemporaryBrush;
	FSlateBrush TemporaryOverlayBrush;

	TAttribute<float> TargetHeight;

	TAttribute<float> MinWidth;
	TAttribute<float> MaxWidth;

	bool AutoRefresh = true;
};

#undef LOCTEXT_NAMESPACE

