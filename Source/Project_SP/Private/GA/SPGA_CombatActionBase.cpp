// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_CombatActionBase.h"
#include "Character/SPGASCharacterBase.h"

USPGA_CombatActionBase::USPGA_CombatActionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USPGA_CombatActionBase::CompleteBattleAction()
{
	if (bBattleActionCompleted)
	{
		return;
	}

	bBattleActionCompleted = true;

	ASPGASCharacterBase* Character = Cast<ASPGASCharacterBase>(GetAvatarActorFromActorInfo());

	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		true,
		false
	);

	if (Character)
	{
		Character->NotifyBattleActionFinished();
	}
}

void USPGA_CombatActionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bBattleActionCompleted = false;

	PrepareBattleAction(TriggerEventData);

	if (!ValidateBattleAction())
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true);

		return;
	}

	if (!CommitBattleAction(
		Handle,
		ActorInfo,
		ActivationInfo))
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);

		return;
	}

	SetupActionEventListeners();
	ExecuteBattleAction();
}

void USPGA_CombatActionBase::PrepareBattleAction(const FGameplayEventData* TriggerEventData)
{
}

bool USPGA_CombatActionBase::CommitBattleAction(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	return CommitAbility(
		Handle,
		ActorInfo,
		ActivationInfo);
}

void USPGA_CombatActionBase::SetupActionEventListeners()
{
}

bool USPGA_CombatActionBase::ValidateBattleAction() const
{
	return true;
}

