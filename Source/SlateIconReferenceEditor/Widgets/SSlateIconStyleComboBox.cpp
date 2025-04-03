// Copyright 2025, Aquanox.

#include "SSlateIconStyleComboBox.h"

#include "SlateIconReference.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Widgets/Input/SComboBox.h"
#include "Internal/SlateStyleHelper.h"
#include "Internal/SlateIconRefDataHelper.h"
#include "Internal/SlateIconRefAccessor.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

void SSlateIconStyleComboBox::Construct(const FArguments& InArgs)
{
	check(InArgs._PropertyHandle);

	PropertyAccess = FSlateIconRefAccessor(InArgs._PropertyHandle);

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	ForegroundColor = FCoreStyle::Get().GetSlateColor("InvertedForeground");
	ContentPadding = FMargin(4.0, 2.0);
#else
	ForegroundColor = FSlateColor::UseStyle();
	ContentPadding = FMargin(0);
#endif

	FSlateIconRefDataHelper::GetDataSource().GatherStyleData(PropertyAccess.AllowClearingValue(), OptionsSource);

	const FComboBoxStyle* ComboStyle = &FStyleHelper::GetWidgetStyle<FComboBoxStyle>("ComboBox");

	SComboButton::Construct(SComboButton::FArguments()
		.ComboButtonStyle(&ComboStyle->ComboButtonStyle)
		.ButtonStyle(&ComboStyle->ComboButtonStyle.ButtonStyle)
		.HasDownArrow(true)
		.MenuPlacement(EMenuPlacement::MenuPlacement_BelowAnchor)
		.ContentPadding(ContentPadding)
		.ForegroundColor(ForegroundColor)
		.OnMenuOpenChanged(this, &SSlateIconStyleComboBox::OnMenuOpenChanged)
		.IsFocusable(true)
		.ButtonContent()
		[
			GenerateButtonContent(InArgs)
		]
		.MenuContent()
		[
			GenerateMenuContent(InArgs)
		]
	);

	SetToolTipText(TAttribute<FText>(this, &SSlateIconStyleComboBox::GetSelectedItemTooltip));
	SetEnabled(TAttribute<bool>(this, &SSlateIconStyleComboBox::CanEdit));

	SetMenuContentWidgetToFocus(SearchField);

	if (SelectedItem.IsValid() && !SelectedItem->IsUnknown())
	{
		ComboListView->Private_SetItemSelection(SelectedItem, true);
	}
}

TSharedRef<SWidget> SSlateIconStyleComboBox::GenerateButtonContent(const FArguments& InArgs)
{
	return SNew(STextBlock)
			.Text(this, &SSlateIconStyleComboBox::GetSelectedItemText)
			.Font(FStyleHelper::GetFontStyle( "PropertyWindow.NormalFont" ))
			.ColorAndOpacity(this, &SSlateIconStyleComboBox::GetSelectedItemColor);
}

TSharedRef<SWidget> SSlateIconStyleComboBox::GenerateMenuContent(const FArguments& InArgs)
{
	constexpr float MaxMenuHeight = 300.f;
	return SNew(SBox)
		.MinDesiredWidth(210.f)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 2)
			[
				SAssignNew(this->SearchField, SEditableTextBox)
				.HintText(LOCTEXT("Search", "Search"))
				.OnTextChanged(this, &SSlateIconStyleComboBox::OnSearchTextChanged)
				.OnTextCommitted(this, &SSlateIconStyleComboBox::OnSearchTextCommitted)
			]

			+ SVerticalBox::Slot()
			.MaxHeight(MaxMenuHeight)
			.AutoHeight()
			[
				SAssignNew(ComboListView, SComboListType)
				.ListItemsSource(&FilteredOptionsSource)
				.OnGenerateRow(this, &SSlateIconStyleComboBox::GenerateMenuItemRow)
				.OnSelectionChanged(this, &SSlateIconStyleComboBox::OnSelectionChanged_Internal)
				.SelectionMode(ESelectionMode::Single)
			]
		];
}

void SSlateIconStyleComboBox::ClearSelection()
{
	ComboListView->ClearSelection();
}

TSharedPtr<FSlateStyleSetDescriptor> SSlateIconStyleComboBox::GetSelectedItem() const
{
	return SelectedItem;
}

bool SSlateIconStyleComboBox::CanEdit() const
{
	return PropertyAccess.IsEditable();
}

FText SSlateIconStyleComboBox::GetSelectedItemText() const
{
	if (SelectedItem.IsValid())
	{
		return SelectedItem->GetDisplayText();
	}
	return INVTEXT("None");
}

FText SSlateIconStyleComboBox::GetSelectedItemTooltip() const
{
	return GetSelectedItemText();
}

FSlateColor SSlateIconStyleComboBox::GetSelectedItemColor() const
{
	if (SelectedItem.IsValid() && SelectedItem->IsUnknown())
	{
		return FLinearColor::Red;
	}
	return ForegroundColor;
}

void SSlateIconStyleComboBox::SetSelectedItem(TSharedPtr<FSlateStyleSetDescriptor> InSelectedItem, ESelectInfo::Type InSelectInfo)
{
	if (InSelectedItem.IsValid() && !InSelectedItem->IsUnknown())
	{
		ComboListView->SetSelection(InSelectedItem, InSelectInfo);
	}
	else
	{
		ComboListView->SetSelection(SelectedItem, InSelectInfo);
	}
}

