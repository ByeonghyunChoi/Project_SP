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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterStatusEffectChangedDelegate);

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

    //상태이상 태그과 남은 턴 수를 가져올 함수
    UFUNCTION(BlueprintPure, Category = "Combat | UI")
    TMap<FGameplayTag, int32> GetActiveDebuffs() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat | UI")
    void OnTargetStateChanged(bool bSelected, bool bIsPrimary);

    // UI 갱신 방송 함수(상태이상)
    void BroadcastStatusUI();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Die();

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
    FGameplayTagContainer WeaknessTags;

    UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
    FOnMonsterHPChangedDelegate OnMonsterHPChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
    FOnMonsterWeaknessDelegate OnMonsterWeaknessInitialized;

    UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
    FOnMonsterStatusEffectChangedDelegate OnMonsterStatusChanged;
protected:

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    void OnMaxHealthChanged(const struct FOnAttributeChangeData& Data);

    // UI 갱신 방송 함수(HP)
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