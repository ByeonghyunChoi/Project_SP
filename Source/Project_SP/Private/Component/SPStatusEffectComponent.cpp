// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SPStatusEffectComponent.h"
#include "Data/Asset/StatusEffectData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Character/SPGASMonsterCharacter.h"

// Sets default values for this component's properties
USPStatusEffectComponent::USPStatusEffectComponent()
{
}

void USPStatusEffectComponent::ApplyWeaponStatusEffectToTarget(FGameplayTag WeaponTag, AActor* TargetActor)
{
	if (!TargetActor || !StatusEffectDataAsset) return;

	// 타겟의 ASC 가져오기 (인터페이스 활용)
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	// 확률 체크(추후 효과 명중과 연동)
	/*if (FMath::RandRange(0.0f, 1.0f) > 0.6f)
	{
		UE_LOG(LogTemp, Log, TEXT("[StatusComponent] 상태이상 부여 실패"));
		return;
	}*/

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();
	FGameplayTag StatusToApply;

	// 무기 태그 -> 상태이상 태그 변환
	if (WeaponTag.MatchesTag(SPTags.Weapon_Fenrir))
		StatusToApply = SPTags.Debuff_Basic_Weathering;
	else if (WeaponTag.MatchesTag(SPTags.Weapon_Surtr))
		StatusToApply = SPTags.Debuff_Basic_Burn;
	else if (WeaponTag.MatchesTag(SPTags.Weapon_Jormungandr))
		StatusToApply = SPTags.Debuff_Basic_Poison;

	if (StatusToApply.IsValid())
	{
		// 🌟 [수정된 부분] 상태이상 융합과 장전은 반드시 "맞는 대상(Target)"의 컴포넌트에서 실행해야 합니다!
		if (USPStatusEffectComponent* TargetStatusComp = TargetActor->FindComponentByClass<USPStatusEffectComponent>())
		{
			// 내(GetOwner())가 너한테 상태이상을 걸 테니, 네 주머니에 장전해라!
			TargetStatusComp->ProcessStatusEffect(StatusToApply, TargetASC, GetOwner());
		}
	}
}

