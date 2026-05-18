// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ASPCombatGameMode.h"
#include "Game/SPBattleCameraActor.h"
#include "Game/SPBattleDirector.h"
#include "Manager/SPCombatTurnManager.h"
#include "SubSystem/SPCombatSubsystem.h"
#include "Data/CombatEncounterData.h"
#include "Map/SPGASSpawnPoint.h"      
#include "Kismet/GameplayStatics.h"
#include "Map/MapManagerSubsystem.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Tag/SPGameplayTags.h"
#include "Data/Asset/WeaponAbilityData.h"
#include "Character/SPGASCharacterBase.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Component/SPStatusEffectComponent.h"
#include "Character/SPGASPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Component/InventoryComponent.h"
#include "Data/RewardDataStructs.h"
#include "GA/SPGA_BattleActionBase.h"


AASPCombatGameMode::AASPCombatGameMode()
{
}

void AASPCombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. 데이터 가져오기 (Subsystem)
	UGameInstance* GI = GetGameInstance();
	USPCombatSubsystem* CombatSys = GI ? GI->GetSubsystem<USPCombatSubsystem>() : nullptr;
	const UCombatEncounterData* EncounterData = CombatSys ? CombatSys->GetPendingEncounter() : nullptr;

	TArray<AActor*> SpawnedEnemies;
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (EncounterData)
	{
		UE_LOG(LogTemp, Log, TEXT("전투 모드 시작! (Level: %s)"), *EncounterData->CombatLevelName.ToString());

		// 적 스폰 
		for (const FEnemySpawnInfo& Info : EncounterData->EnemyGroup)
		{
			if (!Info.MonsterData || !Info.MonsterData->MonsterClass) continue;

			// 스폰 포인트 위치 찾기
			FTransform SpawnTransform = GetSpawnTransformByIndex(Info.SpawnPositionIndex);

			// 지연 스폰 (Deferred Spawn) 시작! 형체만 먼저 만듭니다.
			ASPGASMonsterCharacter* SpawnedMonster = GetWorld()->SpawnActorDeferred<ASPGASMonsterCharacter>(
				Info.MonsterData->MonsterClass,
				SpawnTransform,
				nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
			);

			if (SpawnedMonster)
			{
				// 데이터 주사기 주입 (BeginPlay 전에 세팅 완료!)
				SpawnedMonster->MonsterDataAsset = Info.MonsterData;
				SpawnedMonster->CurrentLevel = Info.SpawnLevel;
				SpawnedMonster->EncounterData = const_cast<UCombatEncounterData*>(EncounterData);

				// 약점 오버라이드가 켜져 있다면 주입!
				if (Info.bOverrideWeakness)
				{
					SpawnedMonster->SetWeaknessOverride(Info.OverriddenWeaknessTags);
				}

				// 스폰 완료! (이 순간 몬스터의 BeginPlay -> ApplyMonsterData가 실행됩니다)
				UGameplayStatics::FinishSpawningActor(SpawnedMonster, SpawnTransform);

				SpawnedEnemies.Add(SpawnedMonster);

				UE_LOG(LogTemp, Log, TEXT("[%s] 레벨 %.0f 스폰 및 데이터 주입 완료!"), *Info.MonsterData->MonsterName.ToString(), Info.SpawnLevel);
			}
		}

		// 플레이어 이동 (SpawnPoint 중 'PlayerStart' 태그가 있는 곳, 혹은 별도 로직)
		// 여기서는 편의상 SpawnPoint_Player 태그를 가진 액터를 찾습니다.
		if (PlayerPawn)
		{
			TArray<AActor*> PlayerStarts;
			UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SpawnPoint_Player"), PlayerStarts);

			if (PlayerStarts.Num() > 0)
			{
				PlayerPawn->SetActorTransform(PlayerStarts[0]->GetActorTransform(), false, nullptr, ETeleportType::ResetPhysics);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("전투 데이터 없음"));
	}

	// 전투 시스템 초기화
	if (PlayerPawn)
	{
		InitializeBattle(SpawnedEnemies, PlayerPawn);
	}
}

