#pragma once
#include "Misc/MessageDialog.h"

#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
namespace DebugHeader
{
	static void PrintToScreen(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	static void PrintToLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMessageLog(EAppMsgType::Type MessageType, const FString& Message, bool bIsWarning = true)
	{
		EAppReturnType::Type userReturnType;
		FText messageHeading = FText::FromString("Warning");
		if (!bIsWarning)
		{
			messageHeading = FText::FromString("Info");
		}
		userReturnType = FMessageDialog::Open(MessageType, FText::FromString(Message), &messageHeading);
		return userReturnType;
	}
	static void ShowNotification(const FString& Message)
	{
		FNotificationInfo notifyInfo(FText::FromString(Message));
		notifyInfo.bUseLargeFont = true;
		notifyInfo.FadeOutDuration = 5.f;

		FSlateNotificationManager::Get().AddNotification(notifyInfo);
	}
}
