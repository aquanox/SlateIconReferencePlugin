// Copyright 2025, Aquanox.

#include "SPropertyEditorSlateIconRef.h"
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "SlateIconRefTypeCustomization.h"
#include "ScopedTransaction.h"
#include "SlateIconReference.h"
#include "Internal/SlateIconRefDataHelper.h"
#include "Internal/SlateIconRefAccessor.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "SSearchableComboBox.h"
#include "SSlateIconStaticPreview.h"
#include "SSlateIconViewer.h"
#include "SSlateIconStyleComboBox.h"
#include "Internal/SlateStyleHelper.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

namespace Switches
{
	constexpr bool bWithButtonsBox = false;
}

SPropertyEditorSlateIconRef::SPropertyEditorSlateIconRef() : DisplayMode{}
{
}

SPropertyEditorSlateIconRef::~SPropertyEditorSlateIconRef()
{
	for (const auto& Tuple : IconSelectors)
	{
		if (Tuple.Value->ButtonWidget.IsValid() && Tuple.Value->ButtonWidget->IsOpen())
		{
			Tuple.Value->ButtonWidget->SetIsOpen(false);
		}
	}
	IconSelectors.Empty();
}

void SPropertyEditorSlateIconRef::Construct(const FArguments& InArgs, IPropertyTypeCustomizationUtils* InUtils)
{
	PropertyAccess = FSlateIconRefAccessor(InArgs._PropertyHandle);
	DisplayMode = InArgs._DisplayMode;

	bNoClear = !PropertyAccess.AllowClearingValue();

	FName SinglePropertyDisplay = InArgs._SinglePropertyDisplay;
	ensure((SinglePropertyDisplay.IsNone() == EnumHasAnyFlags(DisplayMode, ESlateIconDisplayMode::Compact)));

	for (const auto& NameToMask : FSlateIconRefAccessor::IconMemberMasks())
	{
		// initialize the selector
		IconSelectors.Add(NameToMask.Key,
		                  MakeShared<FIconSelector>(NameToMask.Value, InArgs._PropertyHandle, NameToMask.Key));

		// in compact mode pick first displayed
		if (SinglePropertyDisplay.IsNone() && EnumHasAnyFlags(DisplayMode, NameToMask.Value))
		{
			SinglePropertyDisplay = NameToMask.Key;
		}
	}

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	const FVector2D DefaultThumbnailSize(48.0f, 48.0f);
	const FVector2D CompactThumbnailSize(64.0f, 64.0f);
#else
	const FVector2D DefaultThumbnailSize(48.0f, 48.0f);
	const FVector2D CompactThumbnailSize(48.0f, 48.0f);
#endif

	FVector2D ThumbnailSize = EnumHasAnyFlags(InArgs._DisplayMode, ESlateIconDisplayMode::Compact) ? CompactThumbnailSize : DefaultThumbnailSize;

	TSharedRef<FIconSelector> PreviewTarget = IconSelectors.FindChecked(SinglePropertyDisplay);

	OnUpdatePicker();

	if (!Switches::bRealtimeUpdates)
	{ // subscribe for event-based update
		auto Handler = FSimpleDelegate::CreateSP(this, &SPropertyEditorSlateIconRef::OnUpdatePicker);
		PropertyAccess.GetHandle()->SetOnPropertyValueChanged(Handler);
		PropertyAccess.GetHandle()->SetOnChildPropertyValueChanged(Handler);
	}

	// Build preview image ==================================

	if (EnumHasAnyFlags(InArgs._DisplayMode, ESlateIconDisplayMode::Compact))
	{
		// compact mode allows display of icon + overlay in same box
		SAssignNew(PreviewImage, SLayeredImage)
		.Image(PreviewTarget, &FIconSelector::GetImageForPreview)
		.Visibility(EVisibility::SelfHitTestInvisible);

		if (EnumHasAnyFlags(InArgs._DisplayMode, ESlateIconDisplayMode::WithOverlayIcon)
			&& SinglePropertyDisplay != FSlateIconRefAccessor::Member_OverlayIconName())
		{
			TSharedRef<FIconSelector> OverlayPreviewTarget = IconSelectors.FindChecked(FSlateIconRefAccessor::Member_OverlayIconName());
			auto SecondaryLayerAttr = TAttribute<const FSlateBrush*>(OverlayPreviewTarget, &FIconSelector::GetImageForPreview);
			StaticCastSharedPtr<SLayeredImage>(PreviewImage)->AddLayer(SecondaryLayerAttr);
		}
	}
	else
	{
		// standard mode shows just icon for the current property
		SAssignNew(PreviewImage, SImage)
		.Image(PreviewTarget, &FIconSelector::GetImageForPreview)
		.Visibility(EVisibility::SelfHitTestInvisible);
	}

	// Build Selectors section ===================================

	TSharedRef<SVerticalBox> SelectorsBox = SNew(SVerticalBox);

	// in compact mode style set selector is here
	if (EnumHasAnyFlags(InArgs._DisplayMode, ESlateIconDisplayMode::Compact))
	{
		SelectorsBox->AddSlot()
		            .AutoHeight()
		            .Padding(0, 0, 0, 2)
		[
			SNew(SSlateIconStyleComboBox).PropertyHandle(PropertyAccess.GetHandle())
		];
	}

	// add all icon selectors
	for (auto& Selector : IconSelectors)
	{
		TSharedRef<FIconSelector> IconSelector = Selector.Value;
		if (InArgs._SinglePropertyDisplay == Selector.Key // regular mode
			|| EnumHasAllFlags(InArgs._DisplayMode, IconSelector->DesiredMode|ESlateIconDisplayMode::Compact)) // compact mode
		{
			SelectorsBox->AddSlot()
			            .AutoHeight()
			            .Padding(0, 0, 0,  2)
			[
				SAssignNew(IconSelector->ButtonWidget, SComboButton)
				.ComboButtonStyle(&FStyleHelper::GetWidgetStyle<FComboBoxStyle>("ComboBox").ComboButtonStyle)
				.ButtonStyle(&FStyleHelper::GetWidgetStyle<FComboBoxStyle>("ComboBox").ComboButtonStyle.ButtonStyle)
#if UE_VERSION_OLDER_THAN(5, 0, 0)
				.ContentPadding(FMargin(4.0f, 2.0f))
#else
					.ContentPadding(0)
#endif
				.MenuPlacement(EMenuPlacement::MenuPlacement_ComboBox)
				.OnGetMenuContent(IconSelector, &FIconSelector::GenerateMenu)
				.OnMenuOpenChanged(IconSelector, &FIconSelector::MenuOpenChanged)
				.ToolTipText(IconSelector, &FIconSelector::GetTooltipText)
				.IsEnabled(this, &SPropertyEditorSlateIconRef::CanEdit)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Font( FStyleHelper::GetFontStyle(  TEXT("PropertyWindow.NormalFont") ) )
					.ColorAndOpacity(IconSelector, &FIconSelector::GetTextColor)
					.Text(IconSelector, &FIconSelector::GetText)
				]
			];
		}
	}

	// Build buttons section ========================

	TSharedRef<SHorizontalBox> ButtonBox = SNew(SHorizontalBox);
	if (Switches::bWithButtonsBox && !bNoClear)
	{
		ButtonBox->AddSlot()
		         .AutoWidth()
		         .HAlign(HAlign_Center)
		         .VAlign(VAlign_Center)
		         .Padding(0, 0, 2, 0)
		[
			PropertyCustomizationHelpers::MakeClearButton(
				FSimpleDelegate::CreateSP(this, &SPropertyEditorSlateIconRef::OnClear, SinglePropertyDisplay),
				LOCTEXT( "ActionClear", "Clear"),
				TAttribute<bool>(this, &SPropertyEditorSlateIconRef::CanEdit)
			)
		];
	}



	// Build widget content ============================

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(FMargin(3.0f,3.0f,5.0f,3.0f))
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Visibility(EVisibility::SelfHitTestInvisible)

			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			.Padding(FMargin(2.0f, 2.0f, 2.0f, 2.0f))
			.BorderImage(FStyleHelper::GetBrush("AssetThumbnail.ClassBackground"))
