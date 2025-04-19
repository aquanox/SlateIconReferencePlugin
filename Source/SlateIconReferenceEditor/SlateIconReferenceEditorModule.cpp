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

		FModuleManager::Get().OnModulesChanged().AddRaw(this, &FSlateIconReferenceEditorModule::HandleModulesChanged);

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

		FModuleManager::Get().OnModulesChanged().RemoveAll(this);
		
		FSlateIconRefDataHelper::GetDataSource().ClearStyleData();

		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor") )
		{
			FPropertyEditorModule& PropertyEditor = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditor.UnregisterCustomPropertyTypeLayout(FSlateIconRefTypeCustomization::TypeName);
		}
	}
}

void FSlateIconReferenceEditorModule::HandleModulesChanged(FName Name, EModuleChangeReason ModuleChangeReason)
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		switch(ModuleChangeReason)
		{
		case EModuleChangeReason::ModuleLoaded:
		case EModuleChangeReason::ModuleUnloaded:
			// clear known style data in order to re-initialize later
			FSlateIconRefDataHelper::GetDataSource().ForceRescan();
			FSlateIconRefDataHelper::GetDataSource().ClearStyleData();
			break;
		default:
		case EModuleChangeReason::PluginDirectoryChanged:
			break;
		}
	}
}

#undef LOCTEXT_NAMESPACE


