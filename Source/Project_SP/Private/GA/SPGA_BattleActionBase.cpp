#include "GA/SPGA_BattleActionBase.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASCharacterBase.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"
#include "Game/ASPCombatGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Component/SPStatusEffectComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Manager/SPCombatTurnManager.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"

USPGA_BattleActionBase::USPGA_BattleActionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = nullptr;
	CostGameplayEffectClass = nullptr;
}

bool USPGA_BattleActionBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// 해골 수정 + 무기 스킬
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_CrystalSkull) && AbilityTags.HasTag(SPTags.Battle_Action_Skill))
		{
			return true;
		}
		// 옥시계 + 반격
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_JadeClock) && AbilityTags.HasTag(SPTags.Battle_Action_CounterAttack))
		{
			return true;
		}
	}
	bool bCanAfford = Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);

	if (!bCanAfford)
	{
		UE_LOG(LogTemp, Warning, TEXT("[시스템] BP가 부족하여 스킬을 사용할 수 없습니다!"));
	}

	return bCanAfford;
}

bool USPGA_BattleActionBase::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return false;
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// 수정 해골: '무기 스킬(Skill)' 한정 쿨타임 프리패스
	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_CrystalSkull))
	{
		if (AbilityTags.HasTag(SPTags.Battle_Action_Skill)) return true;
	}

	// 옥시계: '반격(CounterAttack)' 한정 쿨타임 프리패스
	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_JadeClock))
	{
		if (AbilityTags.HasTag(SPTags.Battle_Action_CounterAttack)) return true;
	}

	// 쿨타임 체크 로직
	if (CooldownTag.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag))
	{
		if (OptionalRelevantTags) OptionalRelevantTags->AddTag(CooldownTag);
		return false;
	}
	return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
}

void USPGA_BattleActionBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// 해골 수정 + 무기 스킬
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_CrystalSkull) && GetAssetTags().HasTag(SPTags.Battle_Action_Skill))
		{
			return;
		}
		// 옥시계 + 반격
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_JadeClock) && GetAssetTags().HasTag(SPTags.Battle_Action_CounterAttack))
		{
			return;
		}
	}
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}


void USPGA_BattleActionBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 스킬이 취소되지 않고 정상적으로 끝날 때 스택 소모 검사를 합니다.
	if (!bWasCancelled && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

		// 내 몸에 해골 수정 버프가 있고, 방금 쓴 스킬이 무기 스킬일 때만!
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SPTags.State_Buff_CrystalSkull) &&
			AbilityTags.HasTag(SPTags.Battle_Action_Skill))
		{
			ConsumeTimeInterferenceStack(); // 🌟 여기서 마지막으로 스택을 깎습니다!
		}
	}

	// 원래 EndAbility가 해야 할 일(종료 처리)을 마저 수행합니다.
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool USPGA_BattleActionBase::ConsumeTimeInterferenceStack()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return false;

	FGameplayTag TimeInterferenceTag = FSPGameplayTags::Get().State_TimeInterference;

	if (ASC->HasMatchingGameplayTag(TimeInterferenceTag))
	{
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(TimeInterferenceTag));
		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

		for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
		{
			// 🌟 현재 스택이 몇 개인지 가져옵니다.
			int32 CurrentStack = ASC->GetCurrentStackCount(Handle);

			ASC->RemoveActiveGameplayEffect(Handle, 1);

			// 🌟 깎인 후의 남은 스택을 로그로 예쁘게 출력합니다!
			UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 스택 소모! (남은 스택: %d)"), CurrentStack - 1);
			return true;
		}
	}
	return false;
}

void USPGA_BattleActionBase::PrepareBattleAction(const FGameplayEventData* TriggerEventData)
{
	Super::PrepareBattleAction(TriggerEventData);

	if (TriggerEventData)
	{
		CachedEventData = *TriggerEventData;
	}
}

bool USPGA_BattleActionBase::CommitBattleAction(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!Super::CommitBattleAction(
		Handle,
		ActorInfo,
		ActivationInfo))
	{
		return false;
	}

	ApplyTurnBasedCooldown();

	return true;
}

