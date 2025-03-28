// Copyright 2025, Aquanox.

#include "SSlateIconViewer.h"
#include "PropertyCustomizationHelpers.h"
#include "SlateIconReference.h"
#include "SlateIconRefTypeCustomization.h"
#include "SlateIconRefDataHelper.h"
#include "Internal/SlateStyleHelper.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "SListViewSelectorDropdownMenu.h"
#include "SSearchableComboBox.h"
#include "SSlateIconStaticPreview.h"
#include "SSlateIconViewerTooltips.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

TSharedRef<SWidget> SearchSlateWidget( TSharedRef<SWidget> Content, const FName& InType );

namespace Switches
{
	//
	constexpr EMenuPlacement ComboxWithinPopup = MenuPlacement_BelowAnchor;

	// should list contain "none" option
	constexpr bool bShouldListContainNone = false;

	constexpr bool bWithMenuSection = true;
	// prefixes filter dropdown
	constexpr bool bWithGroupSelector = true;
	// options button next to search bar with various filters
	constexpr bool bWithOptions = true;
}

void SSlateIconViewer::Construct(const FArguments& InArgs)
{
	MainPropertyHandle = InArgs._PropertyHandle;
	ChildPropertyHandle = InArgs._PropertyHandle->GetChildHandle(InArgs._SinglePropertyDisplay);
	OnIconSelected = InArgs._OnIconSelected;

	TextFilter = MakeShared<FTextFilterExpressionEvaluator>(ETextFilterExpressionEvaluatorMode::BasicString);

	auto MenuSection = SNew(SVerticalBox);
	if (Switches::bWithMenuSection)
	{

	}

	auto FiltersSection = SNew(SVerticalBox);
	if (Switches::bWithGroupSelector)
	{
		GroupFilter = MakeShared<FIconViewerFilter>();
		GroupFilter->PropertyHandle = MainPropertyHandle;
		GroupFilter->OnSelectionChanged.BindRaw(this, &SSlateIconViewer::Refresh);
		GroupFilter->OptionsSource.BindLambda([this](TArray<TSharedPtr<FSlateIconDescriptor>>& OutData)
		{
			OutData.Reserve(IconViewerDataSource.Num());
			for (auto& Ptr : IconViewerDataSource)
			{
				OutData.Add(Ptr->IconDescriptor);
			}
		});

		// set default value for group filter to match current
		FName SelectedGroup;
		FString Prefix;
		if (ReadPropertyValue(nullptr, &SelectedGroup) && SelectedGroup.ToString().Split(TEXT("."), &Prefix, nullptr))
		{
			GroupFilter->SelectedValue = Prefix;
		}

		FiltersSection->AddSlot()
		.AutoHeight()
		[
			SSlateIconViewer::CreateGroupSelector(GroupFilter.ToSharedRef())
		];
	}

	TSharedPtr<SWidget> IconViewerContent;

	SAssignNew(IconViewerContent, SBox)
	.Visibility(EVisibility::Visible)
	.MinDesiredHeight(200.0f)
	.MaxDesiredHeight(800.0f)
	[
		SNew(SBorder)
		.BorderImage(FStyleDefaults::GetNoBrush())
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				MenuSection
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				FiltersSection
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.Padding(2.0f, 2.0f, 6.0f, 2.0f)
				[
					SAssignNew(SearchBox, SSearchBox)
					.OnTextChanged( this, &SSlateIconViewer::TextFilter_TextChanged )
					.OnTextCommitted( this, &SSlateIconViewer::TextFilter_Commit )
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				[
					SNew(SComboButton)
					.ContentPadding(0)
					.Visibility(Switches::bWithOptions ? EVisibility::Visible : EVisibility::Collapsed)
					.ForegroundColor(FSlateColor::UseForeground())
					.ComboButtonStyle(FStyleHelper::Get(), "SimpleComboButton")
					.HasDownArrow(false)
					.OnGetMenuContent(this, &SSlateIconViewer::OptionsCombo_GenerateMenu)
					.MenuPlacement(Switches::ComboxWithinPopup)
					.ButtonContent()
					[
						SNew(SImage)
						.Image(FStyleHelper::GetBrush("Icons.Settings"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]

			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBox)
				.MinDesiredHeight(150.f)
				[
					SAssignNew(IconViewerList, SListView<TSharedPtr<FViewItem>>)
					.SelectionMode(ESelectionMode::Single)
					.ListItemsSource(&FilteredDataSource)
					.OnGenerateRow(this, &SSlateIconViewer::IconViewerList_GenerateRow)
					.OnSelectionChanged(this, &SSlateIconViewer::IconViewerList_SelectionChanged)
					.HeaderRow
					(
						SNew(SHeaderRow)
						.Visibility(EVisibility::Collapsed)
						+ SHeaderRow::Column(TEXT("Icon"))
						.DefaultLabel(LOCTEXT("Icon", "Icon"))
					)
				]
			]

			// Bottom panel
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(4.0f)
			[
				SNew(STextBlock).Text(this, &SSlateIconViewer::GetSelectedStyleSetIconCountText)
			]
		]
	];

	ChildSlot
	[
		SNew(SListViewSelectorDropdownMenu<TSharedPtr<FViewItem>>, SearchBox, IconViewerList)
		[
			IconViewerContent.ToSharedRef()
		]
	];

	bNeedsRefresh = true;
	bPendingFocusNextFrame = true;
}

void SSlateIconViewer::Populate()
{
	IconViewerDataSource.Empty();
	FilteredDataSource.Empty();

	FName StyleSetName;
	if (ReadPropertyValue(&StyleSetName))
	{
		auto SelectedStyleSet = FSlateIconRefDataHelper::GetDataSource().FindStyleSet(StyleSetName);
		ensure(SelectedStyleSet.IsValid());
		for (auto& IconDescriptor : SelectedStyleSet->GetRegisteredIcons())
		{
			bool bPassFilter = true;

			const FString NameString = IconDescriptor->Name.ToString();
			if (TextFilter->GetFilterType() != ETextFilterExpressionType::Empty)
				bPassFilter = TextFilter->TestTextFilter(FBasicStringFilterExpressionContext(NameString));
			if (GroupFilter.IsValid() && !GroupFilter->TestFilter(*IconDescriptor))
				bPassFilter = false;
			if (DrawTypeFilter.IsValid() && !DrawTypeFilter->TestFilter(*IconDescriptor))
				bPassFilter = false;
			if (ImageTypeFilter.IsValid() && !ImageTypeFilter->TestFilter(*IconDescriptor))
				bPassFilter = false;

			auto Item = MakeShared<FViewItem>();
			Item->IconDescriptor = IconDescriptor;
			Item->bPassesFilter = bPassFilter;
			IconViewerDataSource.Add(Item);

			if (Item->bPassesFilter)
			{
				FilteredDataSource.Add(Item);
			}
		}
	}

	if (Switches::bShouldListContainNone && !bNoClear)
	{ // add None option to list
		auto Item = MakeShared<FViewItem>();
		Item->IconDescriptor = FSlateIconRefDataHelper::GetDataSource().EmptyImage;
		IconViewerDataSource.Insert(Item, 0);
		FilteredDataSource.Insert(Item, 0);
	}

	IconViewerList->RequestListRefresh();
}

bool SSlateIconViewer::ReadPropertyValue(FName* OutStyleSet, FName* OutIcon) const
{
	void* TmpAddress = nullptr;
	if (MainPropertyHandle->GetValueData(TmpAddress) != FPropertyAccess::Success || !TmpAddress)
	{
		return false;
	}
	if (OutStyleSet)
	{
		*OutStyleSet = static_cast<const FSlateIconReference*>(TmpAddress)->StyleSetName;
	}

	TmpAddress = nullptr;
	if (ChildPropertyHandle->GetValueData(TmpAddress) != FPropertyAccess::Success || !TmpAddress)
	{
		return false;
	}
	if (OutIcon)
	{
		*OutIcon = *static_cast<const FName*>(TmpAddress);
	}

	return true;
}

void SSlateIconViewer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Move focus to search box
	if (bPendingFocusNextFrame && SearchBox.IsValid())
	{
		FWidgetPath WidgetToFocusPath;
		FSlateApplication::Get().GeneratePathToWidgetUnchecked( SearchBox.ToSharedRef(), WidgetToFocusPath );
		FSlateApplication::Get().SetKeyboardFocus( WidgetToFocusPath, EFocusCause::SetDirectly );
		bPendingFocusNextFrame = false;
	}

	if (bNeedsRefresh)
	{
		bNeedsRefresh = false;
		Populate();
	}
}

