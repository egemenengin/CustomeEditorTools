// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "DebugHeader.h"

class FFirstPluginTestModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	
#pragma region ContentBrowserMenuExtension
	
	TArray<FString> SelectedFolderPaths;

	void InitCBMenuExtension();

	TSharedRef<FExtender> ExtendCBMenuBrowser(const TArray<FString>& SelectedPaths);

	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	void OnDeleteUnusedAssetsButtonClicked();

	void OnDeleteEmptyFoldersButtonClicked();
	void FixUpRedirectors();
#pragma endregion

};