void USPStatusEffectComponent::ProcessStatusEffect(FGameplayTag IncomingStatusTag, UAbilitySystemComponent* TargetASC, AActor* InstigatorActor)
{
	if (!TargetASC || !StatusEffectDataAsset) return;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	auto BroadcastFusionEvent = [&]() // 상태이상 융합 이벤트를 쏘기위한 로직
	{
		if (InstigatorActor)
		{
			FGameplayEventData Payload;
			Payload.Instigator = InstigatorActor;
			Payload.Target = TargetASC->GetAvatarActor();
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(InstigatorActor, SPTags.Event_Combat_StatusFusion, Payload);
		}
	};

	// 풍화(Weathering)
	if (IncomingStatusTag == SPTags.Debuff_Basic_Weathering)
	{
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Burn))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Basic_Burn);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Mix_HeatWind, TargetASC, InstigatorActor);
			return;
		}
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Poison))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Basic_Poison);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Mix_Faint, TargetASC, InstigatorActor);
			return;
		}
	}
	// 화상(Burn)
	else if (IncomingStatusTag == SPTags.Debuff_Basic_Burn)
	{
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Weathering))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Basic_Weathering);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Mix_HeatWind, TargetASC, InstigatorActor);
			return;
		}
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Poison))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Basic_Poison);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Mix_Plague, TargetASC, InstigatorActor);
			return;
		}
	}
	// 중독(Poison)
	else if (IncomingStatusTag == SPTags.Debuff_Basic_Poison)
	{
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Weathering))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Basic_Weathering);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Mix_Faint, TargetASC, InstigatorActor);
			return;
		}
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Basic_Burn))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Basic_Burn);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Mix_Plague, TargetASC, InstigatorActor);
			return;
		}
	}
	// 열풍(HeatWind) / 역병(Plague) -> 최종 진화 (치명상)
	else if (IncomingStatusTag == SPTags.Debuff_Mix_HeatWind)
	{
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Mix_Plague))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Mix_Plague);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Fatal_FatalWound, TargetASC, InstigatorActor);
			return;
		}
	}
	else if (IncomingStatusTag == SPTags.Debuff_Mix_Plague)
	{
		if (TargetASC->HasMatchingGameplayTag(SPTags.Debuff_Mix_HeatWind))
		{
			RemoveStatusEffectByTag(TargetASC, SPTags.Debuff_Mix_HeatWind);
			BroadcastFusionEvent(); // 융합 이벤트 전송
			ProcessStatusEffect(SPTags.Debuff_Fatal_FatalWound, TargetASC, InstigatorActor);
			return;
		}
	}


	// === 최종 적용 ===
	const FStatusEffectConfig* Config = StatusEffectDataAsset->GetConfig(IncomingStatusTag);
	if (Config && !Config->bIsInstantEffect && Config->EffectClass)
	{
		// 시전자(Instigator)의 ASC를 가져와서 Effect Context를 만듭니다.
		UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor);
		if (!InstigatorASC) return;

		if (TargetASC->HasMatchingGameplayTag(IncomingStatusTag))
		{
			// 기존 상태이상을 싹 지워버립니다! (턴 수 누적 방지)
			RemoveStatusEffectByTag(TargetASC, IncomingStatusTag);
			AActor* AvatarActor = TargetASC->GetAvatarActor();
			if (AvatarActor)
			{
				UE_LOG(LogTemp, Log, TEXT("[%s] 기존 상태이상(%s) 지속 턴 수 갱신!"), *AvatarActor->GetName(), *IncomingStatusTag.ToString());
			}
		}

		FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
		Context.AddSourceObject(InstigatorActor);

		FGameplayEffectSpecHandle SpecHandle = InstigatorASC->MakeOutgoingSpec(Config->EffectClass, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetStackCount(Config->DurationTurns);
			SpecHandle.Data->DynamicGrantedTags.AddTag(IncomingStatusTag);

			// 시전자 -> 타겟에게 적용
			InstigatorASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

			UE_LOG(LogTemp, Warning, TEXT("[StatusComponent] 상태이상 적용됨: %s (%d 턴)"), *IncomingStatusTag.ToString(), Config->DurationTurns);
		}

		if (Config->StatModifierClass)
		{
			FGameplayEffectSpecHandle StatSpec = InstigatorASC->MakeOutgoingSpec(Config->StatModifierClass, 1.0f, Context);
			if (StatSpec.IsValid())
			{
				// SetStackCount를 부르지 않으므로 무조건 1스택(1회)만 적용됩니다!
				InstigatorASC->ApplyGameplayEffectSpecToTarget(*StatSpec.Data.Get(), TargetASC);
			}
		}
	}
	else if (Config && Config->bIsInstantEffect)
	{
		UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor);
		if (!InstigatorASC) return;

		float FinalDamage = 0.0f;
		TSubclassOf<UGameplayEffect> DamageClassToApply = nullptr; // 어떤 총알을 쏠지 결정
		bool bIsExecute = false; // 처형(즉사) 여부

		float TargetMaxHP = TargetASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute());

		// 2. 상태이상 별 처리
		// [혼절] 일반 상태이상 공식을 타야 하므로 StatusDamage 사용!
		if (IncomingStatusTag == SPTags.Debuff_Mix_Faint)
		{
			FinalDamage = Config->InstantDamageCoefficient; // 계수만 넘김 (예: 1.2)
			DamageClassToApply = StatusDamageEffectClass;   // 🌟 혼절은 StatusDamage 총알 장전!

			TargetASC->AddLooseGameplayTag(SPTags.State_Status_SkipTurn);
			UE_LOG(LogTemp, Warning, TEXT("[혼절] %s 가 행동 불능에 빠집니다!"), *TargetASC->GetAvatarActor()->GetName());
		}
		// [치명상] 일반 몹: 즉사 / 보스 몹: 2턴 DoT
		else if (IncomingStatusTag == SPTags.Debuff_Fatal_FatalWound)
		{
			bool bIsBossOrEpic = TargetASC->HasMatchingGameplayTag(SPTags.Enemy_Rank_Boss) ||
				TargetASC->HasMatchingGameplayTag(SPTags.Enemy_Rank_Epic);

			if (bIsBossOrEpic)
			{
				UE_LOG(LogTemp, Warning, TEXT("[치명상] 보스, 에픽 몬스터는 치명적인 지속 피해로 적용됩니다!"));
				FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
				Context.AddSourceObject(InstigatorActor);

				// 보스는 데미지 총알을 쏘지 않고 바구니(GE)만 달아줍니다. (DoT는 턴 시작 시 발동)
				FGameplayEffectSpecHandle DotSpec = InstigatorASC->MakeOutgoingSpec(Config->EffectClass, 1.0f, Context);
				if (DotSpec.IsValid())
				{
					DotSpec.Data->SetStackCount(2);
					DotSpec.Data->DynamicGrantedTags.AddTag(IncomingStatusTag);
					InstigatorASC->ApplyGameplayEffectSpecToTarget(*DotSpec.Data.Get(), TargetASC);
				}

			}
			else
			{
				// 일반 몬스터: 최대 체력만큼 절대 데미지
				FinalDamage = TargetMaxHP * Config->InstantDamageCoefficient;
				DamageClassToApply = FixedDamageEffectClass; // 🌟 치명상은 FixedDamage 총알 장전!
				bIsExecute = true; // 처형 프리패스 예약
				UE_LOG(LogTemp, Warning, TEXT("[치명상] 일반 몬스터 즉사 판정!"));
			}
		}

		// 3. 결정된 총알(GE) 발사!
		if (DamageClassToApply)
		{
			FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
			Context.AddSourceObject(InstigatorActor);

			FGameplayEffectSpecHandle SpecHandle = InstigatorASC->MakeOutgoingSpec(DamageClassToApply, 1.0f, Context);

			if (SpecHandle.IsValid())
			{
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
					SpecHandle,
					SPTags.Data_Damage,
					FinalDamage
				);

				// 일반몹 치명상일 경우에만 '처형' 태그를 몰래 붙여서 쏩니다.
				if (bIsExecute)
				{
					SpecHandle.Data->AddDynamicAssetTag(SPTags.Damage_Type_Execute);
				}

				SpecHandle.Data->AddDynamicAssetTag(IncomingStatusTag);

				AActor* TargetAvatar = TargetASC->GetAvatarActor();

				// 1. 탄약고에 보관
				PendingDamageMap.FindOrAdd(TargetAvatar).Add(SpecHandle);

				// 2. 몬스터에게 연출 중 태그 부착
				if (!TargetASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_VisualPlaying))
				{
					TargetASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Status_VisualPlaying);
				}

				// 3. 블루프린트에게 "즉발 연출 시작!" 방송 송출 (bIsInstant = true)
				OnStatusVisualTriggered.Broadcast(TargetAvatar, IncomingStatusTag, true);
			}
		}
	}

	if (TargetASC)
	{
		if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(TargetASC->GetAvatarActor()))
		{
			Monster->BroadcastStatusUI();
		}
	}
}

