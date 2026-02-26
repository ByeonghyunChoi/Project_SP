// CombatPawn.cpp

#include "Character/SPGASCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Game/ASPCombatGameMode.h"
#include "Tag/SPGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Component/SPStatusEffectComponent.h"


ASPGASCharacterBase::ASPGASCharacterBase()
{
    ASC = nullptr;
    AttributeSet = nullptr;
	StatusEffectComponent = CreateDefaultSubobject<USPStatusEffectComponent>(TEXT("StatusEffectComponent"));
}

UAbilitySystemComponent* ASPGASCharacterBase::GetAbilitySystemComponent() const
{
    return ASC;
}

void ASPGASCharacterBase::FinishTurn()
{
	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->EndTurn(this);
	}
}

void ASPGASCharacterBase::ReduceCooldowns()
{
	if (!ASC) return;

	// 1. 쿨타임 태그 찾기 (State.Cooldown 태그가 있는 이펙트 검색)
	FGameplayTagContainer CooldownTagContainer;
	CooldownTagContainer.AddTag(FSPGameplayTags::Get().State_Cooldown);

	FGameplayEffectQuery Query;
	Query.MakeQuery_MatchAnyOwningTags(CooldownTagContainer);

	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	// 2. 반복문으로 검사
	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		// [중요] 이펙트에 붙은 모든 태그를 가져옵니다.
		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);

		/* 🛡️ [입국 심사] "Cooldown.Weapon" 으로 시작하는 태그가 있는가?
		 유령(1->0)은 태그가 비어있으므로 여기서 걸러집니다.
		if (!GrantedTags.HasTag(FGameplayTag::RequestGameplayTag("Cooldown.Weapon")))
		{
			 유령 발견! -> 조용히 삭제하고 무시합니다.
			ASC->RemoveActiveGameplayEffect(Handle, 1);
			UE_LOG(LogTemp, Warning, TEXT("[Ghost] 유령 쿨타임(태그 없음)이 감지되어 삭제했습니다. (무시됨)"));
			continue;
		}*/

		// --- 여기서부터는 진짜(Valid) 쿨타임만 처리 ---

		if (GrantedTags.HasTag(FGameplayTag::RequestGameplayTag("Debuff")))
		{
			continue;
		}

		int32 CurrentStack = ASC->GetCurrentStackCount(Handle);
		ASC->RemoveActiveGameplayEffect(Handle, 1);

		UE_LOG(LogTemp, Log, TEXT("[%s] 쿨타임 1턴 감소! (%d -> %d) | 태그: %s"),
			*GetName(), CurrentStack, CurrentStack - 1, *GrantedTags.ToString());
	}
}