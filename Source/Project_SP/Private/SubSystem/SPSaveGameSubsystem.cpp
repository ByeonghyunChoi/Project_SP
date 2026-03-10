// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPSaveGameSubsystem.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/RelicComponent.h"
#include "Component/OpartsComponent.h"
#include "Map/MapManagerSubSystem.h"
#include "Save/SPRunSaveGame.h"
#include "Save/SPPermSaveGame.h"
#include "Kismet/GameplayStatics.h"

void USPSaveGameSubsystem::CacheRunDataFromPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	// 1. 스탯 저장
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			bool bFound = false;
			RunData.Stats.CurrentHealth = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetHealthAttribute(), bFound);
			RunData.Stats.CurrentBattlePoint = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute());
			RunData.Stats.CurrentTimePower = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetTimePowerAttribute(), bFound);
			RunData.Stats.CurrentActionGauge = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute());

			RunData.Stats.MaxHealth = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute());
			RunData.Stats.MaxBattlePoint = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxBattlePointAttribute());
			RunData.Stats.MaxTimePower = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxTimePowerAttribute());

			RunData.Stats.Attack = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute());
			RunData.Stats.Defense = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute());
			RunData.Stats.Speed = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute());

			RunData.Stats.DefenseIgnore = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDefenseIgnoreAttribute());
			RunData.Stats.CriticalRate = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCriticalRateAttribute());
			RunData.Stats.CriticalDamage = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCriticalDamageAttribute());
			RunData.Stats.EffectHitRate = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectHitRateAttribute());
			RunData.Stats.EffectAmplify = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectAmplifyAttribute());

			RunData.Stats.OutgoingDamageMultiplier = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute());
			RunData.Stats.IncomingDamageMultiplier = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetIncomingDamageMultiplierAttribute());
			RunData.Stats.Level = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute());
			RunData.Stats.Experience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute());
			RunData.Stats.MaxExperience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxExperienceAttribute());
		}
	}

	// 2. 런 지갑 (골드, 파편) 저장
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		RunData.RunWallet.Money = InventoryComp->GetMoney();
		RunData.RunWallet.Fragment = InventoryComp->GetFragment();
	}

	// 3. 유물 저장
	if (URelicComponent* RelicComp = PlayerPawn->FindComponentByClass<URelicComponent>())
	{
		RunData.RelicData.EquippedRelics = RelicComp->EquippedRelics;
		RunData.RelicData.AcquiredHistory = RelicComp->AcquiredHistory;
	}

	// 4. 맵 진행도 저장
	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		// 맵 매니저가 들고 있는 진행 정보 캐싱
		RunData.MapProgress.CurrentStage = MapManager->GetCurrentStage();
		RunData.MapProgress.CurrentFloor = MapManager->GetCurrentFloor();
		RunData.MapProgress.CurrentMapType = MapManager->GetCurrentMapType();
		RunData.MapProgress.CurrentRoomState = MapManager->GetCurrentRoomState();

		// 로비 맵인지 확인 정보 캐싱
		RunData.MapProgress.bIsSavedInLobby = MapManager->GetIsInLobby();
		// 플레이어의 현재 실제 위치 캐싱
		RunData.MapProgress.SavedPlayerTransform = PlayerPawn->GetActorTransform();
	}
}

void USPSaveGameSubsystem::RestoreRunDataToPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || !RunData.IsValid()) return;

	// 1. 스탯 복구
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute(), RunData.Stats.MaxHealth);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxBattlePointAttribute(), RunData.Stats.MaxBattlePoint);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxTimePowerAttribute(), RunData.Stats.MaxTimePower);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute(), RunData.Stats.Level);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), RunData.Stats.Experience);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxExperienceAttribute(), RunData.Stats.MaxExperience);

			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute(), RunData.Stats.Attack);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute(), RunData.Stats.Defense);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute(), RunData.Stats.Speed);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseIgnoreAttribute(), RunData.Stats.DefenseIgnore);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCriticalRateAttribute(), RunData.Stats.CriticalRate);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCriticalDamageAttribute(), RunData.Stats.CriticalDamage);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetEffectHitRateAttribute(), RunData.Stats.EffectHitRate);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetEffectAmplifyAttribute(), RunData.Stats.EffectAmplify);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute(), RunData.Stats.OutgoingDamageMultiplier);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetIncomingDamageMultiplierAttribute(), RunData.Stats.IncomingDamageMultiplier);

			// 자원 (Current) 복구 - 마지막에!
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), RunData.Stats.CurrentHealth);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute(), RunData.Stats.CurrentBattlePoint);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), RunData.Stats.CurrentTimePower);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), RunData.Stats.CurrentActionGauge);
		}
	}

	// 2. 유물 복구 (스탯 기반으로 적용되므로 스탯 이후에 호출)
	if (URelicComponent* RelicComp = PlayerPawn->FindComponentByClass<URelicComponent>())
	{
		RelicComp->LoadRelicData(RunData.RelicData);
	}

	// 3. 런/영구 지갑 일괄 세팅 (UI 갱신을 위해 한 번에 넘겨줌)
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		InventoryComp->LoadWalletData(RunData.RunWallet, PermData.PermanentWallet);
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 런 데이터 복구 완료!"));
}

