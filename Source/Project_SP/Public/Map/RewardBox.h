// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "RewardBox.generated.h"


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

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	//에디터에서 WBP_RelicReward를 할당할 변수
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> RelicRewardWidgetClass;

	// 상자가 위치한 현재 스테이지 (확률 계산용)
	UPROPERTY(EditAnywhere, Category = "Reward")
	int32 StageLevel = 1;

	bool bIsOpened = false;

};
