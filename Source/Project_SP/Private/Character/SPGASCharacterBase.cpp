// CombatPawn.cpp

#include "Character/SPGASCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Game/ASPCombatGameMode.h"
#include "Tag/SPGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Component/SPStatusEffectComponent.h"
#include "Manager/SPCombatTurnManager.h"



ASPGASCharacterBase::ASPGASCharacterBase()
{
    ASC = nullptr;
    AttributeSet = nullptr;
	StatusEffectComponent = CreateDefaultSubobject<USPStatusEffectComponent>(TEXT("StatusEffectComponent"));
}

void ASPGASCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetSpeedAttribute())
			.AddUObject(this, &ASPGASCharacterBase::OnSpeedChanged);

		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetActionGaugeAttribute())
			.AddUObject(this, &ASPGASCharacterBase::OnActionGaugeChanged);
	}
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

	// 1. 쿨타임 태그 세팅
	FGameplayTagContainer CooldownTagContainer;
	CooldownTagContainer.AddTag(FSPGameplayTags::Get().State_Cooldown);

	// 🌟 [수정됨] Static 함수의 반환값을 직접 받아서 Query를 생성합니다!
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTagContainer);

	// 이제 쿨타임 태그를 가진 GE들만 정확하게 가져옵니다.
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	// 2. 반복문으로 검사 (이제 장비 스탯은 여기 들어오지도 못합니다!)
	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		int32 CurrentStack = ASC->GetCurrentStackCount(Handle);

		// 쿨타임 1턴(1스택) 깎기!
		ASC->RemoveActiveGameplayEffect(Handle, 1);

		// 로그 출력용 태그 추출
		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);

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

void ASPGASCharacterBase::PlayHitReact(const FVector& ImpactPoint)
{

	if (HitReactMontage && GetMesh()->GetAnimInstance())
	{
		PlayAnimMontage(HitReactMontage);
	}
}

void ASPGASCharacterBase::OnBattleStarted()
{
	// 공통된 작업을 여기서 실행 
}

void ASPGASCharacterBase::AddLooseTagToASC(FGameplayTag TagToAdd)
{
	if (ASC && TagToAdd.IsValid())
	{
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

void ASPGASCharacterBase::RemoveLooseTagFromASC(FGameplayTag TagToRemove)
{
	if (ASC && TagToRemove.IsValid())
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

void ASPGASCharacterBase::CancelAbilitiesWithTag(FGameplayTagContainer WithTags)
{
	if (ASC)
	{
		ASC->CancelAbilities(&WithTags, nullptr, nullptr);
	}
}

void ASPGASCharacterBase::OnSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (ASPCombatTurnManager* TM = GM->GetTurnManager())
		{
			TM->OnTurnOrderChanged.Broadcast();
		}
	}
}

void ASPGASCharacterBase::OnActionGaugeChanged(const FOnAttributeChangeData& Data)
{
	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (ASPCombatTurnManager* TM = GM->GetTurnManager())
		{
			TM->OnTurnOrderChanged.Broadcast();
		}
	}
}
