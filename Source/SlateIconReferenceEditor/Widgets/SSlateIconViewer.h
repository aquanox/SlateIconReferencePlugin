// Copyright 2025, Aquanox.

#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Internal/SlateIconRefDataHelper.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "PropertyCustomizationHelpers.h"
#include "Internal/SlateIconRefAccessor.h"

using FOnGatherData = TDelegate<void(TArray<TSharedPtr<FSlateIconDescriptor>>&)>;
using FOnFilterChanged = TDelegate<void()>;
using FOnFilterTest = TDelegate<bool(const FSlateIconDescriptor&, const FString&)>;

struct FIconViewerFilter
{
	TSharedPtr<IPropertyHandle> PropertyHandle;
	FText Label;

	FString DefaultValue;
	FString SelectedValue;

	FOnGatherData OptionsSource;
	FOnFilterChanged OnSelectionChanged;
	FOnFilterTest OnTest;

	//TWeakPtr<SComboButton> Content;

	inline bool TestFilter(const FSlateIconDescriptor& InDesc) const
	{
		return OnTest.IsBound() ? OnTest.Execute(InDesc, SelectedValue) : true;
	}
};

/**
 * Complex content for combo menu
 */
class SSlateIconViewer  : public SCompoundWidget
{
public:
	using FOnIconSelected = TDelegate<void(TSharedPtr<FSlateIconDescriptor>, ESelectInfo::Type InType)>;

	SLATE_BEGIN_ARGS(SSlateIconViewer)
		{}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)
		SLATE_ARGUMENT(FName, SinglePropertyDisplay)
		SLATE_EVENT(FOnIconSelected, OnIconSelected)
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs);
	void Populate();
	void Refresh() { bNeedsRefresh = true; }

	bool ReadPropertyValue(FName* OutStyleSet = nullptr, FName* OutIcon = nullptr) const;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnFocusReceived( const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent ) override;

	// { menu - search box
	void TextFilter_TextChanged( const FText& InFilterText ) ;
	void TextFilter_Commit(const FText& InText, ETextCommit::Type CommitInfo);
	// }

	// { menu - options
	TSharedRef<SWidget> OptionsCombo_GenerateMenu();
	void OptionsCombo_GenerateDrawTypeMenu(FMenuBuilder&);
	void OptionsCombo_GenerateImageTypeSubmenu(FMenuBuilder&);
	void OptionsCombo_ToggleInherited();
	bool OptionsCombo_ToggleInheritedChecked() const { return bShowInheritedFilter; }
	// }

	// { menu - list
	using FViewItem = FSlateIconDescriptor;
	TSharedRef<ITableRow>  IconViewerList_GenerateRow(TSharedPtr<FViewItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void IconViewerList_SelectionChanged( TSharedPtr<FViewItem> Item, ESelectInfo::Type SelectInfo );
	// }

	// menu - footer
	FText GetSelectedStyleSetIconCountText() const;

	// { misc - filters
    static TSharedRef<SWidget> BuildSelectorWidgetRow(TSharedRef<FIconViewerFilter> InContext, FPropertyComboBoxArgs& Args);
    static TSharedRef<SWidget> CreateGroupSelector(TSharedRef<FIconViewerFilter> InContext);
    // }

private:
	TSharedPtr<IPropertyHandle> MainPropertyHandle;
	TSharedPtr<IPropertyHandle> ChildPropertyHandle;

	bool bNoClear = false;
	bool bPendingFocusNextFrame = false;
	bool bNeedsRefresh = false;

	FOnIconSelected OnIconSelected;

	// { menu - styleset combobox
	TSharedPtr<SComboBox<TSharedPtr<FSlateStyleSetDescriptor>>> StyleSelectorCombo;
	TArray<TSharedPtr<FSlateStyleSetDescriptor>> StyleSelectorDataSource;
	// }

	// { menu - filter comboboxes
	TSharedPtr<FIconViewerFilter> GroupFilter;
	TSharedPtr<FIconViewerFilter> DrawTypeFilter;
	TSharedPtr<FIconViewerFilter> ImageTypeFilter;
	static bool					  bShowInheritedFilter;
	// }

	// { menu - search
	TSharedPtr<class SSearchBox> SearchBox;
	TSharedPtr<class FTextFilterExpressionEvaluator> TextFilter;
	// }

	// { menu - icon listview
	TSharedPtr<SListView<TSharedPtr<FViewItem>>> IconViewerList;
	FName LastUsedStyleSet = NAME_None;
	TArray<TSharedPtr<FViewItem>> IconsDataSource;
	TArray<TSharedPtr<FViewItem>> FilteredDataSource;
	// }
};


/**
 * Complex content for menu listview rows
 */
class SSlateIconViewerRow : public STableRow<TSharedPtr<FSlateIconDescriptor>>
{
	using ThisClass = SSlateIconViewerRow;
	using Super = STableRow<TSharedPtr<FSlateIconDescriptor>>;
public:
	SLATE_BEGIN_ARGS( SSlateIconViewerRow )
		{}
		SLATE_ARGUMENT( TSharedPtr<FSlateIconDescriptor>, Descriptor )
		SLATE_ARGUMENT( FText, HighlightText )
		SLATE_ARGUMENT( TSharedPtr<SSlateIconViewer::FViewItem>, AssociatedNode )
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView );

private:
	TWeakPtr<FSlateIconDescriptor> Descriptor;
	TWeakPtr<SSlateIconViewer::FViewItem>  AssociatedNode;
};
