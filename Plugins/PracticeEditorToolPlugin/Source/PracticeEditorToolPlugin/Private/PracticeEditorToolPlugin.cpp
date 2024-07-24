// Copyright Epic Games, Inc. All Rights Reserved.

#include "PracticeEditorToolPlugin.h"
#include "ContentBrowserModule.h"
#include "PracticeDebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

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

		SelectedFolderPaths = SelectedPaths;
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
	if (SelectedFolderPaths.Num() > 1)
	{
		FString oneFolderErrorMsg = TEXT("Please select one folder.");
		ShowMessageLog(EAppMsgType::Ok, oneFolderErrorMsg);
		PrintToLog(oneFolderErrorMsg);
		return;
	}
	const FString selectedFolderPath = SelectedFolderPaths[0];
	PrintToLog(TEXT("Selected Path: ") + selectedFolderPath);

	// Assets paths
	TArray<FString> assetsPaths = UEditorAssetLibrary::ListAssets(selectedFolderPath);

	if (assetsPaths.Num() == 0)
	{
		FString noAssetErrorMsg = TEXT("No asset were found asset under ") + selectedFolderPath + TEXT(" folder.");
		ShowMessageLog(EAppMsgType::Ok, noAssetErrorMsg);
		PrintToLog(noAssetErrorMsg);
		return;
	}

	// Confirmation
	const FString confirmationMsg = TEXT("There are ") + FString::FromInt(assetsPaths.Num()) +
										TEXT(" assets under selected folder.\nWould you like to proceed?");
	EAppReturnType::Type confirmationResult = ShowMessageLog(EAppMsgType::YesNo, confirmationMsg, false);


	if (confirmationResult == EAppReturnType::Yes)
	{
		TArray<FAssetData> unusedAssetsData;
		for (FString assetPath : assetsPaths)
		{
			// Do not touch root folders.
			if (assetPath.Contains("Developer") ||
				assetPath.Contains("Collections"))
			{
				continue;
			}

			PrintToLog(TEXT("Current Asset Path: ") + assetPath);

			TArray<FString> assetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(assetPath);

			if (assetReferencers.Num() == 0)
			{
				const FAssetData currentAssetData = UEditorAssetLibrary::FindAssetData(assetPath);
				PrintToLog(currentAssetData.AssetName.ToString() + TEXT(" has no reference."));
				unusedAssetsData.Add(currentAssetData);
			}
			else
			{
				PrintToLog(TEXT("The asset has referencers."));
			}

		}
		if (unusedAssetsData.Num() == 0)
		{
			FString noUnusedAssetMsg = TEXT("No unused assets found under selected folder!");
			ShowMessageLog(EAppMsgType::Ok, noUnusedAssetMsg);
		}
		else
		{
			int numOfDeletedUnusedAssets = ObjectTools::DeleteAssets(unusedAssetsData);
			FString deletionCompleted = FString::FromInt(numOfDeletedUnusedAssets) +
											TEXT(" assets under selected folder have been deleted successfully.");
			ShowNotification(deletionCompleted);
			PrintToLog(deletionCompleted);
		}
	}
}

#pragma endregion
void FPracticeEditorToolPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPracticeEditorToolPluginModule, PracticeEditorToolPlugin)