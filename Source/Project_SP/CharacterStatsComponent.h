// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UCharacterStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterStatsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fCurrentHealth;
	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fMaxHealth;
	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fAttackPower;
	// 방어력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fDefensePower;
	// 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fMovementSpeed;
	// 치명타 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fCriticalChance;
	// 치명타 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fCriticalDamageMultiplier;
	// 명중치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fHitProbability;
	// 회피치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fEvasion;
	// 상태 저항
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fStatusEffectResistance;
	// 상태 적중
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fStatusEffectAccuracy;
	// 피해량 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fDamageIncreaseMultiplier;
	// 피해량 감소
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fDamageReductionMultiplier;
	// 방어 무시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float fArmorPenetration;

public:
	//Geter 함수
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentHealth() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHealth() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetAttackPower() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetDefensePower() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMovementSpeed() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCriticalChance() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCriticalDamageMultiplier() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHitProbability() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetEvasion() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStatusEffectResistance() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStatusEffectAccuracy() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetDamageIncreaseMultiplier() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetDamageReductionMultiplier() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetArmorPenetration() const;

	//Seter 함수
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCurrentHealth(const float& InCurrentHealth);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetMaxHealth(const float& InMaxHealth);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetAttackPower(const float& InAttackPower);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetDefensePower(const float& InDefensePower);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetMovementSpeed(const float& InMovementSpeed);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCriticalChance(const float& InCriticalChance);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCriticalDamageMultiplier(const float& InCriticalDamageMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetHitProbability(const float& InHitProbability);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetEvasion(const float& InEvasion);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStatusEffectResistance(const float& InStatusEffectResistance);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStatusEffectAccuracy(const float& InStatusEffectAccuracy);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetDamageIncreaseMultiplier(const float& InDamageIncreaseMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetDamageReductionMultiplier(const float& InDamageReductionMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetArmorPenetration(const float& InArmorPenetration);

	//데이터 복사 함수
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void CopyFrom(UCharacterStatsComponent* OtherStats);
};
