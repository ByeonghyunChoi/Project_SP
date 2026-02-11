// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ASPCombatGameMode.h"
#include "SubSystem/SPCombatSubsystem.h"      
#include "Data/CombatEncounterData.h"      
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Manager/SPCombatTurnManager.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h" 
#include "Tag/SPGameplayTags.h"


AASPCombatGameMode::AASPCombatGameMode()
{
}

void AASPCombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	//서브시스템 가져오기
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USPCombatSubsystem* CombatSys = GI->GetSubsystem<USPCombatSubsystem>();
	if (!CombatSys) return;

	TArray<AActor*> SpawnedEnemies;
	APawn* PlayerPawn = nullptr;

	//전투 데이터 가져오기
	const UCombatEncounterData* EncounterData = CombatSys ? CombatSys->GetPendingEncounter() : nullptr;

	if (EncounterData)
	{
		UE_LOG(LogTemp, Log, TEXT("전투 모드 시작!"));

		// A. 적 스폰
		for (const FEnemySpawnInfo& EnemyInfo : EncounterData->EnemyGroup)
		{
			if (EnemyInfo.EnemyClass)
			{
				FTransform SpawnTransform = GetSpawnTransformByIndex(EnemyInfo.SpawnPositionIndex);
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(EnemyInfo.EnemyClass, SpawnTransform, SpawnParams);
				if (NewEnemy)
				{
					SpawnedEnemies.Add(NewEnemy);
				}
			}
		}

		// B. 플레이어 이동
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PlayerPawn = PC->GetPawn();
			if (PlayerPawn)
			{
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SpawnPoint_Player"), FoundActors);

				if (FoundActors.Num() > 0)
				{
					AActor* PlayerSpot = FoundActors[0];
					PlayerPawn->SetActorLocationAndRotation(
						PlayerSpot->GetActorLocation(),
						PlayerSpot->GetActorRotation(),
						false, nullptr, ETeleportType::ResetPhysics
					);

					if (ASPGASPlayerCharacter* SPPlayer = Cast<ASPGASPlayerCharacter>(PlayerPawn))
					{
						SPPlayer->SetCameraProfile(SPPlayer->GetCombatCameraProfile());
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("전달받은 전투 데이터가 없습니다."));
		// 테스트용: 이미 맵에 배치된 적이 있다면 찾아서 목록에 넣음 (개발 편의성)
		// UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASPGASMonsterCharacter::StaticClass(), SpawnedEnemies);
		// PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	}

	// 턴 시스템 초기화 및 시작
	if (PlayerPawn)
	{
		InitializeBattle(SpawnedEnemies, PlayerPawn);
	}
}

void AASPCombatGameMode::StartTurn(AActor* TurnActor)
{
	if (!TurnActor) return;

	CurrentTurnActor = TurnActor;
	UE_LOG(LogTemp, Log, TEXT("턴 시작: %s"), *TurnActor->GetName());

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			//TurnActive 태그 부여 (입력 허용, UI 활성화)
			ASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

			//턴 시작 이벤트 (지속 피해, 버프 틱 등)
			FGameplayEventData Payload;
			Payload.Instigator = TurnActor;
			ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_TurnStart, &Payload);
		}
	}
}

void AASPCombatGameMode::EndTurn(AActor* TurnActor)
{
	if (!TurnActor) return;

	UE_LOG(LogTemp, Log, TEXT("🛑 턴 종료: %s"), *TurnActor->GetName());

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TurnActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// TurnActive 태그 제거 (입력 차단)
			ASC->RemoveLooseGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

			// 행동 게이지 리셋 (0으로 초기화)
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), 0.0f);

			// 턴 종료 이벤트 (쿨타임 감소, 버프 지속시간 감소)
			FGameplayEventData Payload;
			Payload.Instigator = TurnActor;
			ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_TurnEnd, &Payload);
		}
	}

	// 승패 판정 (전멸 여부 확인) -> 나중에 구현
	// CheckBattleState();

	// 다음 턴 계산 요청
	if (TurnManager)
	{
		AActor* NextActor = TurnManager->CalculateNextTurn();
		StartTurn(NextActor);
	}
}

FTransform AASPCombatGameMode::GetSpawnTransformByIndex(int32 Index)
{
	FString TagName = FString::Printf(TEXT("SpawnPoint_%d"), Index);
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*TagName), FoundActors);

	if (FoundActors.Num() > 0)
	{
		return FoundActors[0]->GetActorTransform();
	}

	return FTransform(FRotator::ZeroRotator, FVector(Index * 200.0f, 0.0f, 100.0f));
}

void AASPCombatGameMode::InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player)
{
	// 턴 매니저 생성
	if (TurnManagerClass)
	{
		TurnManager = GetWorld()->SpawnActor<ASPCombatTurnManager>(TurnManagerClass);
	}

	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT(" TurnManager 생성 실패! BP_CombatGameMode에서 클래스를 지정했는지 확인하세요."));
		return;
	}

	// 참가자 목록 구성
	AllParticipants.Empty();
	AllParticipants.Add(Player);
	AllParticipants.Append(Enemies);

	// 선공(Advantage) 처리: 행동 게이지 100% 보정
	if (USPCombatSubsystem* CombatSys = GetGameInstance()->GetSubsystem<USPCombatSubsystem>())
	{
		ECombatAdvantage Advantage = CombatSys->GetAdvantageState();

		if (Advantage == ECombatAdvantage::PlayerAdvantage)
		{
			// 플레이어 선공
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Player))
			{
				if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
				{
					ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), ASPCombatTurnManager::MaxActionGauge);
					UE_LOG(LogTemp, Log, TEXT("플레이어 선공!"));
				}
			}
		}
		else if (Advantage == ECombatAdvantage::EnemyAdvantage)
		{
			// 적 기습 (모든 적에게 적용)
			for (AActor* Enemy : Enemies)
			{
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Enemy))
				{
					if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
					{
						ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), ASPCombatTurnManager::MaxActionGauge);
					}
				}
			}
			UE_LOG(LogTemp, Log, TEXT("적 기습!"));
		}
	}

	//턴 매니저 시작
	TurnManager->InitializeParticipants(AllParticipants);

	//첫 번째 턴 계산
	AActor* FirstActor = TurnManager->CalculateNextTurn();
	StartTurn(FirstActor);
}