FReply SSlateIconViewer::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	if (InFocusEvent.GetCause() == EFocusCause::Navigation)
	{
		FSlateApplication::Get().SetKeyboardFocus(SearchBox.ToSharedRef(), EFocusCause::SetDirectly);
	}
	if (InFocusEvent.GetCause() == EFocusCause::Mouse || InFocusEvent.GetCause() == EFocusCause::Cleared)
	{

	}
	return FReply::Handled();
}

TSharedRef<ITableRow> SSlateIconViewer::IconViewerList_GenerateRow(TSharedPtr<SSlateIconViewer::FViewItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	ensure(Item->IconDescriptor.IsValid());

	return SNew(SSlateIconViewerRow, OwnerTable)
		.Descriptor(Item->IconDescriptor)
		.HighlightText(SearchBox->GetText())
		.AssociatedNode(Item);
}

void SSlateIconViewer::IconViewerList_SelectionChanged(TSharedPtr<SSlateIconViewer::FViewItem> Item, ESelectInfo::Type SelectInfo)
{
	OnIconSelected.ExecuteIfBound(Item->IconDescriptor, SelectInfo);
}

void SSlateIconViewer::TextFilter_TextChanged(const FText& InFilterText)
{
	if (TextFilter.IsValid())
	{
		TextFilter->SetFilterText(InFilterText);
		SearchBox->SetError(TextFilter->GetFilterErrorText());
	}

	Refresh();
}

