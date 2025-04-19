// Copyright 2025, Aquanox.

#include "SlateIconRefDataHelper.h"

#include "IPropertyTypeCustomization.h"
#include "Misc/ConfigCacheIni.h"
#include "PrivateAccessHelper.h"
#include "PropertyHandle.h"
#include "SlateIconRefAccessor.h"
#include "SlateStyleHelper.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SToolTip.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

using FBrushResourcesMap = TMap<FName, FSlateBrush*>;
UE_DEFINE_PRIVATE_MEMBER_PTR(FBrushResourcesMap, GBrushResources, FSlateStyleSet, BrushResources);
UE_DEFINE_PRIVATE_MEMBER_PTR(FSlateBrush*, GDefaultBrush, FSlateStyleSet, DefaultBrush);

static TSharedPtr<FSlateIconRefDataHelper> GDataSource;
FSlateIconRefDataHelper& FSlateIconRefDataHelper::GetDataSource()
{
	if (!GDataSource)
	{
		GDataSource = MakeShared<FSlateIconRefDataHelper>();
	}
	return *GDataSource;
}

void FSlateIconRefDataHelper::SetupStyleData()
{
	if (bInitialized)
	{
		return;
	}

	UE_LOG(LogSlateIcon, Log, TEXT("FSlateIconDataSource::SetupStyleData"));
	bInitialized = true;

	if (!EmptyStyleSet.IsValid())
	{
		EmptyStyleSet = MakeShared<FSlateStyleSetDescriptor>();
		EmptyStyleSet->DisplayTextOverride = LOCTEXT("EmptyStyleSetName", "None");
	}

	if (!AutoStyleSet.IsValid())
	{
		AutoStyleSet = MakeShared<FSlateStyleSetDescriptor>();
		AutoStyleSet->Name = FStyleHelper::GetAppStyleSetName();
		AutoStyleSet->DisplayTextOverride = LOCTEXT("ApplicationStyleSetName", "Application");
	}

	if (!EmptyImage.IsValid())
	{
		EmptyImage = MakeShared<FSlateIconDescriptor>();
		EmptyImage->DisplayTextOverride = LOCTEXT("EmptyImageName", "None");
	}
	
	if (!IgnoredStyleSets.IsSet())
	{
		TArray<FString> Strings;
		GConfig->GetArray(TEXT("SlateIconReference"), TEXT("IgnoredStyleSets"), Strings, GEditorIni);

		TArray<FName> Names;
		Algo::Transform(Strings, Names, [](const FString& S) -> FName { return FName(*S); });

		IgnoredStyleSets = MoveTemp(Names);
	}

	KnownStyleSets.Empty();
	KnownIcons.Empty();
	KnownIconsMap.Empty();

	FSlateStyleRegistry::IterateAllStyles([this](const ISlateStyle& Style)
	{
		FName StyleName = Style.GetStyleSetName();
		if (StyleName.IsNone() || (IgnoredStyleSets.IsSet() && IgnoredStyleSets.GetValue().Contains(StyleName)))
			return true;

		UE_LOG(LogSlateIcon, Verbose, TEXT("Found style: %s"), *StyleName.ToString());

		auto StyleRef = MakeShared<FSlateStyleSetDescriptor>();
		StyleRef->Name = StyleName;
		//StyleRef->Style = &Style;
		KnownStyleSets.Add(StyleRef);

		const FBrushResourcesMap& Map = static_cast<const FSlateStyleSet&>(Style).*GBrushResources;

		for (const TTuple<FName, FSlateBrush*>& Tuple : Map)
		{
			if (Tuple.Key.IsNone() || !Tuple.Value)
				continue;

			UE_LOG(LogSlateIcon, Verbose, TEXT("Found item: %s.%s"), *StyleName.ToString(), *Tuple.Key.ToString());

			auto IconRef = MakeShared<FSlateIconDescriptor>();
			IconRef->StyleSetName = StyleName;
			IconRef->Name = Tuple.Key;
			//IconRef->Brush = Tuple.Value;

			KnownIcons.Add(IconRef);
			KnownIconsMap.Add(MakeTuple(StyleName, Tuple.Key), IconRef);

			StyleRef->Icons.Add(IconRef);
		}

		Algo::Sort(StyleRef->Icons, [](const TSharedPtr<FSlateIconDescriptor>& A, const TSharedPtr<FSlateIconDescriptor>& B)
		{
			return A->Name.ToString() < B->Name.ToString();
		});

		return true;
	});

	Algo::Sort(KnownStyleSets, [](const TSharedPtr<FSlateStyleSetDescriptor>& A, const TSharedPtr<FSlateStyleSetDescriptor>& B)
	{
		return A->Name.ToString() < B->Name.ToString();
	});
	// sort all icons by category and name
	Algo::Sort(KnownIcons, [](const TSharedPtr<FSlateIconDescriptor>& A, const TSharedPtr<FSlateIconDescriptor>& B)
	{
		if (A->StyleSetName != B->StyleSetName)
		{
			return A->StyleSetName.ToString() < B->StyleSetName.ToString();
		}
		return A->Name.ToString() < B->Name.ToString();
	});
}

