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

void ASPGASCharacterBase::BroadcastDamageText(float DamageAmount, bool bIsCritical)
{
	OnDamageTaken.Broadcast(DamageAmount, bIsCritical);
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

	// 쿨타임 태그 찾기
	FGameplayTagContainer CooldownTagContainer;
	CooldownTagContainer.AddTag(FSPGameplayTags::Get().State_Cooldown);

	FGameplayEffectQuery Query;
	Query.MakeQuery_MatchAnyOwningTags(CooldownTagContainer);

	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	// 반복문으로 검사
	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		// 이펙트에 붙은 모든 태그를 가져옵니다.
		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);

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

void ASPGASCharacterBase::ReportReadyToGameMode()
{
	if (AASPCombatGameMode* CombatGM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		CombatGM->ReportCharacterReady(this);
		UE_LOG(LogTemp, Log, TEXT("[%s] 전투 준비 완료 "), *GetName());
	}
}

void ASPGASCharacterBase::OnBattleStarted()
{
	// 추후 공통된 작업을 여기서 실행 
}
