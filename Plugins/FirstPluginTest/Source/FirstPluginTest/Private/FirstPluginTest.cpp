// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstPluginTest.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FFirstPluginTestModule"

void FFirstPluginTestModule::StartupModule()
{
	InitCBMenuExtension();
}

#pragma region ContentBrowserMenuExtension

void FFirstPluginTestModule::InitCBMenuExtension()
{
	FContentBrowserModule& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Get hold of all the menu extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& contentBrowserModuleMenuExtenders = contentBrowserModule.GetAllPathViewContextMenuExtenders();
	
	FContentBrowserMenuExtender_SelectedPaths customCBMenuDelegate;
	customCBMenuDelegate.BindRaw(this, &FFirstPluginTestModule::ExtendCBMenuBrowser);

	// Add custom delegate to all the existing delegates
	contentBrowserModuleMenuExtenders.Add(customCBMenuDelegate);

	// Also this can be used for create and add new delegate: 
	//contentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FFirstPluginTestModule::ExtendCBMenuBrowser));
}

// To define the position for inserting menu entry
TSharedRef<FExtender> FFirstPluginTestModule::ExtendCBMenuBrowser(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> menuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		FMenuExtensionDelegate menuExtensionDelegate;
		menuExtensionDelegate.BindRaw(this, &FFirstPluginTestModule::AddCBMenuEntry);
		//FMenuExtensionDelegate::CreateRaw(this,&FFirstPluginTestModule::AddCBMenuEntry);

		menuExtender->AddMenuExtension(FName("Delete"), // Extension hook, position to insert
										EExtensionHook::After, // Inserting before or after 
										TSharedPtr<FUICommandList>(), // Custom hot keys
										menuExtensionDelegate); // Second binding, will define details for this menu entry 

		SelectedFolderPaths = SelectedPaths;
	}
	return menuExtender;
}

// Defining details for custom menu entry
void FFirstPluginTestModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	FExecuteAction deleteUnusedAssetExecuteActionDelegate;
	deleteUnusedAssetExecuteActionDelegate.BindRaw(this, &FFirstPluginTestModule::OnDeleteUnusedAssetsButtonClicked);

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Unused Assets")), // Title text for menu entry
							 FText::FromString(TEXT("Safely delete all unused assets under folder.")), // Tooltip text
							 FSlateIcon(), // Custom icon
							 deleteUnusedAssetExecuteActionDelegate); // Third binding, the actual function to execute


	FExecuteAction deleteEmptyFoldersExecuteActionDelegate;
	deleteEmptyFoldersExecuteActionDelegate.BindRaw(this, &FFirstPluginTestModule::OnDeleteEmptyFoldersButtonClicked);

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Empty Folder")),
							 FText::FromString(TEXT("Safely delete all empty folders under the selected folder.")),
							 FSlateIcon(),
							 deleteEmptyFoldersExecuteActionDelegate);
}

void FFirstPluginTestModule::OnDeleteUnusedAssetsButtonClicked()
{
	if (SelectedFolderPaths.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please select one folder."));
		return;
	}
	DebugHeader::PrintLog(TEXT("Folder PATH: ") + SelectedFolderPaths[0]);
	TArray<FString> assetsPaths = UEditorAssetLibrary::ListAssets(SelectedFolderPaths[0]);
	
	if (assetsPaths.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder."));
		return;
	}
	const FString confirmationMsg = TEXT("A total of ") + FString::FromInt(assetsPaths.Num()) + TEXT(" assets found under ") +
										SelectedFolderPaths[0] + TEXT(".\nThey need to be checked.\nWould you like to proceed? ");

	EAppReturnType::Type confirmationAnswer = DebugHeader::ShowMessageDialog(EAppMsgType::YesNo, confirmationMsg);
	
	if (confirmationAnswer == EAppReturnType::Yes)
	{
		FixUpRedirectors();

		TArray<FAssetData> unusedAssetsData;

		for (const FString assetPath : assetsPaths)
		{
			DebugHeader::PrintLog(TEXT("ASSET PATH: ") + assetPath);
			// Dont touch the root folder
			if (assetPath.Contains("Developers") ||
				assetPath.Contains("Collections") ||
				assetPath.Contains("__ExternalActors__") ||
				assetPath.Contains("__ExternalObjects__") ||
				!UEditorAssetLibrary::DoesAssetExist(assetPath) )
			{
				continue;
			}
			TArray<FString> assetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(assetPath);

			if (assetReferences.Num() == 0)
			{
				const FAssetData unusedAssetData = UEditorAssetLibrary::FindAssetData(assetPath);
				unusedAssetsData.Add(unusedAssetData);
			}
			
		}
		if (unusedAssetsData.Num() > 0)
		{
			ObjectTools::DeleteAssets(unusedAssetsData);
			DebugHeader::ShowNotifyInfo(TEXT("A total of ") + FString::FromInt(unusedAssetsData.Num()) + TEXT(" unused assets have been deleted."));
		}
		else
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused assets found under selected folder."));
		}
	}
	
}

void FFirstPluginTestModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();

	if (SelectedFolderPaths.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please select one folder."));
		return;
	}
	DebugHeader::PrintLog(TEXT("Folder PATH: ") + SelectedFolderPaths[0]);

	TArray<FString> foldersPaths = UEditorAssetLibrary::ListAssets(SelectedFolderPaths[0], true, true);

	FString currentEmptyFolderPath;
	TArray<FString> emptyFoldersPaths;

	for (const FString& folderPath : foldersPaths)
	{
		// Dont touch the root folder and check is it directory
		if (folderPath.Contains("Developers") ||
			folderPath.Contains("Collections") ||
			folderPath.Contains("__ExternalActors__") ||
			folderPath.Contains("__ExternalObjects__") ||
			!UEditorAssetLibrary::DoesDirectoryExist(folderPath))
		{
			continue;
		}
		
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(folderPath))
		{
			currentEmptyFolderPath.Append(folderPath + TEXT("\n"));
			emptyFoldersPaths.Add(folderPath);
		}

	}
	if (emptyFoldersPaths.Num() == 0)
	{
		FString noEmptyFolderMsg = TEXT("There is no empty folder under the selected folder.");
		DebugHeader::ShowNotifyInfo(noEmptyFolderMsg);
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, noEmptyFolderMsg, false);
		return;
	}
	else
	{
		FString foundedEmptyFolders = TEXT("Empty folders found under ") + SelectedFolderPaths[0] + TEXT(":\n") + currentEmptyFolderPath + 
									  TEXT("Would you like to delete all?");
		EAppReturnType::Type confirmationAnswer = DebugHeader::ShowMessageDialog(EAppMsgType::YesNo, foundedEmptyFolders);
		
		if (confirmationAnswer == EAppReturnType::Yes)
		{
			for (const FString& emptyFolderPath : emptyFoldersPaths)
			{
				bool isDeleted = UEditorAssetLibrary::DeleteDirectory(emptyFolderPath);
			}
		}
	}
}

void FFirstPluginTestModule::FixUpRedirectors()
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

void FFirstPluginTestModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFirstPluginTestModule, FirstPluginTest)