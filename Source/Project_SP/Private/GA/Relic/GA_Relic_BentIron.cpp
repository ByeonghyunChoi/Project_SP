#pragma once

#include "GA/Relic/GA_Relic_BentIron.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

UGA_Relic_BentIron::UGA_Relic_BentIron()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_BentIron::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    // 장착 즉시 대기 모드로 진입
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
    }
}

void UGA_Relic_BentIron::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UE_LOG(LogTemp, Warning, TEXT("🎧 구부러진 쇳덩이 유물 켜짐! 패링 신호 대기 중..."));
    // 🌟 패링 성공(Event.Combat.ParrySuccess) 무전을 기다립니다.
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Combat_ParrySuccess,
        nullptr, false, false);

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_BentIron::OnParrySuccessReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_BentIron::OnParrySuccessReceived(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    // 무전을 받으면 내 몸에 AddBPGEClass (BP 회복 약)을 주사합니다!
    if (ASC && AddBPGEClass)
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AddBPGEClass, 1.0f, Context);

        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("[유물 발동] 구부러진 쇳덩이! 패링 성공으로 BP를 회복합니다."));
        }
    }
}