void AASPCombatGameMode::InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player)
{
	// 턴 매니저 생성
	if (TurnManagerClass && !TurnManager)
	{
		TurnManager = GetWorld()->SpawnActor<ASPCombatTurnManager>(TurnManagerClass);
		if (TurnManager)
		{
			TurnManager->OnTurnOrderChanged.AddDynamic(this, &AASPCombatGameMode::RefreshTurnTimelineUI);
		}
	}

	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("TurnManager 생성 실패! BP_CombatGameMode에 클래스가 할당되었는지 확인하세요."));
		return;
	}

	if (CameraManagerClass && !ActiveCameraManager)
	{
		ActiveCameraManager = GetWorld()->SpawnActor<ASPBattleCameraActor>(CameraManagerClass);
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		ActiveCameraManager->InitCameraManager(PC);
	}

	// 🌟 2. 배틀 디렉터 스폰 및 초기화
	if (BattleDirectorClass && !ActiveBattleDirector)
	{
		ActiveBattleDirector = GetWorld()->SpawnActor<ASPBattleDirector>(BattleDirectorClass);

		// 디렉터에게 카메라 리모컨 넘겨주기
		ActiveBattleDirector->InitDirector(ActiveCameraManager);
	}

	// 🌟 3. 적들을 배틀 디렉터에 등록 (안테나 꽂기)
	if (ActiveBattleDirector)
	{
		for (AActor* Enemy : Enemies)
		{
			ActiveBattleDirector->RegisterMonster(Enemy);
		}
	}

	// 참가자 등록
	AllParticipants.Empty();
	AllParticipants.Add(Player);
	AllParticipants.Append(Enemies);

	TurnManager->InitializeParticipants(AllParticipants);

	// 선제공격 처리
	if (USPCombatSubsystem* CombatSys = GetGameInstance()->GetSubsystem<USPCombatSubsystem>())
	{
		ECombatAdvantage Advantage = CombatSys->GetAdvantageState();

		if (Advantage == ECombatAdvantage::PlayerAdvantage)
		{
			// 플레이어 선공: 게이지 50% 보너스 (밸런스에 따라 조절)
			TurnManager->SetActionGauge(Player, 50.0f);
			UE_LOG(LogTemp, Log, TEXT(">>> 플레이어 선제공격! (게이지 보너스)"));
		}
		else if (Advantage == ECombatAdvantage::EnemyAdvantage)
		{
			// 적 기습: 적 전체 게이지 50% 보너스
			for (AActor* Enemy : Enemies)
			{
				TurnManager->SetActionGauge(Enemy, 50.0f);
			}
			UE_LOG(LogTemp, Warning, TEXT(">>> 적 기습! (적 게이지 보너스)"));
		}
	}

	// 전투 총 인원 수 준비
	TotalExpectedParticipants = AllParticipants.Num();
	bIsBattleInitialized = true;

	CheckAndStartBattle();
}

void AASPCombatGameMode::FinalizeBattleSetup()
{
	bIsBattleRunning = true;

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] 전원 세팅 완료! 전투 UI를 띄우고 즉시 전투를 시작합니다."));

	// 플레이어 컨트롤러에게 명령!
	for (AActor* Participant : AllParticipants)
	{
		if (ASPGASCharacterBase* Character = Cast<ASPGASCharacterBase>(Participant))
		{
			// 다형성(Polymorphism) 폭발! 
			// 플레이어면 컨트롤러 UI가 켜지고, 몬스터면 머리 위 위젯이 켜집니다.
			Character->OnBattleStarted();
		}
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;    // 이벤트를 쏜 사람 (GameMode)
		Payload.Target = PlayerPawn;  // 이벤트를 받을 사람 (Player)

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			PlayerPawn,
			FSPGameplayTags::Get().Event_Battle_Start,
			Payload
		);
		UE_LOG(LogTemp, Log, TEXT("Battle Start Event Sent to Player!"));
	}

	// 턴 매니저에게 첫 턴을 물어보고 시작!
	if (TurnManager)
	{
		RefreshTurnTimelineUI();
		AActor* FirstActor = TurnManager->CalculateNextTurn();
		StartTurn(FirstActor);
	}
}

