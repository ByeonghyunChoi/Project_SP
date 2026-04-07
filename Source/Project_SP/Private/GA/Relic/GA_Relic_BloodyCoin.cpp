// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/Relic/GA_Relic_BloodyCoin.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

UGA_Relic_BloodyCoin::UGA_Relic_BloodyCoin()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_BloodyCoin::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
    }
}

void UGA_Relic_BloodyCoin::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    //  무기 스킬 처치(Event.Combat.SkillKill) 무전을 기다립니다.
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Combat_SkillKill,
        nullptr, false, false);

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_BloodyCoin::OnSkillKillReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_BloodyCoin::OnSkillKillReceived(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    // 무전을 받으면 내 몸에 BP 회복 약을 주사합니다!
    if (ASC && AddBPGEClass)
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AddBPGEClass, 1.0f, Context);

        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("[유물 발동] 피 묻은 동전! 무기 스킬로 적을 처치하여 BP를 1 회복합니다."));
        }
    }
}