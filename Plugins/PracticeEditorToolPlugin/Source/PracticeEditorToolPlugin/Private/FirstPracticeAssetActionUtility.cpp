// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPracticeAssetActionUtility.h"
#include "PracticeDebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"

#include "ObjectTools.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
void UFirstPracticeAssetActionUtility::PracticeTest()
{
	PrintToLog(TEXT("Practice Message on BackLog"));
	PrintToScreen(TEXT("Practice Message on Screen"), FColor::Blue);
	ShowMessageLog(EAppMsgType::Ok, "TEST", false);
}



void UFirstPracticeAssetActionUtility::PracticeDuplicateAsset(int NumOfDuplicate)
{
	if (NumOfDuplicate <= 0)
	{

		PrintToLog(TEXT("Enter Valid Number of Duplicate"));
		PrintToScreen(TEXT("Enter Valid Number of Duplicate"), FColor::Red);
		ShowMessageLog(EAppMsgType::Ok, TEXT("Enter Valid Number of Duplicate"));
		return;
	}
	TArray<FAssetData> selectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 counter = 0;

	for (const FAssetData& assetData : selectedAssetsData)
	{
		const FString& sourcePath = assetData.ObjectPath.ToString();
		for (int i = 1; i <= NumOfDuplicate; i++)
		{
			const FString& newAssetName = assetData.AssetName.ToString() + "_" + FString::FromInt(i);
			const FString& newAssetPath = FPaths::Combine(assetData.PackagePath.ToString(), newAssetName);

			UObject* newAssetObj = UEditorAssetLibrary::DuplicateAsset(sourcePath, newAssetPath);
			if (newAssetObj)
			{
				const FString& duplicateMessage = TEXT("DUPLICATE | CREATE: ") + newAssetName + TEXT(" created.");
				PrintToScreen(duplicateMessage, FColor::Emerald);
				PrintToLog(duplicateMessage);

				UEditorAssetLibrary::SaveAsset(newAssetPath);
				counter++;
			}
		}
		const FString duplicateEndMessage = TEXT("DUPLICATE | END: ") + assetData.AssetName.ToString() + TEXT(" duplicated") + FString::FromInt(counter) + TEXT(" times! ");
		PrintToScreen(duplicateEndMessage, FColor::Emerald);
		PrintToLog(duplicateEndMessage);
		ShowNotification(duplicateEndMessage);
		counter = 0;


	}
}

void UFirstPracticeAssetActionUtility::PracticeAddPrefix()
{

	TArray<UObject*> selectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	int counter = 0;
	for (UObject* selectedObj : selectedObjects)
	{
		FString* prefix = PrefixMap.Find(selectedObj->GetClass());
		if (!prefix || prefix->IsEmpty())
		{
			ShowMessageLog(EAppMsgType::Ok, TEXT("ERROR: Couldnt find prefix for ") + selectedObj->GetClass()->GetName());
			continue;
		}
		FString oldName = selectedObj->GetName();
		if (oldName.StartsWith(*prefix))
		{
			ShowMessageLog(EAppMsgType::Ok, TEXT("ERROR: Couldnt find prefix for ") + selectedObj->GetClass()->GetName());
			continue;
		}
		if (selectedObj->IsA<UMaterialInstanceConstant>())
		{
			oldName.RemoveFromStart(TEXT("M_"));
			oldName.RemoveFromEnd(TEXT("_Inst"));
		}

		FString newName = *prefix + oldName;
		UEditorUtilityLibrary::RenameAsset(selectedObj, newName);
		counter++;
	}

	ShowNotification(TEXT("COMPLETED: Add Prefixes to ") + FString::FromInt(counter) + "assets.");
}

void UFirstPracticeAssetActionUtility::PracticeRemoveUnusedAssets()
{
	TArray<FAssetData> selectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> unusedAssetsData;

	for (FAssetData selectedAssetData : selectedAssetsData)
	{
		TArray<FString> assetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(selectedAssetData.GetObjectPathString());
		if (assetReferences.Num() == 0)
		{
			unusedAssetsData.Add(selectedAssetData);
		}
	}

	if (unusedAssetsData.Num() == 0)
	{
		ShowMessageLog(EAppMsgType::Ok, TEXT("There is no unused asset among selected assets."), false);
	}
	else
	{
		const int numOfDeletedUnusedAsset = ObjectTools::DeleteAssets(unusedAssetsData, true);
		if (numOfDeletedUnusedAsset > 0)
		{
			ShowNotification(TEXT("COMPLETED: ") + FString::FromInt(numOfDeletedUnusedAsset) + TEXT(" unused assets have been deleted."));
		}
	}

}

void UFirstPracticeAssetActionUtility::FixUpRedirectors()
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
