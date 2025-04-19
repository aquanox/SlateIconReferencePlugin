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
#include "Algo/Transform.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

using FBrushResourcesMap = TMap<FName, FSlateBrush*>;
UE_DEFINE_PRIVATE_MEMBER_PTR(FBrushResourcesMap, GBrushResources, FSlateStyleSet, BrushResources);
// using FDynamicBrushResourceMap =  TMap< FName, TWeakPtr< FSlateDynamicImageBrush > >;
// UE_DEFINE_PRIVATE_MEMBER_PTR(FDynamicBrushResourceMap, GDynamicBrushResources, FSlateStyleSet, DynamicBrushes);
UE_DEFINE_PRIVATE_MEMBER_PTR(FSlateBrush*, GDefaultBrush, FSlateStyleSet, DefaultBrush);

#if !UE_VERSION_OLDER_THAN(5,0,0)
UE_DEFINE_PRIVATE_MEMBER_PTR(FName, GParentStyleName, FSlateStyleSet, ParentStyleName);
#endif

struct FDescriptorFinder
{
	const FName Key;
	FDescriptorFinder(const FName& Key) : Key(Key) { }

	bool operator== (const FSlateStyleSetDescriptor& A) const { return A.Name == Key; }
	bool operator== (const TSharedPtr<FSlateStyleSetDescriptor>& A) const { return A->Name == Key; }
	bool operator== (const FSlateIconDescriptor& A) const { return A.Name == Key; }
	bool operator== (const TSharedPtr<FSlateIconDescriptor>& A) const { return A->Name == Key; }
};

struct FDescriptorSorters
{
	bool operator() (const TSharedPtr<FSlateStyleSetDescriptor>& A, const TSharedPtr<FSlateStyleSetDescriptor>& B) const
	{
		return A->Name.ToString() < B->Name.ToString();
	}
	
	bool operator() (const TSharedPtr<FSlateIconDescriptor>& A, const TSharedPtr<FSlateIconDescriptor>& B) const
	{
		return A->Name.ToString() < B->Name.ToString();
	}
};

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
	KnownIconsMap.Empty();

	FSlateStyleRegistry::IterateAllStyles([this](const ISlateStyle& Style)
	{
		FName StyleName = Style.GetStyleSetName();
		if (StyleName.IsNone() || (IgnoredStyleSets.IsSet() && IgnoredStyleSets.GetValue().Contains(StyleName)))
			return true;

		UE_LOG(LogSlateIcon, Verbose, TEXT("Found style: %s"), *StyleName.ToString());

		const FSlateStyleSet& SlateStyleSet = static_cast<const FSlateStyleSet&>(Style);

		auto StyleRef = MakeShared<FSlateStyleSetDescriptor>();
		StyleRef->Name = StyleName;
#if !UE_VERSION_OLDER_THAN(5,0,0)
		StyleRef->ParentStyleName = SlateStyleSet.*GParentStyleName;
#endif
		KnownStyleSets.Add(StyleRef);

		const FBrushResourcesMap& BrushResourcesMap = SlateStyleSet.*GBrushResources;
		for (const auto& KeyToBrush : BrushResourcesMap)
		{
			if (!KeyToBrush.Key.IsNone() && KeyToBrush.Value)
			{
				UE_LOG(LogSlateIcon, Verbose, TEXT("Found item: %s.%s"), *StyleName.ToString(), *KeyToBrush.Key.ToString());

				auto IconRef = MakeShared<FSlateIconDescriptor>();
				IconRef->StyleSetName = StyleName;
				IconRef->Name = KeyToBrush.Key;
				StyleRef->Icons.Add(IconRef);
			}
		}

		Algo::Sort(StyleRef->Icons, FDescriptorSorters());

		return true;
	});

	Algo::Sort(KnownStyleSets, FDescriptorSorters());

	TArray<TSharedPtr<FSlateIconDescriptor>> Temp; 
	for (TSharedPtr<FSlateStyleSetDescriptor>& Descriptor : KnownStyleSets)
	{
		Temp.Empty();
		GatherIconData(false, Descriptor->Name, true, Temp);

		for (const TSharedPtr<FSlateIconDescriptor>& Icon : Temp)
		{
			KnownIconsMap.Add(MakeTuple(Descriptor->Name, Icon->Name), Icon);
		}
	}
}

void FSlateIconRefDataHelper::ClearStyleData()
{
	KnownIconsMap.Empty();
	KnownStyleSets.Empty();
}

void FSlateIconRefDataHelper::GatherStyleData(bool bAllowNone, TArray<TSharedPtr<FSlateStyleSetDescriptor>>& OutArray)
{
	if (bAllowNone)
	{
		OutArray.Add(EmptyStyleSet);
	}
	OutArray.Append(KnownStyleSets);
}

void FSlateIconRefDataHelper::GatherIconData(bool bAllowNone, FName StyleSetName, bool bRecursive, TArray<TSharedPtr<FSlateIconDescriptor>>& OutArray)
{
	if (StyleSetName.IsNone())
		return;

	if (bAllowNone)
		OutArray.Add(EmptyImage);

	if (!bRecursive)
	{
		OutArray.Append(FindStyleSet(StyleSetName, false)->GetRegisteredIcons());
	}
	else
	{
		while (!StyleSetName.IsNone())
		{
			TSharedPtr<FSlateStyleSetDescriptor> Descriptor = FindStyleSet(StyleSetName, false);
			ensure(Descriptor.IsValid());
			OutArray.Append(Descriptor->GetRegisteredIcons());
			StyleSetName = Descriptor->ParentStyleName;
		}

		Algo::Sort(OutArray, FDescriptorSorters());
	}
}

TSharedPtr<FSlateStyleSetDescriptor> FSlateIconRefDataHelper::FindStyleSet(FName StyleSetName, bool bMakeUnknown)
{
	if (StyleSetName.IsNone())
	{
		return EmptyStyleSet;
	}

	if (auto* Found = KnownStyleSets.FindByKey<FDescriptorFinder>(StyleSetName))
	{
		return *Found;
	}

	if (bMakeUnknown)
	{
		TSharedPtr<FSlateStyleSetDescriptor> Unknown = MakeShared<FSlateStyleSetDescriptor>();
		Unknown->Name = StyleSetName;
		Unknown->bUnknown = true;
		return Unknown;
	}

	return EmptyStyleSet;
}

TSharedPtr<FSlateIconDescriptor> FSlateIconRefDataHelper::FindIcon(FName StyleSetName, FName IconName, bool bMakeUnknown)
{
	if (StyleSetName.IsNone() || IconName.IsNone())
	{
		return EmptyImage;
	}

	if (auto* QuickLookup = KnownIconsMap.Find(MakeTuple(StyleSetName, IconName)))
	{
		return *QuickLookup;
	}

	if (auto* StyleSet = KnownStyleSets.FindByKey<FDescriptorFinder>(StyleSetName))
	{
		if (auto* Icon = (*StyleSet)->Icons.FindByKey<FDescriptorFinder>(IconName))
		{
			return *Icon;
		}

		const FName ParentStyleName = (*StyleSet)->ParentStyleName;
		if (!ParentStyleName.IsNone())
		{
			return FindIcon(ParentStyleName, IconName, bMakeUnknown);
		}
	}

	if (bMakeUnknown)
	{
		TSharedPtr<FSlateIconDescriptor> Unknown = MakeShared<FSlateIconDescriptor>();
		Unknown->StyleSetName = StyleSetName;
		Unknown->Name = IconName;
		Unknown->bUnknown = true;
		return Unknown;
	}

	return EmptyImage;
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
