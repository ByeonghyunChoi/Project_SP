#include "GA/Relic/GA_Relic_AddGold.h"
#include "AbilitySystemComponent.h"
#include "Component/InventoryComponent.h"

UGA_Relic_AddGold::UGA_Relic_AddGold()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Relic_AddGold::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        // 🌟 플레이어의 몸에서 '인벤토리 컴포넌트'를 찾습니다.
        if (UInventoryComponent* InventoryComp = ActorInfo->AvatarActor->FindComponentByClass<UInventoryComponent>())
        {
            // 🌟 100 골드(Money) 지급!
            InventoryComp->AddMoney(GoldAmountToAdd);
            UE_LOG(LogTemp, Warning, TEXT("100 골드 유물 획득! %d 골드가 즉시 지급되었습니다. (현재 골드: %d)"), GoldAmountToAdd, InventoryComp->GetMoney());
        }

        // 일회성 효과이므로 어빌리티 즉시 종료
        EndAbility(Spec.Handle, ActorInfo, Spec.ActivationInfo, true, false);
    }
}