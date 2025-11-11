// CombatPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combat/CombatTypes.h" 
#include "CombatPawn.generated.h"

// 컴포넌트들에 대한 전방 선언
class UAttributesComponent;
class UActionComponent;
class UBattleTurnComponent;
class UStatusEffectComponent;
class UGameEventComponent;
class UWidgetComponent;


UCLASS(Abstract) 
class PROJECT_SP_API ACombatPawn : public ACharacter
{
    GENERATED_BODY()

public:
    ACombatPawn();

protected:
    virtual void BeginPlay() override;

    //캐릭터 이름
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Info")
    FText DisplayName;

    // --- 핵심 컴포넌트 ---
    // 모든 수치(체력, SP, 공격력 등)를 관리합니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAttributesComponent> AttributesComponent;

    // 모든 행동(스킬)의 목록을 관리하고 실행을 책임집니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UActionComponent> ActionComponent;

    // 턴 순서 관련 데이터를 관리합니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBattleTurnComponent> BattleTurnComponent;

    // 상태 이상(버프/디버프)을 관리합니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStatusEffectComponent> StatusEffectComponent;

    // 외부 시스템과의 모든 통신(이벤트 방송)을 담당합니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UGameEventComponent> GameEventComponent;

    //데미지 수치를 표시할 UI컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> DamageFloaterWidgetComponent;

    // --- 내부 상태 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ECombatPawnState CurrentPawnState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EFaction CurrentFaction;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
    FTransform HomeTransform;

public:
    // --- 공통 기능 ---
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets) PURE_VIRTUAL(ACombatPawn::OnTurnBegin, );

    // --- 이름Getter함수 --- 
    UFUNCTION(BlueprintPure, Category = "Character Info")
    FText GetCharacterDisplayName() const;
    // --- 이름Setter함수 ---
    void SetCharacterDisplayName(const FText& NewName);

    // --- 접근자(Getter) 함수 ---
    UFUNCTION(BlueprintPure)
    FORCEINLINE UAttributesComponent* GetAttributesComponent() const { return AttributesComponent; }
    FORCEINLINE UActionComponent* GetActionComponent() const { return ActionComponent; }
    FORCEINLINE UBattleTurnComponent* GetBattleTurnComponent() const { return BattleTurnComponent; }
    FORCEINLINE UStatusEffectComponent* GetStatusEffectComponent() const { return StatusEffectComponent; }
    FORCEINLINE UGameEventComponent* GetGameEventComponent() const { return GameEventComponent; }
    FORCEINLINE ECombatPawnState GetCombatPawnState() const { return CurrentPawnState; }
    FORCEINLINE EFaction GetFaction() const { return CurrentFaction; }
    FORCEINLINE UWidgetComponent* GetDamageFloaterWidgetComponent() const { return DamageFloaterWidgetComponent; }

    // --- 상태 변경자(Setter) 함수 ---
    UFUNCTION(BlueprintCallable, Category = "State")
    void SetCombatPawnState(const ECombatPawnState& NewState);

    UFUNCTION(BlueprintCallable, Category = "State")
    void SetFaction(const EFaction& NewFaction);

    UFUNCTION()
    void HandleOwnerHealthDepleted(AActor* Victim, AActor* InInstigator);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "ShowDamageFloaterVFX"))
    void K2_ShowDamageFloater(float DamageAmount, EDamageFloaterType DamageType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat", meta = (DisplayName = "OnDied_VFX"))
    void K2_OnDied();

    void SetHomeTransform(const FTransform& NewHomeTransform) { HomeTransform = NewHomeTransform; }
    FTransform GetHomeTransform() const { return HomeTransform; }
};