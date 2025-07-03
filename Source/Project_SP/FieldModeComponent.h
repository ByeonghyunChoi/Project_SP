// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FieldModeComponent.generated.h"

class APlayerCharacter;   
class AMonsterCharacter;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UFieldModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFieldModeComponent();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 공격 범위
	UPROPERTY(EditAnywhere, Category = "FieldMode")
	float AttackRange = 200.0f;

	//공격 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldMode")
	bool bIsAttacking = false;

	//이동 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldMode")
	bool bIsMoving = true;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void StartAttackSequence();

	//공격 판정 로직 분리
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	AMonsterCharacter* PerformAttackHitDetection();

	//레벨 전환 로직 분리
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void StartBattleTransition(AMonsterCharacter* HitMonster);

	// 공격 애니메이션 종료 시 호출될 함수 (애니메이션 노티파이에서 호출)
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void OnAttackAnimationFinished();
	};
