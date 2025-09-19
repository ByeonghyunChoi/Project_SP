#include "Component/FieldModeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerCharacter.h" 
#include "Character/MonsterCharacter.h"
#include "Core/BattleManager.h"
#include "Engine/TargetPoint.h"
#include "Component/ActionComponent.h"
#include "Component/AttributesComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UFieldModeComponent::UFieldModeComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UFieldModeComponent::BeginPlay()
{
    Super::BeginPlay();

}


void UFieldModeComponent::StartAttackSequence()
{
    APlayerCharacter* OwningPlayer = Cast<APlayerCharacter>(GetOwner());
    if (!OwningPlayer || bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("UFieldModeComponent: OwningPlayer 또는 BasicStats가 유효하지 않거나 이미 공격 중입니다."));
        return;
    }

    bIsAttacking = true;
    bIsMoving = false;
    OwningPlayer->GetCharacterMovement()->bOrientRotationToMovement = false;

    ACharacter* Character = OwningPlayer;
    APlayerController* PlayerController = Character ? Cast<APlayerController>(Character->GetController()) : nullptr;
    if (PlayerController && Character)
    {
        FHitResult HitResult;
        if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
        {
            FVector MouseLocation = HitResult.Location;
            FVector CharacterLocation = Character->GetActorLocation();
            FVector DirectionToMouse = MouseLocation - CharacterLocation;
            DirectionToMouse.Z = 0.0f;
            DirectionToMouse.Normalize();
            FRotator TargetRotation = DirectionToMouse.Rotation();
            Character->SetActorRotation(TargetRotation);
        }
    }
}

AMonsterCharacter* UFieldModeComponent::PerformAttackHitDetection()
{
    APlayerCharacter* OwningPlayer = Cast<APlayerCharacter>(GetOwner());
    if (!OwningPlayer) return nullptr;

    FVector StartLocation = OwningPlayer->GetActorLocation();
    FVector EndLocation = StartLocation + OwningPlayer->GetActorForwardVector() * AttackRange;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwningPlayer);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, Params);

    if (bHit && HitResult.GetActor())
    {
        AMonsterCharacter* HitMonster = Cast<AMonsterCharacter>(HitResult.GetActor());
        if (HitMonster->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Log, TEXT("UFieldModeComponent: 몬스터 %s와 충돌!"), *HitMonster->GetName());
            return HitMonster; // 충돌한 몬스터 반환
        }
    }
    return nullptr; // 몬스터와 충돌하지 않음
}

void UFieldModeComponent::OnAttackAnimationFinished()
{
    APlayerCharacter* OwningPlayer = Cast<APlayerCharacter>(GetOwner());
    if (OwningPlayer)
    {
        // 1. 이동 방향 회전 다시 활성화
        OwningPlayer->GetCharacterMovement()->bOrientRotationToMovement = true;
        // 2. 공격 중 플래그 해제
        bIsAttacking = false;
        bIsMoving = true;
        UE_LOG(LogTemp, Log, TEXT("공격 애니메이션 종료, 이동 방향 회전 복귀."));
    }
}

void UFieldModeComponent::StartBattleTransition(AMonsterCharacter* HitMonster)
{

    if (bIsInBattle || !HitMonster) return;
    bIsInBattle = true;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // 전투할 몬스터 정보와 플레이어의 현재 위치를 저장
    MonsterToBattle = HitMonster;
    LastFieldLocation = OwnerCharacter->GetActorLocation();

    // 필드에 있는 플레이어 캐릭터 숨기기
    OwnerCharacter->SetActorHiddenInGame(true);
    OwnerCharacter->SetActorEnableCollision(false);
    OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();

    // TODO: 화면 전환 연출 시작 (예: UMG로 화면을 검게 암전)

    // 전투 맵(서브레벨)을 비동기적으로 로딩 시작
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnBattleArenaLoaded");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = FMath::Rand();

    UGameplayStatics::LoadStreamLevel(this, BattleArenaMapName, true, false, LatentInfo);
}