#else
			.Padding(FMargin(0.0f, 0.0f, 4.0f, 4.0f))
			.BorderImage(FStyleHelper::GetBrush("PropertyEditor.AssetTileItem.DropShadow"))
#endif
			[
				SNew(SOverlay)
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
				+SOverlay::Slot()
				.Padding(FMargin(1.0f))
				[
					SAssignNew(PreviewBorder, SBorder)
					.Padding(0)
					.BorderImage(FStyleDefaults::GetNoBrush())
					[
						SNew(SBox)
						.WidthOverride(ThumbnailSize.X)
						.HeightOverride(ThumbnailSize.Y)
						[
							PreviewImage.ToSharedRef()
						]
					]
				]

				+ SOverlay::Slot()
				[
					SNew(SImage)
						.Image(this, &SPropertyEditorSlateIconRef::GetImageForThumbnailBorder)
						.Visibility(EVisibility::SelfHitTestInvisible)
				]
#else
				+ SOverlay::Slot()
				.Padding(FMargin(1.0f))
				[
					SNew(SBorder)
					.Padding(0)
					.BorderImage(FEditorStyle::GetBrush("NoBorder"))
					[
						SNew(SBox)
						.WidthOverride(ThumbnailSize.X)
						.HeightOverride(ThumbnailSize.Y)
						[
							PreviewImage.ToSharedRef()
						]
					]
				]
#endif
				+SOverlay::Slot()
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(PreviewTarget, &FIconSelector::GetStatusLabel)
						.Font(FStyleHelper::GetFontStyle("AssetThumbnail.FontSmall"))
						.ColorAndOpacity(FStyleHelper::GetSlateColor("AssetThumbnail.ColorAndOpacity"))
						.Justification(ETextJustify::Center)
						.Visibility(PreviewTarget, &FIconSelector::GetVisibilityForStatusLabel)
					]
				]
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(0.0f,3.0f,5.0f,3.0f)
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		.FillWidth(1.0f)
		[
			SNew(SBox)
			.MinDesiredWidth(150.f)
			.MaxDesiredWidth(400.f)
			[
				SelectorsBox
			]
		]

		+SHorizontalBox::Slot()
		.Padding(0.0f,3.0f,5.0f,3.0f)
		.AutoWidth()
		[
			SNew(SBox)
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				ButtonBox
			]
		]
	];
}

