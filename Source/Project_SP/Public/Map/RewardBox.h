// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "Map/MapInfo.h"
#include "RewardBox.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class PROJECT_SP_API ARewardBox : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ARewardBox();

	//상호작용을 실행할 때 호출
	virtual void ExecuteInteraction(AActor* Interactor) override;
	//UI에 표시할 상호작용 텍스트 반환
	virtual FText GetInteractText() const override;
	// 사운드 재생 함수
	virtual void PlayInteractSound_Implementation() override;
	// 보상 상자 파티클 키는 함수
	void SetupParticleByMapType(EMapType InMapType);

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* RewardParticle;

	//에디터에서 WBP_RelicReward를 할당할 변수
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> RelicRewardWidgetClass;

	// 상자가 위치한 현재 스테이지 (확률 계산용)
	UPROPERTY(EditAnywhere, Category = "Reward")
	int32 StageLevel = 1;

	bool bIsOpened = false;

	UPROPERTY(EditDefaultsOnly, Category = "Reward | Particles")
	UNiagaraSystem* NormalParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Reward | Particles")
	UNiagaraSystem* EpicParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Reward | Particles")
	UNiagaraSystem* BossParticle;
};