void USPGA_BattleActionBase::SetupActionEventListeners()
{
	Super::SetupActionEventListeners();


	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FSPGameplayTags::Get().Event_Battle_ApplyDamage);

	if (!WaitDamageEventTask)
	{
		return;
	}

	WaitDamageEventTask->EventReceived.AddDynamic(
		this,
		&USPGA_BattleActionBase::OnDamageEventReceived);

	WaitDamageEventTask->ReadyForActivation();
}

bool USPGA_BattleActionBase::ValidateBattleAction() const
{
	if (!Super::ValidateBattleAction())
	{
		return false;
	}

	switch (SkillTargetingType)
	{
	case ETargetingType::Single:
	case ETargetingType::Area:
	{
		return IsValidBattleTarget(GetSingleTarget());
	}

	case ETargetingType::All:
	case ETargetingType::Random:
	{
		const TArray<AActor*> ValidTargets = GetAllEnemies();

		if (ValidTargets.IsEmpty())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[BattleAction] 행동 가능한 Target이 없습니다."));

			return false;
		}

		return true;
	}

	case ETargetingType::Self:
		return IsValid(GetAvatarActorFromActorInfo());

	default:
		return false;
	}
}

FGameplayTag USPGA_BattleActionBase::GetCooldownTag() const
{
	return CooldownTag;
}

