// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_BattleActionBase.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASCharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"
#include "Game/ASPCombatGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Component/SPStatusEffectComponent.h"


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
			return true; // 쿨타임 2턴이 남아있어도 프리패스!
		}
	}
	// 1. 부모 클래스(GAS 기본 로직)의 비용 검사를 먼저 실행합니다.
	// (여기서 우리가 등록한 GE_Cost_WeaponSkill_BP를 확인해서 BP가 2 이상인지 체크합니다)
	bool bCanAfford = Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);

	// 2. 만약 BP가 부족해서 검사를 통과하지 못했다면?
	if (!bCanAfford)
	{
		// 🌟 여기에 BP 부족 로그를 띄웁니다!
		UE_LOG(LogTemp, Warning, TEXT("[시스템] BP가 부족하여 스킬을 사용할 수 없습니다!"));

		// (나중에 UI 연동하실 때, 여기서 화면에 "BP 부족!" 위젯을 띄우는 이벤트를 호출하시면 완벽합니다)
	}

	// 3. 검사 결과 반환 (false면 GAS가 알아서 스킬 발동을 취소시켜 줍니다)
	return bCanAfford;
}

bool USPGA_BattleActionBase::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(FSPGameplayTags::Get().State_TimeInterference) &&
			!AbilityTags.HasTag(FSPGameplayTags::Get().Battle_Action_TimeInterference))
		{
			return true; // 쿨타임 2턴이 남아있어도 프리패스!
		}
	}
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
			return; // 결제 안 하고 그냥 도망침!
		}
	}
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}
bool USPGA_BattleActionBase::ConsumeTimeInterferenceStack()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return false;

	// 우리가 에디터에서 만든 시간 간섭 태그
	FGameplayTag TimeInterferenceTag = FGameplayTag::RequestGameplayTag("State.TimeInterference");

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

void USPGA_BattleActionBase::ApplyDamageToTarget(AActor* TargetActor, float DamageMultiplier)
{
	if (!TargetActor || !DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamageToTarget 실패: 타겟이나 GE 클래스가 없습니다."));
		return;
	}

	// Spec 핸들 생성 (이펙트 명세서)
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);

	if (SpecHandle.IsValid())
	{
		// 데미지 배율 전달 (SetByCaller)
		// Tag: Data.Damage -> 데미지 계산기(ExecCalc)가 이 값을 읽어서 공격력에 곱합니다.
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
			SpecHandle,
			FGameplayTag::RequestGameplayTag(FName("Data.Damage")),
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

		if (AbilityTags.HasTag(FSPGameplayTags::Get().Battle_Action_Attack))
		{
			ASPGASCharacterBase* AvatarChar = Cast<ASPGASCharacterBase>(GetAvatarActorFromActorInfo());
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			if (AvatarChar && ASC && AvatarChar->GetStatusEffectComponent())
			{
				FGameplayTag EquippedWeaponTag;

				// 2. 현재 내 몸(ASC)에 어떤 무기 태그가 붙어있는지 확인
				if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.Fenrir")))
					EquippedWeaponTag = FGameplayTag::RequestGameplayTag("Weapon.Fenrir");
				else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.Surtr")))
					EquippedWeaponTag = FGameplayTag::RequestGameplayTag("Weapon.Surtr");
				else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.Jormungandr")))
					EquippedWeaponTag = FGameplayTag::RequestGameplayTag("Weapon.Jormungandr");

				// 3. 컴포넌트에게 "지금 때렸으니까 상태이상 굴려봐!" 라고 명령
				if (EquippedWeaponTag.IsValid())
				{
					AvatarChar->GetStatusEffectComponent()->ApplyWeaponStatusEffectToTarget(EquippedWeaponTag, TargetActor);
				}
			}
		}
	}
}

void USPGA_BattleActionBase::ApplyTurnBasedCooldown()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	if (ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_TimeInterference) &&
		!AbilityTags.HasTag(FSPGameplayTags::Get().Battle_Action_TimeInterference))
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
	if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		return PlayerChar->CurrentCombatTarget;
	}
	return nullptr;
}

TArray<AActor*> USPGA_BattleActionBase::GetAllEnemies() const
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), OutActors);

	// (TODO: HP가 0 이하인 적은 제외하는 로직을 추가하면 좋습니다)
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
