// Copyright 2025, Aquanox.

#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateBrush.h"
#include "Textures/SlateIcon.h"
#include "Misc/EngineVersionComparison.h"

#define WITH_EDITOR_STYLE  (UE_VERSION_OLDER_THAN(5, 0, 0))

#if WITH_EDITOR_STYLE
#include "EditorStyleSet.h"
#else
#include "Styling/AppStyle.h"
#endif

// ReSharper disable CppDeprecatedEntity

/**
 * Collection of functions for cross-engine compliance
 */
struct FStyleHelper
{
	static FName GetAppStyleSetName()
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::Get().GetStyleSetName();
#else
		return FAppStyle::GetAppStyleSetName();
#endif
	}

	static const ISlateStyle& Get()
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::Get();
#else
		return FAppStyle::Get();
#endif
	}

    static const FSlateBrush* GetDefaultBrush()
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::GetDefaultBrush();
#else
		return FAppStyle::Get().GetDefaultBrush();
#endif
	}

    static const FSlateBrush* GetNoBrush()
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::GetNoBrush();
#else
		return FAppStyle::GetNoBrush();
#endif
	}

    static const FSlateBrush* GetBrush(const FName& InName)
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::Get().GetBrush(InName);
#else
		return FAppStyle::Get().GetBrush(InName);
#endif
	}

    static FSlateFontInfo GetFontStyle(const FName& InName)
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::Get().GetFontStyle(InName);
#else
		return FAppStyle::Get().GetFontStyle(InName);
#endif
	}

    static FSlateColor GetSlateColor(const FName& InName)
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::Get().GetSlateColor(InName);
#else
		return FAppStyle::Get().GetSlateColor(InName);
#endif
	}

	template <class T>
	static const T& GetWidgetStyle(FName PropertyName, const ANSICHAR* Specifier = nullptr)
	{
#if WITH_EDITOR_STYLE
		return FEditorStyle::Get().GetWidgetStyle<T>(PropertyName, Specifier);
#else
		return FAppStyle::Get().GetWidgetStyle<T>(PropertyName, Specifier);
#endif
	}

};

template<typename T>
TSharedRef<SWidget> SearchSlateWidget( TSharedRef<SWidget> Content, const FName& InType )
{
	if (Content->GetType() == InType)
	{
		return Content;
	}

	FChildren* Children = Content->GetChildren();
	int32 NumChildren = Children->Num();

	for( int32 Index = 0; Index < NumChildren; ++Index )
	{
		TSharedRef<SWidget> Found = SearchSlateWidget<T>( Children->GetChildAt( Index ), InType);
		if (Found != SNullWidget::NullWidget)
		{
			return Found;
		}
	}
	return SNullWidget::NullWidget;
}


#undef WITH_EDITOR_STYLE
