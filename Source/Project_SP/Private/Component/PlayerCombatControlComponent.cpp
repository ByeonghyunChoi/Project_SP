// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PlayerCombatControlComponent.h"
#include "Character/PlayerCharacter.h"
#include "Component/GameEventComponent.h"
#include "Component/ActionComponent.h"
#include "Combat/GameAction.h"
#include "Component/WeaponSystemComponent.h"
#include "Data/ActionData.h"
#include "Data/WeaponData.h"
#include "Core/BattleManager.h"
#include "EnhancedInputComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/CombatTask.h"
#include "Combat/Tasks/Task_EndTurn.h"
#include "Combat/Tasks/Task_RequestPlayerInterrupt.h"

// Sets default values for this component's properties
UPlayerCombatControlComponent::UPlayerCombatControlComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentTargetIndex = -1;
}

// Called when the game starts
void UPlayerCombatControlComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (OwningPlayerCharacter)
	{
		ActionComponent = OwningPlayerCharacter->GetActionComponent();
		WeaponSystemComponent = OwningPlayerCharacter->FindComponentByClass<UWeaponSystemComponent>();
	}
}

void UPlayerCombatControlComponent::SetupPlayerInput(UEnhancedInputComponent* PlayerInputComponent)
{
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction(IA_SelectBasicAttack, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleSelectBasicAttack);
		PlayerInputComponent->BindAction(IA_SelectMainSkill, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleSelectMainSkill);

		PlayerInputComponent->BindAction(IA_ChangeFirstWeapon, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleChangeWeapon, 1);
		PlayerInputComponent->BindAction(IA_ChangeSecondWeapon, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleChangeWeapon, 2);
		PlayerInputComponent->BindAction(IA_ChangeThirdWeapon, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleChangeWeapon, 3);

		PlayerInputComponent->BindAction(IA_ConfirmAction, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleConfirmAction);
		PlayerInputComponent->BindAction(IA_CycleTarget, ETriggerEvent::Triggered, this, &UPlayerCombatControlComponent::HandleCycleTarget);
		PlayerInputComponent->BindAction(IA_SelectTargetMouse, ETriggerEvent::Started, this, &UPlayerCombatControlComponent::HandleSelectTargetMouse);
	}
}

void UPlayerCombatControlComponent::OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets)
{
	if (!OwningPlayerCharacter || !WeaponSystemComponent || !ActionComponent) return;

	if (bPendingParryInterrupt && !PendingParrySkillID.IsNone() && PendingParryTarget.IsValid())
	{
		ACombatPawn* Target = PendingParryTarget.Get(); 

		bPendingParryInterrupt = false;
		PendingParryTarget = nullptr;
		FName SkillToUse = PendingParrySkillID; 
		PendingParrySkillID = NAME_None;

		UE_LOG(LogTemp, Log, TEXT("Parry Interrupt Turn: Auto-executing skill %s against %s"), *SkillToUse.ToString(), *Target->GetName());

		OwningPlayerCharacter->SetCombatPawnState(ECombatPawnState::PerformingAction);
		ActionComponent->StartActionByID(OwningPlayerCharacter, SkillToUse, { Target });
		return; 
	}

	AllEnemyTargets = PotentialTargets;
	OwningPlayerCharacter->SetCombatPawnState(ECombatPawnState::AwaitingInput); 

	if (SelectedActionID.IsNone())
	{
		const EDamageType DefaultWeaponType = EDamageType::Fenrir;

		WeaponSystemComponent->SwitchWeapon(DefaultWeaponType);

		UWeaponData* CurrentWeaponData = WeaponSystemComponent->GetCurrentWeapon();
		if (CurrentWeaponData)
		{
			const FName BasicAttackID = CurrentWeaponData->BasicAttackActionID;
			SelectAction(BasicAttackID);
		}
		else
		{
			SelectedActionID = NAME_None;
			CurrentTargets.Empty();
			CurrentTargetIndex = -1;
			OnActionSelected.Broadcast(NAME_None);
			OnTargetsChanged.Broadcast(CurrentTargets);
		}
	}
	else
	{
		// 살아있는 적이 아무도 없으면 타겟팅을 중지합니다.
		if (AllEnemyTargets.Num() == 0)
		{
			CurrentTargets.Empty();
			CurrentTargetIndex = -1;
			OnTargetsChanged.Broadcast(CurrentTargets);
			return;
		}
		FActionData ActionData;
		if (!ActionComponent->GetActionData(SelectedActionID, ActionData))
		{
			SelectAction(NAME_None); 
			return;
		}
		CurrentTargetIndex = 0;
		const int32 NumEnemies = AllEnemyTargets.Num();
		const int32 NumberOfTargets = FMath::Min(ActionData.NumberOfTargets, NumEnemies);

		TArray<ACombatPawn*> NewTargets;
		for (int32 i = 0; i < NumberOfTargets; ++i)
		{
			NewTargets.Add(AllEnemyTargets[(CurrentTargetIndex + i) % NumEnemies]);
		}
		SetCurrentTargets(NewTargets);
	}
}

