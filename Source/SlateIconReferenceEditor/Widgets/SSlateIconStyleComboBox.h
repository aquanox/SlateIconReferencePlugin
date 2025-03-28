// Copyright 2025, Aquanox.

#pragma once

#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Internal/SlateIconRefAccessor.h"

/**
 * combobox with search bar for picking stylesets with red font for unknown value and restricted height
 */
class SSlateIconStyleComboBox : public SComboButton
{
public:
	using SComboListType = SListView< TSharedPtr< FSlateStyleSetDescriptor> >;

	using FOnGenerateWidget = TSlateDelegates< TSharedPtr<FSlateStyleSetDescriptor> >::FOnGenerateWidget;
	using FOnSelectionChanged = TSlateDelegates< TSharedPtr<FSlateStyleSetDescriptor> >::FOnSelectionChanged;

	SLATE_BEGIN_ARGS(SSlateIconStyleComboBox)
		{}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	TSharedRef<SWidget> GenerateButtonContent(const FArguments& InArgs);
	TSharedRef<SWidget> GenerateMenuContent(const FArguments& InArgs);

	void ClearSelection();
	void RefreshOptions();
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	TSharedPtr<FSlateStyleSetDescriptor> GetSelectedItem() const;
	bool CanEdit() const;

	FText GetSelectedItemText() const;
	FText GetSelectedItemTooltip() const;
	FSlateColor GetSelectedItemColor() const;
	void SetSelectedItem(TSharedPtr<FSlateStyleSetDescriptor> InSelectedItem, ESelectInfo::Type InSelectInfo = ESelectInfo::OnNavigation);

	TSharedRef<ITableRow> GenerateMenuItemRow(TSharedPtr<FSlateStyleSetDescriptor> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnMenuOpenChanged(bool bOpen);
	void OnSelectionChanged_Internal(TSharedPtr<FSlateStyleSetDescriptor> ProposedSelection, ESelectInfo::Type SelectInfo);
	virtual FReply OnButtonClicked() override;

	void OnSearchTextChanged(const FText& ChangedText);
	void OnSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

private:

	FSlateIconRefAccessor PropertyAccess;

	TArray< TSharedPtr<FSlateStyleSetDescriptor> > OptionsSource;
	TArray< TSharedPtr<FSlateStyleSetDescriptor> > FilteredOptionsSource;
	TSharedPtr<FSlateStyleSetDescriptor> SelectedItem;

	TSharedPtr< SEditableTextBox > SearchField;
	FText SearchText;

	TSharedPtr< SComboListType > ComboListView;
};
