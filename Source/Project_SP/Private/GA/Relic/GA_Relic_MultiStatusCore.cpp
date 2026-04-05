// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/Relic/GA_Relic_MultiStatusCore.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"

UGA_Relic_MultiStatusCore::UGA_Relic_MultiStatusCore()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_MultiStatusCore::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 타격 순간을 기다립니다.
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Montage_Hit,
        nullptr,
        false,
        false
    );

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_MultiStatusCore::OnHitEventReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_MultiStatusCore::OnHitEventReceived(FGameplayEventData Payload)
{
    // 1. 일반 공격(Battle.Action.Attack)인지 확인
    if (!Payload.InstigatorTags.HasTagExact(FSPGameplayTags::Get().Battle_Action_Attack))
    {
        return;
    }

    // 2. 타겟의 ASC 가져오기 (const_cast 적용)
    AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

    // 타겟이 없거나, 에디터에서 Map을 안 채워놨으면 중단
    if (!TargetASC || StatusEffectMap.IsEmpty()) return;

    // 3. 타겟에게 "없는" 상태이상 분류하기
    TArray<FGameplayTag> MissingTags;
    TArray<FGameplayTag> AllTags;

    for (const auto& Pair : StatusEffectMap)
    {
        FGameplayTag StatusTag = Pair.Key;
        AllTags.Add(StatusTag); // 전체 목록 저장

        // 타겟이 이 상태이상 태그를 가지고 있지 않다면 MissingTags에 추가
        if (!TargetASC->HasMatchingGameplayTag(StatusTag))
        {
            MissingTags.Add(StatusTag);
        }
    }

    // 4. 부여할 상태이상 무작위 선정
    FGameplayTag TagToApply;
    if (MissingTags.Num() > 0)
    {
        // 안 걸린 게 하나라도 있으면, 그 '없는 것들' 중에서 무작위 선정
        int32 RandomIndex = FMath::RandRange(0, MissingTags.Num() - 1);
        TagToApply = MissingTags[RandomIndex];
    }
    else
    {
        // 3개 다 걸려있으면 전체 중에서 무작위 선정 (갱신용)
        int32 RandomIndex = FMath::RandRange(0, AllTags.Num() - 1);
        TagToApply = AllTags[RandomIndex];
    }

    // 5. 선정된 GE 적용
    TSubclassOf<UGameplayEffect> GEToApply = StatusEffectMap[TagToApply];
    if (GEToApply)
    {
        FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
        EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

        FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(GEToApply, GetAbilityLevel(), EffectContext);
        if (SpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Log, TEXT("오색 마력핵 발동! 적에게 [%s] 부여 성공!"), *TagToApply.ToString());
        }
    }
}
