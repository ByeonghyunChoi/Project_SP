// CombatPawn.cpp

#include "Character/CombatPawn.h"
#include "Component/AttributesComponent.h"
#include "Component/ActionComponent.h"
#include "Component/BattleTurnComponent.h"
#include "Component/StatusEffectComponent.h"
#include "Component/GameEventComponent.h"

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

        // 피해 사실을 외부에 알리는 것은 GameEventComponent에 위임합니다.
        GameEventComponent->BroadcastDamageReceived(this, ActualDamage, Cast<ACombatPawn>(DamageCauser), nullptr);
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

void ACombatPawn::HandleOwnerHealthDepleted(AActor* InInstigator)
{
    if (CurrentPawnState != ECombatPawnState::Defeated)
    {
        SetCombatPawnState(ECombatPawnState::Defeated);
        // 더 이상 타겟팅되지 않도록 충돌 비활성화
        SetActorEnableCollision(false);
    }
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