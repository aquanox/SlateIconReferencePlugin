// Copyright 2025, Aquanox.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FSlateIconReferenceEditorStyle;

class FSlateIconReferenceEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
private:
    TSharedPtr<FSlateIconReferenceEditorStyle> StyleSet;
};
