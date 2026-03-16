// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SPGASAnimInstance.h"
#include "Character/SPGASCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Tag/SPGameplayTags.h"

USPGASAnimInstance::USPGASAnimInstance()
{
	MovingThreshold = 3.0f;
}

void USPGASAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<ASPGASCharacterBase>(TryGetPawnOwner());
	if (Character)
	{
		Movement = Character->GetCharacterMovement();
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Character))
		{
			CachedASC = ASI->GetAbilitySystemComponent();
		}
	}
}

void USPGASAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Movement)
	{
		Velocity = Movement->Velocity;
		GroundSpeed = Velocity.Size2D();
		bIsIdle = GroundSpeed < MovingThreshold;
		bIsFalling = Movement->IsFalling();
	}

	if (CachedASC.IsValid())
	{
		const FSPGameplayTags& SPTags = FSPGameplayTags::Get();
		bIsInBattle = CachedASC->HasMatchingGameplayTag(SPTags.State_Mode_Battle);
		bIsMyTurn = CachedASC->HasMatchingGameplayTag(SPTags.State_Battle_TurnActive);
		bIsDead = CachedASC->HasMatchingGameplayTag(SPTags.State_Death);
	}
}
