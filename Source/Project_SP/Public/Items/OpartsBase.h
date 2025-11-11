// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpartsBase.generated.h"

USTRUCT(BlueprintType)
struct FOpartStats : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts")
	float Health = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts")
	float Attack = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts")
	float Speed = 20;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCalculateMaterial);

// 나중에 델리게이트 이벤트 추가해서 무슨 함수 실행하면 UI에 뭐 띄우기 ex) 최대 레벨 도달 알림 등 등
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UOpartsBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// 생성자
	UOpartsBase();

	// 오파츠 레벨업 함수
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void LevelUpOparts();

	// 아티팩트 해금 함수
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual void UnlockArtifact();

	// 특수 능력 함수(나중에 같이 구현)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual void ActiveSpecialAbility();


	// 오파츠 현재 레벨 반환
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual int32 GetOpartsCurrentLevel() const;

	// 오파츠 다음 레벨업에 필요한 모래 양 반환
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual int32 GetRequiredSandForNextLevel() const;

	// 다음 아티팩트 해금에 필요한 불완전한 기운 반환
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual int32 GetRequiredIncompleteEnergy() const;

	// [1] 장착 시 호출 (스탯 적용 등)
	virtual void OnEquip(AActor* Instigator);

	// [2] 해제 시 호출 (스탯 제거 등)
	virtual void OnUnequip(AActor* Instigator);

public:
	UPROPERTY(BlueprintAssignable) FCalculateMaterial CalculateMaterial;

protected:
	// 시작함수
	virtual void BeginPlay() override;

	//오파츠의 현재 레벨 - 최대레벨 5로 제한
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	int32 CurrentLevel;

	// 아티팩트 해금 제한 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	TArray<bool> bIsArtifactUnlocked;

	UPROPERTY(EditDefaultsOnly, Category = "Oparts Data")
	class UDataTable* OpartsStatsDataTable;

	//레벨업에 필요한 모래 양
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<int32> RequiredSand;

	// 다음 해금에 필요한 불완전한 기운
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<int32> RequiredIncompleteEnergy;

	//오파츠의 현재 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	FOpartStats CurrentStats;

	//오파츠의 다음 레벨 스탯 반환
	UFUNCTION(BlueprintPure, Category = "Oparts")
	bool GetNextLevelStats(FOpartStats& OutNextStats) const;

	// 스탯이 현재 플레이어에게 적용(Add)된 상태인지 추적하는 플래그
	UPROPERTY(VisibleInstanceOnly, Category = "Oparts State")
	bool bStatsCurrentlyApplied = false; // 기본값은 false
		
private:
	TObjectPtr<class UInventoryComponent> inventoryRef;

	bool GetStatsForLevel(int32 Level, FOpartStats& OutStats);

	bool GetStatsForNextLevel(int32 Level, FOpartStats& OutStats) const;

	int32 ArtifactUnlockedNumber;
};
