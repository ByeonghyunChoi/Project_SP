#include "Combat/MonsterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BattleManager.h"
#include "Combat/GameAction.h"
#include "Event/GameEventComponent.h"
#include "Combat/CombatStatics.h"
#include "Data/ActionData.h"


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{

}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();
    SetFaction(EFaction::Enemy);

    if (GameEventComponent)
    {
        GameEventComponent->OnTurnStarted.AddDynamic(this, &AMonsterCharacter::HandleThisMonsterTurnStarted);
    }
}

void AMonsterCharacter::DecideAction()
{
    UE_LOG(LogTemp, Log, TEXT("%s (Monster) is deciding its action."), *GetName());

    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager)
    {
        UE_LOG(LogTemp, Error, TEXT("AMonsterCharacter could not find ABattleManager. Cannot decide action."));
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this); // BattleManager 없으면 턴 종료
        return;
    }

    // 몬스터 AI 행동 결정 (가장 간단한 로직: 첫 번째 사용 가능한 행동 사용)
    FName ActionIDToPerform = NAME_None;
    if (MyActionIDs.Num() > 0)
    {
        // 사용 가능한 액션 중 하나를 랜덤하게 선택
        int32 RandomIndex = FMath::RandRange(0, MyActionIDs.Num() - 1);
        ActionIDToPerform = MyActionIDs[RandomIndex];
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s has no available actions. Ending turn."), *GetName());
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("액션 선택 완료 매우 성공적임;;;"));
    // 오버라이드된 SelectAction 호출 (몬스터의 경우 즉시 행동 실행)
    SelectAction(ActionIDToPerform);

    UE_LOG(LogTemp, Log, TEXT("%s decided to use %s."), *GetName(), *GetActionDataByID(ActionIDToPerform).DisplayName.ToString());
}

void AMonsterCharacter::SelectAction(FName ActionID)
{
    // ACombatPawn의 SelectAction 로직을 거의 그대로 가져오지만, 타겟 선택 단계에서 즉시 실행합니다.
    if (GetCombatPawnState() == ECombatPawnState::PerformingAction || GetCombatPawnState() == ECombatPawnState::Defeated)
    {
        UE_LOG(LogTemp, Log, TEXT("여기서 걸린듯;;; ㅇㅇ"));
        return;
    }

    FActionData ActionData = GetActionDataByID(ActionID);

    if (!ActionData.GameActionClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActionData for '%s' has no valid GameActionClass assigned."), *ActionData.DisplayName.ToString());
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    ActiveActionInstance = NewObject<UGameAction>(this, ActionData.GameActionClass);
    if (!ActiveActionInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create ActionInstance for %s."), *ActionData.DisplayName.ToString());
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    SelectedActionID = ActionID; // 선택된 ActionID 저장

    // --- 몬스터 고유 로직: 타겟 선택 UI를 거치지 않고 AI가 타겟을 결정하여 즉시 실행 ---
    InternalSetCombatPawnState(ECombatPawnState::SelectingTarget); 

    // ABattleManager를 통해 전체 전투원 목록에 접근하여 AI 타겟 결정
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager)
    {
        UE_LOG(LogTemp, Error, TEXT("AMonsterCharacter could not find ABattleManager for action execution."));
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    TArray<ACombatPawn*> ConfirmedTargets;


    if (ActionData.TargetingType == ETargetingType::Single)
    {
        // 살아있는 플레이어 타겟 찾기
        for (ACombatPawn* Comp : BattleManager->AllCombatants)
        {
            if (Comp && Comp->IsValidLowLevel() && Comp->GetFaction() == EFaction::Player && Comp->GetStatsComponent() && Comp->GetStatsComponent()->GetCurrentHealth() > 0)
            {
                ConfirmedTargets.Add(Comp);
                break;
            }
        }
    }
    // ExecuteConfirmedAction 호출 (몬스터는 타겟 선택 UI를 거치지 않고 바로 실행)
    ExecuteConfirmedAction(ActionID, ConfirmedTargets);
}

void AMonsterCharacter::HandleThisMonsterTurnStarted(ACombatPawn* TurnPawn)
{
    if (TurnPawn == this)
    {
        // 몬스터 AI가 행동을 결정하고 수행하도록 지시
        DecideAction();
    }
}

void AMonsterCharacter::ResolveAction()
{
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager || BattleManager->GetCurrentTurnCharacter() != this) return;

    EParryResult ParryResult = BattleManager->GetCurrentTurnParryResult();

    // 패링에 성공했다면, 턴이 이미 종료되었을 것이므로 아무것도 하지 않고 종료.
    if (ParryResult == EParryResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("%s's attack is cancelled by successful parry."), *GetName());
        return;
    }

    // 플레이어 타겟 찾기
    ACombatPawn* PlayerTarget = nullptr;
    for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
    {
        if (Combatant && Combatant->GetFaction() == EFaction::Player)
        {
            PlayerTarget = Combatant;
            break;
        }
    }
    if (!PlayerTarget) return;

    // UGameAction의 데미지 공식을 참고하여 전체 데미지 계산
    float FullDamage = UCombatStatics::CalculateDamage(
        GetStatsComponent(),
        PlayerTarget->GetStatsComponent(),
        GetActionDataByID(SelectedActionID).SkillCoefficient
    );

    if (ParryResult == EParryResult::PartialSuccess)
    {
        // 부분 성공(가드) 시 70% 데미지
        UGameplayStatics::ApplyDamage(PlayerTarget, FullDamage * 0.7f, GetController(), this, UDamageType::StaticClass());
    }
    else // ParryResult::None
    {
        // 패링 시도 없었으면 100% 데미지
        UGameplayStatics::ApplyDamage(PlayerTarget, FullDamage, GetController(), this, UDamageType::StaticClass());
    }
}

UMonsterGroupObject* AMonsterCharacter::GetCombatMonsterGroup() const
{
    return CombatMonsterGroup;
}