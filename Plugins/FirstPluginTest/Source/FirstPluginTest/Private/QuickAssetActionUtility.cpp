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
		Print(TEXT("Please Enter a Valid Number"), FColor::Red	);
		return;
	}
	TArray<FAssetData> selectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 counter = 0;

	for (const FAssetData& assetData : selectedAssetsData)
	{
		for (int i = 0; i < NumOfDuplicate; i++)
		{
			const FString sourceAssetPath = assetData.ObjectPath.ToString();
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
		counter = 0;
	}
}
