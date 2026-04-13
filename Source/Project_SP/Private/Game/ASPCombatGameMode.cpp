// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ASPCombatGameMode.h"
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
	if (bIsBattleInitialized && ReadyParticipants.Num() >= TotalExpectedParticipants)
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

	USPStatusEffectComponent* StatusComp = TurnActor->FindComponentByClass<USPStatusEffectComponent>();

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
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
							for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
							{
								if (Spec.Ability && Spec.Ability->GetClass() == WeaponData->ParrySkillAbility)
								{
									bool bActivated = ASC->TryActivateAbility(Spec.Handle);
									if (bActivated)
									{
										UE_LOG(LogTemp, Warning, TEXT("데이터 에셋 기반 반격기 발동 완료"));
									}
									else
									{
										EndTurn(TurnActor);
									}
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				bIsCurrentTurnParry = false;
			}

			if (StatusComp)
			{
				StatusComp->ProcessTurnStartDoT();
				StatusComp->ReduceStatusEffectTurns();
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
		Character->ReduceCooldowns();
	}
}

void AASPCombatGameMode::EndTurn(AActor* TurnActor)
{
	if (!TurnActor) return;

	// 안전장치: 현재 턴 주인이 아닌데 종료를 요청하면 무시
	if (CurrentTurnActor != TurnActor) return;

	UE_LOG(LogTemp, Log, TEXT("턴 종료: %s"), *TurnActor->GetName());

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// 턴 활성화 태그 제거 (입력 차단)
			ASC->RemoveLooseGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

			if (Cast<ASPGASPlayerCharacter>(TurnActor))
			{
				if (TurnEndTimeCostGE)
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
				else
				{
					UE_LOG(LogTemp, Error, TEXT("TurnEndTimeCostGE가 게임모드 블루프린트에 설정되지 않았습니다!"));
				}
			}

			// 행동 게이지 0으로 초기화
			if (TurnManager)
			{
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

		// 1. 플레이어의 '전투 후 체력/스탯'을 세이브 시스템에 덮어씌워서 저장!
		if (PlayerPawn)
		{
			if (USPSaveGameSubsystem* SaveSys = GI->GetSubsystem<USPSaveGameSubsystem>())
			{
				// Cache(메모리 덮어쓰기)
				SaveSys->CacheRunDataFromPlayer(PlayerPawn);

				// 안전하게 하드디스크에 한 번 구워줍니다.
				SaveSys->SaveRunToDisk();
			}
		}

		// 맵 매니저에게 필드 복귀 명령! (보상 상자 상태로 맵을 염)
		if (UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>())
		{
			MapManager->ReturnToField(true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("☠️ 전투 패배... 로비로 귀환합니다."));

		// [패배 처리] 런 데이터(유물, 진행도 등)를 싹 날려버립니다.
		if (USPSaveGameSubsystem* SaveSys = GI->GetSubsystem<USPSaveGameSubsystem>())
		{
			SaveSys->ResetRunData();
		}

		// 맵 매니저를 통해 로비 맵으로 강제 이동
		if (UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>())
		{
			MapManager->GoToLobby();
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

	TArray<AActor*> NormalPredicted = TurnManager->PredictTurnOrder(6);
	TArray<AActor*> VIPTurns = TurnManager->GetInterruptQueue();

	// 🌟 [핵심 변경 2: UI 마법] 현재 행동 중인 턴이 VIP 턴이라면?
	// 이미 큐에서 뽑혔지만, 타임라인 0번 자리를 차지해야 하므로 배열 맨 앞(0번)에 강제로 끼워 넣습니다!
	if (bIsCurrentTurnInterrupt && CurrentTurnActor)
	{
		VIPTurns.Insert(CurrentTurnActor, 0);
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerPawn->GetController()))
		{
			PC->UpdateTurnTimelineUI(NormalPredicted, VIPTurns);
		}
	}
}

void AASPCombatGameMode::ProcessEndOfTurn()
{
	// 사망(State.Death) 태그를 가진 액터 수집
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

	// 명단에서 지우고 삭제 예약
	for (AActor* Corpse : DeadMonsters)
	{
		AllParticipants.Remove(Corpse);
		if (TurnManager) TurnManager->RemoveParticipant(Corpse);

		if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(Corpse))
		{
			if (Monster->DeathMontage)
			{
				Monster->PlayAnimMontage(Monster->DeathMontage);
			}

			float DeathDuration = Monster->GetDeathMontageDuration();
			Corpse->SetLifeSpan(DeathDuration + 0.1f);
		}
	}

	// 남은 적군 수 확인
	if (GetCurrentEnemies().Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 적 처치! 승리 시퀀스로 진입합니다."));

		// 즉시 EndBattle을 부르지 않고, 블루프린트 승리 연출로 넘깁니다.
		PlayVictorySequence();
		return;
	}

	// 아직 적이 남았다면 대기열에서 다음 타자 호출
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
