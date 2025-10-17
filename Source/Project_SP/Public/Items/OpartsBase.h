
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpartsComponent.generated.h"

USTRUCT(BlueprintType)
struct FOpartStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts")
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts")
	float Speed;

};

UCLASS()
class PROJECT_SP_API UOpartsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// 생성자
	UOpartsComponent();

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

	UFUNCTION(BlueprintCallable, Category = "Oparts")


protected:
	//오파츠의 현재 레벨 - 최대레벨 5로 제한
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	int32 CurrentLevel;

	//레벨별 스탯 정보(데이터 테이블로 관리하는 것이 좋아보임 아직 데이터 테이블로 만들지 않음)
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<FOpartStats> LevelStats;

	//레벨업에 필요한 모래 양
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<int32> RequiredSand;

	//오파츠의 현재 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	FOpartStats CurrentStats;

private:
	TObjectPtr<class UInventoryComponent> inventoryRef;
};