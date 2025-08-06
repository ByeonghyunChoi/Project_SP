// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterStatsData.h" 
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
	
public:
	//스탯 데이터를 참조하기 위한 변수, 캐릭터가 가질 스탯 데이터 테이블과 해당 데이터 테이블의 RowName을 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|DataTable")
	UDataTable* CharacterStatsDataTable;

	//DataTable에서 사용할 Row의 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|DataTable")
	FName RowName;

	//현재 스탯 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	FCharacterStatsData CurrentStats;

	//현재 스위치 포인트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCurrentSP = 0.0f;

	//최대 스위치 포인트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fMaxSP = 200.0f;

	//스탯 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void InitializeStatsFromDataTable();

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
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentSP() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxSP() const;
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStatusEffectMultiplier() const;

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
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCurrentSP(const float& InSP);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStatusEffectMultiplier(const float& InStatusEffectMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ModifySP(const float& Delta);
	//데이터 복사 함수
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void CopyFrom(UCharacterStatsComponent* OtherStats);
};
