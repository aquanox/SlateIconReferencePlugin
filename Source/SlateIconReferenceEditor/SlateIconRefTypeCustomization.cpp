// Copyright 2025, Aquanox.

#include "SlateIconRefTypeCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "MaterialShared.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorClipboard.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "SlateIconRefDataHelper.h"
#include "SlateIconReference.h"
#include "Containers/Array.h"
#include "Containers/SparseArray.h"
#include "Delegates/Delegate.h"
#include "Engine/Texture2D.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SPropertyEditorSlateIconRef.h"
#include "Widgets/SSlateIconStaticPreview.h"
#include "Widgets/SSlateIconViewer.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SToolTip.h"
#include "Widgets/SSlateIconStyleComboBox.h"
#include "Internal/SlateIconRefAccessor.h"
#include "Internal/SlateIconRefDataHelper.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

TSharedRef<IPropertyTypeCustomization> FSlateIconRefTypeCustomization::MakeInstance()
{
	return MakeShared<FSlateIconRefTypeCustomization>();
}

FSlateIconRefTypeCustomization::~FSlateIconRefTypeCustomization()
{

}

void FSlateIconRefTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InUtils)
{
	UE_LOG(LogSlateIcon, Verbose, TEXT("FSlateIconRefTypeCustomization(%p)::CustomizeHeader %s"), this, *InPropertyHandle->GetPropertyDisplayName().ToString());

	PropertyHandle = InPropertyHandle;
	PropertyUtilities = InUtils.GetPropertyUtilities();

	FSlateIconRefDataHelper::GetDataSource().SetupStyleData();

	ESlateIconDisplayMode DesiredDisplayMode = ESlateIconDisplayMode::Standard;
	if (PropertyHandle->HasMetaData("DisplayMode"))
	{
		TArray<FString> Tokens;
		PropertyHandle->GetMetaData("DisplayMode").ToLower().ParseIntoArrayWS(Tokens, TEXT(","));
		for (const FString& Token : Tokens)
		{
			// display mode values
			if (Token.Equals(TEXT("compact")))		DesiredDisplayMode |= ESlateIconDisplayMode::Compact;
			if (Token.Equals(TEXT("default")))		DesiredDisplayMode |= ESlateIconDisplayMode::Default;
			// mixin-all
			if (Token.Equals(TEXT("withall")))		DesiredDisplayMode |= ESlateIconDisplayMode::WithAll;
			// mixin-icon
			if (Token.Equals(TEXT("withicon")))		DesiredDisplayMode |= ESlateIconDisplayMode::WithIcon;
			if (Token.Equals(TEXT("noicon")))		DesiredDisplayMode &= ~ESlateIconDisplayMode::WithIcon;
			// mixin-smallicon
			if (Token.Equals(TEXT("withsmall")))	DesiredDisplayMode |= ESlateIconDisplayMode::WithSmallIcon;
			if (Token.Equals(TEXT("nosmall")))		DesiredDisplayMode &= ~ESlateIconDisplayMode::WithSmallIcon;
			// mixin-overlay
			if (Token.Equals(TEXT("withoverlay")))	DesiredDisplayMode |= ESlateIconDisplayMode::WithOverlayIcon;
			if (Token.Equals(TEXT("nooverlay")))	DesiredDisplayMode &= ~ESlateIconDisplayMode::WithOverlayIcon;
		}
	}
	// if just base category was selected (just standard or compact) enable defaults for those modes
	switch (DesiredDisplayMode)
	{
		case ESlateIconDisplayMode::Standard:
			DisplayMode = ESlateIconDisplayMode::DefaultStandard;
			break;
		case ESlateIconDisplayMode::Compact:
			DisplayMode = ESlateIconDisplayMode::DefaultCompact;
			break;
		default:
			DisplayMode = DesiredDisplayMode;
			break;
	}

	bNoClear = PropertyHandle->HasMetaData("NoClear") || PropertyHandle->GetProperty()->HasAnyPropertyFlags(CPF_NoClear);
	const bool bAutoExpand = PropertyHandle->HasMetaData("AutoExpand");

	PropertyHandle->GetChildHandle(FSlateIconRefAccessor::Member_IconName())
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &ThisClass::OnTryGuessSmallImage));

	if (EnumHasAnyFlags(DisplayMode, ESlateIconDisplayMode::Compact))
	{
		InHeaderRow
		.AddCustomContextMenuAction(
			FUIAction(
				FExecuteAction::CreateRaw(this, &ThisClass::OnReset, FName()),
				FCanExecuteAction::CreateRaw(this, &ThisClass::CanEdit)
			),
			LOCTEXT("ActionResethName", "Reset to None"),
			LOCTEXT("ActionResetTooltip", "Reset value of this property"),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
		)
		.AddCustomContextMenuAction(
			FUIAction(FExecuteAction::CreateRaw(this, &ThisClass::OnCopyAsBrush, FSlateIconRefAccessor::Member_IconName())),
			LOCTEXT("ActionCopyAsBrushName", "Copy as Brush"),
			LOCTEXT("ActionCopyAsBrushTooltip", "Copy reference to this icon as Slate Brush"),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
		)
		.AddCustomContextMenuAction(
			FUIAction(FExecuteAction::CreateRaw(this, &ThisClass::OnCopyAsBrushArray)),
			LOCTEXT("ActionCopyAsBrushName", "Copy as Brush Array"),
			LOCTEXT("ActionCopyAsBrushTooltip", "Copy all references to this item as Slate Brush"),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
		)
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(210.f)
		.MaxDesiredWidth(0.f)
		[
			SNew(SPropertyEditorSlateIconRef, &InUtils)
				.PropertyHandle(PropertyHandle)
				.DisplayMode(DisplayMode)
		];
	}
	else
	{
		InHeaderRow
		.ShouldAutoExpand(bAutoExpand)
		.AddCustomContextMenuAction(
			FUIAction(
				FExecuteAction::CreateRaw(this, &ThisClass::OnReset, FName()),
				FCanExecuteAction::CreateRaw(this, &ThisClass::CanEdit)
			),
			LOCTEXT("ActionResethName", "Reset to None"),
			LOCTEXT("ActionResetTooltip", "Reset value of this property"),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
		)
		.AddCustomContextMenuAction(
			FUIAction(FExecuteAction::CreateRaw(this, &ThisClass::OnCopyAsBrushArray)),
			LOCTEXT("ActionCopyAsBrushName", "Copy as Brush Array"),
			LOCTEXT("ActionCopyAsBrushTooltip", "Copy all references to this item as Slate Brush"),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
		)
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SSlateIconStaticPreview).TargetHeight(18.f).SourceProperty(PropertyHandle)
		];
	}
}

void FSlateIconRefTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& InBuilder, IPropertyTypeCustomizationUtils& InUtils)
{
	if (EnumHasAnyFlags(DisplayMode, ESlateIconDisplayMode::Compact))
	{
		return;
	}

	{
		TSharedPtr<IPropertyHandle> HandleStyleSetName = InPropertyHandle->GetChildHandle(FSlateIconRefAccessor::Member_StyleSetName());

		IDetailPropertyRow& PropertyRow = InBuilder.AddProperty(HandleStyleSetName.ToSharedRef());
		PropertyRow
			.CustomWidget(true)
			.AddCustomContextMenuAction(
				FUIAction(
					FExecuteAction::CreateRaw(this, &ThisClass::OnReset, FName()),
					FCanExecuteAction::CreateRaw(this, &ThisClass::CanEdit)
				),
				LOCTEXT("ActionResethName", "Reset to None"),
				LOCTEXT("ActionResetTooltip", "Reset value of this property"),
				FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
			)
			.NameContent()
			[
				HandleStyleSetName->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MinDesiredWidth(150.f)
			.MaxDesiredWidth(400.f)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.Padding(0, 2, 2, 2)
				.AutoWidth()
				[
					SNew(SSlateIconStyleComboBox).PropertyHandle(PropertyHandle)
				]
			];
	}

	for (const auto& NameToMask : FSlateIconRefAccessor::IconMemberMasks())
	{
		const bool bEnabled = EnumHasAnyFlags(DisplayMode, NameToMask.Value);
		if (!bEnabled)
		{
			continue;
		}

		TSharedPtr<IPropertyHandle> ChildPropHandle = InPropertyHandle->GetChildHandle(NameToMask.Key);

		IDetailPropertyRow& PropertyRow = InBuilder.AddProperty(ChildPropHandle.ToSharedRef());
		PropertyRow
			.CustomWidget(true)
			.AddCustomContextMenuAction(
				FUIAction(
					FExecuteAction::CreateRaw(this, &ThisClass::OnReset, NameToMask.Key),
					FCanExecuteAction::CreateRaw(this, &ThisClass::CanEdit)
				),
				LOCTEXT("ActionResethName", "Reset to None"),
				LOCTEXT("ActionResetTooltip", "Reset value of this property"),
				FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
			)
			.AddCustomContextMenuAction(
				FUIAction(FExecuteAction::CreateRaw(this, &ThisClass::OnCopyAsBrush, NameToMask.Key)),
				LOCTEXT("ActionCopyAsBrushName", "Copy as Brush"),
				LOCTEXT("ActionCopyAsBrushTooltip", "Copy reference to this asset as Slate Brush"),
				FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "GenericCommands.Copy")
			)
			.NameContent()
			[
				ChildPropHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MinDesiredWidth(150.f)
			.MaxDesiredWidth(400.f)
			[
				SNew(SPropertyEditorSlateIconRef, &InUtils)
					.PropertyHandle(PropertyHandle)
					.DisplayMode(DisplayMode)
					.SinglePropertyDisplay(NameToMask.Key)
			];
	}
}

