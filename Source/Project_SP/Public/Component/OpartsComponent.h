
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
	virtual int32 GetCurrentLevel() const;

	// 오파츠 다음 레벨업에 필요한 모래 양 반환
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	virtual int32 GetRequiredSandForNextLevel() const;

protected:
	//오파츠의 현재 레벨 - 최대레벨 5로 제한
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	int32 CurrentLevel;

	//레벨별 스탯 정보(데이터 테이블로 관리하는 것이 좋아보임)
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<FOpartStats> LevelStats;

	//레벨업에 필요한 모래 양
	UPROPERTY(EditDefaultsOnly, Category = "Oparts")
	TArray<int32> RequiredSand;

	//오파츠의 현재 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	FOpartStats CurrentStats;
};

// ++ 플레이어한테 불완전한 기운, 모레 추가 + 아티팩트 또한 오파츠컴포넌트에 추가하여 관리
// 이 오파츠를 베이스로 잡고 수정해골, 옥시계, 골드버그에게 상속하여 클래스 제작 - 그냥 여기서 베이스 상속받고
// 클래스를 여기다가 다 제작하면 안되나? 어차피 BP에게 상속해주고 플레이어한테 붙힐텐데