void USPGA_BattleActionBase::ApplyDamageToTarget(AActor* TargetActor, float DamageMultiplier)
{
	if (!TargetActor || !DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamageToTarget 실패: 타겟이나 GE 클래스가 없습니다."));
		return;
	}

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// Spec 핸들 생성 (이펙트 명세서)
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);

	if (SpecHandle.IsValid())
	{
		// 데미지 배율 전달 (SetByCaller)
		// Tag: Data.Damage -> 데미지 계산기(ExecCalc)가 이 값을 읽어서 공격력에 곱합니다.
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
			SpecHandle,
			SPTags.Data_Damage,
			DamageMultiplier
		);
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		//  [핵심 방어막] 데미지를 입히기 직전에, 이 몬스터가 아직 살아있는지 기억해둠
		bool bWasAliveBeforeHit = false;
		if (TargetASC && !TargetASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Death))
		{
			bWasAliveBeforeHit = true; // 때리기 전엔 살아있었음!
		}

		// 이펙트 적용
		ApplyGameplayEffectSpecToTarget(
			CurrentSpecHandle,
			CurrentActorInfo,
			CurrentActivationInfo,
			SpecHandle,
			UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor)
		);

		if (HitSound && GetWorld())
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, TargetActor->GetActorLocation());
		}

		if (HitCameraShakeClass && GetWorld())
		{
			UGameplayStatics::PlayWorldCameraShake(GetWorld(), HitCameraShakeClass, TargetActor->GetActorLocation(), 0.0f, 1000.0f, 1.0f);
		}

		if (GetAssetTags().HasTag(SPTags.Battle_Action_Attack))
		{
			ASPGASCharacterBase* AvatarChar = Cast<ASPGASCharacterBase>(GetAvatarActorFromActorInfo());
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			if (AvatarChar && ASC && AvatarChar->GetStatusEffectComponent() && IsValid(TargetActor))
			{
				// 타겟의 체력이 0이하라면 상태이상을 적용하지 않음
				TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
				if (TargetASC && !TargetASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Death))
				{
					FGameplayTag EquippedWeaponTag;
					if (ASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir))
						EquippedWeaponTag = SPTags.Weapon_Fenrir;
					else if (ASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr))
						EquippedWeaponTag = SPTags.Weapon_Surtr;
					else if (ASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr))
						EquippedWeaponTag = SPTags.Weapon_Jormungandr;

					if (EquippedWeaponTag.IsValid())
					{
						float HitRate = ASC->GetNumericAttribute(USPGASAttributeSet::GetEffectHitRateAttribute());

						float FinalChance = HitRate;
						// 2. 0 ~ 100 사이의 주사위를 굴립니다.
						float RandomRoll = FMath::RandRange(0.0f, 1.0f);

						// 3. 주사위 결과가 최종 확률(효과명중)보다 낮거나 같으면 상태이상 적중!
						if (RandomRoll <= FinalChance)
						{
							AvatarChar->GetStatusEffectComponent()->ApplyWeaponStatusEffectToTarget(EquippedWeaponTag, TargetActor);
							UE_LOG(LogTemp, Warning, TEXT("상태이상 적중"), FinalChance, RandomRoll);
						}
						else
						{
							UE_LOG(LogTemp, Log, TEXT("상태이상 빗나감"), FinalChance, RandomRoll);
						}
					}
				}
			}
			if (ASC) // 1. 타격 이벤트 발사
			{
				FGameplayEventData RelicPayload;
				RelicPayload.Instigator = GetAvatarActorFromActorInfo();
				RelicPayload.Target = TargetActor;
				RelicPayload.InstigatorTags.AddTag(SPTags.Battle_Action_Attack);

				// 내 몸에 장착된 유물 어빌리티들이 들을 수 있게 전용 이벤트를 발사합니다.
				ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Combat_AttackHit, &RelicPayload);
			}
			UAbilitySystemComponent* EventTargetASC = nullptr;
			if (IsValid(TargetActor))
			{
				EventTargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			}

			if (ASC && TargetASC) // 2. 처치 이벤트 발사
			{
				//  [수정됨] 방금 전까지 살아있었는데(bWasAliveBeforeHit), 방금 때리고 나니 죽었다면(State_Death)?
				if (bWasAliveBeforeHit && TargetASC->HasMatchingGameplayTag(SPTags.State_Death))
				{
					FGameplayEventData RelicKillPayload;
					RelicKillPayload.Instigator = GetAvatarActorFromActorInfo();
					RelicKillPayload.Target = TargetActor;
					RelicKillPayload.InstigatorTags.AddTag(SPTags.Battle_Action_Attack);

					// 프리즘이 들을 수 있게 "일반 공격 처치!" 신호 쏘기
					ASC->HandleGameplayEvent(SPTags.Event_Combat_AttackKill, &RelicKillPayload);

					UE_LOG(LogTemp, Warning, TEXT("일반 공격 처치 발생! 유물들에게 Kill 신호를 보냅니다."));
				}
			}
		}
		// [추가한 부분] 만약 공격 방식이 '무기 스킬(Skill)' 이라면?
		else if (AbilityTags.HasTag(SPTags.Battle_Action_Skill))
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			if (ASC && TargetASC)
			{
				//  [수정됨] 스킬 처치 판정에도 동일하게 '막타' 방어막 적용
				if (bWasAliveBeforeHit && TargetASC->HasMatchingGameplayTag(SPTags.State_Death))
				{
					FGameplayEventData RelicKillPayload;
					RelicKillPayload.Instigator = GetAvatarActorFromActorInfo();
					RelicKillPayload.Target = TargetActor;

					// 피 묻은 동전이 들을 수 있게 "스킬 처치!" 신호 쏘기
					ASC->HandleGameplayEvent(SPTags.Event_Combat_SkillKill, &RelicKillPayload);

					UE_LOG(LogTemp, Warning, TEXT("무기 스킬 처치 발생! 유물들에게 SkillKill 신호를 보냅니다."));
				}
			}
		}
		

		FHitResult HitResult;
		AActor* AvatarActor = GetAvatarActorFromActorInfo();

		if (AvatarActor && TargetActor)
		{
			FVector StartLoc = AvatarActor->GetActorLocation(); // 내 위치
			FVector EndLoc = TargetActor->GetActorLocation();   // 적 위치

			// 내 몸에서 적의 중심을 향해 레이저를 쏴서 표면에 닿는 점을 찾습니다.
			GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility);

			// 만약 장애물 때문에 안 맞았다면, 그냥 적의 중심 위치를 강제로 타격점으로 지정!
			if (!HitResult.bBlockingHit || HitResult.GetActor() != TargetActor)
			{
				HitResult.ImpactPoint = TargetActor->GetActorLocation();
			}
			HitResult.HitObjectHandle = FActorInstanceHandle(TargetActor); // 맞은 놈이 얘라고 명시

			// 적의 ASC에 Hit VFX(GameplayCue) 실행 명령!
			TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (TargetASC && HitVFXTag.IsValid())
			{
				FGameplayCueParameters CueParams;
				CueParams.EffectContext = TargetASC->MakeEffectContext();
				CueParams.EffectContext.AddHitResult(HitResult); // 방금 찾은 정확한 표면 좌표를 넘김!
				//타격용 GameplayCue 태그 호출!
				TargetASC->ExecuteGameplayCue(HitVFXTag, CueParams);
			}

			// 적에게 피격 애니메이션(Hit React) 재생 명령!
			// (ASPGASCharacterBase에 PlayHitReact 함수가 구현되어 있다고 가정)
			if (ASPGASCharacterBase* TargetBaseChar = Cast<ASPGASCharacterBase>(TargetActor))
			{
				TargetBaseChar->PlayHitReact(HitResult.ImpactPoint);
			}
		}
	}
}

