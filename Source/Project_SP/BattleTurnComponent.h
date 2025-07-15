// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BattleTurnComponent.generated.h"

class UCharacterStatsComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UBattleTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBattleTurnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 현재 턴 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	bool bIsMyTurn;

	// 속도 기반 액션 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	float ActionValue;

	// 캐릭터 스탯 컴포넌트
	UPROPERTY()
	UCharacterStatsComponent* StatsComp;

	// 목표 액션 값 (턴이 돌아오는 기준 값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn", meta = (AllowPrivateAccess = "true"))
	float ActionThreshold;

public:	
	// 상태 확인
	UFUNCTION(BlueprintCallable, Category = "Turn")
	bool GetIsMyTurn() const;

	UFUNCTION(BlueprintCallable, Category = "Turn")
	float GetActionValue() const;

	UFUNCTION(BlueprintCallable, Category = "Turn")
	bool IsReadyForTurn() const;

	UFUNCTION(BlueprintCallable, Category = "Turn")
	float GetTimeLeftToAct() const;

	// 상태 제어
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void StartTurn();

	UFUNCTION(BlueprintCallable, Category = "Turn")
	void EndTurn();

	UFUNCTION(BlueprintCallable, Category = "Turn")
	void AdvanceActionValue(float DeltaTime);
		
};
