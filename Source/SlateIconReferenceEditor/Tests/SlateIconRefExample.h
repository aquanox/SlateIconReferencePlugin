// Copyright 2025, Aquanox.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "SlateIconReference.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPtr.h"

#include "SlateIconRefExample.generated.h"

UCLASS(MinimalAPI, Config=Test, DefaultConfig)
class USlateIconRefExample : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	USlateIconRefExample();
	virtual FName GetContainerName() const override { return "Editor"; }
	virtual FName GetCategoryName() const override { return "Plugins"; }
	virtual bool SupportsAutoRegistration() const override;

public:
	UPROPERTY(EditAnywhere, config, Category="Demo")
	TSoftClassPtr<AActor> ActorTypeRef;

	UPROPERTY(EditAnywhere, config, Category="Demo")
	TSoftObjectPtr<UTexture2D> TextureRef;

	UPROPERTY(EditAnywhere, config, Category="Demo")
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, config, Category="Demo", meta=(GetOptions=GetStyleSetNames))
	FName StyleSetName;

	UPROPERTY(EditAnywhere, config, Category="Demo", NoClear, meta=(GetOptions=GetStyleSetNames))
	FName StyleSetNameNC;

	// @mode Compact
	UPROPERTY(EditAnywhere, config, Category="Compact", meta=(AutoExpand, DisplayMode=Compact))
	FSlateIconReference IconBasic;

	// @mode ReadOnly,Compact
	UPROPERTY(VisibleAnywhere, config, Category="Compact", meta=(DisplayMode=Compact))
	FSlateIconReference IconBasicRO;

	// @mode NoClear,Default
	UPROPERTY(EditAnywhere, config, Category="Compact", NoClear, meta=(DisplayMode=Compact))
	FSlateIconReference IconBasicNC;

	// @mode Default implicit
	UPROPERTY(EditAnywhere, config, Category="Default")
	FSlateIconReference IconDefault;

	// @mode ReadOnly,Default
	UPROPERTY(VisibleAnywhere, config, Category="Default", meta=(AutoExpand, DisplayMode=Default))
	FSlateIconReference IconDefaultRO;

	// @mode NoClear,Default
	UPROPERTY(EditAnywhere, config, Category="Default", NoClear, meta=(AutoExpand, DisplayMode=Default))
	FSlateIconReference IconDefaultNC;

	// @mode Default,NoSmall
	UPROPERTY(EditAnywhere, config, Category="Custom", meta=(AutoExpand, DisplayMode="Default,NoSmall"))
	FSlateIconReference IconCustom;

	// @mode ReadOnly,Default,NoSmall
	UPROPERTY(VisibleAnywhere, config, Category="Custom", meta=(DisplayMode="Default,NoSmall"))
	FSlateIconReference IconCustomRO;

	// @mode Compact,Default
	UPROPERTY(EditAnywhere, config, Category="Custom", meta=(DisplayMode="Compact,Default"))
	FSlateIconReference IconCustomCompact3;

	// @mode Compact,WithIcon,WithOverlay
	UPROPERTY(EditAnywhere, config, Category="Custom", meta=(DisplayMode="Compact,WithIcon,WithOverlay"))
	FSlateIconReference IconCustomCompact2;

	// @mode Compact,WithOverlay
	UPROPERTY(EditAnywhere, config, Category="Custom", meta=(DisplayMode="Compact,WithOverlay"))
	FSlateIconReference IconCustomCompact1;

	// @mode Compact
	UPROPERTY(EditAnywhere, config, Category="Container", meta=(AutoExpand, DisplayMode=Compact))
	TArray<FSlateIconReference> IconBasicArray;

	// @mode Default
	UPROPERTY(EditAnywhere, config, Category="Container", meta=(DisplayMode=Default))
	TArray<FSlateIconReference> IconFullArray;

	UFUNCTION()
	const TArray<FName>& GetStyleSetNames() const;
};

UCLASS(MinimalAPI, Config=Test, DefaultConfig)
class USlateIconRefExampleSmall : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	USlateIconRefExampleSmall();
	virtual FName GetContainerName() const override { return "Editor"; }
	virtual FName GetCategoryName() const override { return "Plugins"; }
	virtual bool SupportsAutoRegistration() const override;

public:
	// Expose icon selector with default settings
	UPROPERTY(EditAnywhere, Config, Category="Examples")
	FSlateIconReference IconBasic;

	// Expose compact icon selector with default settings
	UPROPERTY(EditAnywhere, config, Category="Examples", meta=(DisplayMode=Compact))
	FSlateIconReference IconCompact;

	// Expose icon selector with customized settings (Standard display without Small Icon selector)
	UPROPERTY(EditAnywhere, config, Category="Examples", meta=(DisplayMode="Default,NoSmall"))
	FSlateIconReference IconComplexN1;

	// Expose icon selector with customized settings (Compact display with Icon and Overlay selector)
	UPROPERTY(EditAnywhere, config, Category="Examples", meta=(DisplayMode="Compact,WithIcon,WithOverlay"))
	FSlateIconReference IconComplexN2;
};
