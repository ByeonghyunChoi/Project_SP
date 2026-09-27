// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tag/SPGameplayTags.h"
#include "GameplayEffect.h" //FGameplayEffectSpecHandle을 사용하기 위해
#include "SPStatusEffectComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatusVisualDelegate, AActor*, TargetActor, FGameplayTag, StatusTag, bool, bIsInstant);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API USPStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USPStatusEffectComponent();

protected:
	// 상태이상 데이터 에셋 (기존에 캐릭터에 있던 것)
	UPROPERTY(EditDefaultsOnly, Category = "Status Data")
	TObjectPtr<class UStatusEffectData> StatusEffectDataAsset;

	// 상태 이상 피해를 입힐 때 사용할 GE (태그: Damage.Type.Status)
	UPROPERTY(EditDefaultsOnly, Category = "Status Data")
	TSubclassOf<class UGameplayEffect> StatusDamageEffectClass;

	// 고정 피해를 입힐 때 사용할 GE (태그: Damage.Type.Fixed)
	UPROPERTY(EditDefaultsOnly, Category = "Status Data")
	TSubclassOf<class UGameplayEffect> FixedDamageEffectClass;

public:
	// 공격 적중 시 호출할 함수 (공격자가 타겟에게 부여)
	UFUNCTION(BlueprintCallable, Category = "Combat | Status")
	void ApplyWeaponStatusEffectToTarget(FGameplayTag WeaponTag, AActor* TargetActor);

	// 실제 상태이상 조합 및 적용 로직
	void ProcessStatusEffect(FGameplayTag IncomingStatusTag, class UAbilitySystemComponent* TargetASC, AActor* InstigatorActor);

	// 턴 시작 시 DoT 피해를 계산하고 적용하는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat | Status")
	void ProcessTurnStartDoT();

	//상태이상 턴 감소 함수
	UFUNCTION(BlueprintCallable, Category = "Combat | Status")
	void ReduceStatusEffectTurns();

	UFUNCTION(BlueprintCallable, Category = "Combat | Status")
	void ExecutePendingDamage(AActor* TargetActor, FGameplayTag StatusTag);

	void HandleSkippedTurn();

public:
	UPROPERTY(BlueprintAssignable, Category = "Combat | Visual")
	FOnStatusVisualDelegate OnStatusVisualTriggered;

private:
	// 기존 상태이상 삭제 헬퍼
	void RemoveStatusEffectByTag(class UAbilitySystemComponent* TargetASC, FGameplayTag StatusTagToRemove);

private:
	//받아야 할 데미지
	TMap<AActor*, TArray<FGameplayEffectSpecHandle>> PendingDamageMap;
};
