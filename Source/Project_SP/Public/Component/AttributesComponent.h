// AttributesComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Items/OpartsBase.h"
#include "Data/CharacterStatsData.h"
#include "AttributesComponent.generated.h"


// --- 델리게이트 선언 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, CurrentHealth, float, Delta, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSPChanged, int32, CurrentSP, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExperienceChanged, int32, CurrentExperience, int32, NeededExperience);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyChanged, int32, CurrentMoney, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthDepleted, AActor*, Victim, AActor*, InInstigator);

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
    FCharacterStatsData BaseStats; // 레벨 1 기준 원본 스탯

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Stats")
    FCharacterStatsData CurrentStats; // 현재 레벨 및 버프/디버프가 적용된 실제 스탯

	// 오파츠로부터 얻은 추가 스탯 보너스
    UPROPERTY(VisibleAnywhere, Category = "Attributes|Oparts")
    FCharacterStatsData OpartsBonusStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Growth")
    int32 Level = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Growth")
    int32 Experience = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Growth")
    int32 NextLevelExperience = 100;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes|Resources")
    int32 SkillPoints = 3;



public:
    // --- 이벤트 방송 ---
    UPROPERTY(BlueprintAssignable) FOnHealthChanged OnHealthChanged;
    UPROPERTY(BlueprintAssignable) FOnSPChanged OnSPChanged;
    UPROPERTY(BlueprintAssignable) FOnExperienceChanged OnExperienceChanged;
    UPROPERTY(BlueprintAssignable) FOnLevelChanged OnLevelChanged;
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

    // 오파츠 스탯 적용 함수
	UFUNCTION(BlueprintCallable, Category = "Attributes|Oparts")
    void ApplyOpartsStats(const FOpartStats& OpartsStats);

    UFUNCTION(BlueprintCallable, Category = "Attributes|Oparts")
    void RemoveOpartsStats(const FOpartStats& OpartsStats);

    // --- 접근자(Getter) ---
    UFUNCTION(BlueprintPure, Category = "Attributes")
    const FCharacterStatsData& GetCurrentStats() const { return CurrentStats; }

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