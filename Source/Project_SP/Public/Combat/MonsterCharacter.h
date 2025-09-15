#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatPawn.h"
#include "Combat/MonsterGroupObject.h"
#include "MonsterCharacter.generated.h"

UCLASS()
class PROJECT_SP_API AMonsterCharacter : public ACombatPawn
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AMonsterCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Monster Group")
    UMonsterGroupObject* CombatMonsterGroup;
    // 몬스터 애니메이션 액션과 연결함
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animations")
    TMap<FName, TSoftObjectPtr<UAnimMontage>> ActionMontageMap;
    // 공격할 타겟
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
    TWeakObjectPtr<ACombatPawn> CurrentTarget;
public:
 
    // --- 몬스터 전용 이벤트 핸들러 ---
    UFUNCTION()
    void HandleThisMonsterTurnStarted(ACombatPawn* TurnPawn);


    UFUNCTION(BlueprintPure, Category = "Monster Group")
    UMonsterGroupObject* GetCombatMonsterGroup() const;

    virtual void OnTurnBegin() override;

};