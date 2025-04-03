// Copyright 2025, Aquanox.

#include "SlateIconReferenceEditorStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"

const FName FSlateIconReferenceEditorStyle::StyleName(TEXT("SlateIconReferenceEditor"));

FSlateIconReferenceEditorStyle::FSlateIconReferenceEditorStyle() : FSlateStyleSet(TEXT("SlateIconReferenceEditor"))
{
	SetupStyles();

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FSlateIconReferenceEditorStyle::~FSlateIconReferenceEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

void FSlateIconReferenceEditorStyle::SetupStyles()
{
}
