// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "RewardBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewardInteractedSignature);

UCLASS()
class PROJECT_SP_API ARewardBox : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARewardBox();

	//RewardBox Structure Section
protected:
	//박스 외형
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RewardBox")
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	//Interaction Logic Section
public:
	//상호작용 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "RewardBox")
	FOnRewardInteractedSignature OnRewardInteracted;
	//상호작용 로직
	UFUNCTION(BlueprintCallable, Category = "RewardBox")
	void PerformInteraction();
protected:
	//상호작용 여부 판단(중복 방지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RewardBox")
	bool bHasBeenInteracted = false;

	//Interface Logic Section
public:
	//플레이어가 상호작용 키를 눌러서 호출할 함수
	virtual void ExecuteInteraction(APlayerCharacter* Interactor) override;

	//플레이어 UI에 표시할 텍스트를 반환
	virtual FText GetInteractText() override;
};
