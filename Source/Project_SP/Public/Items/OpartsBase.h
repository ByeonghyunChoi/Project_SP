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

UCLASS()
class PROJECT_SP_API UOpartsBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// 생성자
	UOpartsBase();

	// 오파츠 레벨업 함수
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void LevelUpOparts();

	// 특수 능력 함수(나중에 같이 구현)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual void ActiveSpecialAbility();

	// 오파츠 현재 레벨 반환
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual int32 GetOpartsCurrentLevel() const;

	// 오파츠 다음 레벨업에 필요한 모래 양 반환
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual int32 GetRequiredSandForNextLevel() const;

	// [1] 장착 시 호출 (스탯 적용 등)
	virtual void OnEquip(AActor* Instigator);

	// [2] 해제 시 호출 (스탯 제거 등)
	virtual void OnUnequip(AActor* Instigator);


protected:
	// 시작함수
	virtual void BeginPlay() override;

	//오파츠의 현재 레벨 - 최대레벨 5로 제한
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	int32 CurrentLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Oparts Data")
	class UDataTable* OpartsStatsDataTable;

	//레벨업에 필요한 모래 양
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<int32> RequiredSand;

	//오파츠의 현재 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	FOpartStats CurrentStats;
		
private:
	TObjectPtr<class UInventoryComponent> inventoryRef;

	bool GetStatsForLevel(int32 Level, FOpartStats& OutStats);
};
