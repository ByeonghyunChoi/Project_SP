// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/Relic/GA_Relic_OnHitStatusBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Component/SPStatusEffectComponent.h"

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
        FSPGameplayTags::Get().Event_Combat_AttackHit, // 우리가 등록한 타격 태그
        nullptr,
        false,
        false
    );

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_OnHitStatusBase::OnHitEventReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_OnHitStatusBase::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);

    // 부여받은 즉시, 자신의 ASC에게 "나를 활성화해줘!" 라고 요청합니다.
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
        UE_LOG(LogTemp, Log, TEXT("유물 자동 활성화 완료: 대기 모드 진입"));
    }
}

void UGA_Relic_OnHitStatusBase::OnHitEventReceived(FGameplayEventData Payload)
{
    // 1. 일반 공격(Battle.Action.Attack)인지 확인 (방금 우리가 추가한 그 증명서!)
    if (!Payload.InstigatorTags.HasTagExact(FSPGameplayTags::Get().Battle_Action_Attack))
    {
        return;
    }

    // 2. 확률 주사위 굴리기
    float RandValue = FMath::FRandRange(0.0f, 100.0f);
    if (RandValue <= TriggerChance)
    {
        AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

        if (TargetASC && TargetStatusGEClass)
        {
            FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
            EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

            FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(TargetStatusGEClass, GetAbilityLevel(), EffectContext);

            if (SpecHandle.IsValid())
            {
                // [1] 엔진(혈관)에 직접 GE 부여
                TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

                // [2] 몬스터 몸에서 '전광판 매니저' 찾기
                USPStatusEffectComponent* TargetStatusComp = TargetActor->FindComponentByClass<USPStatusEffectComponent>();

                // [3] 매니저가 있고, 에디터에서 태그를 제대로 세팅해 뒀다면 보고!
                if (TargetStatusComp && TargetStatusTag.IsValid())
                {
                    TargetStatusComp->ProcessStatusEffect(TargetStatusTag, TargetASC, GetAvatarActorFromActorInfo());
                }

                UE_LOG(LogTemp, Log, TEXT("기초 유물 효과 발동: [%s] 부여 및 매니저 등록 완료!"), *TargetStatusTag.ToString());
            }
        }
    }
}