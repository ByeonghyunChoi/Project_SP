// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/PlayerCharacter.h"
#include "Core/BattleManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Event/GameEventComponent.h"
#include "GameMode/FieldModeComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	FieldModeComp = CreateDefaultSubobject<UFieldModeComponent>(TEXT("FieldModeComponent"));

	FieldModeComp->SetComponentTickEnabled(false);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	CurrentlySelectedTarget = nullptr;
}

void APlayerCharacter::EnterFieldMode()
{
	// 필드 모드 컴포넌트 활성화
	if (FieldModeComp)
	{
		FieldModeComp->SetComponentTickEnabled(true);
		UE_LOG(LogTemp, Log, TEXT("플레이어 필드 모드 진입."));
	}
}

void APlayerCharacter::EnterBattleMode()
{
	// 전투 진입 시 필드 모드 컴포넌트 비활성화
	if (FieldModeComp)
	{
		FieldModeComp->SetComponentTickEnabled(false);
		UE_LOG(LogTemp, Log, TEXT("필드 모드 비활성화."));
	}
}

// -- - 이벤트 핸들러 구현-- -
void APlayerCharacter::HandlePlayerTurnStarted(ACombatPawn * TurnPawn)
{
    if (TurnPawn == this) // 내 턴이 시작되었을 때
    {
        UE_LOG(LogTemp, Log, TEXT("%s (Player) Turn Started. Awaiting Action Selection."), *GetName());
        // TODO: 플레이어 턴 UI (행동 선택 메뉴) 활성화
        // ACombatPawn의 InternalSetCombatPawnState를 통해 ECombatPawnState::SelectingAction으로 상태 변경
        InternalSetCombatPawnState(ECombatPawnState::SelectingAction);
    }
}

void APlayerCharacter::HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState)
{
    if (Pawn == this) // 내 Pawn 상태가 변경되었을 때
    {
        UE_LOG(LogTemp, Log, TEXT("%s's CombatPawnState changed to %s."), *GetName(), *UEnum::GetValueAsString(TEXT("ECombatPawnState"), NewState));
        // TODO: 상태에 따른 UI 피드백 (예: SelectingTarget 상태 시 타겟 선택 UI 활성화)
    }
}

// --- 플레이어 행동/타겟 선택 로직 ---
void APlayerCharacter::PlayerSelectAction(FName ActionID)
{
    // ACombatPawn의 SelectAction 호출 (코스트 체크, ActiveActionInstance 생성, 상태 변경 등 ACombatPawn의 공통 로직 실행)
    Super::SelectAction(ActionID);

    // SelectAction 호출 후 CombatPawnState가 ECombatPawnState::SelectingTarget으로 변경되었는지 확인
    if (GetCombatPawnState() == ECombatPawnState::SelectingTarget)
    {
        // TODO: 타겟 선택 UI 활성화 (예: 모든 몬스터 머리 위에 타겟링 아이콘 표시)
        UE_LOG(LogTemp, Log, TEXT("%s has selected action %s. Awaiting Target Selection."), *GetName(), *ActionID.ToString());
    }
}

void APlayerCharacter::PlayerTargetMonsterClicked(ACombatPawn* Target)
{
    // 타겟 선택 상태가 아니면 무시
    if (GetCombatPawnState() != ECombatPawnState::SelectingTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is not in SelectingTarget state. Target click ignored."), *GetName());
        return;
    }

    // 유효한 타겟인지, 적 진영인지, 살아있는지 확인
    if (!Target || !Target->IsValidLowLevel() || Target->GetFaction() == GetFaction() || !Target->GetStatsComponent() || Target->GetStatsComponent()->GetCurrentHealth() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s selected invalid target. Please select a valid enemy."), *Target->GetName());
        // TODO: 유효하지 않은 타겟에 대한 UI 피드백
        return;
    }

    CurrentlySelectedTarget = Target; // 현재 선택된 타겟 저장
    UE_LOG(LogTemp, Log, TEXT("%s selected target: %s."), *GetName(), *Target->GetName());

    // TODO: 타겟을 UI에 표시 (테두리 하이라이트 등)
    // 다음 행동으로 넘어갈 수 있는 조건 확인
    // 현재는 단일 타겟팅이므로 타겟 하나만 있어도 확정 가능

    // PlayerConfirmTargets()를 명시적으로 호출하도록 하거나, 
    // 마우스 좌클릭을 두 번 누르면 발동되도록 구현할 수도 있습니다.
}

