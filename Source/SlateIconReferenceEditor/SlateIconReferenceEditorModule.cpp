// Copyright 2025, Aquanox.

#include "SlateIconReferenceEditorModule.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SlateIconRefTypeCustomization.h"
#include "Internal/SlateIconRefDataHelper.h"
#include "SlateIconReference.h"
#include "SlateIconReferenceEditorStyle.h"

#define LOCTEXT_NAMESPACE "SlateIconReferenceEditor"

IMPLEMENT_MODULE(FSlateIconReferenceEditorModule, SlateIconReferenceEditor);

void FSlateIconReferenceEditorModule::StartupModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		StyleSet = MakeShared<FSlateIconReferenceEditorStyle>();

		FPropertyEditorModule& PropertyEditor = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditor.RegisterCustomPropertyTypeLayout(FSlateIconRefTypeCustomization::TypeName,
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSlateIconRefTypeCustomization::MakeInstance)
		);
	}
}

void FSlateIconReferenceEditorModule::ShutdownModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		StyleSet.Reset();

		FSlateIconRefDataHelper::GetDataSource().ClearStyleData();

		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor") )
		{
			FPropertyEditorModule& PropertyEditor = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditor.UnregisterCustomPropertyTypeLayout(FSlateIconRefTypeCustomization::TypeName);
		}
	}
}

#undef LOCTEXT_NAMESPACE