TSharedRef<ITableRow> SSlateIconStyleComboBox::GenerateMenuItemRow(TSharedPtr<FSlateStyleSetDescriptor> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	auto ComboStyle = &FStyleHelper::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox");

#if !UE_VERSION_OLDER_THAN(5, 0, 0)
	auto ComboRowStyle = &FStyleHelper::GetWidgetStyle<FTableRowStyle>("ComboBox.Row");
#else
	auto ComboRowStyle = &FStyleHelper::GetWidgetStyle<FTableRowStyle>("TableView.Row");
#endif

	return SNew(SComboRow<TSharedPtr<FSlateStyleSetDescriptor>>, OwnerTable)
		.Style(ComboRowStyle)
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
		.Padding(ComboStyle->MenuRowPadding)
#endif
		[
			SNew(STextBlock)
				.Text(InItem->GetDisplayText())
				.Font(FStyleHelper::GetFontStyle( "PropertyWindow.NormalFont" ))
		];
}

void SSlateIconStyleComboBox::OnMenuOpenChanged(bool bOpen)
{
	if (bOpen == false)
	{
		if (SelectedItem.IsValid() && !SelectedItem->IsUnknown())
		{
			// Ensure the ListView selection is set back to the last committed selection
			ComboListView->SetSelection(SelectedItem, ESelectInfo::OnNavigation);
		}

		// Set focus back to ComboBox for users focusing the ListView that just closed
		FSlateApplication::Get().ForEachUser([this](FSlateUser& User)
		{
			TSharedRef<SWidget> ThisRef = this->AsShared();
			if (User.IsWidgetInFocusPath(ComboListView))
			{
				User.SetFocus(ThisRef);
			}
		});
	}
	else
	{
		SearchText = FText::GetEmpty();
		RefreshOptions();
	}
}

void SSlateIconStyleComboBox::OnSelectionChanged_Internal(TSharedPtr<FSlateStyleSetDescriptor> ProposedSelection, ESelectInfo::Type SelectInfo)
{
	if (!ProposedSelection)
	{
		return;
	}

	UE_LOG(LogSlateIcon, Log, TEXT("SSlateIconStyleComboBox(%p)::OnStyleSetSelected %s"), this, ProposedSelection.IsValid() ? *ProposedSelection->Name.ToString() : TEXT(""));

	bool bAlwaysSelectItem = false;
	if (ProposedSelection != SelectedItem || bAlwaysSelectItem)
	{
		SelectedItem = ProposedSelection;
		//OnSelectionChanged.ExecuteIfBound(ProposedSelection, SelectInfo);
		PropertyAccess.SetPropertyStyleSetValue(ProposedSelection->Name);
	}

	// close combo as long as the selection wasn't from navigation
	if (SelectInfo != ESelectInfo::OnNavigation)
	{
		this->SetIsOpen(false);
	}
	else
	{
		ComboListView->RequestScrollIntoView(SelectedItem, 0);
	}
}

FReply SSlateIconStyleComboBox::OnButtonClicked()
{
	if (this->IsOpen())
	{
		// Re-select first selected item, just in case it was selected by navigation previously
		TArray<TSharedPtr<FSlateStyleSetDescriptor>> SelectedItems = ComboListView->GetSelectedItems();
		if (SelectedItems.Num() > 0)
		{
			OnSelectionChanged_Internal(SelectedItems[0], ESelectInfo::Direct);
		}
	}
	else
	{
		SearchField->SetText(FText::GetEmpty());
		//OnComboBoxOpening.ExecuteIfBound();
	}

	return SComboButton::OnButtonClicked();
}

void SSlateIconStyleComboBox::OnSearchTextChanged(const FText& ChangedText)
{
	SearchText = ChangedText;
	RefreshOptions();
}

void SSlateIconStyleComboBox::OnSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if ((InCommitType == ETextCommit::Type::OnEnter) && FilteredOptionsSource.Num() > 0)
	{
		ComboListView->SetSelection(FilteredOptionsSource[0], ESelectInfo::OnKeyPress);
	}
}

void SSlateIconStyleComboBox::RefreshOptions()
{
	FilteredOptionsSource.Empty();

	if (SearchText.IsEmpty())
	{
		FilteredOptionsSource.Append(OptionsSource);
	}
	else
	{
		TArray<FString> SearchTokens;
		SearchText.ToString().ParseIntoArrayWS(SearchTokens);

		for (const TSharedPtr<FSlateStyleSetDescriptor>& Option : OptionsSource)
		{
			const FString ShownString = Option->GetDisplayText().ToString();

			bool bAllTokensMatch = true;
			for (const FString& SearchToken : SearchTokens)
			{
				if (!ShownString.Contains(SearchToken, ESearchCase::Type::IgnoreCase))
				{
					bAllTokensMatch = false;
					break;
				}
			}

			if (bAllTokensMatch)
			{
				FilteredOptionsSource.Add(Option);
			}
		}
	}

	ComboListView->RequestListRefresh();
}

void SSlateIconStyleComboBox::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SelectedItem = PropertyAccess.GetStyleDescriptor();

	SComboButton::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

#undef LOCTEXT_NAMESPACE
