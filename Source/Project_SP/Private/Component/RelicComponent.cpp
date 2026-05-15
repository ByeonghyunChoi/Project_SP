

#include "Component/RelicComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Tag/SPGameplayTags.h"
#include "Character/SPGASPlayerCharacter.h"

URelicComponent::URelicComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void URelicComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool URelicComponent::CanReroll() const
{
	return CurrentRerollCount > 0;
}

bool URelicComponent::UseReroll()
{
	if (CanReroll())
	{
		CurrentRerollCount--;
		return true;
	}
	return false;
}

bool URelicComponent::AddRelic(const URelicDefinition* NewRelic)
{
	// 빈 값이 들어오지 않도록 검사
	if (!NewRelic) return false;

	// 1. 최대 장착 개수 초과 검사
	if (EquippedRelics.Num() >= MaxRelicCount)
	{
		// 나중에 UI에서 "교체할 유물을 선택하세요" 창을 띄우기 위한 false 반환
		return false;
	}
	// 2. 중복 착용 검사
	if (EquippedRelics.Contains(NewRelic))
	{
		return false;
	}

	// 플레이어의 AbilitySystemComponent(ASC) 가져오기
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return false;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (!ASC) return false;

	// 3. 유물 고유 효과(GE) 적용
	if (NewRelic->RelicEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(NewRelic->RelicEffectClass, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			RelicEffectHandles.Add(ActiveHandle); // 나중에 지우기 위해 핸들 저장
		}
	}

	// 4. 최초 획득 검사 및 등급별 공격력 보너스 부여
	if (!AcquiredHistory.Contains(NewRelic))
	{
		AcquiredHistory.Add(NewRelic); // 획득 기록에 추가

		if (RarityAttackBonusEffectClass)
		{
			FGameplayEffectContextHandle BonusContext = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle BonusSpec = ASC->MakeOutgoingSpec(RarityAttackBonusEffectClass, 1.0f, BonusContext);

			if (BonusSpec.IsValid())
			{
				// 기획서 기준 등급별 공격력 보너스 수치 결정
				float BonusValue = 0.0f;
				switch (NewRelic->Rarity)
				{
				case ERelicRarity::Normal:  BonusValue = 1.007f; break; // 0.7%
				case ERelicRarity::Rare:    BonusValue = 1.01f;  break; // 1.0%
				case ERelicRarity::Unique:  BonusValue = 1.02f;  break; // 2.0%
				}

				// GAS의 SetByCaller를 활용해 C++에서 결정한 수치를 블루프린트 GE로 전달
				BonusSpec.Data.Get()->SetSetByCallerMagnitude(FSPGameplayTags::Get().Relic_Bonus_Attack, BonusValue);

				ASC->ApplyGameplayEffectSpecToSelf(*BonusSpec.Data.Get());
			}
		}
	}
	// 5. 장착 목록에 최종 추가
	EquippedRelics.Add(NewRelic);
	//  [핵심 추가됨] 유물이 장착되었으니 UI를 갱신하라고 방송을 쏩니다!
	if (OnRelicUpdated.IsBound())
	{
		OnRelicUpdated.Broadcast();
	}
	return true;
}

bool URelicComponent::RemoveRelicAtIndex(int32 SlotIndex)
{
	// 1. 방어 코드: 요청한 인덱스가 정상적인 범위(0 ~ 5)인지, 그리고 비어있지 않은지 확인
	if (!EquippedRelics.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const URelicDefinition* RelicToRemove = EquippedRelics[SlotIndex];
	if (!RelicToRemove) return false;

	// 2. 플레이어의 ASC 가져오기
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);

		// 3. 버리는 유물이 줬던 '고유 효과(GE)'만 제거
		if (ASC && RelicToRemove->RelicEffectClass)
		{
			ASC->RemoveActiveGameplayEffectBySourceEffect(RelicToRemove->RelicEffectClass, ASC);
		}
	}

	// 4. 장착 목록(배열)에서 해당 유물 삭제
	EquippedRelics.RemoveAt(SlotIndex);

	/*
	* 단 휙득 기록 에서는 지우지 않음 최초 휙득시 얻는 공격력 보너스는 유지 되어야 하기 때문
	*/

	//  [핵심 추가됨] 유물이 삭제(교체)되었으니 UI를 갱신하라고 방송을 쏩니다!
	if (OnRelicUpdated.IsBound())
	{
		OnRelicUpdated.Broadcast();
	}

	return true;
}

void URelicComponent::ResetAllRelics()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
		if (ASC)
		{
			// 장착했던 유물의 고유 효과들 모두 제거
			for (FActiveGameplayEffectHandle Handle : RelicEffectHandles)
			{
				ASC->RemoveActiveGameplayEffect(Handle);
			}

			// 등급별 공격력 보너스 이펙트도 제거 (클래스 기반으로 모두 삭제)
			if (RarityAttackBonusEffectClass)
			{
				ASC->RemoveActiveGameplayEffectBySourceEffect(RarityAttackBonusEffectClass, ASC);
			}
		}
	}

	// 배열 초기화
	RelicEffectHandles.Empty();
	EquippedRelics.Empty();
	AcquiredHistory.Empty();

	//  [추가] 런이 초기화될 때 리롤 횟수도 가득 채워줍니다!
	CurrentRerollCount = MaxRerollCount;
}

