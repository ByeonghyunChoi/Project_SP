#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "Data/CombatEncounterData.h"
#include "Character/MonsterInfo.h"
#include "Components/WidgetComponent.h"
#include "Tag/SPGameplayTags.h"
#include "SPGASMonsterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterHPChangedDelegate, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterWeaknessDelegate, const FGameplayTagContainer&, Weaknesses);

UCLASS()
class PROJECT_SP_API ASPGASMonsterCharacter : public ASPGASCharacterBase
{
    GENERATED_BODY()

public:
    ASPGASMonsterCharacter();
    virtual void BeginPlay() override;
    virtual void OnBattleStarted() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UCombatEncounterData> EncounterData;

    UFUNCTION(BlueprintPure, Category = "Enemy Stats")
    EEnemyRank GetEnemyRank() const { return EnemyRank; }

    UFUNCTION(BlueprintCallable, Category = "GAS")
    void InitializeEnemyStats(int32 NewLevel, float StatMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetSelectedWidget(bool bSelected, bool bIsPrimary);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat | UI")
    void OnTargetStateChanged(bool bSelected, bool bIsPrimary);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
    FGameplayTagContainer WeaknessTags;

    UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
    FOnMonsterHPChangedDelegate OnMonsterHPChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
    FOnMonsterWeaknessDelegate OnMonsterWeaknessInitialized;
protected:

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    void OnMaxHealthChanged(const struct FOnAttributeChangeData& Data);

    // UI 갱신 방송 함수
    void BroadcastHPUI();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Stats")
    EEnemyRank EnemyRank = EEnemyRank::Normal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> TargetIndicatorWidget;

    //몬스터 등급 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster | Tags")
    FGameplayTag MonsterRankTag;

    //몬스터 정보 위젯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UWidgetComponent> StatusWidgetComponent;
};