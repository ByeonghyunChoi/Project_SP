// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FPlayerMaterial
{
	GENERATED_BODY()

public:

	FPlayerMaterial() 
		: Sand(0)
		, IncompleteEnergy(0)
		, Money(0)
	{
	}

	FPlayerMaterial(int32 InSand, int32 InIncompleteEnergy, int32 InMoney)
		: Sand(InSand)
		, IncompleteEnergy(InIncompleteEnergy)
		, Money(InMoney)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Material")
	int32 Sand = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Material")
	int32 IncompleteEnergy = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Material")
	int32 Money = 0;
};

UCLASS()
class PROJECT_SP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// Sand (모래)
	UFUNCTION(BlueprintPure, Category = "Material")
	int32 GetCurrentSand() const;

	UFUNCTION(BlueprintCallable, Category = "Material")
	void SetCurrentSand(int32 NewAmount);

	// Incomplete Energy (불완전한 기운)
	UFUNCTION(BlueprintPure, Category = "Material")
	int32 GetCurrentIncompleteEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Material")
	void SetCurrentIncompleteEnergy(int32 NewAmount);

	// 유물 관련 기능 함수
	// 장착할 수 있는 유물 빈칸
	// int32 GetAvailableRelicSlot() const;
	// 장착
	void EquipRelic();
	// 해제
	void UnequipRelic();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material")
	FPlayerMaterial PlayerMaterials;
};