void SSlateIconViewer::TextFilter_Commit(const FText& InText, ETextCommit::Type InCommitType)
{
	if ((InCommitType == ETextCommit::Type::OnEnter) && FilteredDataSource.Num() > 0)
	{
		IconViewerList->SetSelection(FilteredDataSource[0], ESelectInfo::OnKeyPress);
	}
}

TSharedRef<SWidget> SSlateIconViewer::OptionsCombo_GenerateMenu()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, nullptr, nullptr, /*bCloseSelfOnly=*/ true);

	MenuBuilder.BeginSection("Filters", LOCTEXT("IconViewerFiltersHeading", "Filters"));
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("DrawTypeMenuOption", "Draw Type"),
			LOCTEXT("DrawTypeMenuOptionTooltip", "Filters list by specific draw type."),
			FNewMenuDelegate::CreateSP(this, &SSlateIconViewer::OptionsCombo_GenerateDrawTypeMenu),
			false, FSlateIcon(), false
			);
		MenuBuilder.AddSubMenu(
			LOCTEXT("IconTypeMenuOption", "Image Type"),
			LOCTEXT("IconTypeMenuOptionTooltip", "Filters list by specific image type."),
			FNewMenuDelegate::CreateSP(this, &SSlateIconViewer::OptionsCombo_GenerateImageTypeSubmenu),
			false, FSlateIcon(), false
			);

	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SSlateIconViewer::OptionsCombo_GenerateDrawTypeMenu(FMenuBuilder& MenuBuilder)
{
	static const FString OPTION_ANY = TEXT("Any");

	if (!DrawTypeFilter.IsValid())
	{
		DrawTypeFilter = MakeShared<FIconViewerFilter>();

		DrawTypeFilter->DefaultValue = OPTION_ANY;
		DrawTypeFilter->SelectedValue = OPTION_ANY;
		DrawTypeFilter->OnTest.BindLambda([=](const FSlateIconDescriptor& InDesc, const FString& Selected)
		{
			const FSlateBrush* SlateBrush = InDesc.GetBrush();
			if (!Selected.IsEmpty() && Selected != OPTION_ANY && SlateBrush)
			{
				auto Current = StaticEnum<ESlateBrushDrawType::Type>()->GetNameStringByValue(SlateBrush->GetDrawType());
				return Selected == Current;
			}
			return true;
		});
	}

	auto Enum = StaticEnum<ESlateBrushDrawType::Type>();
	for (int Index = 0; Index < Enum->NumEnums(); ++Index)
	{
		const FString NameValue = Index == Enum->GetMaxEnumValue() ? OPTION_ANY : Enum->GetNameStringByIndex(Index);
		const FText DisplayText = Index == Enum->GetMaxEnumValue() ? LOCTEXT("FilterAnyLabel", "Any") : Enum->GetDisplayNameTextByIndex(Index);

		MenuBuilder.AddMenuEntry(
			DisplayText,
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this, NameValue](){ DrawTypeFilter->SelectedValue = NameValue; Refresh(); }),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([this, NameValue]() { return DrawTypeFilter->SelectedValue == NameValue; })
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);
	}
}