void AASPCombatGameMode::CheckAndStartBattle()
{
	if (bIsBattleInitialized && !bIsBattleRunning && ReadyParticipants.Num() >= TotalExpectedParticipants)
	{
		FinalizeBattleSetup();
	}
}

void AASPCombatGameMode::ApplyPlayerSavedData()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
		{
			// ASC가 완벽하게 준비된 상태이므로 스탯이 찰떡같이 들어갑니다!
			SaveSys->RestorePermDataToPlayer(PlayerPawn);
			SaveSys->RestoreRunDataToPlayer(PlayerPawn);

			UE_LOG(LogTemp, Warning, TEXT("[지연 적용 성공] 전투 진입: 오파츠 및 런 데이터 완벽 연동 완료!"));
		}
	}
}

void AASPCombatGameMode::StartTurn(AActor* TurnActor)
{
	if (!TurnActor) return;

	CurrentTurnActor = TurnActor;
	UE_LOG(LogTemp, Log, TEXT("턴 시작: %s"), *TurnActor->GetName());

	if (TurnManager)
	{
		TurnManager->SetRoundIterating(true);
	}

	USPStatusEffectComponent* StatusComp = TurnActor->FindComponentByClass<USPStatusEffectComponent>();

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// ==============================================================
			// VIP 반격 턴 (패링 성공 시)
			// ==============================================================
			if (ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_AutoCounterReady))
			{
				bIsCurrentTurnParry = true;
				UE_LOG(LogTemp, Warning, TEXT("VIP 반격 턴 시작! 자동으로 반격 스킬을 발사합니다!"));
				ASC->RemoveLooseGameplayTag(FSPGameplayTags::Get().State_AutoCounterReady);

				if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(TurnActor))
				{
					if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerChar->GetController()))
					{
						FGameplayTag CurrentWeapon = PC->GetCurrentWeaponTag();
						UWeaponAbilityData* WeaponData = PlayerChar->GetWeaponData(CurrentWeapon);

						if (WeaponData && WeaponData->ParrySkillAbility)
						{
							bool bActivated = false;

							// 1. 스킬 발동 시도 (쿨타임이 남아있다면 GAS가 스스로 막고 false를 반환합니다)
							for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
							{
								if (Spec.Ability && Spec.Ability->GetClass() == WeaponData->ParrySkillAbility)
								{
									bActivated = ASC->TryActivateAbility(Spec.Handle);
									break; // 찾았으니 루프 종료
								}
							}

							// 2. 결과 판정 및 강력한 return
							if (bActivated)
							{
								UE_LOG(LogTemp, Warning, TEXT("데이터 에셋 기반 반격기 발동 완료"));
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("반격기 발동 실패(쿨타임 등)! 턴을 강제 종료하고 빠져나갑니다."));
								EndTurn(TurnActor);
								return; // 🚨 [핵심] 여기서 함수를 즉시 탈출하여 '내 턴' 팻말이 억지로 붙는 버그를 차단합니다!
							}
						}
					}
				}
			}
			else
			{
				bIsCurrentTurnParry = false;
			}

			// ==============================================================
			// 상태이상(DoT) 및 사망/혼절 체크
			// ==============================================================
			if (StatusComp && !bIsCurrentTurnInterrupt)
			{
				StatusComp->ProcessTurnStartDoT();
				StatusComp->ReduceStatusEffectTurns();
			}
			else if (StatusComp && bIsCurrentTurnInterrupt)
			{
				UE_LOG(LogTemp, Warning, TEXT("시간이 멈춘 상태라 도트 딜 및 상태이상 턴 감소가 무시됩니다."));
			}

			if (ASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute()) <= 0.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] 사망하여 턴을 취소합니다."), *TurnActor->GetName());
				EndTurn(TurnActor);
				return;
			}

			if (ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_SkipTurn))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] 혼절 상태! 턴 강제 종료."), *TurnActor->GetName());
				ASC->RemoveLooseGameplayTag(FSPGameplayTags::Get().State_Status_SkipTurn);
				EndTurn(TurnActor);
				return;
			}

			// 위쪽에서 return 당하지 않고 무사히 살아남은 녀석만 드디어 '내 턴' 팻말을 얻습니다.
			ASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

			FGameplayEventData Payload;
			Payload.Instigator = TurnActor;
			ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_TurnStart, &Payload);
		}
	}

	// 턴이 확정된 후 UI를 갱신합니다.
	RefreshTurnTimelineUI();

	if (ASPGASCharacterBase* Character = Cast<ASPGASCharacterBase>(TurnActor))
	{
		// 🌟 1. 이 캐릭터가 '시간 정지(수정해골 버프)' 상태인지 확인하기 위한 스위치
		bool bIsTimeStopped = false;

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
		{
			if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
			{
				if (TargetASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Buff_CrystalSkull))
				{
					bIsTimeStopped = true;
				}
			}
		}

		// 🌟 2. 시간이 멈추지 않았을 때만 기존처럼 쿨타임을 깎아줍니다!
		if (!bIsCurrentTurnInterrupt && !bIsTimeStopped)
		{
			Character->ReduceCooldowns();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] 시간 간섭 버프 활성화 중! 쿨타임이 줄어들지 않습니다."), *TurnActor->GetName());
		}
	}
}

