// CombatPawn.cpp

#include "Character/SPGASCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Game/ASPCombatGameMode.h"
#include "Tag/SPGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Component/SPStatusEffectComponent.h"
#include "Manager/SPCombatTurnManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"



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
	UE_LOG(LogTemp, Warning, TEXT("[%s] FinishTurn 노드 호출됨!"), *GetName());

	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode 찾음! EndTurn을 실행합니다."));
		GM->EndTurn(this);
	}
}

void ASPGASCharacterBase::NotifyBattleActionFinished()
{
	OnBattleActionFinished.Broadcast(this);
}

ETurnAvailability ASPGASCharacterBase::GetTurnAvailability() const
{
	if (!ASC)
	{
		return ETurnAvailability::Unavailable;
	}

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	const float CurrentHealth = ASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute());
	
	if (CurrentHealth <= 0.0f || ASC->HasMatchingGameplayTag(SPTags.State_Death))
	{
		return ETurnAvailability::Unavailable;
	}

	if (ASC->HasMatchingGameplayTag(SPTags.State_Status_SkipTurn))
	{
		return ETurnAvailability::SkipTurn;
	}

	return ETurnAvailability::CanAct;
}

void ASPGASCharacterBase::ReduceCooldowns()
{
	if (!ASC) return;

	// 1. Cooldown 최상위 부모 태그 준비
	FGameplayTag CooldownParentTag = FGameplayTag::RequestGameplayTag(FName("Cooldown"));

	// 2. [핵심 수정] 조건 없이 내 몸에 있는 '모든' GE를 다 가져옵니다!
	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);
		ActiveGE->Spec.GetAllAssetTags(GrantedTags);

		// 3. HasTag를 쓰면 "Cooldown.xxx.xxx" 등 자식 태그를 100% 완벽하게 찾아냅니다!
		if (GrantedTags.HasTag(CooldownParentTag))
		{
			int32 CurrentStack = ASC->GetCurrentStackCount(Handle);

			// 쿨타임 1턴(1스택) 깎기
			ASC->RemoveActiveGameplayEffect(Handle, 1);

			UE_LOG(LogTemp, Log, TEXT("[%s] 쿨타임 1턴 감소! (%d -> %d) | 태그: %s"),
				*GetName(), CurrentStack, CurrentStack - 1, *GrantedTags.ToString());
		}
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
	if (ASC && ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Death))
	{
		return;
	}

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

void ASPGASCharacterBase::HandleSkippedTurn()
{
	if (StatusEffectComponent)
	{
		StatusEffectComponent->HandleSkippedTurn();
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