void SSlateIconViewer::OptionsCombo_GenerateImageTypeSubmenu(FMenuBuilder& MenuBuilder)
{
	static const FString OPTION_ANY = TEXT("Any");

	if (!ImageTypeFilter.IsValid())
	{
		ImageTypeFilter = MakeShared<FIconViewerFilter>();
		ImageTypeFilter->DefaultValue = OPTION_ANY;
		ImageTypeFilter->SelectedValue = OPTION_ANY;
		ImageTypeFilter->OnTest.BindLambda([=](const FSlateIconDescriptor& InDesc, const FString& Selected)
		{
			const FSlateBrush* SlateBrush = InDesc.GetBrush();
			if (!Selected.IsEmpty() && Selected != OPTION_ANY && SlateBrush)
			{
				auto Current = StaticEnum<ESlateBrushImageType::Type>()->GetNameStringByValue(SlateBrush->GetImageType());
				return Selected == Current;
			}
			return true;
		});
	}

	auto Enum = StaticEnum<ESlateBrushImageType::Type>();
	for (int Index = 0; Index < Enum->NumEnums(); ++Index)
	{
		const FString NameValue = Index == Enum->GetMaxEnumValue() ? OPTION_ANY : Enum->GetNameStringByIndex(Index);
		const FText DisplayText = Index == Enum->GetMaxEnumValue() ? LOCTEXT("FilterAnyLabel", "Any") : Enum->GetDisplayNameTextByIndex(Index);

		MenuBuilder.AddMenuEntry(
			DisplayText,
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this, NameValue](){ ImageTypeFilter->SelectedValue = NameValue; Refresh(); }),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([this, NameValue]() { return ImageTypeFilter->SelectedValue == NameValue; })
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);
	}

}

FText SSlateIconViewer::GetSelectedStyleSetIconCountText() const
{
	const int32 NumAssets = IconViewerDataSource.Num() + (bNoClear ? 0 : 1) ;
	const int32 NumFilteredAssets = FilteredDataSource.Num();

	FText AssetCount = LOCTEXT("IconCountLabelSingular", "1 item");

	if (NumFilteredAssets == NumAssets)
	{
		if (NumAssets == 1)
		{
			AssetCount = LOCTEXT("IconCountLabelSingular", "1 item");
		}
		else
		{
			AssetCount = FText::Format(LOCTEXT("IconCountLabelPlural", "{0} items"), FText::AsNumber(NumAssets));
		}
	}
	else
	{
		AssetCount = FText::Format(LOCTEXT("IconCountLabelPluralPlusSelection", "{0} of {1} items"), FText::AsNumber(NumFilteredAssets), FText::AsNumber(NumAssets));
	}

	return AssetCount;
}

// ==========================================================================================
// ==========================================================================================


TSharedRef<SWidget> SSlateIconViewer::BuildSelectorWidgetRow(TSharedRef<FIconViewerFilter> InContext,  FPropertyComboBoxArgs& InArgs)
{
	InArgs.ShowSearchForItemCount = -1;

	TSharedRef<SWidget> PropertyEditorWidget = PropertyCustomizationHelpers::MakePropertyComboBox(InArgs);
	//TSharedRef<SSearchableComboBox> ComboBox = StaticCastSharedRef<SSearchableComboBox>(SearchSlateWidget(PropertyEditorWidget, TEXT("SPropertyComboBox")));
	//check(ComboBox->GetType() == TEXT("SPropertyComboBox"));
	//ComboBox->SetMenuPlacement(Switches::ViewerMenuPopup);
	//InContext->Content = ComboBox;

	TSharedRef<SHorizontalBox> SelectorRow =
	SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(STextBlock).Text(InContext->Label).MinDesiredWidth(80.f)
		]

		+ SHorizontalBox::Slot()
		.Padding(0.0f, 2.0f, 6.0f, 2.0f)
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			PropertyEditorWidget
		]

		+ SHorizontalBox::Slot()
		.Padding(2.0f, 2.0f)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			PropertyCustomizationHelpers::MakeResetButton(FSimpleDelegate::CreateLambda([InContext]()
			{
				InContext->SelectedValue = InContext->DefaultValue;
				InContext->OnSelectionChanged.ExecuteIfBound();

				// if (auto Combobox = InContext->Content.Pin())
				// {
				// 	if (Combobox->IsOpen())
				// 	{
				// 		Combobox->SetIsOpen(false);
				// 	}
				// }
			}), LOCTEXT("ActionClearGroupFilter", "Reset Filter"))
		];

	return SelectorRow;
}