void FSlateIconRefDataHelper::ClearStyleData()
{
	bInitialized = true;
	KnownIconsMap.Empty();
	KnownStyleSets.Empty();
	KnownIcons.Empty();
}

void FSlateIconRefDataHelper::GatherStyleData(bool bAllowNone, TArray<TSharedPtr<FSlateStyleSetDescriptor>>& OutArray)
{
	if (bAllowNone)
	{
		OutArray.Add(EmptyStyleSet);
	}
	OutArray.Append(KnownStyleSets);
}

void FSlateIconRefDataHelper::GatherIconData(bool bAllowNone, TArray<TSharedPtr<FSlateIconDescriptor>>& OutArray)
{
	if (bAllowNone)
	{
		OutArray.Add(EmptyImage);
	}
	OutArray.Append(KnownIcons);
}

TSharedPtr<FSlateStyleSetDescriptor> FSlateIconRefDataHelper::FindStyleSet(FName StyleSetName)
{
	if (StyleSetName.IsNone())
	{
		return EmptyStyleSet;
	}

	if (auto Found = KnownStyleSets.FindByPredicate([&](const TSharedPtr<FSlateStyleSetDescriptor>& Ref) { return Ref->Name == StyleSetName; }))
	{
		return *Found;
	}

	TSharedPtr<FSlateStyleSetDescriptor> Unknown = MakeShared<FSlateStyleSetDescriptor>();
	Unknown->Name = StyleSetName;
	Unknown->bUnknown = true;
	return Unknown;
}

TSharedPtr<FSlateIconDescriptor> FSlateIconRefDataHelper::FindIcon(FName StyleSetName, FName IconName)
{
	if (StyleSetName.IsNone() || IconName.IsNone())
	{
		return EmptyImage;
	}
	if (auto* Found = KnownIconsMap.Find(MakeTuple(StyleSetName, IconName)))
	{
		return *Found;
	}

	TSharedPtr<FSlateIconDescriptor> Unknown = MakeShared<FSlateIconDescriptor>();
	Unknown->StyleSetName = StyleSetName;
	Unknown->Name = IconName;
	Unknown->bUnknown = true;
	return Unknown;
}

// =============================================================

const FSlateBrush* FSlateIconDescriptor::GetBrushSafe() const
{
	if (!bUnknown)
	{
		const ISlateStyle* StyleSet = FSlateStyleRegistry::FindSlateStyle(StyleSetName);
		if (StyleSet)
		{
			if (const FSlateBrush* Result = StyleSet->GetBrush(Name))
			{
				return Result;
			}
		}
	}
	return FStyleDefaults::GetNoBrush();
}

const FSlateBrush* FSlateIconDescriptor::GetBrush() const
{
	if (!bUnknown)
	{
		const ISlateStyle* StyleSet = FSlateStyleRegistry::FindSlateStyle(StyleSetName);
		if (StyleSet)
		{
			return StyleSet->GetBrush(Name);
		}
	}
	return nullptr;
}

FText FSlateIconDescriptor::GetDisplayText() const
{
	return DisplayTextOverride.IsSet() ? DisplayTextOverride.GetValue() : FText::FromName(Name);
}

bool FSlateIconDescriptor::IsUnknown() const
{
	return bUnknown || (!Name.IsNone() && !GetBrush());
}

// =============================================================

const ISlateStyle* FSlateStyleSetDescriptor::GetStyleSet() const
{
	return FSlateStyleRegistry::FindSlateStyle(Name);
}

FText FSlateStyleSetDescriptor::GetDisplayText() const
{
	return DisplayTextOverride.IsSet() ? DisplayTextOverride.GetValue() : FText::FromName(Name);
}

bool FSlateStyleSetDescriptor::IsUnknown() const
{
	return bUnknown || (!Name.IsNone() && !GetStyleSet());
}


#undef LOCTEXT_NAMESPACE
