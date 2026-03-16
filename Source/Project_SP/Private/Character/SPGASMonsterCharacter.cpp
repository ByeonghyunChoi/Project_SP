#include "Character/SPGASMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Game/ASPCombatGameMode.h"




ASPGASMonsterCharacter::ASPGASMonsterCharacter()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<USPGASAttributeSet>(TEXT("AttributeSet"));

	TargetIndicatorWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetIndicatorWidget"));
	TargetIndicatorWidget->SetupAttachment(GetRootComponent());
	TargetIndicatorWidget->SetWidgetSpace(EWidgetSpace::Screen);
	TargetIndicatorWidget->SetVisibility(false);

	StatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusWidgetComponent"));
	StatusWidgetComponent->SetupAttachment(GetRootComponent());
	StatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	StatusWidgetComponent->SetVisibility(false);
}

void ASPGASMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ASPGASMonsterCharacter::OnHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &ASPGASMonsterCharacter::OnMaxHealthChanged);

		if (USPGASAttributeSet* SPAS = Cast<USPGASAttributeSet>(AttributeSet))
		{
			SPAS->OnDamageTakenEvent.AddUObject(this, &ASPGASCharacterBase::BroadcastDamageText);
		}

		if (WeaknessTags.IsValid())
		{
			ASC->AddLooseGameplayTags(WeaknessTags);
			UE_LOG(LogTemp, Log, TEXT("몬스터 태그 적용됨: %s"), *WeaknessTags.ToString());
			
			if (MonsterRankTag.IsValid())
			{
				ASC->AddLooseGameplayTag(MonsterRankTag);

				UE_LOG(LogTemp, Log, TEXT("[%s] 몬스터 등급 태그 부여됨: %s"), *GetName(), *MonsterRankTag.ToString());
			}
		}
	}

	if (GetMesh())
	{
		GetMesh()->SetCustomDepthStencilValue(1); 
	}

	ReportReadyToGameMode();
}

void ASPGASMonsterCharacter::OnBattleStarted()
{
	Super::OnBattleStarted();

	if (StatusWidgetComponent)
	{
		StatusWidgetComponent->SetVisibility(true);
	}

	// 전투 시작 시 현재 체력과 약점 정보를 방송해줍니다!
	BroadcastHPUI();
	OnMonsterWeaknessInitialized.Broadcast(WeaknessTags);
}

void ASPGASMonsterCharacter::InitializeEnemyStats(int32 NewLevel, float StatMultiplier)
{
	USPGASAttributeSet* AS = GetAttributeSet();
	if (!AS) return;

	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->SetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute(), (float)NewLevel);
	}

	// 체력
	float NewMaxHP = AS->GetMaxHealth() * StatMultiplier;
	GetAbilitySystemComponent()->SetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute(), NewMaxHP);
	GetAbilitySystemComponent()->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), NewMaxHP);

	// 공격력
	float NewAttack = AS->GetAttack() * StatMultiplier;
	GetAbilitySystemComponent()->SetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute(), NewAttack);

	// 방어력
	float NewDefense = AS->GetDefense() * StatMultiplier;
	GetAbilitySystemComponent()->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute(), NewDefense);
}

void ASPGASMonsterCharacter::SetSelectedWidget(bool bSelected, bool bIsPrimary)
{
	if (TargetIndicatorWidget)
	{
		TargetIndicatorWidget->SetVisibility(bSelected);
	}

	// 외곽선 (포스트 프로세스)
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(bSelected);
	}

	//블루프린트로 상태를 넘겨서 UI 크기나 애니메이션을 직접 처리하게 합니다.
	OnTargetStateChanged(bSelected, bIsPrimary);
}

TMap<FGameplayTag, int32> ASPGASMonsterCharacter::GetActiveDebuffs() const
{
	TMap<FGameplayTag, int32> ActiveDebuffs;
	if (!ASC) return ActiveDebuffs;

	// 내 몸에 붙은 모든 버프/디버프 바구니(Active Effect)를 뒤져봅니다.
	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);

		// 태그 중에 "Debuff"로 시작하는 놈이 있다면?
		for (const FGameplayTag& Tag : GrantedTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Debuff")))
			{
				// 남은 스택(턴 수)을 가져와서 Map에 [태그 : 남은 턴] 형태로 저장!
				int32 TurnsLeft = ASC->GetCurrentStackCount(Handle);
				ActiveDebuffs.Add(Tag, TurnsLeft);
			}
		}
	}
	return ActiveDebuffs;
}

void ASPGASMonsterCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	BroadcastHPUI();
	if(Data.NewValue <= 0.0f && Data.OldValue > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 사망했습니다!"), *GetName());
		Die(); // 몬스터 사망 함수 호출
	}
}

void ASPGASMonsterCharacter::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	BroadcastHPUI();
}

void ASPGASMonsterCharacter::BroadcastHPUI()
{
	if (!ASC) return;
	float CurrentHP = ASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute());
	float MaxHP = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
	OnMonsterHPChanged.Broadcast(CurrentHP, MaxHP);
}

void ASPGASMonsterCharacter::BroadcastStatusUI()
{
	OnMonsterStatusChanged.Broadcast();
}

void ASPGASMonsterCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] 사망했습니다!"), *GetName());

	// 1. UI 끄기 (체력바, 타겟팅 마커 등 지우기)
	if (StatusWidgetComponent) StatusWidgetComponent->SetVisibility(false);
	if (TargetIndicatorWidget) TargetIndicatorWidget->SetVisibility(false);

	// 2. 콜리전 끄기 (죽은 시체를 다시 때리거나 길을 막지 않게)
	SetActorEnableCollision(false);

	// 3. 지휘관(GameMode)에게 전사 보고!
	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnCharacterDied(this);
	}

	if (ASC)
	{
		ASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Death);
		ASC->CancelAllAbilities();
	}
	// 4. (선택) 몬스터 파괴 - 나중에 죽는 애니메이션(몽타주)이 끝나면 파괴하도록 블루프린트로 빼도 됩니다.
	SetLifeSpan(1.5f);
}