void USPGA_BattleActionBase::ApplyTurnBasedCooldown()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// =========================================================================
	// 시간 간섭이 발동 중인가?
	// =========================================================================
	if (ASC->HasMatchingGameplayTag(SPTags.State_TimeInterference))
	{
		// 1. 해골 수정 + 무기 스킬일 경우
		if (ASC->HasMatchingGameplayTag(SPTags.State_Buff_CrystalSkull) &&
			AbilityTags.HasTag(SPTags.Battle_Action_Skill))
		{
			UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 해골 수정 활성화! 쿨타임을 부여하지 않고 스택만 소모합니다."));
			return; 
		}

		// 2. 옥시계 + 패링 반격일 경우
		if (ASC->HasMatchingGameplayTag(SPTags.State_Buff_JadeClock) &&
			AbilityTags.HasTag(SPTags.Battle_Action_CounterAttack))
		{
			FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(SPTags.State_Buff_JadeClock));
			TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

			for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
			{
				ASC->RemoveActiveGameplayEffect(Handle, 1); // 옥시계 스택 1 차감
				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 옥시계 반격! 쿨타임을 부여하지 않고 스택만 소모합니다."));
				return; 
			}
		}
	}
	// =========================================================================

	//  '공명하는 룬' 효과: 무기 스킬일 경우 25% 확률로 쿨타임 무시!
	if (AbilityTags.HasTag(SPTags.Battle_Action_Skill))
	{
		if (ASC->HasMatchingGameplayTag(SPTags.Relic_Passive_ResonatingRune))
		{
			if (FMath::RandRange(1, 100) <= 25) // 난수 범위 수정 (1~100 중 25 이하)
			{
				UE_LOG(LogTemp, Warning, TEXT("✨ [유물 발동] 공명하는 룬! 이번 스킬은 쿨타임이 돌지 않습니다!"));
				return; 
			}
		}
	}

	// -------------------------------------------------------------------------
	// 위 방어막들을 모두 무사히 통과했다면? 정상적으로 쿨타임을 부여합니다.
	// -------------------------------------------------------------------------
	if (CooldownTurns <= 0 || !CooldownEffectClass || !CooldownTag.IsValid()) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CooldownEffectClass, 1.0f, Context);

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetStackCount(CooldownTurns);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		UE_LOG(LogTemp, Log, TEXT("쿨타임 시작: %s (%d 턴)"), *CooldownTag.ToString(), CooldownTurns);
	}
}

AActor* USPGA_BattleActionBase::GetSingleTarget() const
{
	return const_cast<AActor*>(CachedEventData.Target.Get());
}

TArray<AActor*> USPGA_BattleActionBase::GetAllEnemies() const
{
	TArray<AActor*> OutActors;
	TArray<AActor*> FoundActors;

	// 일단 월드의 모든 캐릭터나 전투 참여자를 찾습니다. 
	// (가장 좋은 건 TurnManager가 살아있는 적 리스트를 주는 것이지만, 일단 유지합시다)
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundActors);

	AActor* MyAvatar = GetAvatarActorFromActorInfo();

	for (AActor* Actor : FoundActors)
	{
		// 1. 나 자신은 때리지 않음 (몬스터가 이 스킬을 쓸 때 자해 방지)
		if (Actor == MyAvatar) continue;

		// 2. 체력이 0보다 큰 '살아있는' 녀석만 타겟으로 잡음
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

		// ASC가 존재하고, 체력이 0보다 큰지 확인!
		if (TargetASC && TargetASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute()) > 0.0f)
		{
			OutActors.Add(Actor); // 통과! 데미지 명단에 추가!
		}
	}

	return OutActors;
}

