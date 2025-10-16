#include "Combat/AnimNotify/AnimNotify_SignalTask.h"
#include "Core/BattleManager.h" // BattleManager 헤더 추가
#include "Kismet/GameplayStatics.h"

void UAnimNotify_SignalTask::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetWorld()) return;

	// 월드에 있는 BattleManager를 찾습니다.
	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(MeshComp->GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		// BattleManager에게 '어떤 이름의 신호'가 울렸는지 알려줍니다.
		BattleManager->SignalTaskByNotifyName(SignalName);
	}
}