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

USPGA_BattleActionBase::USPGA_BattleActionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = nullptr;
	CostGameplayEffectClass = nullptr;
}

bool USPGA_BattleActionBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(FSPGameplayTags::Get().State_TimeInterference) &&
			!AbilityTags.HasTag(FSPGameplayTags::Get().Battle_Action_TimeInterference))
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

	// 🌟 1. 시간 간섭 발동 중이면 쿨타임 무시 (프리패스!)
	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(FSPGameplayTags::Get().State_TimeInterference) &&
		!AbilityTags.HasTag(FSPGameplayTags::Get().Battle_Action_TimeInterference))
	{
		UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 쿨타임 무시 로직 작동! 강제 발동!"));
		return true;
	}

	// 🌟 2. 수동 쿨타임 검사 (기존에 블루프린트 Blocked Tag로 막던 것을 여기서 처리)
	if (CooldownTag.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag))
	{
		// 쿨타임 태그가 내 몸에 있다면? 스킬 발동 차단!
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(CooldownTag);
		}
		UE_LOG(LogTemp, Warning, TEXT("GAS 시스템: 쿨타임 중이라 발동 불가 (%s)"), *CooldownTag.ToString());
		return false;
	}

	// 3. 기본 쿨타임 체크
	return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
}

void USPGA_BattleActionBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(FSPGameplayTags::Get().State_TimeInterference) &&
			!AbilityTags.HasTag(FSPGameplayTags::Get().Battle_Action_TimeInterference))
		{
			UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] BP 소모를 무시합니다."));
			return; 
		}
	}
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void USPGA_BattleActionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData) CachedEventData = *TriggerEventData;

	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FSPGameplayTags::Get().Event_Battle_ApplyDamage
	);

	if (WaitDamageEventTask)
	{
		WaitDamageEventTask->EventReceived.AddDynamic(this, &USPGA_BattleActionBase::OnDamageEventReceived);
		WaitDamageEventTask->ReadyForActivation();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

bool USPGA_BattleActionBase::ConsumeTimeInterferenceStack()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return false;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// 우리가 에디터에서 만든 시간 간섭 태그
	FGameplayTag TimeInterferenceTag = SPTags.State_TimeInterference;

	// 1. 내 몸에 시간 간섭 태그가 있는지 확인!
	if (ASC->HasMatchingGameplayTag(TimeInterferenceTag))
	{
		// 2. 이 태그를 부여하고 있는 바구니(Active Effect)를 찾습니다.
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(TimeInterferenceTag));
		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

		for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
		{
			// 현재 몇 스택이 남았는지 확인
			int32 CurrentStacks = ASC->GetCurrentStackCount(Handle);
			if (CurrentStacks > 0)
			{
				// 3. 스택을 1개만 깎습니다! (GAS 내장 함수)
				ASC->RemoveActiveGameplayEffect(Handle, 1);

				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 스택 소모! 남은 횟수: %d"), CurrentStacks - 1);

				// 4. 스택을 깎았는데도 아직 남았다면? -> "턴 넘기지 마!"(true) 반환
				if (CurrentStacks - 1 > 0)
				{
					return true;
				}
				else
				{
					// 스택을 다 썼다면? -> "이제 턴 넘겨!"(false) 반환
					UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 모든 스택을 소모했습니다. 턴을 종료합니다."));
					return false;
				}
			}
		}
	}

	// 시간 간섭 태그가 아예 없다면 평범하게 턴 종료 (false)
	return false;
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

		// 이펙트 적용
		ApplyGameplayEffectSpecToTarget(
			CurrentSpecHandle,
			CurrentActorInfo,
			CurrentActivationInfo,
			SpecHandle,
			UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor)
		);

		if (HitCameraShakeClass && GetWorld())
		{
			UGameplayStatics::PlayWorldCameraShake(GetWorld(), HitCameraShakeClass, TargetActor->GetActorLocation(), 0.0f, 1000.0f, 1.0f);
		}

		if (AbilityTags.HasTag(SPTags.Battle_Action_Attack))
		{
			ASPGASCharacterBase* AvatarChar = Cast<ASPGASCharacterBase>(GetAvatarActorFromActorInfo());
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			if (AvatarChar && ASC && AvatarChar->GetStatusEffectComponent() && IsValid(TargetActor))
			{
				// 타겟의 체력이 0이하라면 상태이상을 적용하지 않음
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
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
						AvatarChar->GetStatusEffectComponent()->ApplyWeaponStatusEffectToTarget(EquippedWeaponTag, TargetActor);
					}
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
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
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

	if (ASC->HasMatchingGameplayTag(SPTags.State_TimeInterference) &&
		!AbilityTags.HasTag(SPTags.Battle_Action_TimeInterference))
	{
		UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 수동 쿨타임(TurnBased)을 적용하지 않고 무시합니다."));
		return;
	}

	// 쿨타임이 없거나 클래스가 없으면 패스
	if (CooldownTurns <= 0 || !CooldownEffectClass || !CooldownTag.IsValid()) return;

	// 쿨타임 GE 생성
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	// Spec 생성 (Level은 1.0)
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CooldownEffectClass, 1.0f, Context);

	if (SpecHandle.IsValid())
	{
		// [핵심] 쿨타임 태그를 "동적으로" 부여
		// GE_TurnBasedCooldown 자체는 껍데기일 뿐이고, 
		// 실제로는 "Cooldown.Weapon.Fenrir.Skill" 같은 태그를 붙여야 스킬이 막힙니다.
		SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);

		// 스택 개수 = 턴 수
		SpecHandle.Data->SetStackCount(CooldownTurns);

		// 적용 (나 자신에게)
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
		ApplyDamageToTarget(PayloadTarget, PrimaryMultiplier);
		return;
	}

	// -------------------------------------------------------------------------
	// 아래는 기존 로직 (Payload에 타겟이 없을 때, 즉 일반적인 방법으로 데미지를 줄 때)
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