void SPropertyEditorSlateIconRef::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (Switches::bRealtimeUpdates)
	{
		OnUpdatePicker();
	}
}

SPropertyEditorSlateIconRef::FIconSelector::FIconSelector(ESlateIconDisplayMode InMode, TSharedPtr<IPropertyHandle> InHandle, FName InMember)
	: PropertyAccess(InHandle), DesiredMode(InMode), TargetName(InMember)
{
}

void SPropertyEditorSlateIconRef::FIconSelector::OnUpdate()
{
	SelectedIcon = PropertyAccess.SelectDescriptorByName(TargetName);
	check(SelectedIcon.IsValid());
}

const FSlateBrush* SPropertyEditorSlateIconRef::FIconSelector::GetImageForPreview() const
{
	return SelectedIcon->GetBrush();
}

FText SPropertyEditorSlateIconRef::FIconSelector::GetStatusLabel() const
{
	if (SelectedIcon->IsUnknown())
	{
		return LOCTEXT("UnknownImageLabel", "Missing");
	}
	if (SelectedIcon->IsNone())
	{
		return LOCTEXT("EmptyImageName", "None");
	}
	return FText::GetEmpty();
}

FText SPropertyEditorSlateIconRef::FIconSelector::GetText() const
{
	return SelectedIcon->GetDisplayText();
}

