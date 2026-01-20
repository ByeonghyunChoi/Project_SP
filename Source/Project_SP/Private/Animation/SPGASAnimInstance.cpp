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

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Character))
	{
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (ASC)
		{
			const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

			// 태그 기반 상태 업데이트 예시
			/*bIsInBattle = ASC->HasMatchingGameplayTag(SPTags.State_Mode_Battle);
			bIsMyTurn = ASC->HasMatchingGameplayTag(SPTags.State_Turn_Active);
			bIsDead = ASC->HasMatchingGameplayTag(SPTags.State_Status_Dead);*/
		}
	}
}
