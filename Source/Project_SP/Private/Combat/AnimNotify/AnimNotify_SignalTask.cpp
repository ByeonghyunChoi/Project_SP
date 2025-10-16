// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotify/AnimNotify_SignalTask.h"
#include "Core/BattleManager.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotify_SignalTask::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp || !MeshComp->GetWorld()) return;

	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(MeshComp->GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		// BattleManager에게 SignalName을 전달하며 신호를 보냅니다.
		BattleManager->SignalTaskByNotifyName(SignalName);
	}
}

