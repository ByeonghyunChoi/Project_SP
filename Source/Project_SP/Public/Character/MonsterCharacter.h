#pragma once

#include "CoreMinimal.h"
#include "Character/CombatPawn.h"
#include "Data/ActionData.h"
#include "MonsterCharacter.generated.h"

class UAnimMontage;
class UMonsterGroupObject;

UCLASS()
class PROJECT_SP_API AMonsterCharacter : public ACombatPawn
{
    GENERATED_BODY()

public:
    AMonsterCharacter();

protected:
    virtual void BeginPlay() override;

    // --- 몬스터 고유 데이터 ---

    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Monster Group")
    UMonsterGroupObject* CombatMonsterGroup;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Actions")
    TArray<FName> DefaultActionIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animations")
    TMap<FName, TSoftObjectPtr<UAnimMontage>> ActionMontageMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    EDamageType WeaknessType;
public:
    // --- 외부(주로 AIController)에서 호출하는 함수 ---

    void PlayActionMontage(FName ActionID);

    virtual void OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets) override;

    // Getter 및 Setter 함수
    UFUNCTION(BlueprintPure, Category = "Monster Group")
    UMonsterGroupObject* GetCombatMonsterGroup() const { return CombatMonsterGroup; }

    void SetWeaknessType(EDamageType NewType) { WeaknessType = NewType; }
};