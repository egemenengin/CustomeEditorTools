// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstPluginTest.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#define LOCTEXT_NAMESPACE "FFirstPluginTestModule"

void FFirstPluginTestModule::StartupModule()
{
	InitCBMenuExtension();
}

#pragma region ContentBrowserMenuExtension

void FFirstPluginTestModule::InitCBMenuExtension()
{
	FContentBrowserModule& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& contentBrowserModuleMenuExtenders = contentBrowserModule.GetAllPathViewContextMenuExtenders();
	
	FContentBrowserMenuExtender_SelectedPaths customCBMenuDelegate;
	customCBMenuDelegate.BindRaw(this, &FFirstPluginTestModule::ExtendCBMenuBrowser);

	contentBrowserModuleMenuExtenders.Add(customCBMenuDelegate);

	// Also this can be used for create and add new delegate: 
	//contentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FFirstPluginTestModule::ExtendCBMenuBrowser));
}

TSharedRef<FExtender> FFirstPluginTestModule::ExtendCBMenuBrowser(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> menuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		FMenuExtensionDelegate menuExtensionDelegate;
		menuExtensionDelegate.BindRaw(this, &FFirstPluginTestModule::AddCBMenuEntry);
		//FMenuExtensionDelegate::CreateRaw(this,&FFirstPluginTestModule::AddCBMenuEntry);

		menuExtender->AddMenuExtension(FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(), menuExtensionDelegate);

		SelectedFolderPaths = SelectedPaths;
	}
	return menuExtender;
}

void FFirstPluginTestModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	FExecuteAction executeActionDelegate;
	executeActionDelegate.BindRaw(this, &FFirstPluginTestModule::OnDeleteUnusedAssetsButtonClicked);

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Unused Assets")),
							 FText::FromString(TEXT("Safely delete all unused assets under folder.")),
							 FSlateIcon(),
						     executeActionDelegate);
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
										SelectedFolderPaths[0] + TEXT(".\nWould you like to proceed? ");

	EAppReturnType::Type confirmationAnswer = DebugHeader::ShowMessageDialog(EAppMsgType::YesNo, confirmationMsg);
	
	if (confirmationAnswer == EAppReturnType::Yes)
	{
		TArray<FAssetData> unusedAssetsData;

		for (const FString assetPath : assetsPaths)
		{
			DebugHeader::PrintLog(TEXT("ASSET PATH: ") + assetPath);
			// Dont touch the root folder
			if (assetPath.Contains("Developers") ||
				assetPath.Contains("Collections") ||
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

#pragma endregion 

void FFirstPluginTestModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFirstPluginTestModule, FirstPluginTest)