// Copyright 2025, Aquanox.

#include "SSlateIconStaticPreview.h"

#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "MaterialShared.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "SlateIconReference.h"
#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "Delegates/Delegate.h"
#include "Layout/Children.h"
#include "Layout/Geometry.h"
#include "Materials/Material.h"
#include "Styling/SlateBrush.h"
#include "Types/SlateStructs.h"
#include "UObject/UnrealType.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Internal/SlateIconRefDataHelper.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

void SSlateIconStaticPreview::Construct(const FArguments& InArgs)
{
	TargetHeight = InArgs._TargetHeight;
	MinWidth = InArgs._MinWidth;
	MaxWidth = InArgs._MaxWidth;
	AutoRefresh = InArgs._AutoRefresh;

	TSharedRef<SWidget> ImageWidget = SNullWidget::NullWidget;

	if (InArgs._SourceDescriptor)
	{
		UpdateVisualsFunc.BindRaw(this, &SSlateIconStaticPreview::UpdateVisuals, InArgs._SourceDescriptor);

		ImageWidget = SNew(SImage)
			.Visibility(this, &SSlateIconStaticPreview::GetVisibilityForPreviewImage)
			.Image(this, &SSlateIconStaticPreview::GetPropertyBrush);
	}
	else if (InArgs._SourceProperty)
	{
		UpdateVisualsFunc.BindRaw(this, &SSlateIconStaticPreview::UpdateVisuals, InArgs._SourceProperty);

		auto Widget = SNew(SLayeredImage)
			.Visibility(this, &SSlateIconStaticPreview::GetVisibilityForPreviewImage)
			.Image(this, &SSlateIconStaticPreview::GetPropertyBrush);

		Widget->AddLayer(TAttribute<const FSlateBrush*>(this, &SSlateIconStaticPreview::GetPropertyOverlayBrush));

		ImageWidget = Widget;
	}

	UpdateVisualsFunc.ExecuteIfBound();

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SBorder)
			.Visibility(this, &SSlateIconStaticPreview::GetVisibilityForPreviewBorder)
			.BorderImage(this, &SSlateIconStaticPreview::GetPropertyBrush)
			[
				SNew(SSpacer)
				.Size(FVector2D(1, 1))
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(this, &SSlateIconStaticPreview::GetScaledImageBrushWidth)
			.HeightOverride(this, &SSlateIconStaticPreview::GetScaledImageBrushHeight)
			[
				ImageWidget
			]
		]
	];
}

void SSlateIconStaticPreview::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (AutoRefresh)
	{
		UpdateVisualsFunc.ExecuteIfBound();
	}
}

const FSlateBrush* SSlateIconStaticPreview::GetPropertyBrush() const
{
	return &TemporaryBrush;
}

const FSlateBrush* SSlateIconStaticPreview::GetPropertyOverlayBrush() const
{
	return &TemporaryOverlayBrush;
}

FOptionalSize SSlateIconStaticPreview::GetScaledImageBrushWidth() const
{
	FOptionalSize Result;

	if (TargetHeight.IsSet())
	{
		Result = TargetHeight.Get();

		if (TemporaryBrush.DrawAs == ESlateBrushDrawType::Image)
		{
			const FVector2D& Size = TemporaryBrush.ImageSize;
			if (Size.X > 0 && Size.Y > 0)
			{
				Result = Size.X * TargetHeight.Get() / Size.Y;
			}
		}
	}

	if (Result.IsSet() && MinWidth.IsSet())
	{
		Result = FMath::Max<float>(Result.Get(), MinWidth.Get());
	}

	if (Result.IsSet() && MaxWidth.IsSet())
	{
		Result = FMath::Min<float>(Result.Get(), MaxWidth.Get());
	}

	return Result;
}

FOptionalSize SSlateIconStaticPreview::GetScaledImageBrushHeight() const
{
	if (TargetHeight.IsSet())
	{
		return TargetHeight.Get();
	}

	return FOptionalSize();
}

EVisibility SSlateIconStaticPreview::GetVisibilityForPreviewBorder() const
{
	return TemporaryBrush.DrawAs == ESlateBrushDrawType::Image ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SSlateIconStaticPreview::GetVisibilityForPreviewImage() const
{
	return TemporaryBrush.DrawAs == ESlateBrushDrawType::Image ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SSlateIconStaticPreview::GetVisibilityForPreviewIcon() const
{
	return TemporaryOverlayBrush.DrawAs  == ESlateBrushDrawType::Image ? EVisibility::Visible : EVisibility::Collapsed;
}

void SSlateIconStaticPreview::UpdateVisuals(TSharedPtr<IPropertyHandle> IconPropertyHandle)
{
	if (!IconPropertyHandle.IsValid())
	{
		TemporaryBrush = *FStyleDefaults::GetNoBrush();
		TemporaryOverlayBrush = *FStyleDefaults::GetNoBrush();
		return;
	}

	void* RawData = nullptr;
	if (IconPropertyHandle->GetValueData(RawData) == FPropertyAccess::Success && RawData)
	{
		FSlateIconReference* SlateBrush = static_cast<FSlateIconReference*>(RawData);
		if (SlateBrush && SlateBrush->IsSet())
		{
			TemporaryBrush = *SlateBrush->GetIcon();
			TemporaryOverlayBrush =  *SlateBrush->GetOverlayIcon();
		}
		else
		{
			TemporaryBrush = *FStyleDefaults::GetNoBrush();
			TemporaryOverlayBrush = *FStyleDefaults::GetNoBrush();
		}
	}
}

void SSlateIconStaticPreview::UpdateVisuals(TSharedPtr<FSlateIconDescriptor> IconDescriptor)
{
	if (!IconDescriptor.IsValid() || IconDescriptor->IsNone())
	{
		TemporaryBrush = *FStyleDefaults::GetNoBrush();
		return;
	}

	TemporaryBrush = *IconDescriptor->GetBrushSafe();
}

#undef LOCTEXT_NAMESPACE
