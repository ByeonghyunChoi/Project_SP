// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/OpartsAbility/GA_Oparts_CrystalSkull.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"

UGA_Oparts_CrystalSkull::UGA_Oparts_CrystalSkull()
{
	// 패시브 스킬이므로 인스턴싱 정책 설정 (내부 변수 CurrentGauge를 유지하기 위해 필요)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 활성화 시기를 부여 즉시(Passive)로 설정 (원하시는 태그로 맞추시면 됩니다)
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Oparts_CrystalSkull::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentGauge = 0.0f; // 시작할 때 게이지 0으로 초기화

	// Event.Combat.CriticalHit 가 달린 이벤트가 발생할 때까지 대기하는 태스크 생성
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Event.Combat.CriticalHit")));

	if (WaitEventTask)
	{
		// 이벤트가 발생하면 OnCriticalHitReceived 함수를 실행하도록 연결
		WaitEventTask->EventReceived.AddDynamic(this, &UGA_Oparts_CrystalSkull::OnCriticalHitReceived);
		// 태스크 실행! 이제 백그라운드에서 계속 이벤트를 기다립니다.
		WaitEventTask->ReadyForActivation();
	}
}

void UGA_Oparts_CrystalSkull::OnCriticalHitReceived(FGameplayEventData Payload)
{
	// 치명타 이벤트가 발생하면 게이지 20 증가
	float GaugeToAdd = 20.0f;

	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Oparts.CrystalSkull.Artifact2"))))
	{
		GaugeToAdd = 25.0f;
	}

	CurrentGauge += GaugeToAdd;

	// 게이지가 100에 도달했는지 체크
	if (CurrentGauge >= 100.0f)
	{
		CurrentGauge = 0.0f; // 100 도달 시 게이지 초기화

		// 다음 스킬 데미지 100% 증가 버프 적용
		if (SkillDamageBuffClass)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			if (ASC)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				ASC->ApplyGameplayEffectToSelf(SkillDamageBuffClass->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
			}
		}
	}
}