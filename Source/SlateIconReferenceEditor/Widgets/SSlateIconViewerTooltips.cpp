#include "SSlateIconViewerTooltips.h"

#include "Internal/SlateStyleHelper.h"
#include "Internal/SlateIconRefDataHelper.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SlateIconReference"

void SIconTooltip::Construct(const FArguments& InArgs, TSharedRef<FSlateIconDescriptor> InDesc)
{
	Descriptor = InDesc;

	SToolTip::Construct(SToolTip::FArguments()
		.TextMargin(1.f)
		.BorderImage(FStyleHelper::GetBrush("AssetThumbnail.Tooltip.Border"))
	);
}

bool SIconTooltip::IsEmpty() const
{
	auto Pinned = Descriptor.Pin();
	return !Pinned.IsValid() || !Pinned->GetBrush();
}

void SIconTooltip::OnOpening()
{
	if (auto Pinned = Descriptor.Pin())
	{
		SetContentWidget(
			SNew(SBox)
			.MinDesiredHeight(32.f)
			.MinDesiredWidth(32.f)
			.Padding(6)
			[
				SNew(SImage).Image(Pinned->GetBrushSafe())
			]
		);
	}
}

void SIconTooltip::OnClosed()
{
	ResetContentWidget();
}

// ==========================================================================================
// ==========================================================================================

void SSlateIconViewerRowTooltip::Construct(const FArguments& InArgs, TSharedRef<FSlateIconDescriptor> InDesc)
{
	Descriptor = InDesc;

	SToolTip::Construct(SToolTip::FArguments()
		.TextMargin(1.f)
		.BorderImage(FStyleHelper::GetBrush("ContentBrowser.TileViewTooltip.ToolTipBorder"))
	);
}

bool SSlateIconViewerRowTooltip::IsEmpty() const
{
	auto Pinned = Descriptor.Pin();
	return !Pinned.IsValid() || !Pinned->GetBrush();
}

void SSlateIconViewerRowTooltip::AddToToolTipInfoBox(const TSharedRef<SVerticalBox>& InfoBox, const FText& Key, const FText& Value)
{
	InfoBox->AddSlot()
	.AutoHeight()
	.Padding(0, 1)
	[
		SNew(SHorizontalBox)

		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 4, 0)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("IconDetailFormat", "{0}:"), Key))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]

		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(Value)
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
	];
}

void SSlateIconViewerRowTooltip::BuildInfo(const FSlateIconDescriptor& Desc, const TSharedRef<SVerticalBox>& InfoBox)
{
	if (const FSlateBrush* Brush = Desc.GetBrush())
	{
		const ESlateBrushDrawType::Type DrawAsType = Brush->GetDrawType();

		AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipStyleSet","StyleSet"), FText::FromName(Desc.StyleSetName));

		AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipDrawAs","Draw As"), StaticEnum<ESlateBrushDrawType::Type>()->GetDisplayNameTextByValue(DrawAsType));

		{
			FVector2D ImageSize = Brush->GetImageSize();
			if (ImageSize.IsZero() && Brush->GetDrawType() != ESlateBrushImageType::Vector)
			{
				auto& Resource = Brush->GetRenderingResource();
				if (Resource.IsValid())
				{
					ImageSize = Brush->GetRenderingResource().GetResourceProxy()->ActualSize;
				}
			}

			TStringBuilder<64> Builder;
			Builder.Appendf(TEXT("%.2f x %.2f"), ImageSize.X, ImageSize.Y);
			AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipImageSize","Image Size"), FText::FromString(Builder.ToString()));
		}

		AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipImageType","Image Type"), StaticEnum<ESlateBrushImageType::Type>()->GetDisplayNameTextByValue(Brush->GetImageType()));
		AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipTiling","Tiling"), StaticEnum<ESlateBrushTileType::Type>()->GetDisplayNameTextByValue(Brush->GetTiling()));

		{
			TStringBuilder<64> Builder;
			Builder.Appendf(TEXT("%.2f "), Brush->GetMargin().Left);
			Builder.Appendf(TEXT("%.2f "), Brush->GetMargin().Top);
			Builder.Appendf(TEXT("%.2f "), Brush->GetMargin().Right);
			Builder.Appendf(TEXT("%.2f "), Brush->GetMargin().Bottom);
			AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipMargin", "Margin"), FText::FromString(Builder.ToString()));
		}

		AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipMirroring", "Mirroring"), StaticEnum<ESlateBrushMirrorType::Type>()->GetDisplayNameTextByValue(Brush->GetMirroring()));

		AddToToolTipInfoBox(InfoBox, LOCTEXT("ImageTooltipResource","Resource"), FText::FromName(Brush->GetResourceName()));
	}
}

void SSlateIconViewerRowTooltip::OnOpening()
{
	auto InDesc = Descriptor.Pin();
	if (InDesc.IsValid())
	{
		TSharedRef<SVerticalBox> OverallTooltipVBox = SNew(SVerticalBox);

		OverallTooltipVBox->AddSlot()
			.AutoHeight()
			.Padding(0)
			[
				SNew(SBorder)
				.Padding(6)
				.BorderImage(FStyleHelper::GetBrush("ContentBrowser.TileViewTooltip.ContentBorder"))
				[
					SNew(STextBlock)
					.Text(InDesc->GetDisplayText())
					.Font(FStyleHelper::GetFontStyle("ContentBrowser.TileViewTooltip.NameFont"))
				]
			];

		TSharedRef<SVerticalBox> InfoBox = SNew(SVerticalBox);
		BuildInfo(*InDesc, InfoBox);
		if (InfoBox->NumSlots() > 0)
		{
			OverallTooltipVBox->AddSlot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					SNew(SBorder)
					.Padding(6)
					.BorderImage(FStyleHelper::GetBrush("ContentBrowser.TileViewTooltip.ContentBorder"))
					[
						InfoBox
					]
				];
		}

		SetContentWidget(
			SNew(SBorder)
			.Padding(6)
			.BorderImage(FStyleHelper::GetBrush("ContentBrowser.TileViewTooltip.NonContentBorder"))
			[
				OverallTooltipVBox
			]
		);
	}
}

void SSlateIconViewerRowTooltip::OnClosed()
{
	ResetContentWidget();
}

#undef LOCTEXT_NAMESPACE