#pragma once

#include "Templates/SharedPointer.h"
#include "Widgets/SToolTip.h"
#include "Widgets/SBoxPanel.h"

struct FSlateIconDescriptor;

/**
 * Fullsize image tooltip for list view
 */
class SIconTooltip : public SToolTip
{
public:
	SLATE_BEGIN_ARGS(SIconTooltip)
	{ }
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FSlateIconDescriptor> InDesc);
	bool IsEmpty() const override;
	virtual void OnOpening() override;
	virtual void OnClosed() override;
private:
	TWeakPtr<FSlateIconDescriptor> Descriptor;
};

/**
 * Image details tooltip for list view
 */
class SSlateIconViewerRowTooltip : public SToolTip
{
public:
	SLATE_BEGIN_ARGS(SSlateIconViewerRowTooltip)
	{ }
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FSlateIconDescriptor> InDesc);

	void BuildInfo(const FSlateIconDescriptor& Desc,  const TSharedRef<SVerticalBox>& InfoBox);
	static void AddToToolTipInfoBox(const TSharedRef<SVerticalBox>& InfoBox, const FText& Key, const FText& Value);

	virtual bool IsEmpty() const override;
	virtual void OnOpening() override;
	virtual void OnClosed() override;

private:
	TWeakPtr<FSlateIconDescriptor> Descriptor;
};