void UPlayerCombatControlComponent::HandleSelectBasicAttack(const FInputActionValue& Value)
{
	if (WeaponSystemComponent && WeaponSystemComponent->GetCurrentWeapon())
	{
		SelectAction(WeaponSystemComponent->GetCurrentWeapon()->BasicAttackActionID);
	}
}

void UPlayerCombatControlComponent::HandleSelectMainSkill(const FInputActionValue& Value)
{
	if (WeaponSystemComponent && WeaponSystemComponent->GetCurrentWeapon())
	{
		SelectAction(WeaponSystemComponent->GetCurrentWeapon()->SpecialSkillActionID);
	}
}

void UPlayerCombatControlComponent::OnReceiveParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration)
{
	// 유효성 검사 및 중복 방지
	if (bIsParryWindowOpen || !Attacker || Attacker->GetFaction() == EFaction::Player) return;

	// 패링 상태 저장
	bIsParryWindowOpen = true;
	RequiredParryType = AttackType;
	CurrentParryAttacker = Attacker;
	UE_LOG(LogTemp, Log, TEXT("Parry Window Opened! Attacker: %s, Required Type: %s"), *Attacker->GetName(), *UEnum::GetValueAsString(AttackType));
	// TODO: UI에 패링 가능 알림 표시 (예: 델리게이트 방송)
}

void UPlayerCombatControlComponent::OnReceiveParryWindowClosed(ACombatPawn* Attacker)
{
	// 올바른 Attacker가 닫았는지 확인
	if (bIsParryWindowOpen && CurrentParryAttacker == Attacker)
	{
		bIsParryWindowOpen = false;
		CurrentParryAttacker = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Parry Window Closed for %s"), *Attacker->GetName());
		// TODO: UI 패링 알림 제거
	}
}

void UPlayerCombatControlComponent::HandleChangeWeapon(int32 WeaponIndex)
{
	if (!WeaponSystemComponent || !OwningPlayerCharacter) return;

	EDamageType SelectedType = static_cast<EDamageType>(WeaponIndex - 1);

	// 1. [순서 변경] 실제 무기 교체를 먼저 시도 (SwitchWeapon은 중복 방지 기능 있음)
	WeaponSystemComponent->SwitchWeapon(SelectedType);

	// 2. 패링 시도 확인 (이제 교체된 무기 기준으로 판단)
	bool bIsPlayerTurnActive = (OwningPlayerCharacter->GetCombatPawnState() == ECombatPawnState::AwaitingInput);
	bool bParryCheckPerformed = false;
	bool bParrySucceeded = false;

	if (!bIsPlayerTurnActive && bIsParryWindowOpen && CurrentParryAttacker.IsValid())
	{
		bParryCheckPerformed = true;
		ACombatPawn* ParriedAttacker = CurrentParryAttacker.Get();

		// 현재 *교체된* 무기의 타입과 RequiredParryType 비교
		if (WeaponSystemComponent->GetCurrentWeapon() && WeaponSystemComponent->GetCurrentWeapon()->WeaponType == RequiredParryType)
		{
			OnParrySuccess(ParriedAttacker); // 성공 처리
			bParrySucceeded = true;
			// 패링 성공 시에는 여기서 함수 종료 (아래 자동 선택 로직 건너뛰기)
			return;
		}
		else
		{
			OnParryFailure(ParriedAttacker, EParryResult::PartialSuccess); // 실패 처리
		}

		// 패링 시도 후 창 닫기
		bIsParryWindowOpen = false;
		CurrentParryAttacker = nullptr;
	}

	// 3. 일반적인 무기 교체 후 기본 공격 자동 선택 (플레이어 턴일 때만)
	if (bIsPlayerTurnActive) // 패링 시도가 아니었거나 실패했을 때만 실행됨
	{
		// 이미 장착된 무기 버튼을 다시 누른 경우도 처리
		bool bIsAlreadyEquipped = false;
		if (WeaponSystemComponent->GetCurrentWeapon() && WeaponSystemComponent->GetCurrentWeapon()->WeaponType == SelectedType) {
			bIsAlreadyEquipped = true;
		}

		if (bIsAlreadyEquipped && WeaponSystemComponent->GetCurrentWeapon()) {
			SelectAction(WeaponSystemComponent->GetCurrentWeapon()->BasicAttackActionID);
		}
		else
		{
			// 다른 무기로 교체된 경우
			UWeaponData* NewWeaponData = WeaponSystemComponent->GetCurrentWeapon();
			if (NewWeaponData) SelectAction(NewWeaponData->BasicAttackActionID);
			else SelectAction(NAME_None);
		}
	}
}

