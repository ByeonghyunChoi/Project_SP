// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Component/OpartsComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "Component/InventoryComponent.h" 

UOpartsComponent::UOpartsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOpartsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ASC 가져오기
	if (IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		ASC = Interface->GetAbilitySystemComponent();
	}
	else if (GetOwner())
	{
		ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	}
}

void UOpartsComponent::EquipOparts(const UOpartsDefinition* NewOpartsDef)
{
	if (!ASC || !NewOpartsDef) return;
	if (RuntimeData.Definition == NewOpartsDef) return; // 이미 같은 거 장착 중

	// 1. 기존 것 싹 비우기
	UnequipCurrentOparts();

	// 2. 새 데이터 세팅 (초기 상태: 1레벨, 아티팩트 0개)
	// (만약 세이브 파일에서 불러오는 경우라면 여기서 Load 로직을 태워야 합니다)
	RuntimeData.Definition = NewOpartsDef;
	RuntimeData.CurrentLevel = 1;
	RuntimeData.UnlockedArtifactCount = 0;

	// 3. 능력 적용
	ApplyOpartsStatsAndAbilities();

	UE_LOG(LogTemp, Log, TEXT("오파츠 장착: %s"), *NewOpartsDef->DisplayName.ToString());
}

void UOpartsComponent::UnequipCurrentOparts()
{
	if (!ASC) return;

	// 1. 기본 패시브 제거
	if (RuntimeData.PassiveAbilityHandle.IsValid())
	{
		ASC->ClearAbility(RuntimeData.PassiveAbilityHandle);
	}

	// 2. 기본 스탯 이펙트 제거
	if (RuntimeData.StatEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(RuntimeData.StatEffectHandle);
	}

	// 3. 아티팩트 능력들 제거
	for (auto& Handle : RuntimeData.ArtifactAbilityHandles)
	{
		if (Handle.IsValid()) ASC->ClearAbility(Handle);
	}

	// 4. 아티팩트 스탯들 제거
	for (auto& Handle : RuntimeData.ArtifactEffectHandles)
	{
		if (Handle.IsValid()) ASC->RemoveActiveGameplayEffect(Handle);
	}

	// 데이터 초기화 및 UI 알림
	RuntimeData.Clear();
	if (OnOpartsUpdated.IsBound()) OnOpartsUpdated.Broadcast(RuntimeData);
}

void UOpartsComponent::ApplyOpartsStatsAndAbilities()
{
	if (!ASC || !RuntimeData.Definition) return;

	const UOpartsDefinition* Def = RuntimeData.Definition;

	// ====================================================
	// 1. 기본 스탯 적용 (Infinite Duration Effect)
	// ====================================================
	// 기존에 적용된 스탯이 있다면 제거 후 재적용 (레벨 변경 대응)
	if (RuntimeData.StatEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(RuntimeData.StatEffectHandle);
		RuntimeData.StatEffectHandle = FActiveGameplayEffectHandle();
	}

	if (Def->BaseStatEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		// [중요] 레벨을 오파츠 레벨로 설정! (CurveTable에서 값 읽어옴)
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Def->BaseStatEffectClass, (float)RuntimeData.CurrentLevel, Context);

		if (SpecHandle.IsValid())
		{
			RuntimeData.StatEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// ====================================================
	// 2. 기본 패시브 능력 적용 (최초 1회만)
	// ====================================================
	if (!RuntimeData.PassiveAbilityHandle.IsValid() && Def->BasePassiveAbility)
	{
		FGameplayAbilitySpec Spec(Def->BasePassiveAbility, 1, -1, this);
		RuntimeData.PassiveAbilityHandle = ASC->GiveAbilityAndActivateOnce(Spec);
	}

	// ====================================================
	// 3. 아티팩트 능력/스탯 적용 (현재 해금 수만큼 반복)
	// ====================================================

	// 기존 아티팩트 효과들 일단 다 제거 (깔끔하게 재계산)
	// 최적화를 위해선 델타만 적용할 수도 있지만, 안전하게 전체 갱신 방식을 추천 해서 다 지우고 다시 계산하는 로직 구성
	for (auto& Handle : RuntimeData.ArtifactAbilityHandles) ASC->ClearAbility(Handle);
	for (auto& Handle : RuntimeData.ArtifactEffectHandles) ASC->RemoveActiveGameplayEffect(Handle);
	RuntimeData.ArtifactAbilityHandles.Empty();
	RuntimeData.ArtifactEffectHandles.Empty();

	// 해금된 개수만큼 순회
	int32 MaxIndex = FMath::Min(RuntimeData.UnlockedArtifactCount, Def->Artifacts.Num());

	for (int32 i = 0; i < MaxIndex; i++)
	{
		const FOpartsArtifactData& Artifact = Def->Artifacts[i];

		// 3-1. 아티팩트 스탯 (GE)
		if (Artifact.ArtifactStatEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			// 아티팩트는 보통 레벨 개념이 없으므로 1.0f 고정
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Artifact.ArtifactStatEffectClass, 1.0f, Context);
			if (Spec.IsValid())
			{
				FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				RuntimeData.ArtifactEffectHandles.Add(Handle);
			}
		}

		// 3-2. 아티팩트 능력 (GA)
		if (Artifact.ArtifactAbilityClass)
		{
			FGameplayAbilitySpec Spec(Artifact.ArtifactAbilityClass, 1, -1, this);
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbilityAndActivateOnce(Spec);
			RuntimeData.ArtifactAbilityHandles.Add(Handle);
		}
	}

	// UI 갱신 알림
	if (OnOpartsUpdated.IsBound()) OnOpartsUpdated.Broadcast(RuntimeData);
}

