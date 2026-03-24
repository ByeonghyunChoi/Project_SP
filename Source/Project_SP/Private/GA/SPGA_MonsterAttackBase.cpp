// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_MonsterAttackBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

USPGA_MonsterAttackBase::USPGA_MonsterAttackBase()
{
	// 이벤트 리스너(Task)를 쓰려면 무조건 InstancedPerActor 여야 합니다!
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USPGA_MonsterAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 🌟 어빌리티가 시작되자마자 '패링 무전'을 기다리는 잠복 근무(Task)를 시작합니다!
	UAbilityTask_WaitGameplayEvent* WaitParriedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FSPGameplayTags::Get().Event_Battle_Parried
	);

	if (WaitParriedTask)
	{
		// 무전이 들어오면 OnParriedEventCallback 함수를 실행하라고 바인딩합니다.
		WaitParriedTask->EventReceived.AddDynamic(this, &USPGA_MonsterAttackBase::OnParriedEventCallback);

		// 잠복 근무 시작!
		WaitParriedTask->ReadyForActivation();
	}
}

void USPGA_MonsterAttackBase::OnParriedEventCallback(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[Monster GA] 패링 무전 수신! 하던 공격을 멈추고 복귀합니다."));

	// 블루프린트에서 구현한 연출(튕겨 나가기, 제자리 복귀 등)을 실행하도록 이벤트 호출!
	OnParried();
}
