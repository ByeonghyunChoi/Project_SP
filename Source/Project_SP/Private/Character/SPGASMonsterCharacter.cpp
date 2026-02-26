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
}

void ASPGASMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
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

void ASPGASMonsterCharacter::SetSelectedWidget(bool bSelected)
{
	if (TargetIndicatorWidget)
	{
		TargetIndicatorWidget->SetVisibility(bSelected);
	}
	//외곽선
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(bSelected);
	}
}


