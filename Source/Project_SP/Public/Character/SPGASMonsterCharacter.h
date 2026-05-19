#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "Data/CombatEncounterData.h"
#include "Data/Asset/SPMonsterData.h"
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
    virtual void Destroyed() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TObjectPtr<UCombatEncounterData> EncounterData;

    //몬스터 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    class USPMonsterData* MonsterDataAsset;

    //해당 몬스터의 현재 레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    float CurrentLevel = 1.0f;

    // AI 패턴을 위한 상태 변수들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | AI")
    int32 CurrentTurnCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | AI")
    bool bWasAttackedLastTurn = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | AI")
    bool bIsSummonedMinion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | AI", meta = (ExposeOnSpawn = "true"))
    TObjectPtr<AActor> Summoner = nullptr;

public:
    UFUNCTION(BlueprintPure, Category = "Enemy Stats")
    EMonsterRank GetEnemyRank() const { return MonsterDataAsset ? MonsterDataAsset->MonsterRank : EMonsterRank::Normal; }

    UFUNCTION(BlueprintCallable, Category = "GAS")
    void ApplyMonsterData();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetSelectedWidget(bool bSelected, bool bIsPrimaryMarker, bool bShowOnHubUI);

    //상태이상 태그과 남은 턴 수를 가져올 함수
    UFUNCTION(BlueprintPure, Category = "Combat | UI")
    TMap<FGameplayTag, int32> GetActiveDebuffs() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat | UI")
    void OnTargetStateChanged(bool bSelected, bool bIsPrimaryMarker, bool bShowOnHubUI);

    // UI 갱신 방송 함수(상태이상)
    void BroadcastStatusUI();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteVisualDeath();

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetDeathMontageDuration() const;

    UFUNCTION(BlueprintPure, Category = "Combat | Data")
    FGameplayTagContainer GetCurrentWeaknessTags() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetWeaknessOverride(const FGameplayTagContainer& NewWeaknesses)
    {
        bHasWeaknessOverride = true;
        OverriddenWeaknessTags = NewWeaknesses;
    }

    UFUNCTION(BlueprintCallable, Category = "Combat | Status")
    void RemoveVisualPlayingTag();

    UFUNCTION(BlueprintCallable, Category = "Combat | AI")
    void SetAttackedLastTurn(bool bAttacked) { bWasAttackedLastTurn = bAttacked; }

public:
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

    void GiveDefaultAbilities();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> TargetIndicatorWidget;

    //몬스터 정보 위젯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UWidgetComponent> StatusWidgetComponent;

    bool bHasWeaknessOverride = false;
    FGameplayTagContainer OverriddenWeaknessTags;

    UPROPERTY(VisibleAnywhere)
    bool bIsDead = false;

    bool bDeathMontagePlayed = false;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS | Abilities")
    TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float TimeOfDeath = 0.0f;


};