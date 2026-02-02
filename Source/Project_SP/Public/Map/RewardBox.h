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

	bool bIsOpened = false;

};
