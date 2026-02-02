// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ASPCombatGameMode.h"
#include "SubSystem/SPCombatSubsystem.h"      
#include "Data/CombatEncounterData.h"      
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

void AASPCombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. 서브시스템 가져오기
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USPCombatSubsystem* CombatSys = GI->GetSubsystem<USPCombatSubsystem>();
	if (!CombatSys) return;

	// 2. 맡겨둔 전투 데이터 꺼내오기
	const UCombatEncounterData* EncounterData = CombatSys->GetPendingEncounter();

	if (EncounterData)
	{
		UE_LOG(LogTemp, Log, TEXT("⚔️ 전투 모드 시작! 적 그룹 소환 중..."));

		// 3. 적 그룹 순회하며 소환
		for (const FEnemySpawnInfo& EnemyInfo : EncounterData->EnemyGroup)
		{
			if (EnemyInfo.EnemyClass)
			{
				// 위치 계산 (스폰 포인트 찾기)
				FTransform SpawnTransform = GetSpawnTransformByIndex(EnemyInfo.SpawnPositionIndex);

				// 몬스터 스폰!
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				GetWorld()->SpawnActor<AActor>(EnemyInfo.EnemyClass, SpawnTransform, SpawnParams);
			}
		}

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			APawn* PlayerPawn = PC->GetPawn();
			if (PlayerPawn)
			{
				// 1. "SpawnPoint_Player" 태그를 가진 액터 찾기
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SpawnPoint_Player"), FoundActors);

				if (FoundActors.Num() > 0)
				{
					AActor* PlayerSpot = FoundActors[0];

					// 2. 플레이어를 그 위치로 순간이동 & 회전
					PlayerPawn->SetActorLocationAndRotation(
						PlayerSpot->GetActorLocation(),
						PlayerSpot->GetActorRotation(),
						false, nullptr, ETeleportType::ResetPhysics
					);

					UE_LOG(LogTemp, Log, TEXT("플레이어 전투 위치로 이동 완료!"));
				}
			}
		}

		// 4. (선택) 선공 여부에 따른 처리
		ECombatAdvantage Advantage = CombatSys->GetAdvantageState();
		if (Advantage == ECombatAdvantage::PlayerAdvantage)
		{
			UE_LOG(LogTemp, Warning, TEXT("⚡ 플레이어 선공! (적 체력 감소 or 턴 우선권)"));
			// 여기에 적들 체력을 깎거나, 플레이어 턴 게이지를 채워주는 로직 추가
		}
		else if (Advantage == ECombatAdvantage::EnemyAdvantage)
		{
			UE_LOG(LogTemp, Warning, TEXT("💀 적 기습! (플레이어 경직)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ 전달받은 전투 데이터가 없습니다. (테스트 맵인가요?)"));
	}
}

FTransform AASPCombatGameMode::GetSpawnTransformByIndex(int32 Index)
{
	// 맵에 미리 배치된 'TargetPoint' 같은 액터들을 찾아서 위치를 반환합니다.
	// 간단하게 태그("SpawnPoint_0", "SpawnPoint_1"...)를 사용한다고 가정합니다.

	FString TagName = FString::Printf(TEXT("SpawnPoint_%d"), Index);
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*TagName), FoundActors);

	if (FoundActors.Num() > 0)
	{
		return FoundActors[0]->GetActorTransform();
	}

	// 못 찾으면 대충 (0,0,0)이나 기본 위치 반환
	return FTransform(FRotator::ZeroRotator, FVector(Index * 200.0f, 0.0f, 100.0f));
}
