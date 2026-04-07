#include "GA/Relic/GA_Relic_ChaosPrism.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"
#include "Component/SPStatusEffectComponent.h"
#include "Kismet/GameplayStatics.h"

UGA_Relic_ChaosPrism::UGA_Relic_ChaosPrism()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_ChaosPrism::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle); // 자동 켜기!
    }
}

void UGA_Relic_ChaosPrism::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 타격이 아니라 "처치(Kill)" 이벤트를 기다립니다!
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        FSPGameplayTags::Get().Event_Combat_AttackKill,
        nullptr, false, false);

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Relic_ChaosPrism::OnKillEventReceived);
    WaitEventTask->ReadyForActivation();
}

void UGA_Relic_ChaosPrism::OnKillEventReceived(FGameplayEventData Payload)
{
    // 일반 공격인지 검증
    if (!Payload.InstigatorTags.HasTagExact(FSPGameplayTags::Get().Battle_Action_Attack)) return;

    AActor* DeadEnemy = const_cast<AActor*>(Payload.Target.Get());
    UAbilitySystemComponent* DeadASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(DeadEnemy);
    if (!DeadASC || StatusEffectMap.IsEmpty()) return;

    // 1. 죽은 적이 가지고 있는 상태이상(태그) 색출하기
    TArray<FGameplayTag> TagsToTransfer;
    for (const auto& Pair : StatusEffectMap)
    {
        if (DeadASC->HasMatchingGameplayTag(Pair.Key))
        {
            TagsToTransfer.Add(Pair.Key); // 걸려있던 태그들만 수집
        }
    }

    if (TagsToTransfer.IsEmpty()) return; // 뺏을 상태이상이 없으면 그냥 종료

    //  2. 살아있는 무작위 적 찾기
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), AllEnemies);
    TArray<AActor*> AliveEnemies;

    for (AActor* Enemy : AllEnemies)
    {
        if (Enemy == DeadEnemy) continue; // 방금 죽은 애는 패스

        UAbilitySystemComponent* EnemyASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
        // 사망 태그가 없는 살아있는 적만 리스트업
        if (EnemyASC && !EnemyASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Death))
        {
            AliveEnemies.Add(Enemy);
        }
    }

    if (AliveEnemies.IsEmpty()) return; // 남은 적이 한 명도 없으면(전투 종료) 전이 안 함

    // 무작위로 한 명 뽑기
    int32 RandIdx = FMath::RandRange(0, AliveEnemies.Num() - 1);
    AActor* RandomTarget = AliveEnemies[RandIdx];
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(RandomTarget);

    // 3. 수집해둔 상태이상들을 무작위 적에게 덮어씌우고 매니저에게 보고
    if (TargetASC)
    {
        USPStatusEffectComponent* TargetStatusComp = RandomTarget->FindComponentByClass<USPStatusEffectComponent>();

        for (FGameplayTag Tag : TagsToTransfer)
        {
            TSubclassOf<UGameplayEffect> GEToApply = StatusEffectMap[Tag];
            if (GEToApply)
            {
                FGameplayEffectContextHandle Context = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
                FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(GEToApply, GetAbilityLevel(), Context);

                if (SpecHandle.IsValid())
                {
                    TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

                    if (TargetStatusComp)
                    {
                        // 전광판 매니저에게 아이콘 띄우라고 지시!
                        TargetStatusComp->ProcessStatusEffect(Tag, TargetASC, GetAvatarActorFromActorInfo());
                    }
                }
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("혼돈의 프리즘 발동! 죽은 적의 상태이상 %d개를 무작위 적에게 전이했습니다."), TagsToTransfer.Num());
    }
}