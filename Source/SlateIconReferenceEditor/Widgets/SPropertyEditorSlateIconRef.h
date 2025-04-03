// Copyright 2025, Aquanox.

#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Components/SlateWrapperTypes.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "Internal/SlateIconRefAccessor.h"

class IPropertyTypeCustomizationUtils;
class IPropertyHandle;
class SBorder;
class SImage;
struct FSlateIconDescriptor;

/**
 * Content for property editor value widget for FSlateIcon
 */
class SPropertyEditorSlateIconRef : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SPropertyEditorSlateIconRef )
		: _DisplayMode() {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)
		SLATE_ARGUMENT(ESlateIconDisplayMode, DisplayMode)
		SLATE_ARGUMENT(FName, SinglePropertyDisplay)
	SLATE_END_ARGS()

	SPropertyEditorSlateIconRef();
	virtual ~SPropertyEditorSlateIconRef();

	void Construct(const FArguments& InArgs, IPropertyTypeCustomizationUtils* InUtils);

private:
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	void OnUpdatePicker();
	void OnClear(FName InTarget);
	bool CanEdit() const;

	// { preview image
	const FSlateBrush* GetImageForThumbnailBorder() const;
	// }

	// { icon selector combo
	struct FIconSelector : public TSharedFromThis<FIconSelector>
	{
		FSlateIconRefAccessor		PropertyAccess;
		const ESlateIconDisplayMode DesiredMode;
		const FName					TargetName;

		TSharedPtr<FSlateIconDescriptor> SelectedIcon;

		TSharedPtr<class SComboButton> ButtonWidget;

		explicit FIconSelector(ESlateIconDisplayMode InMode, TSharedPtr<IPropertyHandle> InHandle, FName InMember);

		void OnUpdate();

		const FSlateBrush* GetImageForPreview() const;

		FText GetStatusLabel() const;
		EVisibility GetVisibilityForStatusLabel() const;

		FText GetText() const;
		FSlateColor GetTextColor() const;
		FText GetTooltipText() const;

		TSharedRef<SWidget> GenerateMenu() const;
		void MenuOpenChanged(bool bIsOpen) const;
		void OnIconSelected(TSharedPtr<FSlateIconDescriptor> InIcon, ESelectInfo::Type InType) const;
	};
	// }

private:
	// { general
	FSlateIconRefAccessor  PropertyAccess;
	ESlateIconDisplayMode  DisplayMode;

	bool bNoClear = false;

	TMap<FName, TSharedRef<FIconSelector>> IconSelectors;
	// }

	// { preview image
	TSharedPtr<SBorder> PreviewBorder;
	TSharedPtr<SImage> PreviewImage;
	// }

};
