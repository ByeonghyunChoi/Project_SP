// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/Relic/GA_Relic_OnHitStatusBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_Relic_OnHitStatusBase::UGA_Relic_OnHitStatusBase()
{
    // 패시브니까 인스턴싱 정책을 '기능별 하나'로 설정
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_OnHitStatusBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 타격 이벤트가 발생할 때까지 무한 대기
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Montage_Hit, // 우리가 등록한 타격 태그
        nullptr,
        false,
        false
    );

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_OnHitStatusBase::OnHitEventReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_OnHitStatusBase::OnHitEventReceived(FGameplayEventData Payload)
{
    // 1. 일반 공격(Battle.Action.Attack)인지 확인
    if (!Payload.InstigatorTags.HasTagExact(FSPGameplayTags::Get().Battle_Action_Attack))
    {
        return;
    }

    // 2. 확률 주사위
    float RandValue = FMath::FRandRange(0.0f, 100.0f);
    if (RandValue <= TriggerChance)
    {
        // 3. 타겟(적)의 ASC를 가져와서 GE 적용 (const_cast 적용)
        AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());

        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
        {
            FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
            EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

            // 변경된 변수명 TargetStatusGEClass 사용
            FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(TargetStatusGEClass, GetAbilityLevel(), EffectContext);
            if (SpecHandle.IsValid())
            {
                TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

                // 로그도 범용적으로 변경
                UE_LOG(LogTemp, Log, TEXT("유물 효과 발동: %s 상태이상 부여 성공!"), *TargetStatusGEClass->GetName());
            }
        }
    }
}