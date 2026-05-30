// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SPDataStructs.h"
#include "InventoryComponent.generated.h"

// UI와 데이터 판별에 쓸 재화 타입 (추가된 부분!)
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Gold		UMETA(DisplayName = "골드"),
	Sand		UMETA(DisplayName = "모래"),
	IncompleteEnergy    UMETA(DisplayName = "불완전한 기운"),
	Fragment		UMETA(DisplayName = "파편")

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdated, const FPlayerRunWallet&, CurrentWallet, const FPlayerPermanentWallet&, CurrentPermWallet);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// 에디터 테스트용: 세이브 파일을 무시하고 초기 재화를 강제로 주입할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Debug")
	bool bForceGiveTestCurrencies = false;

	// UI 버튼으로 언제든 호출할 수 있는 재화 지급 치트키!
	UFUNCTION(BlueprintCallable, Category = "Inventory|Cheat")
	void GiveCheatCurrencies();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FPlayerRunWallet RunWallet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FPlayerPermanentWallet PermanentWallet;

public:
	// UI 업데이트 알림용
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	// =========================================================
	// 1. 조회 (Getter)
	// =========================================================
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSand() const { return PermanentWallet.Sand; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetIncompleteEnergy() const { return PermanentWallet.IncompleteEnergy; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMoney() const { return RunWallet.Money; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetFragment() const { return PermanentWallet.Fragment; }

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

	// 권능의 파편 획득
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddFragment(int32 Amount);

	// 권능의 파편 소모 (성공 시 true 반환)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeFragment(int32 Amount);

	//로드 시스템
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void LoadWalletData(const FPlayerRunWallet& InRunWallet, const FPlayerPermanentWallet& InPermWallet);

private:
	// 재화가 변동될 때마다 세이브 서브시스템의 메모리를 즉각 갱신하는 함수
	void SyncWalletToSaveSystem();
};