void UFieldModeComponent::OnBattleArenaLoaded()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // 태그를 이용해 '전투 무대' Target Point 액터 찾기
    TArray<AActor*> BattleStageActors;
    UGameplayStatics::GetAllActorsWithTag(this, BattleStageTag, BattleStageActors);

    if (BattleStageActors.Num() > 0)
    {
        // 플레이어를 전투 무대로 순간이동시키고 다시 보이게 함
        OwnerCharacter->SetActorLocation(BattleStageActors[0]->GetActorLocation());
        OwnerCharacter->SetActorHiddenInGame(false);
    }

    // BattleManager를 찾아 전투 시작 명령
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(this, ABattleManager::StaticClass()));
    if (BattleManager && MonsterToBattle.IsValid())
    {
        // 플레이어 파티와 몬스터 파티 정보를 구성하여 전달
        TArray<ACombatPawn*> PlayerParty = { Cast<ACombatPawn>(OwnerCharacter) };
        TArray<ACombatPawn*> EnemyParty;
        UMonsterGroupObject* MonsterGroup = MonsterToBattle->GetCombatMonsterGroup();
        if (MonsterGroup)
        {
            // 1. 몬스터 그룹에서 스폰할 모든 몬스터 데이터를 가져옵니다.
            TArray<FMonsterData> MonstersToSpawn = MonsterGroup->GetAllMonsterDataInGroup();

            // 2. 전투 무대를 기준으로 몬스터들을 배치할 위치를 계산합니다.
            FVector SpawnOrigin = BattleStageActors[0]->GetActorLocation();
            FRotator SpawnRotation = OwnerCharacter->GetActorRotation().GetInverse(); // 플레이어를 바라보도록

            for (int32 i = 0; i < MonstersToSpawn.Num(); ++i)
            {
                FVector SpawnLocation = SpawnOrigin + FVector(500.f, i * 200.f - 100.f, 0.f); // 예시 위치

                // 3. 몬스터를 월드에 스폰합니다.
                AMonsterCharacter* SpawnedMonster = GetWorld()->SpawnActor<AMonsterCharacter>(MonstersToSpawn[i].MonsterClass, SpawnLocation, SpawnRotation);
                if (SpawnedMonster)
                {
                    // 1. AttributesComponent 초기화
                    if (UAttributesComponent* AttrComp = SpawnedMonster->GetAttributesComponent())
                    {
                        // 데이터 테이블에서 읽어올 RowName을 지정
                        AttrComp->SetCharacterID(MonstersToSpawn[i].CharacterStatsRowName);
                        // AttributesComponent가 스스로 데이터를 로드하도록 초기화 함수 호출
                        AttrComp->InitializeAttributes();
                    }

                    // 2. ActionComponent 초기화
                    if (UActionComponent* ActionComp = SpawnedMonster->GetActionComponent())
                    {
                        // 몬스터 데이터에 정의된 스킬 목록으로 ActionComponent를 초기화
                        ActionComp->InitializeDefaultActions(MonstersToSpawn[i].AvailableActionIDs);
                    }

                    // 3. 기타 몬스터 데이터 설정
                    SpawnedMonster->SetWeaknessType(MonstersToSpawn[i].WeaknessType);

                    EnemyParty.Add(SpawnedMonster);
                }
            }
        }

        BattleManager->StartBattle(PlayerParty, EnemyParty);
    }
}

void UFieldModeComponent::EndBattleTransition()
{
    if (!bIsInBattle) return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // 전투 공간에 있는 플레이어 캐릭터 다시 숨기기
    OwnerCharacter->SetActorHiddenInGame(true);

    // TODO: 필드 복귀 연출 시작 (예: UMG 화면 암전)

    // 전투 맵(서브레벨)을 언로딩 시작
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnBattleArenaUnloaded");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = FMath::Rand();

    UGameplayStatics::UnloadStreamLevel(this, BattleArenaMapName, LatentInfo, false);
}

void UFieldModeComponent::OnBattleArenaUnloaded()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // 플레이어를 원래 위치로 복귀
    OwnerCharacter->SetActorLocation(LastFieldLocation);

    // 필드의 플레이어 캐릭터 다시 표시 및 조작 가능하게 설정
    OwnerCharacter->SetActorHiddenInGame(false);
    OwnerCharacter->SetActorEnableCollision(true);

    bIsInBattle = false;
    MonsterToBattle = nullptr;
}

