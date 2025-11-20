// CombatPawn.cpp

#include "Character/CombatPawn.h"
#include "Component/AttributesComponent.h"
#include "Component/ActionComponent.h"
#include "Component/BattleTurnComponent.h"
#include "Component/StatusEffectComponent.h"
#include "Component/GameEventComponent.h"
#include "Components/WidgetComponent.h"

ACombatPawn::ACombatPawn()
{
    // 이 Pawn은 AI에 의해 직접 제어될 수 있도록 설정합니다.
    bUseControllerRotationYaw = false;
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // --- 5대 핵심 컴포넌트 생성 ---
    AttributesComponent = CreateDefaultSubobject<UAttributesComponent>(TEXT("AttributesComponent"));
    ActionComponent = CreateDefaultSubobject<UActionComponent>(TEXT("ActionComponent"));
    BattleTurnComponent = CreateDefaultSubobject<UBattleTurnComponent>(TEXT("BattleTurnComponent"));
    StatusEffectComponent = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComponent"));
    GameEventComponent = CreateDefaultSubobject<UGameEventComponent>(TEXT("GameEventComponent"));

    // --- 초기 상태 설정 ---
    CurrentPawnState = ECombatPawnState::Idle;
    CurrentFaction = EFaction::None;

    // --- UI 컴포넌트 생성 ---
    DamageFloaterWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageFloaterWidgetComponent"));
    DamageFloaterWidgetComponent->SetupAttachment(GetRootComponent());
    DamageFloaterWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    DamageFloaterWidgetComponent->SetVisibility(false);
    DamageFloaterWidgetComponent->SetDrawSize(FVector2D(300.f, 300.f));
}

void ACombatPawn::BeginPlay()
{
    Super::BeginPlay();

    if (AttributesComponent)
    {
        AttributesComponent->OnHealthDepleted.AddDynamic(this, &ACombatPawn::HandleOwnerHealthDepleted);
    }
}

float ACombatPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.f)
    {
        // 실제 피해 적용은 AttributesComponent에 위임합니다.
        AttributesComponent->ApplyHealthChange(-ActualDamage, DamageCauser);
    }

    return ActualDamage;
}

FText ACombatPawn::GetCharacterDisplayName() const
{
    return DisplayName;
}

void ACombatPawn::SetCharacterDisplayName(const FText& NewName)
{
    DisplayName = NewName;
}

void ACombatPawn::HandleOwnerHealthDepleted(AActor* Victim, AActor* InInstigator)
{
    if (CurrentPawnState == ECombatPawnState::Defeated || bIsVisualDeathPending)
    {
        return;
    }

    // 1. 상태는 'Defeated'로 변경 (더 이상 타겟팅되거나 행동하지 못하게)
    SetCombatPawnState(ECombatPawnState::Defeated);

    // 2. [핵심] 충돌은 끄지 않습니다! (남은 연타를 다 맞아야 하니까)
    // SetActorEnableCollision(false); <--- 주석 처리 또는 삭제

    // 3. [핵심] 사망 연출도 지금 안 합니다. 플래그만 켭니다.
    bIsVisualDeathPending = true;

    UE_LOG(LogTemp, Log, TEXT("%s is defeated but waiting for action to end."), *GetName());
}

void ACombatPawn::ExecuteDelayedDeath()
{
    if (bIsVisualDeathPending)
    {
        bIsVisualDeathPending = false;

        // 미뤄뒀던 충돌 해제
        SetActorEnableCollision(false);

        // 미뤄뒀던 사망 연출(Ragdoll or Anim) 재생
        K2_OnDied();

        UE_LOG(LogTemp, Log, TEXT("%s Visual Death Executed."), *GetName());
    }
}

void ACombatPawn::ReviveFromDefeat(float HealthPercentage)
{
    if (AttributesComponent)
    {
        float MaxHealth = AttributesComponent->GetCurrentStats().fMaxHealth;
        float CurrentHealth = AttributesComponent->GetCurrentStats().fCurrentHealth;
        float HealAmount = (MaxHealth * HealthPercentage) - CurrentHealth;
        if (HealAmount > 0)
        {
            AttributesComponent->ApplyHealthChange(HealAmount, this);
        }
    }

    // 1. 사망 예약 취소 (핵심!)
    bIsVisualDeathPending = false;

    // 2. 상태 복구
    // (죽었다가 살아났으니, 다시 대기 상태나 맞고 있는 상태로 변경)
    if (CurrentPawnState == ECombatPawnState::Defeated)
    {
        SetCombatPawnState(ECombatPawnState::Idle);
    }

    // 3. 충돌 및 기타 상태 확실하게 복구
    SetActorEnableCollision(true);

    UE_LOG(LogTemp, Warning, TEXT("Pawn %s Revived! Death Pending Cancelled."), *GetName());
}

void ACombatPawn::SetCombatPawnState(const ECombatPawnState& NewState)
{
    if (CurrentPawnState != NewState)
    {
        ECombatPawnState OldState = CurrentPawnState;
        CurrentPawnState = NewState;

        // 상태 변경 사실을 외부에 이벤트로 방송합니다.
        GameEventComponent->BroadcastCombatPawnStateChanged(this, NewState);

        UE_LOG(LogTemp, Log, TEXT("%s's state changed to %s"), *GetName(), *UEnum::GetValueAsString(NewState));
    }
}

void ACombatPawn::SetFaction(const EFaction& NewFaction)
{
    if (CurrentFaction != NewFaction)
    {
        CurrentFaction = NewFaction;
    }
}