void UPlayerCombatControlComponent::HandleConfirmAction(const FInputActionValue& Value)
{
	ConfirmAndExecuteAction();
}

void UPlayerCombatControlComponent::HandleCycleTarget(const FInputActionValue& Value)
{
	CycleTarget(Value.Get<float>());
}

void UPlayerCombatControlComponent::HandleSelectTargetMouse(const FInputActionValue& Value)
{
	SelectTargetByMouse();
}

void UPlayerCombatControlComponent::SelectAction(FName ActionID)
{
	if (!OwningPlayerCharacter || OwningPlayerCharacter->GetCombatPawnState() != ECombatPawnState::AwaitingInput || !ActionComponent) return;

	FActionData FoundData;
	if (ActionComponent->GetActionData(ActionID, FoundData))
	{
		SelectedActionID = ActionID;

		// "액션 선택됨!" 방송
		OnActionSelected.Broadcast(ActionID);

		if (CurrentTargetIndex == -1)
		{
			// 선택된 타겟이 없었을 경우에만 처음부터 타겟 선택을 시작합니다.
			BeginTargetSelection();
		}
		else
		{
			// 이미 타겟이 있었다면, 그 타겟(CurrentTargetIndex)을 기준으로 그룹만 다시 계산합니다.
			const int32 NumEnemies = AllEnemyTargets.Num();
			const int32 NumberOfTargets = FMath::Min(FoundData.NumberOfTargets, NumEnemies);

			TArray<ACombatPawn*> NewTargets;
			for (int32 i = 0; i < NumberOfTargets; ++i)
			{
				int32 TargetIndex = (CurrentTargetIndex + i) % NumEnemies;
				NewTargets.Add(AllEnemyTargets[TargetIndex]);
			}
			SetCurrentTargets(NewTargets);
		}
	}
}

void UPlayerCombatControlComponent::OnParrySuccess(ACombatPawn* ParriedAttacker)
{
	// 1. 유효성 검사
	if (!ParriedAttacker || !OwningPlayerCharacter || !ActionComponent || !WeaponSystemComponent) return;

	UE_LOG(LogTemp, Warning, TEXT("!!! PARRY SUCCESS vs %s !!!"), *ParriedAttacker->GetName());

	// 2. 적의 공격 행동 중단 (BattleManager 호출)
	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		// 적이 공격하던 태스크들을 모두 취소시킵니다.
		BattleManager->ClearTaskQueue();
	}

	// 3. 무기 데이터에서 패링 스킬 ID 가져오기
	FName ParryActionID = NAME_None;
	if (UWeaponData* CurrentWeapon = WeaponSystemComponent->GetCurrentWeapon())
	{
		ParryActionID = CurrentWeapon->ParrySkillActionID; // 예: Fenrir_Parry
	}

	// 4. 패링 스킬 즉시 실행
	if (!ParryActionID.IsNone())
	{
		// 기존처럼 태스크를 수동으로 Inject하지 않고, 일반적인 액션 실행 함수를 사용합니다.
		// 이 액션(BP_Action_Fenrir_Parry) 안에 '시퀀스 재생(Task_PlayLevelSequence)'과 '데미지 처리'가 모두 들어있습니다.
		ActionComponent->StartActionByID(OwningPlayerCharacter, ParryActionID, { ParriedAttacker });
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Parry Success but no ParrySkillActionID found for current weapon!"));
	}

	// 5. 패링 성공 이벤트 방송 (UI 표시 등)
	if (OwningPlayerCharacter->GetGameEventComponent())
	{
		OwningPlayerCharacter->GetGameEventComponent()->BroadcastParryAttempted(ParriedAttacker, OwningPlayerCharacter, EParryResult::Success);
	}
}

void UPlayerCombatControlComponent::OnParryFailure(ACombatPawn* ParriedAttacker, EParryResult Result)
{
	if (!OwningPlayerCharacter || !ParriedAttacker) return;
	UE_LOG(LogTemp, Warning, TEXT("Parry Failed / Partial Success vs %s"), *ParriedAttacker->GetName());
	// 적 행동은 계속됨. 부분 성공 시 데미지 감소 등의 로직 추가 가능.

	// 패링 실패 이벤트 방송
	if (OwningPlayerCharacter->GetGameEventComponent())
	{
		OwningPlayerCharacter->GetGameEventComponent()->BroadcastParryAttempted(ParriedAttacker, OwningPlayerCharacter, Result);
	}
}