bool FSlateIconRefTypeCustomization::CanEdit() const
{
	return PropertyHandle.IsValid() && PropertyHandle->IsEditable();
}

void FSlateIconRefTypeCustomization::OnTryGuessSmallImage() const
{
	FSlateIconRefAccessor Accessor(PropertyHandle);

	FSlateIconReference Reference;
	if (Accessor.ReadPropertyValue(Reference))
	{
		FSlateIcon AsIcon = Reference.ToSlateIcon();
		if (AsIcon.GetOptionalIcon() && (Reference.SmallIconName.IsNone() || AsIcon.GetOptionalSmallIcon()))
		{ // if got a valid icon, guess small icon from it if possible
			Accessor.SetPropertyIconValue( FSlateIconRefAccessor::Member_SmallIconName(), AsIcon.GetSmallStyleName() );
		}
	}
}

void FSlateIconRefTypeCustomization::OnReset(FName InMember) const
{
	FSlateIconRefAccessor Accessor(PropertyHandle);

	if (InMember.IsNone())
	{
		Accessor.SetPropertyStyleSetValue(TEXT(""));
	}
	else
	{
		Accessor.SetPropertyIconValue(InMember, TEXT(""));
	}
}

void FSlateIconRefTypeCustomization::OnCopyAsBrush(FName InMember) const
{
	FSlateIconRefAccessor Accessor(PropertyHandle);

	if (const FSlateBrush* Brush = Accessor.SelectDescriptorByName(InMember)->GetBrushSafe())
	{
		FString Value;
		StaticStruct<FSlateBrush>()->ExportText(Value, Brush, nullptr, nullptr, PPF_Copy, nullptr);
		if (!Value.IsEmpty())
		{
			FPropertyEditorClipboard::ClipboardCopy(*Value);
		}
	}
}

void FSlateIconRefTypeCustomization::OnCopyAsBrushArray() const
{
	FSlateIconRefAccessor Accessor(PropertyHandle);

	TStringBuilder<1024> Value;

	for (auto& NameToMask : FSlateIconRefAccessor::IconMemberMasks())
	{
		if (EnumHasAnyFlags(DisplayMode, NameToMask.Value))
		{
			if (const FSlateBrush* Brush = Accessor.SelectDescriptorByName(NameToMask.Key)->GetBrushSafe())
			{
				FString InnerValue;
				StaticStruct<FSlateBrush>()->ExportText(InnerValue, Brush, nullptr, nullptr, PPF_Copy, nullptr);

				if (Value.Len()) Value.Append(TEXT(",\n"));
				Value.Append(InnerValue);
			}
		}
	}

	Value.InsertAt(0, TEXT("("));
	Value.Append(TEXT(")"));

	FPropertyEditorClipboard::ClipboardCopy(*Value);
}

void FSlateIconRefTypeCustomization::OnExportBrushAsset(FName InHandle) const
{
}

#undef LOCTEXT_NAMESPACE
