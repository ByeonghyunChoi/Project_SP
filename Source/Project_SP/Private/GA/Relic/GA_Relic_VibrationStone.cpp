// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/Relic/GA_Relic_VibrationStone.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

UGA_Relic_VibrationStone::UGA_Relic_VibrationStone()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_VibrationStone::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 이번에는 타격이 아니라 "전투 시작(Event.Battle.Start)" 이벤트를 기다립니다!
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Battle_Start,
        nullptr,
        false,
        false
    );

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_VibrationStone::OnBattleStartReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_VibrationStone::OnBattleStartReceived(FGameplayEventData Payload)
{
    // 나 자신의 ASC(능력 시스템 컴포넌트)를 가져옵니다.
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ASC && AddBPGEClass)
    {
        // 나에게 BP 회복 GE를 적용합니다.
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AddBPGEClass, GetAbilityLevel(), Context);

        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Log, TEXT("불안정한 진동석 발동! BP 1 획득"));
        }
    }
}
