// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetActionUtility.h"
#include "DebugHeader.h" 
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"


void UQuickAssetActionUtility::TestFunc()
{
	Print(TEXT("ITS WORKING!!!"), FColor::Purple);
	PrintLog(TEXT("ITS WORKING!!!"));
}

void UQuickAssetActionUtility::DuplicateAsset(int NumOfDuplicate)
{
	if (NumOfDuplicate <= 0)
	{
		Print(TEXT("Please Enter a Valid Number"), FColor::Red);
		EAppReturnType::Type userReturnType = ShowMessageDialog(EAppMsgType::Ok, TEXT("Please Enter a Valid Number"));

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
				PrintLog(TEXT("CREATED: ") + duplicatedAsset->GetName());
				UEditorAssetLibrary::SaveAsset(newAssetPath);
				counter++;
			}
		}
		const FString& duplicatedMessage = assetData.AssetName.ToString() + TEXT(" has been duplicated ") + FString::FromInt(counter) + TEXT(" times.");
		PrintLog(duplicatedMessage);
		ShowNotifyInfo(duplicatedMessage);
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
			ShowMessageDialog(EAppMsgType::Ok, notFoundMessage);
			PrintLog(notFoundMessage);
			Print(notFoundMessage, FColor::Red);
			continue;
		}

		FString oldName = selectedObject->GetName();

		if (oldName.StartsWith(*foundedPrefix))
		{
			FString notFoundMessage = TEXT("Failed: ") + selectedObject->GetName() + TEXT(" already has prefix!");
			ShowMessageDialog(EAppMsgType::Ok, notFoundMessage);
			PrintLog(notFoundMessage);
			Print(notFoundMessage, FColor::Red);
			continue;
		}

		const FString newNameWithPrefix = *foundedPrefix + oldName;
		UEditorUtilityLibrary::RenameAsset(selectedObject, newNameWithPrefix);

		counter++;
	}

	ShowNotifyInfo(TEXT("COMPLETED: ") + FString::FromInt(counter) + TEXT(" assets have been renamed successfully."));

}