void USPStatusEffectComponent::ProcessTurnStartDoT()
{
	AActor* Owner = GetOwner();
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	if (!OwnerASC || !StatusEffectDataAsset || !StatusDamageEffectClass) return;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// 1. 내 몸(Owner)에 붙어있는 모든 활성화된 이펙트를 가져옵니다.
	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = OwnerASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = OwnerASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		// 2. 이 이펙트가 무슨 태그를 부여하고 있는지 확인합니다.
		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);

		// 3. 우리 데이터 에셋을 순회하며 비교합니다.
		for (const auto& Pair : StatusEffectDataAsset->StatusConfigs)
		{
			FGameplayTag StatusTag = Pair.Key;
			const FStatusEffectConfig& Config = Pair.Value;

			// DoT 계수가 존재하고 & 내 몸에 그 상태이상 태그가 붙어있다면!
			if (Config.DotDamageCoefficient > 0.0f && GrantedTags.HasTag(StatusTag))
			{
				// 4. 이 상태이상을 걸었던 시전자(Instigator)를 찾습니다.
				AActor* Instigator = ActiveGE->Spec.GetContext().GetInstigator();
				UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);

				if (InstigatorASC)
				{
					// 순수 계수만 가져옵니다.
					float TargetCoefficient = Config.DotDamageCoefficient;

					// 5. 데미지 적용
					FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
					Context.AddSourceObject(Instigator);

					// 🌟 [추가된 핵심 로직] 어떤 총알(GE)을 쏠지 결정합니다.
					TSubclassOf<UGameplayEffect> DamageClassToUse = StatusDamageEffectClass; // 기본은 상태이상 데미지

					// 만약 발동하려는 DoT가 '치명상' 이라면? -> 고정 데미지 GE로 교체!
					if (StatusTag == FSPGameplayTags::Get().Debuff_Fatal_FatalWound)
					{
						DamageClassToUse = FixedDamageEffectClass;
						UE_LOG(LogTemp, Warning, TEXT("[치명상 DoT] 고정 데미지 타입으로 발동 준비!"));
					}

					// 결정된 GE 클래스로 Spec을 만듭니다.
					FGameplayEffectSpecHandle DamageSpec = InstigatorASC->MakeOutgoingSpec(DamageClassToUse, 1.0f, Context);

					if (DamageSpec.IsValid())
					{
						// 계산기(DamageCalculation)로 '계수'만 던져줌
						UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
							DamageSpec,
							SPTags.Data_Damage,
							TargetCoefficient
						);

						DamageSpec.Data->AddDynamicAssetTag(StatusTag);

						PendingDamageMap.FindOrAdd(Owner).Add(DamageSpec);

						// 2. 몬스터(Owner)에게 '지금 연출 중임!' 이라는 이름표(태그) 부착 (AI 행동 정지용)
						if (!OwnerASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_VisualPlaying))
						{
							OwnerASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Status_VisualPlaying);
						}

						// 3. 블루프린트에게 "도트 연출 시작!" 방송 송출 (bIsInstant = false)
						OnStatusVisualTriggered.Broadcast(Owner, StatusTag, false);

						UE_LOG(LogTemp, Warning, TEXT("[%s] 턴 시작! 상태이상(%s) 장전 완료! (계수: %f) - 연출 대기 중"),
							*Owner->GetName(), *StatusTag.ToString(), TargetCoefficient);
					}
				}
			}
		}
	}
}