void UPlayerCombatControlComponent::ConfirmAndExecuteAction()
{
	if (!OwningPlayerCharacter || OwningPlayerCharacter->GetCombatPawnState() != ECombatPawnState::AwaitingInput || SelectedActionID.IsNone()) return;
	if (CurrentTargets.IsEmpty() || !CurrentTargets[0]) return;

	if (ActionComponent && ActionComponent->StartActionByID(OwningPlayerCharacter, SelectedActionID, CurrentTargets))
	{
		OwningPlayerCharacter->SetCombatPawnState(ECombatPawnState::PerformingAction);
	}
}

void UPlayerCombatControlComponent::BeginTargetSelection()
{
	if (AllEnemyTargets.Num() == 0 || !ActionComponent) return;
	FActionData ActionData;
	if (!ActionComponent->GetActionData(SelectedActionID, ActionData)) return;

	const int32 NumberOfTargets = FMath::Min(ActionData.NumberOfTargets, AllEnemyTargets.Num());

	CurrentTargetIndex = 0;

	TArray<ACombatPawn*> NewTargets;
	for (int32 i = 0; i < NumberOfTargets; ++i)
	{
		int32 TargetIndex = (CurrentTargetIndex + i) % AllEnemyTargets.Num();
		NewTargets.Add(AllEnemyTargets[TargetIndex]);
	}

	SetCurrentTargets(NewTargets);
}

void UPlayerCombatControlComponent::CycleTarget(float Direction)
{
	UE_LOG(LogTemp, Warning, TEXT("CycleTarget called with Direction: %f"), Direction);
	if (!OwningPlayerCharacter || OwningPlayerCharacter->GetCombatPawnState() != ECombatPawnState::AwaitingInput || AllEnemyTargets.Num() <= 1 || !ActionComponent) return;

	// 인덱스 이동
	CurrentTargetIndex += FMath::RoundToInt(Direction);

	// 인덱스가 배열 범위를 벗어나면 순환시킴 (음수 처리 포함)
	const int32 NumEnemies = AllEnemyTargets.Num();
	CurrentTargetIndex = (CurrentTargetIndex % NumEnemies + NumEnemies) % NumEnemies;

	// 새 앵커 인덱스를 기준으로 타겟 그룹 재구성
	FActionData ActionData;
	if (!ActionComponent->GetActionData(SelectedActionID, ActionData)) return;

	const int32 NumberOfTargets = FMath::Min(ActionData.NumberOfTargets, NumEnemies);

	TArray<ACombatPawn*> NewTargets;
	for (int32 i = 0; i < NumberOfTargets; ++i)
	{
		int32 TargetIndex = (CurrentTargetIndex + i) % NumEnemies;
		NewTargets.Add(AllEnemyTargets[TargetIndex]);
	}

	SetCurrentTargets(NewTargets);
}

void UPlayerCombatControlComponent::SelectTargetByMouse()
{
	if (!OwningPlayerCharacter || OwningPlayerCharacter->GetCombatPawnState() != ECombatPawnState::AwaitingInput || !ActionComponent) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Pawn, false, HitResult))
	{
		if (ACombatPawn* HitPawn = Cast<ACombatPawn>(HitResult.GetActor()))
		{
			int32 FoundIndex;
			if (AllEnemyTargets.Find(HitPawn, FoundIndex))
			{
				// 클릭된 적을 새로운 앵커 인덱스로 설정
				CurrentTargetIndex = FoundIndex;

				// 새 앵커 인덱스를 기준으로 타겟 그룹 재구성
				FActionData ActionData;
				if (!ActionComponent->GetActionData(SelectedActionID, ActionData)) return;

				const int32 NumEnemies = AllEnemyTargets.Num();
				const int32 NumberOfTargets = FMath::Min(ActionData.NumberOfTargets, NumEnemies);

				TArray<ACombatPawn*> NewTargets;
				for (int32 i = 0; i < NumberOfTargets; ++i)
				{
					int32 TargetIndex = (CurrentTargetIndex + i) % NumEnemies;
					NewTargets.Add(AllEnemyTargets[TargetIndex]);
				}

				SetCurrentTargets(NewTargets);
			}
		}
	}
}


void UPlayerCombatControlComponent::SetCurrentTargets(const TArray<ACombatPawn*>& NewTargets)
{
	CurrentTargets = NewTargets;
	// "타겟 변경됨!" 방송
	OnTargetsChanged.Broadcast(CurrentTargets);
}
