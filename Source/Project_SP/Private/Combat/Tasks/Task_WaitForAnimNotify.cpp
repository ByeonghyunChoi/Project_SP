// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_WaitForAnimNotify.h"

void UTask_WaitForAnimNotify::ExecuteTask_Implementation()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	UE_LOG(LogTemp, Log, TEXT("[Time: %f] Wait Task Started (Waiting for: %s)"), CurrentTime, *NotifyName.ToString());
}

void UTask_WaitForAnimNotify::OnNotifyReceived(FName ReceivedNotifyName)
{
	if (ReceivedNotifyName == NotifyName)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		// [로그 추가] 신호를 받고 끝나는 시간 (Error로 찍어서 빨간색으로 잘 보이게 함)
		UE_LOG(LogTemp, Error, TEXT("[Time: %f] Notify Received: %s -> Wait Finished!"), CurrentTime, *ReceivedNotifyName.ToString());
		FinishTask();
	}
}
