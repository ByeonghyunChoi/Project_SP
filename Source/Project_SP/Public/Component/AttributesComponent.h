// AttributesComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Items/OpartsBase.h"
#include "AttributesComponent.generated.h"


USTRUCT(BlueprintType)
struct FCombatStats : public FTableRowBase
{
    GENERATED_BODY()

    // 최대 체력, 공격력, 방어력 등 모든 스탯의 '상한선' 또는 '성장 최대치'
    // 예: MaxHealthCap은 레벨 50일 때 도달할 수 있는 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Caps")
    float MaxHealthCap = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Caps")
    float AttackPowerCap = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Caps")
    float DefensePowerCap = 250.f;

    // --- 스탯 목록 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fCurrentHealth = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fMaxHealth = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fAttackPower = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fDefensePower = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fMovementSpeed = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fCriticalChance = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fCriticalDamageMultiplier = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fHitProbability = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fEvasion = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fStatusEffectResistance = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fStatusEffectAccuracy = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fDamageIncreaseMultiplier = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fDamageReductionMultiplier = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fArmorPenetration = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float fStatusEffectMultiplier = 0.0f;
};

// --- 델리게이트 선언 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, CurrentHealth, float, Delta, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSPChanged, int32, CurrentSP, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExperienceChanged, int32, CurrentExperience, int32, NeededExperience);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyChanged, int32, CurrentMoney, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDepleted, AActor*, Instigator);

/**
 * @class UAttributesComponent
 * @brief 캐릭터의 성장과 관련된 모든 데이터(스탯, 레벨, 경험치, 재화 등)를 관리하는 중앙 컴포넌트.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UAttributesComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttributesComponent();

protected:
    virtual void BeginPlay() override;

    // --- 상수 ---
    const int32 MAX_LEVEL = 50;
    const int32 MAX_SP = 6;

    // --- 데이터 테이블 설정 ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Data")
    TObjectPtr<UDataTable> AttributesDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Data")
    FName CharacterID;

    // --- 데이터 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Stats")
    FCombatStats BaseStats; // 레벨 1 기준 원본 스탯

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Stats")
    FCombatStats CurrentStats; // 현재 레벨 및 버프/디버프가 적용된 실제 스탯

	// 오파츠로부터 얻은 추가 스탯 보너스
    UPROPERTY(VisibleAnywhere, Category = "Attributes|Oparts")
    FCombatStats OpartsBonusStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Growth")
    int32 Level = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Growth")
    int32 Experience = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Growth")
    int32 NextLevelExperience = 100;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Resources")
    int32 SkillPoints = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Resources")
    int32 Money = 0;

public:
    // --- 이벤트 방송 ---
    UPROPERTY(BlueprintAssignable) FOnHealthChanged OnHealthChanged;
    UPROPERTY(BlueprintAssignable) FOnSPChanged OnSPChanged;
    UPROPERTY(BlueprintAssignable) FOnExperienceChanged OnExperienceChanged;
    UPROPERTY(BlueprintAssignable) FOnLevelChanged OnLevelChanged;
    UPROPERTY(BlueprintAssignable) FOnMoneyChanged OnMoneyChanged;
    UPROPERTY(BlueprintAssignable) FOnHealthDepleted OnHealthDepleted;

    // --- 핵심 기능 함수 ---
    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void InitializeAttributes();

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void ApplyHealthChange(float Delta, AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void ApplySPChange(int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void GainExperience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void ApplyMoneyChange(int32 Delta);

    // 오파츠 스탯 적용 함수
	UFUNCTION(BlueprintCallable, Category = "Attributes|Oparts")
    void ApplyOpartsStats(const FOpartStats& OpartsStats);

    UFUNCTION(BlueprintCallable, Category = "Attributes|Oparts")
    void RemoveOpartsStats(const FOpartStats& OpartsStats);

    // --- 접근자(Getter) ---
    UFUNCTION(BlueprintPure, Category = "Attributes")
    const FCombatStats& GetCurrentStats() const { return CurrentStats; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    int32 GetLevel() const { return Level; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    int32 GetSkillPoint() const { return SkillPoints; }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    FName GetCharacterID() const { return CharacterID; }

    // --- 설정자(Setter) ---
    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetCharacterID(const FName& InID) { CharacterID = InID; }

private:
    // --- 내부 헬퍼 함수 ---
    void LevelUp();
    void RecalculateStatsForLevel(int32 NewLevel);
    void RecalculateFinalStats();
};