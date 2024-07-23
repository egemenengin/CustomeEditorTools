// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPracticeEditorToolPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:

#pragma region ContentBrowserMenuExtension
	void InitCBMenuExtension();

	TSharedRef<FExtender> ExtendCBMenuBrowser(const TArray<FString>& SelectedPaths);
	
	void AddCBMenuEntry(class FMenuBuilder& menuBuilder);

	void DeleteUnusedAssets();
#pragma endregion
};