TSharedRef<SWidget> SSlateIconViewer::CreateGroupSelector(TSharedRef<FIconViewerFilter> InContext)
{
	static const FString EmptyValueText(TEXT("None"));

	InContext->Label = LOCTEXT("IconViewerGroupFilterLabel", "Icon Group");
	InContext->DefaultValue = EmptyValueText;
	if (InContext->SelectedValue.IsEmpty())
	{
		InContext->SelectedValue = InContext->DefaultValue;
	}

	FPropertyComboBoxArgs Args;
	Args.PropertyHandle = InContext->PropertyHandle;
	Args.ShowSearchForItemCount = -1;

	Args.OnGetStrings.BindLambda([InContext](TArray<TSharedPtr<FString>>& OutStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems)
	{
		TArray<TSharedPtr<FSlateIconDescriptor>> SourceData;
		InContext->OptionsSource.ExecuteIfBound(SourceData);

		TArray<FString> UniqueGroups;
		for (const TSharedPtr<FSlateIconDescriptor>& Item : SourceData)
		{
			if (!Item->IsNone())
			{
				FString IconName = Item->Name.ToString();
				FString Group;
				if (IconName.Split(TEXT("."), &Group, nullptr))
				{
					UniqueGroups.AddUnique(Group);
				}
			}
		}

		Algo::Sort(UniqueGroups);

		OutStrings.Add(MakeShared<FString>(TEXT("None")));
		OutToolTips.Add(nullptr);
		OutRestrictedItems.Add(false);

		for (const FString& UniqueGroup : UniqueGroups)
		{
			OutStrings.Add(MakeShared<FString>(UniqueGroup));
			OutToolTips.Add(nullptr);
			OutRestrictedItems.Add(false);
		}
	});
	Args.OnGetValue.BindLambda([InContext]()
	{
		if (InContext->SelectedValue.IsEmpty())
		{
			return EmptyValueText;
		}
		return InContext->SelectedValue;
	});
	Args.OnValueSelected.BindLambda([InContext](const FString& In)
	{
		if (In == EmptyValueText)
		{
			InContext->SelectedValue.Empty();
		}
	   else
	   {
		   InContext->SelectedValue = In;
	   }

	  InContext->OnSelectionChanged.ExecuteIfBound();
	});

	InContext->OnTest.BindLambda([=](const FSlateIconDescriptor& InDesc, const FString& Selected)
	{
		if (!Selected.IsEmpty() && Selected != EmptyValueText)
		{
			return InDesc.Name.ToString().StartsWith(Selected, ESearchCase::IgnoreCase);
		}
		return true;
	});

	return BuildSelectorWidgetRow(InContext, Args);
}

// ==========================================================================================
// ==========================================================================================

void SSlateIconViewerRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._Descriptor.IsValid());

	Descriptor = InArgs._Descriptor;
	AssociatedNode = InArgs._AssociatedNode;

	ChildSlot
	[
		SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f, 6.0f, 2.0f)
				[
					SNew(SBox)
					.HeightOverride(24.f)
					.WidthOverride(24.f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(EVisibility::Visible)
					.ToolTip(SNew(SIconTooltip, InArgs._Descriptor.ToSharedRef()))
					[
						SNew(SSlateIconStaticPreview)
							.TargetHeight(24.f)
							.MaxWidth(24.f)
							.SourceDescriptor(InArgs._Descriptor)
							.AutoRefresh(false)
					]
				]

			+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 3.0f, 6.0f, 3.0f)
				[
					SNew(SBox)
					.Visibility(EVisibility::Visible)
					.ToolTip(SNew(SSlateIconViewerRowTooltip, InArgs._Descriptor.ToSharedRef()))
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(InArgs._Descriptor->GetDisplayText())
							.Font(FStyleHelper::GetFontStyle("NormalFont"))
							.HighlightText(InArgs._HighlightText)
							.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
	];

	Super::ConstructInternal(STableRow::FArguments()
		.ShowSelection(true)
		, InOwnerTableView
	);
}

#undef LOCTEXT_NAMESPACE