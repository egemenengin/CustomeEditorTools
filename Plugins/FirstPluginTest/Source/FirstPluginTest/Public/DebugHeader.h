#pragma once
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications//NotificationManager.h"

namespace DebugHeader
{
	static void Print(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}
	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMessageDialog(EAppMsgType::Type MessageType, const FString& Message, bool bIsWarning = true)
	{

		EAppReturnType::Type userReturnType;
		FText msgTitle = FText::FromString(TEXT("Warning"));

		if (!bIsWarning)
		{
			msgTitle = FText::FromString(TEXT("Log"));
		}

		userReturnType = FMessageDialog::Open(MessageType, FText::FromStringView(Message), &msgTitle);

		return userReturnType;

	}

	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 5.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);

	}
}
