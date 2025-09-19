#pragma once

#include "CoreMinimal.h"
#include "Character/CombatPawn.h"
#include "Combat/MonsterGroupObject.h"
#include "MonsterCharacter.generated.h"

class UAnimMontage;

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

    UPROPERTY(EditDefaultsOnly, Category = "AI|Actions")
    TArray<FName> DefaultActionIDs;

    // 몬스터 애니메이션 목록
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animations")
    TMap<FName, TSoftObjectPtr<UAnimMontage>> ActionMontageMap;

    // 약점 속성
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    EDamageType WeaknessType;
public:
 
    void PlayActionMontage(FName ActionID);

    UFUNCTION(BlueprintPure, Category = "Monster Group")
    UMonsterGroupObject* GetCombatMonsterGroup() const;

    void SetWeaknessType(EDamageType NewType);

    virtual void OnTurnBegin() override;

};