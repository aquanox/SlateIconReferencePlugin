// Copyright 2025, Aquanox.

#include "SlateIconRefAccessor.h"

#include "SlateIconReference.h"
#include "PropertyHandle.h"
#include "PropertyEditorModule.h"
#include "SlateIconRefDataHelper.h"
#include "Misc/EngineVersionComparison.h"

DEFINE_LOG_CATEGORY(LogSlateIcon);

FName FSlateIconRefAccessor::Member_StyleSetName()
{
	static const FName Value = GET_MEMBER_NAME_CHECKED(FSlateIconReference, StyleSetName);
	return Value;
}

FName FSlateIconRefAccessor::Member_IconName()
{
	static const FName Value = GET_MEMBER_NAME_CHECKED(FSlateIconReference, IconName);
	return Value;
}

FName FSlateIconRefAccessor::Member_SmallIconName()
{
	static const FName Value = GET_MEMBER_NAME_CHECKED(FSlateIconReference, SmallIconName);
	return Value;
}

FName FSlateIconRefAccessor::Member_OverlayIconName()
{
	static const FName Value = GET_MEMBER_NAME_CHECKED(FSlateIconReference, OverlayIconName);
	return Value;
}

TConstArrayView<FName> FSlateIconRefAccessor::IconMembers()
{
	static const std::initializer_list<FName> Values = {
		GET_MEMBER_NAME_CHECKED(FSlateIconReference, IconName),
		GET_MEMBER_NAME_CHECKED(FSlateIconReference, SmallIconName),
		GET_MEMBER_NAME_CHECKED(FSlateIconReference, OverlayIconName)
	};
	return TConstArrayView<FName>(Values);
}

TConstArrayView<TTuple<FName, ESlateIconDisplayMode>> FSlateIconRefAccessor::IconMemberMasks()
{
	static const std::initializer_list<TTuple<FName, ESlateIconDisplayMode>> Values = {
		MakeTuple(GET_MEMBER_NAME_CHECKED(FSlateIconReference, IconName), ESlateIconDisplayMode::WithIcon),
		MakeTuple(GET_MEMBER_NAME_CHECKED(FSlateIconReference, SmallIconName), ESlateIconDisplayMode::WithSmallIcon),
		MakeTuple(GET_MEMBER_NAME_CHECKED(FSlateIconReference, OverlayIconName), ESlateIconDisplayMode::WithOverlayIcon)
	};
	return TConstArrayView<TTuple<FName, ESlateIconDisplayMode>>(Values);
}

bool FSlateIconRefAccessor::IsEditable() const
{
	return PropertyHandle->IsEditable();
}

bool FSlateIconRefAccessor::AllowClearingValue() const
{
	if (PropertyHandle->HasMetaData("NoClear"))
		return false;
	if (PropertyHandle->GetProperty()->HasAnyPropertyFlags(CPF_NoClear))
		return false;
	return true;
}

bool FSlateIconRefAccessor::ReadPropertyValue(FSlateIconReference& OutData) const
{
	void* RawData = nullptr;
	FPropertyAccess::Result Result = PropertyHandle->GetValueData(RawData);
	if (Result == FPropertyAccess::Success && RawData != nullptr)
	{
		OutData = *static_cast<FSlateIconReference*>(RawData);
	}
	return Result == FPropertyAccess::Success;
}

bool FSlateIconRefAccessor::ReadPropertyValueByName(const FName& InName, FName& OutData) const
{
	TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(InName);
	return ChildHandle->GetValue(OutData) == FPropertyAccess::Success;
}

TSharedPtr<FSlateIconDescriptor> FSlateIconRefAccessor::SelectDescriptorByName(FName InName) const
{
	FName StyleSetName, StyleName;
	if (ReadPropertyValueByName(Member_StyleSetName(), StyleSetName) && ReadPropertyValueByName(InName, StyleName))
	{
		return FSlateIconRefDataHelper::GetDataSource().FindIcon( StyleSetName, StyleName );
	}
	return FSlateIconRefDataHelper::GetDataSource().EmptyImage;
}

TSharedPtr<FSlateStyleSetDescriptor> FSlateIconRefAccessor::GetStyleDescriptor() const
{
	FName StyleSetName;
	if (ReadPropertyValueByName(Member_StyleSetName(), StyleSetName))
	{
		return FSlateIconRefDataHelper::GetDataSource().FindStyleSet(StyleSetName);
	}
	return FSlateIconRefDataHelper::GetDataSource().EmptyStyleSet;
}

void FSlateIconRefAccessor::SetPropertyStyleSetValue(const FName& ValueToSet) const
{
	PropertyHandle->NotifyPreChange();

	void* Data = nullptr;
	FPropertyAccess::Result Result = PropertyHandle->GetValueData(Data);
	if (Result == FPropertyAccess::Success && Data != nullptr)
	{
		auto* Pre = static_cast<FSlateIconReference*>(Data);
		if (Pre->StyleSetName != ValueToSet)
		{ // reset rest
			*Pre = FSlateIconReference(ValueToSet, NAME_None, NAME_None);

			PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			PropertyHandle->NotifyFinishedChangingProperties();
		}
	}
}

void FSlateIconRefAccessor::SetPropertyIconValue(FName InName, const FName& InValue) const
{
	auto InChildHandle = GetHandle()->GetChildHandle(InName);
	check(InChildHandle.IsValid());
	InChildHandle->SetValue(InValue, EPropertyValueSetFlags::NotTransactable);
}