void AASPCombatGameMode::EndTurn(AActor* TurnActor)
{
	if (!TurnActor || CurrentTurnActor != TurnActor) return;

	// 중복 호출 방지!
	CurrentTurnActor = nullptr;

	UE_LOG(LogTemp, Log, TEXT("턴 종료: %s"), *TurnActor->GetName());

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// 턴 활성화 태그 제거 (입력 차단)
			ASC->RemoveLooseGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

			if (Cast<ASPGASPlayerCharacter>(TurnActor))
			{
				if (!bIsCurrentTurnInterrupt && TurnEndTimeCostGE)
				{
					FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
					Context.AddSourceObject(this);

					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(TurnEndTimeCostGE, 1.0f, Context);
					if (SpecHandle.IsValid())
					{
						ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
						UE_LOG(LogTemp, Warning, TEXT("[턴 종료] 플레이어의 시간의 힘이 1 감소했습니다."));
					}
				}
				else if (bIsCurrentTurnInterrupt)
				{
					UE_LOG(LogTemp, Warning, TEXT("[특수 턴 종료] 찰나의 순간이므로 시간의 힘이 소모되지 않습니다."));
				}
				else if (!TurnEndTimeCostGE)
				{
					UE_LOG(LogTemp, Error, TEXT("TurnEndTimeCostGE가 게임모드 블루프린트에 설정되지 않았습니다!"));
				}
			}

			// 행동 게이지 0으로 초기화
			if (TurnManager)
			{
				TurnManager->ClearActorFromQueue(TurnActor);

				// VIP 턴이면서, 그게 '패링(반격)'일 때만 게이지를 보존합니다!
				if (bIsCurrentTurnInterrupt && bIsCurrentTurnParry)
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s] 패링 반격 턴 종료! 행동 게이지가 보존됩니다."), *TurnActor->GetName());
				}
				else
				{
					// 정규 턴이거나, 시간 간섭으로 얻은 추가 턴이라면 평소처럼 게이지를 비웁니다!
					float CurrentGauge = TurnManager->GetActionGauge(TurnActor);
					float OverflowGauge = FMath::Max(0.0f, CurrentGauge - ASPCombatTurnManager::MaxActionGauge);
					TurnManager->SetActionGauge(TurnActor, OverflowGauge);

					if (bIsCurrentTurnInterrupt)
					{
						UE_LOG(LogTemp, Warning, TEXT("[%s] 시간 간섭(추가) 턴 종료! 행동 게이지가 0으로 초기화됩니다."), *TurnActor->GetName());
					}
				}

				// 🌟 턴 종료 시 특수 상태 플래그들은 깔끔하게 초기화
				bIsCurrentTurnInterrupt = false;
				bIsCurrentTurnParry = false;
			}

			// 턴 종료 이벤트 전송 (버프 지속시간 감소 등)
			FGameplayEventData Payload;
			Payload.Instigator = TurnActor;
			ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_TurnEnd, &Payload);
		}
	}

	ProcessEndOfTurn();
}

