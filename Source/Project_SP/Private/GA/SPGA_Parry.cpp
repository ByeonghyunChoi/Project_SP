// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_Parry.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerController.h"
#include "GA/SPGA_BattleActionBase.h"
#include "Tag/SPGameplayTags.h"
#include "Sound/SoundBase.h"     
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Game/SPProjectileBase.h"
#include "SubSystem/SPCombatSubsystem.h"


bool USPGA_Parry::CheckWeaponMatch(ASPGASMonsterCharacter* TargetMonster)
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (!PlayerASC) return false;

	if (USPCombatSubsystem* CombatSys = GetWorld()->GetGameInstance()->GetSubsystem<USPCombatSubsystem>())
	{
		if (CombatSys->GetCurrentTutorialStage() != ETutorialStage::None && CombatSys->GetCurrentTutorialStep() == 6)
		{
			UE_LOG(LogTemp, Warning, TEXT("[튜토리얼 각본] 튜토리얼 패링! 강제 성공 처리."));
			if (TargetMonster)
			{
				if (UAbilitySystemComponent* TargetASC = TargetMonster->GetAbilitySystemComponent())
				{
					TargetASC->RemoveLooseGameplayTag(SPTags.State_ParryWindow);
					TargetASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(SPTags.State_ParryWindow));
				}
			}
			return true;
		}
	}

	// 1. 투사체(Projectile) 레이더 검사
	if (MyAvatar)
	{
		FVector StartLoc = MyAvatar->GetActorLocation();
		FVector EndLoc = StartLoc + (MyAvatar->GetActorForwardVector() * 100.0f);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(MyAvatar);

		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult, StartLoc, EndLoc, FQuat::Identity,
			ECC_GameTraceChannel3, FCollisionShape::MakeSphere(80.0f), Params
		);

		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();
			if (ASPProjectileBase* Projectile = Cast<ASPProjectileBase>(HitActor))
			{
				if (!Projectile->bIsParried)
				{
					bool bIsProjMatch = false;
					FGameplayTag ProjElement = Projectile->ElementTag;

					if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir) && ProjElement == SPTags.Weakness_Fenrir) bIsProjMatch = true;
					else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr) && ProjElement == SPTags.Weakness_Surtr) bIsProjMatch = true;
					else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr) && ProjElement == SPTags.Weakness_Jormungandr) bIsProjMatch = true;

					if (bIsProjMatch)
					{
						Projectile->bIsParried = true;
						Projectile->Destroy();
						UE_LOG(LogTemp, Warning, TEXT("[패링 성공] 투사체를 튕겨냈습니다!"));
						return true;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("[패링 실패]"));
						ApplyParryPenalty();
						return false;
					}
				}
			}
		}
	}

	// 2. 근접 공격 패링 검사
	if (!TargetMonster)
	{
		ApplyParryPenalty();
		return false;
	}

	UAbilitySystemComponent* TargetASC = TargetMonster->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		ApplyParryPenalty(); // 🚨 페널티!
		return false;
	}

	if (!TargetASC->HasMatchingGameplayTag(SPTags.State_ParryWindow))
	{
		UE_LOG(LogTemp, Warning, TEXT("패링 실패: 몬스터가 패링 가능 상태가 아닙니다."));
		ApplyParryPenalty(); // 🚨 타이밍 못 맞춤 페널티!
		return false;
	}

	bool bIsMatch = false;
	if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Fenrir)) bIsMatch = true;
	else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Surtr)) bIsMatch = true;
	else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Jormungandr)) bIsMatch = true;

	if (bIsMatch)
	{
		TargetASC->RemoveLooseGameplayTag(SPTags.State_ParryWindow);
		TargetASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(SPTags.State_ParryWindow));
		UE_LOG(LogTemp, Warning, TEXT("근접 패링 성공!"));
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("패링 실패: 무기와 몬스터의 약점이 불일치합니다."));
	ApplyParryPenalty(); // 🚨 약점 못 맞춤 페널티!
	return false;
}

bool USPGA_Parry::CheckCounterConditions()
{
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerASC) return false;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (PlayerASC->HasMatchingGameplayTag(SPTags.State_Buff_JadeClock))
	{
		UE_LOG(LogTemp, Warning, TEXT("[옥시계] 버프가 존재하여 쿨타임 검사를 패스합니다!"));
		return true;
	}

	if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerChar->GetController()))
		{
			FGameplayTag CurrentWeapon = PC->GetCurrentWeaponTag();
			UWeaponAbilityData* WeaponData = PlayerChar->GetWeaponData(CurrentWeapon);

			if (WeaponData && WeaponData->ParrySkillAbility)
			{
				// 기본 UGameplayAbility가 아니라, 베이스 클래스로 캐스팅합니다!
				if (USPGA_BattleActionBase* AbilityCDO = WeaponData->ParrySkillAbility->GetDefaultObject<USPGA_BattleActionBase>())
				{
					// CooldownTag 변수를 가져옵니다.
					FGameplayTag TargetCooldownTag = AbilityCDO->GetCooldownTag();

					// 태그가 유효하고, 내 몸(ASC)에 그 태그가 붙어있다면 쿨타임 중인 것!
					if (TargetCooldownTag.IsValid() && PlayerASC->HasMatchingGameplayTag(TargetCooldownTag))
					{
						UE_LOG(LogTemp, Warning, TEXT("패링 성공: 하지만 반격 스킬(%s)이 쿨타임 중이라 추가 턴을 얻지 못합니다!"), *WeaponData->ParrySkillAbility->GetName());
						return false; // ❌ 쿨타임 컷!
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("패링 성공 & 쿨타임 통과! 즉시 반격 턴을 획득합니다."));
	return true;
}

void USPGA_Parry::SendParriedEventToMonster(AActor* TargetMonster)
{
	if (ParrySuccessSounds.Num() > 0)
	{
		int index = FMath::RandRange(0, ParrySuccessSounds.Num() - 1);
		UGameplayStatics::PlaySound2D(this, ParrySuccessSounds[index]);
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetMonster))
	{
		// 몬스터에게 무전을 날림
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo(); // 내가 때렸다
		ASI->GetAbilitySystemComponent()->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_Parried, &Payload);
		UE_LOG(LogTemp, Warning, TEXT("몬스터에게 패링 이벤트를 성공적으로 전송했습니다."));
	}

	// 내 몸의 유물들에게 패링 성공했다고 알림
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayEventData ParryPayload;
		ParryPayload.Instigator = GetAvatarActorFromActorInfo();
		ParryPayload.Target = TargetMonster;

		ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Combat_ParrySuccess, &ParryPayload);
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetMonster))
	{
		TargetASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Status_DamageDisabled);
	}
}

void USPGA_Parry::ApplyParryPenalty()
{
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (PlayerASC && ParryPenaltyGE)
	{
		FGameplayEffectContextHandle Context = PlayerASC->MakeEffectContext();
		Context.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = PlayerASC->MakeOutgoingSpec(ParryPenaltyGE, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			PlayerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			UE_LOG(LogTemp, Error, TEXT("[패링] 헛손질 페널티! 0.5초간 패링이 금지됩니다."));
		}
	}
}
