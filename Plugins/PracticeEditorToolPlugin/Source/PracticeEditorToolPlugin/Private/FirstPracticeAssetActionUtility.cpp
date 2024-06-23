// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPracticeAssetActionUtility.h"
#include "PracticeDebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"

void UFirstPracticeAssetActionUtility::PracticeTest()
{
	PrintToLog(TEXT("Practice Message on BackLog"));
	PrintToScreen(TEXT("Practice Message on Screen"), FColor::Blue);
}

void UFirstPracticeAssetActionUtility::PracticeDuplicateAsset(int NumOfDuplicate)
{
	if (NumOfDuplicate <= 0)
	{
		PrintToLog(TEXT("Enter Valid Number of Duplicate"));
		PrintToScreen(TEXT("Enter Valid Number of Duplicate"), FColor::Red);
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
		counter = 0;


	}
}
