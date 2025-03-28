// Copyright 2025, Aquanox.

#pragma once

#include "Styling/SlateStyle.h"

class FSlateIconReferenceEditorStyle : public FSlateStyleSet
{
public:
	static const FName StyleName;

	FSlateIconReferenceEditorStyle();
	virtual ~FSlateIconReferenceEditorStyle();

private:
	void SetupStyles();
};
