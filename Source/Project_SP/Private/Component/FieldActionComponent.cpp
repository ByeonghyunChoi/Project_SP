// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/FieldActionComponent.h"
#include "Character/PlayerCharacter.h"
#include "Character/MonsterCharacter.h"
#include "Combat/MonsterGroupObject.h"
#include "Combat/BattleTransitionManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/InteractableInterface.h"

// Sets default values for this component's properties
UFieldActionComponent::UFieldActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UFieldActionComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningPlayer = Cast<APlayerCharacter>(GetOwner());

	// --- 디버깅 로그 추가 ---
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		// UBattleTransitionManager 타입으로 서브시스템을 요청합니다.
		UBattleTransitionManager* TransitionManager = GameInstance->GetSubsystem<UBattleTransitionManager>();
		if (TransitionManager)
		{
			// 실제로 생성된 인스턴스의 정확한 클래스 이름을 출력합니다.
			FString ClassName = TransitionManager->GetClass()->GetName();
			UE_LOG(LogTemp, Error, TEXT("Currently active BattleTransitionManager class is: %s"), *ClassName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BattleTransitionManager subsystem NOT FOUND!"));
		}
	}
	// --- 여기까지 ---
}

void UFieldActionComponent::StartAttackSequence()
{
	if (!OwningPlayer || bIsAttacking) return;

	bIsAttacking = true;
	bIsMoving = false;
	OwningPlayer->GetCharacterMovement()->bOrientRotationToMovement = false;

    APlayerController* PlayerController = OwningPlayer->GetController<APlayerController>();
    if (PlayerController && OwningPlayer)
    {
        FHitResult HitResult;
        if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
        {
            FVector MouseLocation = HitResult.Location;
            FVector CharacterLocation = OwningPlayer->GetActorLocation();
            FVector DirectionToMouse = MouseLocation - CharacterLocation;
            DirectionToMouse.Z = 0.0f;
            DirectionToMouse.Normalize();
            FRotator TargetRotation = DirectionToMouse.Rotation();
            OwningPlayer->SetActorRotation(TargetRotation);
        }
    }
}

void UFieldActionComponent::PerformAttackHitCheck()
{
	if (!OwningPlayer) return;

	FVector StartLocation = OwningPlayer->GetActorLocation();
	FVector EndLocation = StartLocation + OwningPlayer->GetActorForwardVector() * AttackRange;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningPlayer);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, Params);

	if (bHit && HitResult.GetActor())
	{
		AMonsterCharacter* HitMonster = Cast<AMonsterCharacter>(HitResult.GetActor());
		if (HitMonster)
		{
			if (HitMonster->GetCombatMonsterGroup())
			{
				if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
				{
					UBattleTransitionManager* TransitionManager = GameInstance->GetSubsystem<UBattleTransitionManager>();
					if (TransitionManager)
					{
						UE_LOG(LogTemp, Error, TEXT("[FLOW 1] Attack successful. Requesting battle transition..."));
						TransitionManager->RequestEnterBattle(OwningPlayer, HitMonster->GetCombatMonsterGroup());
					}
				}
			}
		}
	}
}

void UFieldActionComponent::OnAttackAnimationFinished()
{
    if (OwningPlayer)
    {
        OwningPlayer->GetCharacterMovement()->bOrientRotationToMovement = true;
        bIsAttacking = false; 
        bIsMoving = true;
    }
}

void UFieldActionComponent::PerformInteraction(IInteractableInterface* Target)
{
	if (Target == nullptr)
	{
		return;
	}

	if (OwningPlayer == nullptr)
	{
		return;
	}

	Target->ExecuteInteraction(OwningPlayer);
}


