#include "Map/OpartsAltar.h"
#include "Component/OpartsComponent.h" // 경로 확인 필요

AOpartsAltar::AOpartsAltar()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOpartsAltar::ExecuteInteraction(AActor* Interactor)
{
	if (!Interactor || !OpartsToEquip) return;

	// 상호작용한 플레이어의 오파츠 컴포넌트 찾기
	UOpartsComponent* OpartsComp = Interactor->FindComponentByClass<UOpartsComponent>();
	if (OpartsComp)
	{
		// C++에서 다이렉트로 장착 함수 호출!
		OpartsComp->EquipOparts(OpartsToEquip);

		// 시각적 피드백(파티클 등)을 위해 BP 이벤트 호출
		OnAltarInteracted();

		UE_LOG(LogTemp, Log, TEXT("제단 작동: %s 장착 완료"), *OpartsToEquip->GetName());
	}
}

FText AOpartsAltar::GetInteractText() const
{
	return InteractText;
}