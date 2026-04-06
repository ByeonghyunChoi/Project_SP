// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/Relic/GA_Relic_BrokenWatch.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

UGA_Relic_BrokenWatch::UGA_Relic_BrokenWatch()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_BrokenWatch::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
    }
}

void UGA_Relic_BrokenWatch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 🌟 패링 성공 무전을 대기합니다. (이미 구부러진 쇳덩이 때 발사기 설치 완료!)
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Combat_ParrySuccess,
        nullptr, false, false);

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_BrokenWatch::OnParrySuccessReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_BrokenWatch::OnParrySuccessReceived(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    // 무전을 받으면 내 몸에 HealGEClass (체력 회복 약)을 주사합니다!
    if (ASC && HealGEClass)
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealGEClass, 1.0f, Context);

        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("[유물 발동] 깨진 회중시계! 패링 성공으로 체력을 회복합니다."));
        }
    }
}