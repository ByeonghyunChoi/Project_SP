#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/StatusEffectData.h"
#include "StatusEffectComponent.generated.h"

class ACombatPawn;
class UCharacterStatsComponent;

// 활성화된 상태 이상 정보를 담을 구조체
USTRUCT(BlueprintType)
struct FActiveStatusEffect
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName EffectID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RemainingTurns;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<ACombatPawn> Instigator; // 효과를 건 시전자

	FActiveStatusEffect() : RemainingTurns(0), Instigator(nullptr) {}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* StatusEffectDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effect")
	TArray<FActiveStatusEffect> ActiveStatusEffects;

	UPROPERTY()
	ACombatPawn* OwnerPawn;

	UPROPERTY()
	UCharacterStatsComponent* OwnerStatsComp;

	// 스탯 변경 효과를 모두 재계산하고 적용하는 함수
	void RecalculateStatModifiers();

public:
	// 상태 이상 적용 함수
	void ApplyStatusEffect(FName StatusEffectID, ACombatPawn* Instigator);

	// 턴이 시작될 때 BattleManager가 호출해 줄 함수
	void OnTurnStarted();
};