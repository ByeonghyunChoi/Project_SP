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
#include "Character/SPGASCharacterBase.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Component/SPStatusEffectComponent.h"
#include "Character/SPGASPlayerController.h"


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

		// 2. 적 스폰
		for (const FEnemySpawnInfo& Info : EncounterData->EnemyGroup)
		{
			if (!Info.EnemyClass) continue;

			// 스폰 포인트 위치 찾기
			FTransform SpawnTransform = GetSpawnTransformByIndex(Info.SpawnPositionIndex);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(Info.EnemyClass, SpawnTransform, SpawnParams);
			if (NewEnemy)
			{
				SpawnedEnemies.Add(NewEnemy);
			}
		}

		// 3. 플레이어 이동 (SpawnPoint 중 'PlayerStart' 태그가 있는 곳, 혹은 별도 로직)
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

	// 4. 전투 시스템 초기화
	if (PlayerPawn)
	{
		InitializeBattle(SpawnedEnemies, PlayerPawn);
	}
}

void AASPCombatGameMode::InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player)
{
	// 1. 턴 매니저 생성
	if (TurnManagerClass && !TurnManager)
	{
		TurnManager = GetWorld()->SpawnActor<ASPCombatTurnManager>(TurnManagerClass);
	}

	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("TurnManager 생성 실패! BP_CombatGameMode에 클래스가 할당되었는지 확인하세요."));
		return;
	}

	// 2. 참가자 등록
	AllParticipants.Empty();
	AllParticipants.Add(Player);
	AllParticipants.Append(Enemies);

	TurnManager->InitializeParticipants(AllParticipants);

	// 3. 선제공격(Advantage) 처리
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

	// 1. [UI 출력] 플레이어 컨트롤러에게 명령!
	for (AActor* Participant : AllParticipants)
	{
		if (ASPGASCharacterBase* Character = Cast<ASPGASCharacterBase>(Participant))
		{
			// 다형성(Polymorphism) 폭발! 
			// 플레이어면 컨트롤러 UI가 켜지고, 몬스터면 머리 위 위젯이 켜집니다.
			Character->OnBattleStarted();
		}
	}

	// 2. [전투 시작] 턴 매니저에게 첫 턴을 물어보고 시작!
	if (TurnManager)
	{
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
			

			if (StatusComp)
			{
				StatusComp->ProcessTurnStartDoT();
				StatusComp->ReduceStatusEffectTurns();
			}

			if (ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_SkipTurn))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] 혼절 상태! 턴을 강제로 넘깁니다."), *TurnActor->GetName());

				// 태그 지워주기 (1회용이므로)
				ASC->RemoveLooseGameplayTag(FSPGameplayTags::Get().State_Status_SkipTurn);

				// 바로 턴 종료시켜버리기
				EndTurn(TurnActor);
				return;
			}

			// 턴 활성화 태그 부여 (PlayerController가 입력을 받기 시작함)
			ASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

			// 턴 시작 이벤트 전송
			FGameplayEventData Payload;
			Payload.Instigator = TurnActor;
			ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_TurnStart, &Payload);
		}
	}

	if (TurnManager)
	{
		TArray<AActor*> PredictedOrder = TurnManager->PredictTurnOrder(6);

		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerPawn->GetController()))
			{
				PC->UpdateTurnTimelineUI(PredictedOrder);
			}
		}
	}

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
			// [중요] 턴 활성화 태그 제거 (입력 차단)
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
				TurnManager->SetActionGauge(TurnActor, 0.0f);
			}

			// 턴 종료 이벤트 전송 (버프 지속시간 감소 등)
			FGameplayEventData Payload;
			Payload.Instigator = TurnActor;
			ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_TurnEnd, &Payload);
		}
	}

	// 다음 턴 계산 요청
	if (TurnManager)
	{
		AActor* NextActor = TurnManager->CalculateNextTurn();
		StartTurn(NextActor);
	}
}

void AASPCombatGameMode::ReportCharacterReady(AActor* Character)
{
	if (ReadyParticipants.Contains(Character)) return;
	ReadyParticipants.Add(Character);
	CheckAndStartBattle();
}

void AASPCombatGameMode::OnCharacterDied(AActor* DeadActor)
{
	if (TurnManager)
	{
		TurnManager->RemoveParticipant(DeadActor);
	}

	// 2. 월드에 남은 "Enemy"가 몇 마리인지 셉니다.
	TArray<AActor*> RemainingEnemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), RemainingEnemies);

	int32 AliveEnemiesCount = 0;
	for (AActor* Enemy : RemainingEnemies)
	{
		// 아직 삭제(Destroy) 대기 중인 방금 죽은 애는 빼고 셉니다.
		if (Enemy != DeadActor && IsValid(Enemy))
		{
			AliveEnemiesCount++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("남은 적 수: %d"), AliveEnemiesCount);

	// 3. 남은 적이 0마리라면? 플레이어 승리!
	if (AliveEnemiesCount <= 0)
	{
		EndBattle(true);
	}
}

void AASPCombatGameMode::EndBattle(bool bPlayerWon)
{
	if (bPlayerWon)
	{
		UE_LOG(LogTemp, Warning, TEXT("🎉 전투 승리! 필드로 복귀합니다."));

		UGameInstance* GI = GetGameInstance();
		if (!GI) return;

		// 1. 플레이어의 '전투 후 체력/스탯'을 세이브 시스템에 덮어씌워서 저장!
		// (이걸 안 하면 필드로 돌아갈 때 전투 전 풀피 상태로 돌아가 버립니다)
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr)
		{
			if (USPSaveGameSubsystem* SaveSys = GI->GetSubsystem<USPSaveGameSubsystem>())
			{
				SaveSys->RestoreRunDataToPlayer(PlayerPawn);
			}
		}

		// 2. 🌟 맵 매니저에게 필드 복귀 명령!
		// (선생님이 짜두신 ReturnToField -> LoadStageLevel -> OnPostLoadMapWithWorld 가 연쇄적으로 작동합니다)
		if (UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>())
		{
			MapManager->ReturnToField(true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("전투 패배... 게임 오버 화면을 띄웁니다."));
		// 게임 오버 처리
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