void AASPCombatGameMode::ReportCharacterReady(AActor* Character)
{
	if (ReadyParticipants.Contains(Character)) return;
	ReadyParticipants.Add(Character);

	if (Character == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		ApplyPlayerSavedData();
	}

	CheckAndStartBattle();
}


void AASPCombatGameMode::EndBattle(bool bPlayerWon)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	// 🌟 1. [가장 먼저 실행] 세이브하기 전에 유물들에게 "전투 끝났으니 버프 다 빼라!" 라고 방송합니다.
	if (PlayerPawn)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				FGameplayEventData EndPayload;
				ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_End, &EndPayload);
				UE_LOG(LogTemp, Warning, TEXT("📢 전투 종료 방송 송출! 유물들이 버프를 초기화합니다."));
			}
		}
	}

	if (bPlayerWon)
	{
		UE_LOG(LogTemp, Warning, TEXT("🎉 전투 승리! 필드로 복귀합니다."));

		if (PlayerPawn)
		{
			UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>();

			if (PlayerPawn && MapManager)
			{
				// 🌟 1. 맵 매니저의 통합 정산기에 장부, 스테이지, 맵 타입을 넘겨서 결과를 받아옵니다!
				FRewardResult TotalReward = MapManager->CalculateCombatRewards(
					DefeatedMonsterRanks,
					MapManager->GetCurrentStage(),
					MapManager->GetCurrentMapType()
				);

				// 🌟 2. 경험치 즉시 지급
				if (ASPGASPlayerCharacter* SPPlayer = Cast<ASPGASPlayerCharacter>(PlayerPawn))
				{
					if (TotalReward.Exp > 0) SPPlayer->AddExperience(TotalReward.Exp);
				}

				// 🌟 3. 재화 대기열(PendingToastRewards)에 안전하게 보관!
				UInventoryComponent* Inv = PlayerPawn->FindComponentByClass<UInventoryComponent>();
				if (Inv)
				{
					auto GiveReward = [&](EResourceType Type, int32 Amount) {
						if (Amount > 0) {
							// 인벤토리에 실시간 지급 (선택사항: 필드 복귀 시점에 지급해도 됨)
							switch (Type) {
							case EResourceType::Gold: Inv->AddMoney(Amount); break;
							case EResourceType::Sand: Inv->AddSand(Amount); break;
							case EResourceType::IncompleteEnergy: Inv->AddIncompleteEnergy(Amount); break;
							case EResourceType::Fragment: Inv->AddFragment(Amount); break;
							}
							// 팝업을 위해 대기열 누적
							int32& SavedAmount = MapManager->PendingToastRewards.FindOrAdd(Type);
							SavedAmount += Amount;
						}
						};

					GiveReward(EResourceType::Gold, TotalReward.Gold);
					GiveReward(EResourceType::Sand, TotalReward.Sand);
					GiveReward(EResourceType::IncompleteEnergy, TotalReward.IncompleteEnergy);
					GiveReward(EResourceType::Fragment, TotalReward.Fragment);
				}

				// ==========================================
				// 🌟 5. 세이브 파일 덮어쓰기
				// ==========================================
				if (USPSaveGameSubsystem* SaveSys = GI->GetSubsystem<USPSaveGameSubsystem>())
				{
					SaveSys->CacheRunDataFromPlayer(PlayerPawn);
					SaveSys->SaveRunToDisk();
					UE_LOG(LogTemp, Log, TEXT("[AutoSave] 전투 보상을 획득하고 게임을 저장했습니다."));
				}
			}

			// ==========================================
			// 🌟 6. 맵 매니저에게 필드 복귀 명령!
			// ==========================================
			if (MapManager)
			{
				MapManager->ReturnToField(true);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("☠️ 전투 패배... 로비로 귀환합니다."));

			if (USPSaveGameSubsystem* SaveSys = GI->GetSubsystem<USPSaveGameSubsystem>())
			{
				SaveSys->ResetRunData();
			}

			if (UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>())
			{
				MapManager->GoToLobby();
			}
		}
	}
}