void USPSaveGameSubsystem::ResetRunData()
{
	RunData.Reset();
	if (UGameplayStatics::DoesSaveGameExist(RunSlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(RunSlotName, 0);
	}
	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 사망/클리어: 런(Run) 데이터만 초기화되었습니다."));
}

void USPSaveGameSubsystem::CachePermDataFromPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	// 1. 영구 지갑 저장
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		PermData.PermanentWallet.Sand = InventoryComp->GetSand();
		PermData.PermanentWallet.IncompleteEnergy = InventoryComp->GetIncompleteEnergy();
	}

	// 2. 오파츠 저장
	if (UOpartsComponent* OpartsComp = PlayerPawn->FindComponentByClass<UOpartsComponent>())
	{
		PermData.OpartsData.EquippedOparts = OpartsComp->GetCurrentOpartsData().Definition;
		PermData.OpartsData.ProgressMap = OpartsComp->OpartsProgressMap;
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 영구 데이터 캐싱 완료!"));
}

void USPSaveGameSubsystem::RestorePermDataToPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	// 1. 오파츠 복구
	if (UOpartsComponent* OpartsComp = PlayerPawn->FindComponentByClass<UOpartsComponent>())
	{
		OpartsComp->LoadOpartsData(PermData.OpartsData);
	}

	// 2. 지갑 복구 (로비 진입 시 영구 데이터만 먼저 불릴 수 있으므로 여기서도 세팅)
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		InventoryComp->LoadWalletData(RunData.RunWallet, PermData.PermanentWallet);
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 영구 데이터 복구 완료!"));
}

void USPSaveGameSubsystem::ResetAllData()
{
	RunData.Reset();
	PermData.Reset();

	if (UGameplayStatics::DoesSaveGameExist(RunSlotName, 0)) UGameplayStatics::DeleteGameInSlot(RunSlotName, 0);
	if (UGameplayStatics::DoesSaveGameExist(PermSlotName, 0)) UGameplayStatics::DeleteGameInSlot(PermSlotName, 0);

	UE_LOG(LogTemp, Warning, TEXT("[SaveSystem] 새로하기: 영구 데이터와 런 데이터가 모두 공장 초기화되었습니다! (Hard Reset)"));
}


void USPSaveGameSubsystem::SaveRunToDisk()
{
	USPRunSaveGame* SaveInst = Cast<USPRunSaveGame>(UGameplayStatics::CreateSaveGameObject(USPRunSaveGame::StaticClass()));
	if (SaveInst)
	{
		SaveInst->RunData = RunData; // 서브시스템 메모리 데이터를 껍데기에 복사
		UGameplayStatics::SaveGameToSlot(SaveInst, RunSlotName, 0); // 디스크에 굽기!
		UE_LOG(LogTemp, Warning, TEXT("런 데이터 디스크 저장 완료! [%s]"), *RunSlotName);
	}
}

bool USPSaveGameSubsystem::LoadRunFromDisk()
{
	if (UGameplayStatics::DoesSaveGameExist(RunSlotName, 0))
	{
		USPRunSaveGame* LoadInst = Cast<USPRunSaveGame>(UGameplayStatics::LoadGameFromSlot(RunSlotName, 0));
		if (LoadInst)
		{
			RunData = LoadInst->RunData; // 디스크 데이터를 서브시스템 메모리로 가져오기
			UE_LOG(LogTemp, Warning, TEXT("런 데이터 디스크 로드 성공!"));
			return true;
		}
	}
	return false;
}

bool USPSaveGameSubsystem::HasValidRunSave() const
{
	return UGameplayStatics::DoesSaveGameExist(RunSlotName, 0);
}

void USPSaveGameSubsystem::SavePermToDisk()
{
	USPPermSaveGame* SaveInst = Cast<USPPermSaveGame>(UGameplayStatics::CreateSaveGameObject(USPPermSaveGame::StaticClass()));
	if (SaveInst)
	{
		SaveInst->PermData = PermData;
		UGameplayStatics::SaveGameToSlot(SaveInst, PermSlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("영구 데이터 디스크 저장 완료! [%s]"), *PermSlotName);
	}
}

bool USPSaveGameSubsystem::LoadPermFromDisk()
{
	if (UGameplayStatics::DoesSaveGameExist(PermSlotName, 0))
	{
		USPPermSaveGame* LoadInst = Cast<USPPermSaveGame>(UGameplayStatics::LoadGameFromSlot(PermSlotName, 0));
		if (LoadInst)
		{
			PermData = LoadInst->PermData;
			UE_LOG(LogTemp, Warning, TEXT("영구 데이터 디스크 로드 성공!"));
			return true;
		}
	}
	return false;
}