TArray<AActor*> USPGA_BattleActionBase::GetSecondaryTargets(AActor* PrimaryTarget) const
{
	TArray<AActor*> AllEnemies = GetAllEnemies();
	TArray<AActor*> SecondaryTargets;

	for (AActor* Enemy : AllEnemies)
	{
		// 주 타겟과 동일한 액터가 아니면 보조 타겟 배열에 추가합니다.
		if (Enemy != PrimaryTarget)
		{
			SecondaryTargets.Add(Enemy);
		}
	}

	return SecondaryTargets;
}

AActor* USPGA_BattleActionBase::GetRandomEnemy() const
{
	TArray<AActor*> Enemies = GetAllEnemies();

	if (Enemies.Num() == 0) return nullptr;

	int32 RandIndex = FMath::RandRange(0, Enemies.Num() - 1);
	return Enemies[RandIndex];
}




void USPGA_BattleActionBase::OnDamageEventReceived(FGameplayEventData Payload)
{
	float PrimaryMultiplier = (Payload.EventMagnitude > 0.0f) ? Payload.EventMagnitude : DefaultDamageMultiplier;
	float Ratio = (DefaultDamageMultiplier > 0.0f) ? (PrimaryMultiplier / DefaultDamageMultiplier) : 1.0f;
	float SecondaryMultiplier = SecondaryDamageMultiplier * Ratio;

	AActor* PayloadTarget = const_cast<AActor*>(Payload.Target.Get());

	if (PayloadTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("마녀: 무전 받음!"));
		ApplyDamageToTarget(PayloadTarget, PrimaryMultiplier);
		return;
	}

	// -------------------------------------------------------------------------
	// 아래는 기존 로직 (Payload에 타겟이 없을 때, 즉 일반적인 방법으로 데미지를 줄 때)

	UE_LOG(LogTemp, Warning, TEXT("마녀: 무전 받음!"));
	switch (SkillTargetingType)
	{
	case ETargetingType::Single:
	{
		if (AActor* Target = GetSingleTarget())
		{
			ApplyDamageToTarget(Target, PrimaryMultiplier);
		}
		break;
	}
	case ETargetingType::Area:
	{
		AActor* CenterTarget = PayloadTarget ? PayloadTarget : GetSingleTarget();

		if (CenterTarget)
		{
			ApplyDamageToTarget(CenterTarget, PrimaryMultiplier); 

			TArray<AActor*> SecTargets = GetSecondaryTargets(CenterTarget);
			for (AActor* SecTarget : SecTargets)
			{
				ApplyDamageToTarget(SecTarget, SecondaryMultiplier); 
			}
		}
		break;
	}
	case ETargetingType::All:
	{
		TArray<AActor*> AllEnemies = GetAllEnemies();
		for (AActor* Enemy : AllEnemies)
		{
			ApplyDamageToTarget(Enemy, PrimaryMultiplier);
		}
		break;
	}
	case ETargetingType::Random:
	{
		if (AActor* RandomTarget = GetRandomEnemy())
		{
			ApplyDamageToTarget(RandomTarget, PrimaryMultiplier);
		}
		break;
	}
	}
}

void USPGA_BattleActionBase::GrantExtraTurns(int32 ExtraTurns)
{
	AASPCombatGameMode* GameMode = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	GameMode->RequestInterrupt(Avatar, ExtraTurns);

	UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 발동! 현재 턴을 유지한 채 추가 턴 %d개를 예약합니다."), ExtraTurns);
}

