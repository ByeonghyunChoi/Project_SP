// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PlayerCombatControlComponent.h"
#include "Character/PlayerCharacter.h"
#include "Component/ActionComponent.h"
#include "Component/WeaponSystemComponent.h"
#include "Data/ActionData.h"
#include "Data/WeaponData.h"
#include "EnhancedInputComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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

void UPlayerCombatControlComponent::HandleChangeWeapon(int32 WeaponIndex)
{
	if (!WeaponSystemComponent) return;

	EDamageType SelectedType = static_cast<EDamageType>(WeaponIndex - 1);

	EDamageType OldWeaponType = EDamageType::Fenrir;
	bool bIsAlreadyEquipped = false;
	if (WeaponSystemComponent->GetCurrentWeapon())
	{
		OldWeaponType = WeaponSystemComponent->GetCurrentWeapon()->WeaponType;
		if (OldWeaponType == SelectedType)
		{
			bIsAlreadyEquipped = true;
		}
	}
	if (bIsAlreadyEquipped)
	{
		return;
	}

	WeaponSystemComponent->SwitchWeapon(SelectedType);
	UWeaponData* NewWeaponData = WeaponSystemComponent->GetCurrentWeapon();
	if (NewWeaponData)
	{
		const FName BasicAttackID = NewWeaponData->BasicAttackActionID;
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
