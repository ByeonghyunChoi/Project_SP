#include "GA/Relic/GA_Relic_SeedOfDisaster.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

UGA_Relic_SeedOfDisaster::UGA_Relic_SeedOfDisaster()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_SeedOfDisaster::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
    }
}

void UGA_Relic_SeedOfDisaster::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FGameplayTag::RequestGameplayTag(FName("Event.Combat.AttackHit")),
        nullptr, false, false);

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_SeedOfDisaster::OnNormalAttackHitReceived);
    WaitEventTask->ReadyForActivation();

    UAbilityTask_WaitGameplayEvent* WaitEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, 
        FSPGameplayTags::Get().Event_Battle_End, 
        nullptr, false, false);

    WaitEndTask->EventReceived.AddDynamic(this, &UGA_Relic_SeedOfDisaster::OnBattleEndReceived);
    WaitEndTask->ReadyForActivation();
}

void UGA_Relic_SeedOfDisaster::OnNormalAttackHitReceived(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ASC && SeedStackGEClass)
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SeedStackGEClass, 1.0f, Context);

        if (SpecHandle.IsValid())
        {
            // 몸에 버프 씌우기 (엔진이 5스택까지만 알아서 컷해줍니다)
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            int32 CurrentStack = ASC->GetGameplayEffectCount(SeedStackGEClass, ASC);
            UE_LOG(LogTemp, Warning, TEXT("🌱 [재앙의 씨앗] 평타 적중! 효과 명중률 버프 (현재 스택: %d / 5)"), CurrentStack);
        }
    }
}

void UGA_Relic_SeedOfDisaster::OnBattleEndReceived(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ASC && SeedStackGEClass)
    {
        // 🌟 핵심: 내가 부여했던 '이 클래스(SeedStackGEClass)'의 GE를 몸에서 전부 제거합니다.
        // GE가 제거되는 순간 엔진이 알아서 EffectHitRate를 원래대로 깎아버립니다!
        ASC->RemoveActiveGameplayEffectBySourceEffect(SeedStackGEClass, ASC);

        UE_LOG(LogTemp, Warning, TEXT("🌱 [재앙의 씨앗] 전투 종료! 명중률 스택이 0으로 초기화되었습니다."));
    }
}