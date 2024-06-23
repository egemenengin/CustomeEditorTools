// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "FirstPracticeAssetActionUtility.generated.h"

/**
 * 
 */
UCLASS()
class PRACTICEEDITORTOOLPLUGIN_API UFirstPracticeAssetActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()

	UFUNCTION(CallInEditor)
	void PracticeTest();

	UFUNCTION(CallInEditor)
	void PracticeDuplicateAsset(int NumOfDuplicate);
	
};