TArray<TObjectPtr<AActor>> AASPCombatGameMode::GetCurrentEnemies()
{
	TArray<TObjectPtr<AActor>> AliveEnemies;

	// GameMode가 이미 들고 있는 참가자 명단을 순회합니다.
	for (AActor* Participant : AllParticipants)
	{
		// 유효성 및 'Enemy' 태그 검사
		if (IsValid(Participant) && Participant->ActorHasTag(FName("Enemy")))
		{
			// 살아있는지 체력 검사
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Participant))
			{
				UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
				if (ASC && ASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute()) > 0.0f)
				{
					AliveEnemies.Add(Participant);
				}
			}
		}
	}
	return AliveEnemies;
}

void AASPCombatGameMode::RefreshTurnTimelineUI()
{
	if (!TurnManager) return;

	// 🌟 1. 결과를 받아올 빈 변수를 하나 만듭니다.
	int32 CycleEndIndex = -1;

	// 🌟 2. 인자를 2개(예측 개수, 결과 담을 변수) 전달합니다!
	TArray<AActor*> NormalPredicted = TurnManager->PredictTurnOrder(6, CycleEndIndex);

	TArray<AActor*> VIPTurns = TurnManager->GetInterruptQueue();

	if (bIsCurrentTurnInterrupt && CurrentTurnActor)
	{
		VIPTurns.Insert(CurrentTurnActor, 0);
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerPawn->GetController()))
		{
			// 🌟 3. 컨트롤러의 UI 업데이트 이벤트에도 이 인덱스를 같이 넘겨줍니다!
			PC->UpdateTurnTimelineUI(NormalPredicted, VIPTurns, CycleEndIndex);
		}
	}
}

void AASPCombatGameMode::OnCharacterDied(AActor* DeadActor)
{
	if (!DeadActor) return;

	if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(DeadActor))
	{
		if (!Monster->bIsSummonedMinion)
		{
			DefeatedMonsterRanks.Add(Monster->GetEnemyRank());
			UE_LOG(LogTemp, Log, TEXT("[전투 정산] %d 등급 원본 몬스터 처치 기록. (누적: %d마리)"),
				(int32)Monster->GetEnemyRank(), DefeatedMonsterRanks.Num());
		}
		else
		{
			// 소환수라면 장부에 적지 않고 로그만 남깁니다.
			UE_LOG(LogTemp, Warning, TEXT("[전투 정산] 소환된 몬스터(%s) 처치. 보상 장부에서 제외됩니다."), *Monster->GetName());
		}
	}

	bool bIsActionExecuting = false;

	// 현재 턴 주인이 액션(스킬)을 진행 중인지 확인
	if (CurrentTurnActor && CurrentTurnActor != DeadActor)
	{
		UAbilitySystemComponent* CurrentASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentTurnActor);
		if (CurrentASC && CurrentASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_ActionExecuting))
		{
			bIsActionExecuting = true;
		}
	}

	if (bIsActionExecuting)
	{
		// 🟢 [플레이어 스킬 공격 중]
		UE_LOG(LogTemp, Warning, TEXT("[%s] 사망 연출 대기 (현재 액션 진행 중! 오버킬 허용)"), *DeadActor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 즉시 사망 연출 (진행 중인 액션 없음)"), *DeadActor->GetName());

		if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(DeadActor))
		{
			Monster->ExecuteVisualDeath();
		}

		AllParticipants.Remove(DeadActor);
		if (TurnManager) TurnManager->RemoveParticipant(DeadActor);

		if (GetCurrentEnemies().Num() <= 0)
		{
			PlayVictorySequence();
		}
		else if (DeadActor == CurrentTurnActor)
		{
			// 🌟 [수정된 핵심 파트] 상태이상 연출이 진행 중이라면 턴 종료를 잠시 보류(대기)합니다!
			UAbilitySystemComponent* CurrentASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentTurnActor);
			if (CurrentASC && CurrentASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_VisualPlaying))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] 연출 도중 사망! 배틀 디렉터가 연출을 마칠 때까지 턴을 대기합니다."), *DeadActor->GetName());
			}
			else
			{
				// 연출 중이 아니면 원래대로 즉시 턴 강제 스킵!
				EndTurn(CurrentTurnActor);
			}
		}
	}
}

