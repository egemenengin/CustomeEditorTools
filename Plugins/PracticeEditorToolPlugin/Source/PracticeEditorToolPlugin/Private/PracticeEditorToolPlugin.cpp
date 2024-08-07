// Copyright Epic Games, Inc. All Rights Reserved.

#include "PracticeEditorToolPlugin.h"
#include "ContentBrowserModule.h"
#include "PracticeDebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

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
	FExecuteAction deleteUnusedAssetsExecuteActionDelegate;
	deleteUnusedAssetsExecuteActionDelegate.BindRaw(this, &FPracticeEditorToolPluginModule::DeleteUnusedAssets);
	menuBuilder.AddMenuEntry(FText::FromString(TEXT("Practice Delete Unused Assets")),
							 FText::FromString(TEXT("Safely delete all unused assets under the selected folder.")),
							 FSlateIcon(),
							 deleteUnusedAssetsExecuteActionDelegate);

	FExecuteAction deleteEmptyFoldersExecuteActionDelegate;
	deleteEmptyFoldersExecuteActionDelegate.BindRaw(this, &FPracticeEditorToolPluginModule::DeleteEmtpyFolders);
	menuBuilder.AddMenuEntry(FText::FromString(TEXT("Practice Delete Empty Folders")),
							 FText::FromString(TEXT("Safely delete all empty folders under the selected folder.")),
							 FSlateIcon(),
							 deleteEmptyFoldersExecuteActionDelegate);
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
		FixUpRedirectors();

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

void FPracticeEditorToolPluginModule::DeleteEmtpyFolders()
{
	FixUpRedirectors();

	if (SelectedFolderPaths.Num() > 1)
	{
		ShowNotification(TEXT("Please Select One Folder!"));
		return;
	}
	TArray<FString> folderPaths = UEditorAssetLibrary::ListAssets(SelectedFolderPaths[0], true, true);

	FString emptyFoldersPathsText;
	TArray<FString> emptyFoldersPaths;

	for (FString folderPath : folderPaths)
	{
		if (folderPath.Contains("Developers") ||
			folderPath.Contains("Collections") ||
			folderPath.Contains("__ExternalObjects__") ||
			folderPath.Contains("__ExternalActors__") ||
			!UEditorAssetLibrary::DoesDirectoryExist(folderPath))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(folderPath, true))
		{
			emptyFoldersPathsText.Append(folderPath + TEXT("\n"));
			emptyFoldersPaths.Add(folderPath);
		}
	}

	for (FString emptyFolderPath : emptyFoldersPaths)
	{
		UEditorAssetLibrary::DeleteDirectory(emptyFolderPath);
	}

}

void FPracticeEditorToolPluginModule::FixUpRedirectors()
{
	IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");


	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0) return;

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	TArray<UObject*> Objects;
	const bool bAllowedToPromptToLoadAssets = true;
	const bool bLoadRedirects = true;

	AssetViewUtils::FLoadAssetsSettings Settings;
	Settings.bFollowRedirectors = false;
	Settings.bAllowCancel = true;

	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, Settings);

	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);

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