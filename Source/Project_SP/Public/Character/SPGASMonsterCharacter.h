#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "Data/CombatEncounterData.h"
#include "Character/MonsterInfo.h"
#include "Components/WidgetComponent.h"
#include "Tag/SPGameplayTags.h"
#include "SPGASMonsterCharacter.generated.h"



UCLASS()
class PROJECT_SP_API ASPGASMonsterCharacter : public ASPGASCharacterBase
{
    GENERATED_BODY()

public:
    ASPGASMonsterCharacter();
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatEncounterData> EncounterData;

    UFUNCTION(BlueprintPure, Category = "Enemy Stats")
    EEnemyRank GetEnemyRank() const { return EnemyRank; }

    UFUNCTION(BlueprintCallable, Category = "GAS")
    void InitializeEnemyStats(int32 NewLevel, float StatMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetSelectedWidget(bool bSelected);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
    FGameplayTagContainer WeaknessTags;


protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Stats")
    EEnemyRank EnemyRank = EEnemyRank::Normal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> TargetIndicatorWidget;

    //몬스터 등급 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster | Tags")
    FGameplayTag MonsterRankTag;
};