void AASPCombatGameMode::AdvanceBattleTime(float TimePassed)
{
	if (TimePassed <= 0.0f) return;

	// 흐른 시간(행동 수치)을 누적시킵니다.
	PassedTimeInCurrentRound += TimePassed;

	// 누적된 시간이 100(TimePerRound)을 넘어설 때마다 라운드를 올립니다.
	while (PassedTimeInCurrentRound >= TimePerRound)
	{
		PassedTimeInCurrentRound -= TimePerRound; // 100을 빼고 남은 짜투리 시간은 다음 라운드로 이월
		CurrentRound++;

		UE_LOG(LogTemp, Warning, TEXT("⏳ [라운드 진행] %d 라운드가 시작되었습니다!"), CurrentRound);

		// 🚨 최대 제한 라운드(예: 3)를 초과했는지 검사!
		if (CurrentRound > MaxRoundsPerCycle)
		{
			ApplyRoundPenalty();
		}
	}
}

ASPGASMonsterCharacter* AASPCombatGameMode::SummonMonsterMidBattle(USPMonsterData* MinionData)
{
	if (!MinionData || !MinionData->MonsterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Summon] 실패: 몬스터 데이터 에셋이 비어있거나, MonsterClass가 할당되지 않았습니다!"));
		return nullptr;
	}

	int32 EmptyIndex = -1;
	TArray<AActor*> AliveEnemies = GetCurrentEnemies();

	for (int32 i = 0; i < 3; ++i)
	{
		FTransform SpawnTransform = GetSpawnTransformByIndex(i);
		bool bIsOccupied = false;

		// 현재 살아있는 적들의 위치와 스폰 포인트의 거리를 비교해서 자리가 찼는지 확인!
		for (AActor* Enemy : AliveEnemies)
		{
			if (FVector::Dist(Enemy->GetActorLocation(), SpawnTransform.GetLocation()) < 150.0f)
			{
				bIsOccupied = true;
				break;
			}
		}

		// 아무도 안 서 있다면 이 자리가 내 자리!
		if (!bIsOccupied)
		{
			EmptyIndex = i;
			break;
		}
	}

	// 3자리가 꽉 찼다면 소환 실패 (nullptr 반환)
	if (EmptyIndex == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Summon] 빈 자리가 없어 소환이 취소되었습니다."));
		return nullptr;
	}

	// ==========================================
	// 2. 현재 층수에 맞는 레벨 계산하기
	// ==========================================
	int32 SpawnLevel = 1;
	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		SpawnLevel = MapManager->CalculateMonsterLevel(); // 맵 매니저의 만능 계산기 호출!
	}

	// ==========================================
	// 3. 지연 스폰 및 데이터 주입 (BeginPlay 전에 세팅)
	// ==========================================
	FTransform FinalTransform = GetSpawnTransformByIndex(EmptyIndex);

	ASPGASMonsterCharacter* SpawnedMinion = GetWorld()->SpawnActorDeferred<ASPGASMonsterCharacter>(
		MinionData->MonsterClass,
		FinalTransform,
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (SpawnedMinion)
	{
		SpawnedMinion->MonsterDataAsset = MinionData;
		SpawnedMinion->CurrentLevel = SpawnLevel;
		SpawnedMinion->bIsSummonedMinion = true;

		UGameplayStatics::FinishSpawningActor(SpawnedMinion, FinalTransform);

		// ==========================================
		// 4. 전투 명단에 정식 등록!
		// ==========================================
		AllParticipants.Add(SpawnedMinion);

		if (TurnManager)
		{
			// 턴 매니저의 대기열에도 넣어줍니다 
			TurnManager->AddParticipant(SpawnedMinion); 
		}

		if (ActiveBattleDirector)
		{
			ActiveBattleDirector->RegisterMonster(SpawnedMinion); // 배틀 카메라/디렉터에 연결
		}

		// 체력바 UI 등을 띄우기 위해 전투 시작 이벤트를 강제로 불어넣음!
		SpawnedMinion->OnBattleStarted();

		UE_LOG(LogTemp, Warning, TEXT("[Summon] %s (Lv.%d) 소환 완료! (위치: %d번)"),
			*MinionData->MonsterName.ToString(), SpawnLevel, EmptyIndex);

		// 타임라인 UI 즉시 새로고침!
		RefreshTurnTimelineUI();

		return SpawnedMinion;
	}

	return nullptr;
}