void USPGA_BattleActionBase::ExecuteJadeClockInterference(TSubclassOf<class UGameplayEffect> JadeClockBuffClass)
{
	AASPCombatGameMode* GameMode = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode());
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!GameMode || !Avatar) return;

	// 1. 살아있는 모든 적을 가져와서 숫자를 셉니다.
	TArray<AActor*> Enemies = GetAllEnemies();
	int32 EnemyCount = Enemies.Num();

	if (EnemyCount > 0 && JadeClockBuffClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// 2. 나에게 적의 수만큼 '옥시계 버프 스택' 부여
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(JadeClockBuffClass, 1.0f, Context);

			if (SpecHandle.IsValid())
			{
				// 🌟 스택 갯수를 적의 수로 세팅!
				SpecHandle.Data->SetStackCount(EnemyCount);
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		// 3. 모든 적의 행동 게이지를 100(Max)으로 꽉 채웁니다!
		for (AActor* Enemy : Enemies)
		{
			GameMode->GetTurnManager()->SetActionGauge(Enemy, ASPCombatTurnManager::MaxActionGauge);
		}

		UE_LOG(LogTemp, Warning, TEXT("[옥시계] 적 %d명의 게이지를 100으로 만들고 패링 프리패스 버프를 %d스택 획득했습니다!"), EnemyCount, EnemyCount);
	}

	// 4. 내 턴 강제 종료 (적들이 미친 듯이 달려들기 시작합니다)
	GameMode->EndTurn(Avatar);
}

void USPGA_BattleActionBase::ExecuteGoldBugInterference()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	// 내 몸(플레이어)에 있는 상태이상 컴포넌트를 가져옵니다. 
	USPStatusEffectComponent* StatusComp = Avatar->FindComponentByClass<USPStatusEffectComponent>();
	if (!StatusComp) return;

	TArray<AActor*> Enemies = GetAllEnemies();
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	for (AActor* Enemy : Enemies)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
		if (!TargetASC) continue;

		USPStatusEffectComponent* TargetStatusComp = Enemy->FindComponentByClass<USPStatusEffectComponent>();
		if (!TargetStatusComp) continue;

		// 1. 현재 이 몬스터에게 '없는' 기본 상태이상을 조사합니다.
		TArray<FGameplayTag> UnappliedTags;

		if (!TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Burn))
			UnappliedTags.Add(SPTags.Debuff_Basic_Burn);

		if (!TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Weathering))
			UnappliedTags.Add(SPTags.Debuff_Basic_Weathering);

		if (!TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Poison))
			UnappliedTags.Add(SPTags.Debuff_Basic_Poison);

		// 2. 모인 상태이상 후보들을 무작위로 섞습니다. (셔플)
		for (int32 i = UnappliedTags.Num() - 1; i > 0; i--)
		{
			int32 j = FMath::RandRange(0, i);
			UnappliedTags.Swap(i, j);
		}

		// 3. 앞에서부터 최대 2개를 뽑아서 대상에게 차례대로 주입합니다!
		int32 ApplyCount = FMath::Min(2, UnappliedTags.Num());
		for (int32 i = 0; i < ApplyCount; i++)
		{
			// 🌟 타겟의 컴포넌트에서 ProcessStatusEffect를 실행합니다!
			TargetStatusComp->ProcessStatusEffect(UnappliedTags[i], TargetASC, Avatar);

			UE_LOG(LogTemp, Warning, TEXT("[골드 버그] %s 에게 %s 를 부여했습니다!"), *Enemy->GetName(), *UnappliedTags[i].ToString());
		}
	}
}

void USPGA_BattleActionBase::SetupDamageEventListener()
{
	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FSPGameplayTags::Get().Event_Battle_ApplyDamage
		);

	if (!WaitDamageEventTask)
	{
		return;
	}

	WaitDamageEventTask->EventReceived.AddDynamic(
		this,
		&USPGA_BattleActionBase::OnDamageEventReceived
	);

	WaitDamageEventTask->ReadyForActivation();
}

bool USPGA_BattleActionBase::IsValidBattleTarget(AActor* Target) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::
		GetAbilitySystemComponent(Target);

	if (!TargetASC)
	{
		return false;
	}

	const float Health =
		TargetASC->GetNumericAttribute(
			USPGASAttributeSet::GetHealthAttribute());

	return Health > 0.0f;
}



