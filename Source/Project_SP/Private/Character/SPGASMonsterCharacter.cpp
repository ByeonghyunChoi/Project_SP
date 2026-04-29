#include "Character/SPGASMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Game/ASPCombatGameMode.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"




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

	FocusSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FocusSpringArm"));
	FocusSpringArm->SetupAttachment(GetRootComponent());
	FocusSpringArm->TargetArmLength = 400.0f; // 카메라 기본 거리 (에디터에서 수정 가능)
	FocusSpringArm->bDoCollisionTest = false; // 카메라가 벽에 부딪혀서 앞으로 당겨지는 현상 방지
	// 몬스터를 살짝 올려다보거나 내려다보는 각도 기본값 (원하는 대로 수정 가능)
	FocusSpringArm->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	FocusSpringArm->SetRelativeLocation(FVector(-100.0f, 0.0f, 120.0f));

	FocusCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FocusCamera"));
	FocusCamera->SetupAttachment(FocusSpringArm, USpringArmComponent::SocketName);

	// 이 카메라는 평소에는 꺼져있다가(비활성화), SetViewTargetWithBlend가 호출될 때만 사용됩니다.
	// 활성화 상태를 false로 두어 평소 게임 플레이 카메라와 충돌하지 않게 합니다.
	FocusCamera->bAutoActivate = false;
}

void ASPGASMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		GiveDefaultAbilities();

		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ASPGASMonsterCharacter::OnHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &ASPGASMonsterCharacter::OnMaxHealthChanged);

		if (USPGASAttributeSet* SPAS = Cast<USPGASAttributeSet>(AttributeSet))
		{
			SPAS->OnDamageTakenEvent.AddUObject(this, &ASPGASCharacterBase::BroadcastDamageText);
		}

		ApplyMonsterData();
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
	OnMonsterWeaknessInitialized.Broadcast(GetCurrentWeaknessTags());
}

void ASPGASMonsterCharacter::Destroyed()
{
	Super::Destroyed();

	if (UWorld* World = GetWorld())
	{
		if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(World->GetAuthGameMode()))
		{
			// 3. UI 갱신 지시! (이제 턴 타임라인에서 빈자리나 찌꺼기가 완벽하게 정리됩니다)
			GM->RefreshTurnTimelineUI();

			UE_LOG(LogTemp, Warning, TEXT("[%s] 턴 UI를 갱신합니다."), *GetName());
		}
	}
}


void ASPGASMonsterCharacter::ApplyMonsterData()
{
	if (!MonsterDataAsset || !ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] MonsterDataAsset이 비어있습니다! 블루프린트에 데이터를 꽂아주세요!"), *GetName());
		return;
	}

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute(), CurrentLevel);

	float ScaledMaxHealth = MonsterDataAsset->BaseStats.MaxHealth.GetValueAtLevel(CurrentLevel);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute(), ScaledMaxHealth);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), ScaledMaxHealth);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute(), MonsterDataAsset->BaseStats.Attack.GetValueAtLevel(CurrentLevel));
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute(), MonsterDataAsset->BaseStats.Defense.GetValueAtLevel(CurrentLevel));
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute(), MonsterDataAsset->BaseStats.Speed.GetValueAtLevel(CurrentLevel));

	FGameplayTagContainer FinalWeaknessTags = GetCurrentWeaknessTags();

	if (FinalWeaknessTags.Num() > 0)
	{
		ASC->AddLooseGameplayTags(FinalWeaknessTags);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] 데이터베이스 스탯 & 약점 적용 완료!"), *MonsterDataAsset->MonsterName.ToString());
}

void ASPGASMonsterCharacter::SetSelectedWidget(bool bSelected, bool bIsPrimaryMarker, bool bShowOnHubUI)
{
	if (TargetIndicatorWidget)
	{
		TargetIndicatorWidget->SetVisibility(bSelected);
	}
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(bSelected);
	}
	// 블루프린트로 3개의 값을 전달!
	OnTargetStateChanged(bSelected, bIsPrimaryMarker, bShowOnHubUI);
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
		Die(); // 몬스터 사망 함수 호출

		if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->OnCharacterDied(this);
		}
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

void ASPGASMonsterCharacter::GiveDefaultAbilities()
{
	// ASC가 없거나, 서버 권한이 없으면(싱글이면 항상 통과) 무시
	if (!ASC || !HasAuthority()) return;

	// 배열에 등록된 모든 GA를 ASC에 꽂아줍니다!
	for (TSubclassOf<UGameplayAbility> Ability : DefaultAbilities)
	{
		if (Ability)
		{
			// 레벨 1, 입력 키 없음(INDEX_NONE), 주체는 나 자신(this)으로 스킬을 부여합니다.
			ASC->GiveAbility(FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this));
		}
	}
}

void ASPGASMonsterCharacter::BroadcastStatusUI()
{
	OnMonsterStatusChanged.Broadcast();
}

void ASPGASMonsterCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	UE_LOG(LogTemp, Warning, TEXT("[%s] 사망했습니다!"), *GetName());

	// UI 끄기
	if (StatusWidgetComponent) StatusWidgetComponent->SetVisibility(false);
	if (TargetIndicatorWidget) TargetIndicatorWidget->SetVisibility(false);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // 길막 방지
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 카메라 가림 방지
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // 마우스 클릭 타겟팅 무시

	TimeOfDeath = GetWorld()->GetTimeSeconds();

	if (ASC)
	{
		ASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Death);
		ASC->CancelAllAbilities();
	}
}

void ASPGASMonsterCharacter::ExecuteVisualDeath()
{
	if (bDeathMontagePlayed) return;
	bDeathMontagePlayed = true;

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	float DeathDuration = DeathMontage ? DeathMontage->GetPlayLength() : 0.0f;
	SetLifeSpan(DeathDuration + 0.5f);
}

float ASPGASMonsterCharacter::GetDeathMontageDuration() const
{
	if (DeathMontage)
	{
		return DeathMontage->GetPlayLength();
	}
	return 0.0f;
}

FGameplayTagContainer ASPGASMonsterCharacter::GetCurrentWeaknessTags() const
{
	if (bHasWeaknessOverride)
	{
		return OverriddenWeaknessTags;
	}
	else if (MonsterDataAsset)
	{
		return MonsterDataAsset->WeaknessTags;
	}

	return FGameplayTagContainer();
}


