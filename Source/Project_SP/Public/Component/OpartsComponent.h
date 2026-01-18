// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "OpartsComponent.generated.h"

class UOpartsDefinition;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpartsChangedDelegate, const UOpartsDefinition*, NewOparts);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UOpartsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOpartsComponent();

protected:
	virtual void BeginPlay() override;

	// GAS 컴포넌트 캐싱
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	// 현재 장착중인 오파츠 데이터
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Oparts")
	TObjectPtr<const UOpartsDefinition> CurrentOparts;

	// [GAS 관리용] 장착 시 부여했던 능력과 이펙트의 핸들(ID)을 저장해둬야 나중에 뺄 수 있음
	FGameplayAbilitySpecHandle ActiveAbilityHandle;
	FActiveGameplayEffectHandle ActiveEffectHandle;

public:
	// [UI 연동용 델리게이트] 블루프린트에서 바인딩 가능
	UPROPERTY(BlueprintAssignable, Category = "Oparts|Events")
	FOnOpartsChangedDelegate OnOpartsChanged;

	// 오파츠 장착 (교체)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void EquipOparts(const UOpartsDefinition* NewOparts);

	// 오파츠 해제
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void UnequipOparts();

	// 현재 오파츠 가져오기
	UFUNCTION(BlueprintPure, Category = "Oparts")
	const UOpartsDefinition* GetCurrentOparts() const { return CurrentOparts; }
};
