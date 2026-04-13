#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "Data/Asset/OpartsDefinition.h" // 경로 확인 필요
#include "OpartsAltar.generated.h"

UCLASS()
class PROJECT_SP_API AOpartsAltar : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AOpartsAltar();

	// 이 제단이 플레이어에게 줄 오파츠 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts|Setup")
	TObjectPtr<const UOpartsDefinition> OpartsToEquip;

	// UI에 띄울 상호작용 텍스트 (예: "수정해골 장착")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts|Setup")
	FText InteractText = FText::FromString(TEXT("오파츠 장착"));

protected:
	// 인터페이스 오버라이드
	virtual void ExecuteInteraction(class AActor* Interactor) override;
	virtual FText GetInteractText() const override;

	// [선택] 장착 성공 시 블루프린트에서 파티클이나 사운드를 넣을 수 있도록 구멍만 뚫어둠
	UFUNCTION(BlueprintImplementableEvent, Category = "Oparts|Event")
	void OnAltarInteracted();
};