// Copyright Epic Games, Inc. All Rights Reserved.

#include "PracticeEditorToolPlugin.h"
#include "ContentBrowserModule.h"
#include "PracticeDebugHeader.h"

using namespace DebugHeader;

#define LOCTEXT_NAMESPACE "FPracticeEditorToolPluginModule"

void FPracticeEditorToolPluginModule::StartupModule()
{
	InitCBMenuExtension();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

#pragma region ContentBrowserMenuExtension

void FPracticeEditorToolPluginModule::InitCBMenuExtension()
{
	FContentBrowserModule& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(FName("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& contentBrowserMenuExtenders = contentBrowserModule.GetAllPathViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedPaths contentBrowserExtendDelegate;

	contentBrowserExtendDelegate.BindRaw(this, &FPracticeEditorToolPluginModule::ExtendCBMenuBrowser);
	contentBrowserMenuExtenders.Add(contentBrowserExtendDelegate);
}

TSharedRef<FExtender> FPracticeEditorToolPluginModule::ExtendCBMenuBrowser(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> menuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		FMenuExtensionDelegate menuExtensionDelegate;
		menuExtensionDelegate.BindRaw(this, &FPracticeEditorToolPluginModule::AddCBMenuEntry);
		menuExtender->AddMenuExtension(FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(), menuExtensionDelegate);

	}
	return menuExtender;
}

void FPracticeEditorToolPluginModule::AddCBMenuEntry(FMenuBuilder& menuBuilder)
{
	FExecuteAction executeActionDelegate;
	executeActionDelegate.BindRaw(this, &FPracticeEditorToolPluginModule::DeleteUnusedAssets);
	menuBuilder.AddMenuEntry(FText::FromString(TEXT("Practice Delete Unused Assets")),
							 FText::FromString(TEXT("Safely delete all unused assets under folder.")),
							 FSlateIcon(),
							 executeActionDelegate);

}

void FPracticeEditorToolPluginModule::DeleteUnusedAssets()
{
	PrintToLog(TEXT("WORKING!!!"));
}

#pragma endregion
void FPracticeEditorToolPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPracticeEditorToolPluginModule, PracticeEditorToolPlugin)