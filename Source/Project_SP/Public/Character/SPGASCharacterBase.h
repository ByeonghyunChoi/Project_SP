#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Manager/SPGASBattleTypes.h"
#include "SPGASCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterDamageDelegate, float, DamageAmount, bool, bIsCritical);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBattleActionFinished, AActor*);

UCLASS(Abstract) 
class PROJECT_SP_API ASPGASCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ASPGASCharacterBase();
    virtual void BeginPlay() override;
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void BroadcastDamageText(float DamageAmount, bool bIsCritical);

protected:
    UPROPERTY(EditAnywhere, Category = "GAS")
    TObjectPtr<class UAbilitySystemComponent> ASC;

    UPROPERTY(EditAnywhere, Category = "GAS")
    TObjectPtr<class USPGASAttributeSet> AttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<class USPStatusEffectComponent> StatusEffectComponent;

    // 피격 몽타주
    UPROPERTY(EditAnywhere, Category = "Combat|Animation")
    UAnimMontage* HitReactMontage;
public:
    // 사망 몽타주
    UPROPERTY(EditAnywhere, Category = "Combat|Animation")
    UAnimMontage* DeathMontage;
    
public:
    UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
    FCharacterDamageDelegate OnDamageTaken;

    FOnBattleActionFinished OnBattleActionFinished;

public:
    FORCEINLINE class USPGASAttributeSet* GetAttributeSet() const { return AttributeSet; }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<class UAnimMontage> FieldAttackMontage;

    //턴을 종료하고 GameMode에게 알림
    UFUNCTION(BlueprintCallable, Category = "GAS | Turn")
    virtual void FinishTurn();

    //행동 종료를 알림
    void NotifyBattleActionFinished();

    //현재 캐릭터가 턴을 수행할 수 있는지 여부 반환
    UFUNCTION(BlueprintPure, Category = "GAS | Turn")
    ETurnAvailability GetTurnAvailability() const;

    //턴 시작 시 쿨타임 감소
    UFUNCTION(BlueprintCallable, Category = "GAS | Turn")
    virtual void ReduceCooldowns();

    // 전투 준비가 끝났을 때 알리는 함수
    UFUNCTION(BlueprintCallable, Category = "GAS | ReadyCheck")
    virtual void ReportReadyToGameMode();

    // 데미지를 입었을 때 피격 애니메이션을 재생하는 함수
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void PlayHitReact(const FVector& ImpactPoint);

    // 전투가 시작할 때 사용할 함수(UI 키기 등)
    UFUNCTION(BlueprintCallable, Category = "Combat | UI")
    virtual void OnBattleStarted();

    UFUNCTION(BlueprintCallable, Category = "GAS | Tags")
    void AddLooseTagToASC(FGameplayTag TagToAdd);

    UFUNCTION(BlueprintCallable, Category = "GAS | Tags")
    void RemoveLooseTagFromASC(FGameplayTag TagToRemove);

    UFUNCTION(BlueprintCallable, Category = "GAS | Abilities")
    void CancelAbilitiesWithTag(FGameplayTagContainer WithTags);

    FORCEINLINE class USPStatusEffectComponent* GetStatusEffectComponent() const { return StatusEffectComponent; }

    void HandleSkippedTurn();

protected:
    virtual void OnSpeedChanged(const struct FOnAttributeChangeData& Data);
    virtual void OnActionGaugeChanged(const struct FOnAttributeChangeData& Data);
};