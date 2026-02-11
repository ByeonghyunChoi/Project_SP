// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

// 재화 데이터를 관리하는 구조체
USTRUCT(BlueprintType)
struct FPlayerWallet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 Sand = 0; // 모래 (오파츠 강화)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 IncompleteEnergy = 0; // 불완전한 기운 (아티팩트 해금)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 Money = 0; // 골드
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const FPlayerWallet&, CurrentWallet);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FPlayerWallet Wallet;

	//추가 적인 내용물

public:
	// UI 업데이트 알림용
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	// =========================================================
	// 1. 조회 (Getter)
	// =========================================================
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSand() const { return Wallet.Sand; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetIncompleteEnergy() const { return Wallet.IncompleteEnergy; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMoney() const { return Wallet.Money; }

	// =========================================================
	// 2. 조작 (Gain / Consume)
	// =========================================================

	// 모래 획득
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddSand(int32 Amount);

	// 모래 소모 (성공 시 true 반환)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeSand(int32 Amount);

	// 기운 획득
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddIncompleteEnergy(int32 Amount);

	// 기운 소모 (성공 시 true 반환)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeIncompleteEnergy(int32 Amount);

	// 돈 획득
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddMoney(int32 Amount);

	// 돈 소모 (성공 시 true 반환)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeMoney(int32 Amount);
};