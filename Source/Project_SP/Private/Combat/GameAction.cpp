#include "Combat/GameAction.h"
#include "Component/ActionComponent.h"
#include "Component/AttributesComponent.h"
#include "Character/CombatPawn.h"
#include "Component/GameEventComponent.h"
#include "Combat/CombatStatics.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UGameAction::Initialize(UActionComponent* InOwningComponent, FName InActionID)
{
    OwningComponent = InOwningComponent;
    ActionID = InActionID;

    // 데이터 테이블에서 ActionID에 해당하는 데이터를 찾아 'Data' 변수에 저장
    if (OwningComponent && OwningComponent->GetActionDataTable())
    {
        const FActionData* FoundRow = OwningComponent->GetActionDataTable()->FindRow<FActionData>(ActionID, TEXT(""));
        if (FoundRow)
        {
            Data = *FoundRow;
        }
    }
}

void UGameAction::OpenParryWindow()
{
    ACombatPawn* Instigator = Cast<ACombatPawn>(GetOuter());
    if (!Instigator) return;

    if (Data.ParryWindowDuration > 0.f && Instigator->GetFaction() == EFaction::Enemy)
    {
        if (UGameEventComponent* EventComp = Instigator->GetGameEventComponent())
        {
            // 1. "패링 창 열림!" 이라고 월드에 방송합니다.
            EventComp->BroadcastParryWindowOpened(Instigator, Data.DamageType, Data.ParryWindowDuration);

            // --- 바로 이 부분이 핵심입니다 ---
            // 2. 정해진 시간(ParryWindowDuration) 후에 CloseParryWindow 함수를 호출하도록 타이머를 설정합니다.
            if (UWorld* World = Instigator->GetWorld())
            {
                World->GetTimerManager().SetTimer(ParryWindowTimerHandle, this, &UGameAction::CloseParryWindow, Data.ParryWindowDuration, false);
            }
        }
    }
}

void UGameAction::CloseParryWindow()
{
    ACombatPawn* Instigator = Cast<ACombatPawn>(GetOuter());
    if (!Instigator) return;

    // 타이머가 여러 번 호출되는 것을 방지하기 위해 즉시 클리어합니다.
    if (UWorld* World = Instigator->GetWorld())
    {
        World->GetTimerManager().ClearTimer(ParryWindowTimerHandle);
    }

    // 1. 논리적인 '닫힘' 신호를 다른 C++ 클래스(플레이어 등)에 방송합니다.
    if (UGameEventComponent* EventComp = Instigator->GetGameEventComponent())
    {
        EventComp->BroadcastParryWindowClosed(Instigator);
    }
}

bool UGameAction::CanStartAction_Implementation(ACombatPawn* Instigator)
{
    if (!Instigator) return false;

    // 비용이 0이면 항상 실행 가능
    if (Data.CostSP <= 0)
    {
        return true;
    }

    // 시전자의 AttributesComponent를 가져와 SP가 충분한지 확인
    UAttributesComponent* AttributesComp = Instigator->GetAttributesComponent();
    if (AttributesComp && AttributesComp->GetSkillPoint() >= Data.CostSP)
    {
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("SP가 부족하여 '%s'을(를) 사용할 수 없습니다."), *Data.DisplayName.ToString());
    return false;
}

void UGameAction::StartAction_Implementation(ACombatPawn* Instigator, const TArray<ACombatPawn*>& Targets)
{
    UE_LOG(LogTemp, Log, TEXT("'%s' 액션 시작. 시전자: %s"), *Data.DisplayName.ToString(), *Instigator->GetName());

  
    UAttributesComponent* AttributesComp = Instigator->GetAttributesComponent();
    if (AttributesComp)
    {
        AttributesComp->ApplySPChange(-Data.CostSP);
    }
    

    //데미지 적용 로직
    UAttributesComponent* InstigatorStats = Instigator->GetAttributesComponent();
    if (InstigatorStats)
    {
        for (ACombatPawn* Target : Targets)
        {
            if (Target && Target->GetCombatPawnState() != ECombatPawnState::Defeated)
            {
                UAttributesComponent* TargetStats = Target->GetAttributesComponent();
                if (TargetStats)
                {
                    // 1. 데미지 계산
                    float FinalDamage = UCombatStatics::CalculateDamage(InstigatorStats, TargetStats, Data.SkillCoefficient);

                    UE_LOG(LogTemp, Log, TEXT("%s attacks %s for %.1f damage."), *Instigator->GetName(), *Target->GetName(), FinalDamage);

                    // 2. 데미지 적용
                    UGameplayStatics::ApplyDamage(Target, FinalDamage, Instigator->GetController(), Instigator, UDamageType::StaticClass());
                }
            }
        }
    }
    EndAction(Instigator);
}

void UGameAction::EndAction(ACombatPawn* Instigator)
{
    if (Instigator && Instigator->GetGameEventComponent())
    {
        // GameEventComponent를 통해 "액션 실행이 끝났다"고 방송함.
        Instigator->GetGameEventComponent()->BroadcastActionExecutionFinished(Instigator);
    }
}
