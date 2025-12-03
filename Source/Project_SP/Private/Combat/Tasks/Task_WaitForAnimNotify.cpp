// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_WaitForAnimNotify.h"

void UTask_WaitForAnimNotify::ExecuteTask_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT(">>> [WAIT START] Waiting for Notify: %s"), *NotifyName.ToString());
}

void UTask_WaitForAnimNotify::OnNotifyReceived(FName ReceivedNotifyName)
{
	UE_LOG(LogTemp, Warning, TEXT(">>> [SIGNAL RECEIVED] Name: %s (Expected: %s)"), *ReceivedNotifyName.ToString(), *NotifyName.ToString());
	if (ReceivedNotifyName == NotifyName)
	{
		UE_LOG(LogTemp, Warning, TEXT(">>> [MATCH!] Finishing Task."));
		FinishTask();
	}
}