void USPStatusEffectComponent::ReduceStatusEffectTurns()
{
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!OwnerASC) return;

	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = OwnerASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = OwnerASC->GetActiveGameplayEffect(Handle);
		if (!ActiveGE) continue;

		FGameplayTagContainer GrantedTags;
		ActiveGE->Spec.GetAllGrantedTags(GrantedTags);

		// 이펙트가 "Debuff"로 시작하는 태그를 가지고 있다면 (즉, 상태이상이라면)
		if (GrantedTags.HasTag(FGameplayTag::RequestGameplayTag("Debuff")))
		{
			// 🌟 [새로 추가된 부분] 로그에 출력할 상태이상 태그 이름 찾기
			FString StatusName = TEXT("알수없음");
			for (const FGameplayTag& Tag : GrantedTags)
			{
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Debuff")))
				{
					StatusName = Tag.ToString();
					break; // 디버프 태그를 찾으면 루프 종료
				}
			}

			int32 CurrentStack = OwnerASC->GetCurrentStackCount(Handle);

			// 스택(턴)을 1 깎습니다.
			OwnerASC->RemoveActiveGameplayEffect(Handle, 1);
		}
	}

	if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(GetOwner()))
	{
		Monster->BroadcastStatusUI();
	}
}

void USPStatusEffectComponent::ExecutePendingDamage(AActor* TargetActor, FGameplayTag StatusTag)
{
	if (!TargetActor) return;

	if (TArray<FGameplayEffectSpecHandle>* PendingSpecs = PendingDamageMap.Find(TargetActor))
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (TargetASC)
		{
			// 🌟 역순(for 루프 거꾸로) 순회하며 요청한 태그와 일치하는 데미지만 폭발!
			for (int32 i = PendingSpecs->Num() - 1; i >= 0; --i)
			{
				FGameplayEffectSpecHandle& Spec = (*PendingSpecs)[i];
				if (Spec.IsValid())
				{
					// 🌟 [수정됨] GrantedTags가 아니라 AssetTags(데미지 총알 자체의 이름표)를 검사합니다!
					if (Spec.Data->DynamicAssetTags.HasTagExact(StatusTag))
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
						PendingSpecs->RemoveAt(i); // 터뜨렸으니 탄약고에서 제거

						if (ASPGASCharacterBase* GASChar = Cast<ASPGASCharacterBase>(TargetActor))
						{
							// 상태이상은 날아오는 방향이 없으므로, 몬스터 자신의 현재 위치를 ImpactPoint로 줍니다.
							GASChar->PlayHitReact(GASChar->GetActorLocation());
						}
					}
				}
			}
		}
	}
}

void USPStatusEffectComponent::RemoveStatusEffectByTag(UAbilitySystemComponent* TargetASC, FGameplayTag StatusTagToRemove)
{
	if (!TargetASC) return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(StatusTagToRemove);

	TargetASC->RemoveActiveEffectsWithGrantedTags(TagContainer);

	if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(GetOwner()))
	{
		Monster->BroadcastStatusUI();
	}

	UE_LOG(LogTemp, Log, TEXT("[StatusComponent] 상태이상 제거됨: %s"), *StatusTagToRemove.ToString());
}