void UOpartsComponent::TryUpgradeLevel()
{
	if (!RuntimeData.Definition) return;
	if (RuntimeData.CurrentLevel >= 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 최대 레벨입니다."));
		return;
	}

	// 2. 인벤토리 컴포넌트 찾기
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Error, TEXT("인벤토리 컴포넌트를 찾을 수 없습니다!"));
		return;
	}
	int32 Cost = RuntimeData.CurrentLevel * 100;

	// 4. 자원 소모 시도
	if (Inventory->ConsumeSand(Cost))
	{
		// 성공 시 레벨업 진행
		RuntimeData.CurrentLevel++;

		// 스탯 재적용 (Curve Table 값이 바뀜)
		ApplyOpartsStatsAndAbilities();

		UE_LOG(LogTemp, Log, TEXT("오파츠 레벨업 성공! (Lv.%d -> Lv.%d)"), RuntimeData.CurrentLevel - 1, RuntimeData.CurrentLevel);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("레벨업 실패: 모래가 부족합니다."));
	}
}

void UOpartsComponent::TryUnlockNextArtifact()
{
	if (!RuntimeData.Definition) return;

	// 1. 최대 해금 체크 (5개)
	if (RuntimeData.UnlockedArtifactCount >= 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 아티팩트가 해금되었습니다."));
		return;
	}

	// 2. 인벤토리 찾기
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return;

	// 3. 비용 계산 (기획서: 1 -> 2 -> 3 -> 3 -> 4)
	// 배열 인덱스: 0(첫해금), 1, 2, 3, 4
	int32 Costs[] = { 1, 2, 3, 3, 4 };
	int32 CurrentIndex = RuntimeData.UnlockedArtifactCount; // 현재 0개면 0번 인덱스 비용(1) 필요

	// 안전장치
	if (!Costs[CurrentIndex]) return;
	int32 Cost = Costs[CurrentIndex];

	// 4. 자원 소모 시도
	if (Inventory->ConsumeIncompleteEnergy(Cost))
	{
		// 성공 시 해금
		RuntimeData.UnlockedArtifactCount++;

		// 아티팩트 능력 재적용
		ApplyOpartsStatsAndAbilities();

		UE_LOG(LogTemp, Log, TEXT("아티팩트 해금 성공! (현재 개수: %d)"), RuntimeData.UnlockedArtifactCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("해금 실패: 불완전한 기운이 부족합니다."));
	}
}