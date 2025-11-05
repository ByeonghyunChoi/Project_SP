// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FieldActionComponent.generated.h"

class APlayerCharacter;
class AMonsterCharacter;
class IInteractableInterface;
class UBoxComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UFieldActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFieldActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Field Action")
	float AttackRange = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Field Action")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldMode")
	bool bIsMoving = true;
	UPROPERTY()
	TObjectPtr<APlayerCharacter> OwningPlayer;


public:	
	// 플레이어의 공격 입력을 받았을 때 호출됩니다.
	UFUNCTION(BlueprintCallable, Category = "Field Action")
	void StartAttackSequence();

	// 공격 애니메이션의 특정 시점(AnimNotify)에서 호출되어 실제 피격 판정을 수행합니다.
	UFUNCTION(BlueprintCallable, Category = "Field Action")
	void PerformAttackHitCheck();

	// 공격 애니메이션이 끝났을 때 호출됩니다.
	UFUNCTION(BlueprintCallable, Category = "Field Action")
	void OnAttackAnimationFinished();

	//상호작용 실행 함수
	void PerformInteraction(IInteractableInterface* Target);

		
};
