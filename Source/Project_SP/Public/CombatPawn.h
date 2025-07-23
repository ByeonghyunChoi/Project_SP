// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterStatsComponent.h"
#include "BattleTurnComponent.h"
#include "CombatPawn.generated.h"

UENUM(BlueprintType)
enum class EFaction : uint8
{
	Player UMETA(DisplayName = "플레이어"),
	Enemy UMETA(DisplayName = "적"),
	None UMETA(DisplayName = "None")
};

UCLASS()
class PROJECT_SP_API ACombatPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatPawn();

protected:

	virtual void BeginPlay() override;

	// 캐릭터 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString CharacterName;

	// 팩션 (플레이어 / 적)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EFaction Faction;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCharacterStatsComponent* StatsComponent;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBattleTurnComponent* BattleTurnComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "Character")
	FString GetCharacterName() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterName(const FString& NewName);

	// 팩션
	UFUNCTION(BlueprintCallable, Category = "Character")
	EFaction GetFaction() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetFaction(EFaction NewFaction);

	// 스탯 컴포넌트 접근
	UFUNCTION(BlueprintCallable, Category = "Stats")
	UCharacterStatsComponent* GetStatsComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	UBattleTurnComponent* GetBattleTurnComponent() const;
};