TArray<URelicDefinition*> URelicComponent::GenerateRelicRewards(int32 CurrentStage, const TArray<URelicDefinition*>& AllRelicPool, bool bIsBossReward)
{
	TArray<URelicDefinition*> FinalRewards;

	// 1. 스테이지별 등급 등장 확률 계산 (1 ~ 100 사이 난수)
	float RandomValue = FMath::RandRange(0.0f, 100.0f);
	ERelicRarity SelectedRarity = ERelicRarity::Normal; // 기본값

	if (bIsBossReward)
	{
		if (RandomValue <= 60.0f) SelectedRarity = ERelicRarity::Rare;
		else SelectedRarity = ERelicRarity::Unique;
	}
	else if (CurrentStage <= 1) // 스테이지 1
	{
		if (RandomValue <= 75.0f) SelectedRarity = ERelicRarity::Normal; // 1~ 75 (75%)
		else if (RandomValue <= 95.0f) SelectedRarity = ERelicRarity::Rare; // 76~ 95 (20%)
		else SelectedRarity = ERelicRarity::Unique; // 96~100 (5%)
	}
	else if (CurrentStage == 2) // 스테이지 2
	{
		if (RandomValue <= 35.0f) SelectedRarity = ERelicRarity::Normal; //1~ 35 (35%)
		else if (RandomValue <= 90.0f) SelectedRarity = ERelicRarity::Rare; // 36~ 90 (55%)
		else SelectedRarity = ERelicRarity::Unique; // 91~100 (10%)
	}
	else if (CurrentStage == 3) // 스테이지 3 이상
	{
		if (RandomValue <= 20.0f) SelectedRarity = ERelicRarity::Normal; // 1~ 20 (20%)
		else if (RandomValue <= 70.0f) SelectedRarity = ERelicRarity::Rare; // 21~ 70 (50%)
		else SelectedRarity = ERelicRarity::Unique; // 71~100 (30%)
	}

	// 2. 전체 풀에서 조건에 맞는 유물만 1차 필터링 (선택된 등급 일치 & 장착 중이지 않은 것)
	TArray<URelicDefinition*> FilteredRelics;
	for (URelicDefinition* Relic : AllRelicPool)
	{
		if (Relic && Relic->Rarity == SelectedRarity)
		{
			// 장착 중인 유물 목록에 없어야 함 (중복 방지)
			if (!EquippedRelics.Contains(Relic))
			{
				FilteredRelics.Add(Relic);
			}
		}
	}

	// 3. 필터링된 배열 섞기 (셔플)
	const int32 NumFiltered = FilteredRelics.Num();
	if (NumFiltered > 0)
	{
		int32 LastIndex = NumFiltered - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 SwapIndex = FMath::RandRange(i, LastIndex);
			if (i != SwapIndex)
			{
				FilteredRelics.Swap(i, SwapIndex);
			}
		}
	}

	// 4. 앞에서부터 최대 3개까지만 뽑아서 최종 배열에 담기
	int32 RewardsCount = FMath::Min(3, FilteredRelics.Num());
	for (int32 i = 0; i < RewardsCount; ++i)
	{
		FinalRewards.Add(FilteredRelics[i]);
	}

	return FinalRewards;
}

void URelicComponent::LoadRelicData(const FPlayerRelicData& SavedRelicData)
{
	ResetAllRelics();

	ASPGASPlayerCharacter* OwnerCharacter = Cast<ASPGASPlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (!ASC) return;

	for (const URelicDefinition* HistroyRelic : SavedRelicData.AcquiredHistory)
	{
		if (HistroyRelic)
		{
			AcquiredHistory.Add(HistroyRelic);

			// (AddRelic에 있던 등급별 보너스 부여 로직 재실행)
			if (RarityAttackBonusEffectClass)
			{
				FGameplayEffectContextHandle BonusContext = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle BonusSpec = ASC->MakeOutgoingSpec(RarityAttackBonusEffectClass, 1.0f, BonusContext);

				if (BonusSpec.IsValid())
				{
					float BonusValue = 0.0f;
					switch (HistroyRelic->Rarity)
					{
					case ERelicRarity::Normal:  BonusValue = 1.007f; break;
					case ERelicRarity::Rare:    BonusValue = 1.01f;  break;
					case ERelicRarity::Unique:  BonusValue = 1.02f;  break;
					}
					BonusSpec.Data.Get()->SetSetByCallerMagnitude(FSPGameplayTags::Get().Relic_Bonus_Attack, BonusValue);
					ASC->ApplyGameplayEffectSpecToSelf(*BonusSpec.Data.Get());
				}
			}
		}
	}

	for (const URelicDefinition* EquippedRelic : SavedRelicData.EquippedRelics)
	{
		if (EquippedRelic)
		{
			EquippedRelics.Add(EquippedRelic);

			// 고유 효과(GE) 다시 발라주기
			if (EquippedRelic->RelicEffectClass)
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EquippedRelic->RelicEffectClass, 1.0f, Context);
				if (SpecHandle.IsValid())
				{
					FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					RelicEffectHandles.Add(ActiveHandle);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Relic] 세이브 로드 완료! (장착: %d개, 이력: %d개)"), EquippedRelics.Num(), AcquiredHistory.Num());
	
	//  [핵심 추가됨] 세이브 데이터를 불러온 직후에도 UI 갱신 방송을 쏩니다!
	if (OnRelicUpdated.IsBound())
	{
		OnRelicUpdated.Broadcast();
	}
}