void APlayerCharacter::PlayerConfirmTargets()
{
    // 타겟 선택 상태가 아니면 무시
    if (GetCombatPawnState() != ECombatPawnState::SelectingTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is not in SelectingTarget state. Confirm ignored."), *GetName());
        return;
    }

    FActionData ActionData = GetActionDataByID(SelectedActionID); // 현재 선택된 액션 데이터 가져오기

    TArray<ACombatPawn*> ConfirmedTargets;

    // 타겟팅 타입에 따른 확정 로직 (ACombatPawn의 PerformAction에서는 ConfirmedTargets가 비어있어도 ExecuteConfirmedAction이 실행되므로, 여기서 확정 필요)
    if (ActionData.TargetingType == ETargetingType::Single)
    {
        if (CurrentlySelectedTarget)
        {
            ConfirmedTargets.Add(CurrentlySelectedTarget);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("%s tried to confirm single target action '%s' but no target was selected."), *GetName(), *ActionData.DisplayName.ToString());
            // TODO: 타겟 미선택 시 UI 피드백 및 타겟 선택 모드 유지
            return;
        }
    }
    else if (ActionData.TargetingType == ETargetingType::All)
    {
        // ABattleManager를 통해 모든 적 전투원을 가져옴
        ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
        if (BattleManager)
        {
            for (ACombatPawn* Comp : BattleManager->AllCombatants)
            {
                if (Comp && Comp->IsValidLowLevel() && Comp->GetFaction() != GetFaction() && Comp->GetStatsComponent() && Comp->GetStatsComponent()->GetCurrentHealth() > 0)
                {
                    ConfirmedTargets.Add(Comp);
                }
            }
        }
        else { UE_LOG(LogTemp, Error, TEXT("BattleManager not found for AllEnemies targeting.")); return; }
    }
    else if (ActionData.TargetingType == ETargetingType::Dual)
    {
        ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
        if (BattleManager)
        {
            for (ACombatPawn* Comp : BattleManager->AllCombatants)
            {
                if (Comp && Comp->IsValidLowLevel() && Comp->GetFaction() != GetFaction() && Comp->GetStatsComponent() && Comp->GetStatsComponent()->GetCurrentHealth() > 0)
                {
                    ConfirmedTargets.Add(Comp);
                    if (ConfirmedTargets.Num() >= 2)
                    {
                        break;
                    }
                }
            }
        }
    }

    // 타겟 확정 후 ACombatPawn의 ExecuteConfirmedAction 호출
    if (ConfirmedTargets.Num() > 0)
    {
        Super::ExecuteConfirmedAction(SelectedActionID, ConfirmedTargets);
        // ExecuteConfirmedAction 호출 후 PawnState는 PerformingAction으로 변경됩니다.
        // ActingCombatPawnState는 PerformAction 완료 후 BroadcastActionExecutionFinished 이벤트에서 Idle로 돌아갈 것입니다.
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s confirmed action '%s' but no valid targets found. Action cancelled."), *GetName(), *ActionData.DisplayName.ToString());
        // TODO: 타겟을 찾을 수 없거나 유효하지 않아 행동을 취소했음을 알리는 UI/사운드 피드백
        InternalSetCombatPawnState(ECombatPawnState::Idle); // 상태 초기화
        SelectedActionID = NAME_None;
        ActiveActionInstance = nullptr;
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this); // 행동을 실행할 수 없으므로 즉시 턴 종료 신호
    }
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetFaction(EFaction::Player);
	if (GameEventComponent)
	{
		GameEventComponent->OnTurnStarted.AddDynamic(this, &APlayerCharacter::HandlePlayerTurnStarted);
		GameEventComponent->OnCombatPawnStateChanged.AddDynamic(this, &APlayerCharacter::HandleMyPawnStateChanged);
	}
}