FSlateColor SPropertyEditorSlateIconRef::FIconSelector::GetTextColor() const
{
	static const FSlateColor ErrorTextColor { FLinearColor::Red };

	if (SelectedIcon->IsUnknown())
	{
		return ErrorTextColor;
	}
	return FSlateColor::UseForeground();
}

FText SPropertyEditorSlateIconRef::FIconSelector::GetTooltipText() const
{
	return FText::Format(LOCTEXT("IconBoxTooltipFormat", "{0}: {1}"), FText::FromName(TargetName), GetText());
}

EVisibility SPropertyEditorSlateIconRef::FIconSelector::GetVisibilityForStatusLabel() const
{
	if (SelectedIcon->IsNone() || SelectedIcon->IsUnknown())
	{
		return EVisibility::SelfHitTestInvisible;
	}
	return EVisibility::Collapsed;
}

const FSlateBrush* SPropertyEditorSlateIconRef::GetImageForThumbnailBorder() const
{
	return PreviewBorder->IsHovered()
		? FStyleHelper::GetBrush("PropertyEditor.AssetThumbnailBorderHovered")
		: FStyleHelper::GetBrush("PropertyEditor.AssetThumbnailBorder");
}

void SPropertyEditorSlateIconRef::OnClear(FName InTarget)
{
	UE_LOG(LogSlateIcon, Log, TEXT("SSlateIconPickerBox(%p)::OnClear"), this);

	if (InTarget.IsNone() || EnumHasAnyFlags(DisplayMode, ESlateIconDisplayMode::Compact))
	{ // compact mode clears entire property
		 PropertyAccess.SetPropertyStyleSetValue(TEXT(""));
	}
	else
	{ // regular mode clears only specific property
		 PropertyAccess.SetPropertyIconValue(InTarget, TEXT(""));
	}
}

bool SPropertyEditorSlateIconRef::CanEdit() const
{
	return PropertyAccess.IsEditable();
}

void SPropertyEditorSlateIconRef::OnUpdatePicker()
{
	for (auto& IconSelector : IconSelectors)
	{
		IconSelector.Value->OnUpdate();
	}
}

TSharedRef<SWidget> SPropertyEditorSlateIconRef::FIconSelector::GenerateMenu() const
{
	constexpr float MaxMenuHeight = 500.f;
	return SNew(SBox)
		.Visibility(EVisibility::Visible)
		.WidthOverride(300.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(MaxMenuHeight)
			[
				SNew(SSlateIconViewer)
					.PropertyHandle(PropertyAccess.GetHandle())
					.SinglePropertyDisplay(TargetName)
					.OnIconSelected(SSlateIconViewer::FOnIconSelected::CreateRaw(this, &FIconSelector::OnIconSelected))
			]
		];
}

void SPropertyEditorSlateIconRef::FIconSelector::MenuOpenChanged(bool bIsOpen) const
{
	if (bIsOpen == false)
	{
		ButtonWidget->SetMenuContent(SNullWidget::NullWidget);
	}
}

void SPropertyEditorSlateIconRef::FIconSelector::OnIconSelected(TSharedPtr<FSlateIconDescriptor> InIcon, ESelectInfo::Type InType) const
{
	UE_LOG(LogSlateIcon, Log, TEXT("SSlateIconPickerBox(%p)::OnIconSelected %s"), this, InIcon.IsValid() ? *InIcon->Name.ToString() : TEXT(""));

	PropertyAccess.SetPropertyIconValue(TargetName, InIcon->Name);

	ButtonWidget->SetIsOpen(false);
}

#undef LOCTEXT_NAMESPACE
