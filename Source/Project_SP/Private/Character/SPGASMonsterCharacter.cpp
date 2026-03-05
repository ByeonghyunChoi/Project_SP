#include "Character/SPGASMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Components/WidgetComponent.h"




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

void ASPGASMonsterCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	BroadcastHPUI();
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