void AASPCombatGameMode::ProcessEndOfTurn()
{
	// 1. 사망(State.Death) 태그를 가진 액터 수집
	TArray<AActor*> DeadMonsters;
	for (AActor* Participant : AllParticipants)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Participant))
		{
			if (ASI->GetAbilitySystemComponent()->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Death))
			{
				DeadMonsters.Add(Participant);
			}
		}
	}

	// 2. 명단에서 지우고, 참아왔던 사망 애니메이션 일제히 재생!
	for (AActor* Corpse : DeadMonsters)
	{
		AllParticipants.Remove(Corpse);
		if (TurnManager) TurnManager->RemoveParticipant(Corpse);

		if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(Corpse))
		{
			// 🌟 드디어 여기서 쓰러집니다!
			Monster->ExecuteVisualDeath();
		}
	}

	// 3. 승리 판정
	if (GetCurrentEnemies().Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 적 처치! 승리 시퀀스로 진입합니다."));
		PlayVictorySequence();
		return;
	}

	if (TurnManager)
	{
		TurnManager->SetRoundIterating(false);
	}

	// 4. 다음 타자 호출
	if (TurnManager)
	{
		if (AActor* VIPActor = TurnManager->PopInterruptActor())
		{
			bIsCurrentTurnInterrupt = true;
			StartTurn(VIPActor);
			return;
		}

		AActor* NextActor = TurnManager->CalculateNextTurn();
		if (TurnManager->GetActionGauge(NextActor) < ASPCombatTurnManager::MaxActionGauge)
		{
			bIsCurrentTurnInterrupt = true;
		}
		StartTurn(NextActor);
	}
}

void AASPCombatGameMode::ApplyRoundPenalty()
{
	UE_LOG(LogTemp, Error, TEXT("제한 라운드(%d) 초과! 시간의 힘을 %f 소모합니다."), MaxRoundsPerCycle, PenaltyTPCost);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			float CurrentTP = ASC->GetNumericAttribute(USPGASAttributeSet::GetTimePowerAttribute());
			float NewTP = FMath::Max(0.0f, CurrentTP - PenaltyTPCost);

			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), NewTP);

			// 사이클 초기화
			CurrentRound = 1;
			PassedTimeInCurrentRound = 0.0f;
			UE_LOG(LogTemp, Error, TEXT("🚨 사이클 종료! 시간의 힘 %f 소모. 신규 사이클 시작."), PenaltyTPCost);
		}
	}
}

FTransform AASPCombatGameMode::GetSpawnTransformByIndex(int32 Index)
{
	// 맵에 배치된 모든 스폰 포인트 검색
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASPGASSpawnPoint::StaticClass(), SpawnPoints);

	for (AActor* Actor : SpawnPoints)
	{
		if (ASPGASSpawnPoint* SP = Cast<ASPGASSpawnPoint>(Actor))
		{
			if (SP->SpawnIndex == Index)
			{
				return SP->GetActorTransform();
			}
		}
	}

	// 못 찾았을 경우 기본값
	UE_LOG(LogTemp, Warning, TEXT("스폰 포인트 [%d]를 찾지 못했습니다."), Index);
	return FTransform(FRotator::ZeroRotator, FVector(Index * 200.0f, 0.0f, 100.0f));
}
