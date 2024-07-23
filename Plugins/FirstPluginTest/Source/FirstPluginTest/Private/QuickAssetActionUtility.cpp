// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetActionUtility.h"
#include "DebugHeader.h" 
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"




void UQuickAssetActionUtility::TestFunc()
{
	DebugHeader::Print(TEXT("ITS WORKING!!!"), FColor::Purple);
	DebugHeader::PrintLog(TEXT("ITS WORKING!!!"));
}

void UQuickAssetActionUtility::DuplicateAsset(int NumOfDuplicate)
{
	if (NumOfDuplicate <= 0)
	{
		DebugHeader::Print(TEXT("Please Enter a Valid Number"), FColor::Red);
		EAppReturnType::Type userReturnType = DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please Enter a Valid Number"));

		return;

	}
	TArray<FAssetData> selectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 counter = 0;

	for (const FAssetData& assetData : selectedAssetsData)
	{
		const FString sourceAssetPath = assetData.ObjectPath.ToString();

		for (int i = 0; i < NumOfDuplicate; i++)
		{
			const FString duplicatedAssetName = assetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString newAssetPath = FPaths::Combine(assetData.PackagePath.ToString(), duplicatedAssetName);

			UObject* duplicatedAsset = UEditorAssetLibrary::DuplicateAsset(sourceAssetPath, newAssetPath);
			if (duplicatedAsset)
			{
				DebugHeader::PrintLog(TEXT("CREATED: ") + duplicatedAsset->GetName());
				UEditorAssetLibrary::SaveAsset(newAssetPath);
				counter++;
			}
		}
		const FString& duplicatedMessage = assetData.AssetName.ToString() + TEXT(" has been duplicated ") + FString::FromInt(counter) + TEXT(" times.");
		DebugHeader::PrintLog(duplicatedMessage);
		DebugHeader::ShowNotifyInfo(duplicatedMessage);
		counter = 0;
	}
}

void UQuickAssetActionUtility::AddPrefix()
{
	TArray<UObject*> selectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 counter = 0;

	for (UObject* selectedObject : selectedObjects)
	{
		if (!selectedObject) continue;
		
		FString* foundedPrefix = PrefixMap.Find(selectedObject->GetClass());
		
		if (!foundedPrefix || foundedPrefix->IsEmpty())
		{
			FString notFoundMessage = TEXT("Failed: Couldn't Find Prefix for ") + selectedObject->GetName() + TEXT("!");
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, notFoundMessage);
			DebugHeader::PrintLog(notFoundMessage);
			DebugHeader::Print(notFoundMessage, FColor::Red);
			continue;
		}

		FString oldName = selectedObject->GetName();

		if (oldName.StartsWith(*foundedPrefix))
		{
			FString notFoundMessage = TEXT("Failed: ") + selectedObject->GetName() + TEXT(" already has prefix!");
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, notFoundMessage);
			DebugHeader::PrintLog(notFoundMessage);
			DebugHeader::Print(notFoundMessage, FColor::Red);
			continue;
		}

		if (selectedObject->IsA<UMaterialInstanceConstant>())
		{
			oldName.RemoveFromStart(TEXT("M_"));
			oldName.RemoveFromEnd(TEXT("_Inst"));
		}
		const FString newNameWithPrefix = *foundedPrefix + oldName;
		UEditorUtilityLibrary::RenameAsset(selectedObject, newNameWithPrefix);

		counter++;
	}

	DebugHeader::ShowNotifyInfo(TEXT("COMPLETED: ") + FString::FromInt(counter) + TEXT(" assets have been renamed successfully."));

}

void UQuickAssetActionUtility::RemoveUnusedAsset()
{
	FixUpRedirectors();


	TArray<FAssetData> selectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> unusedAssetsData;


	for (const FAssetData selectedAssetData : selectedAssetsData)
	{
		TArray<FString> assetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(selectedAssetData.ObjectPath.ToString());
		if (assetReferences.Num() == 0)
		{
			unusedAssetsData.Add(selectedAssetData);
		}
	}
	if (unusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets!"), false);
	}
	else
	{
		const int32 numOfAssetsDeleted = ObjectTools::DeleteAssets(unusedAssetsData);
		// FORCE DELETE: UEditorAssetLibrary::DeleteLoadedAssets(unusedAssetsData);

		if (numOfAssetsDeleted > 0)
		{
			FString deletedSuccessMessage = TEXT("COMPLETED: Deleted ") + FString::FromInt(numOfAssetsDeleted) + TEXT(" Unused Assets.");
			DebugHeader::ShowNotifyInfo(deletedSuccessMessage);
			DebugHeader::Print(deletedSuccessMessage, FColor::Blue);
			DebugHeader::PrintLog(deletedSuccessMessage);
		}

	}

}

void UQuickAssetActionUtility::FixUpRedirectors()
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
