// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldModeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h"
#include "PlayerCharacter.h" 
#include "MonsterCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UFieldModeComponent::UFieldModeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFieldModeComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UFieldModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFieldModeComponent::StartAttackSequence()
{
    APlayerCharacter* OwningPlayer = Cast<APlayerCharacter>(GetOwner());
    if (!OwningPlayer || !OwningPlayer->GetCombatData() || bIsAttacking)
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
        if (HitMonster && HitMonster->GetCombatData())
        {
            UE_LOG(LogTemp, Log, TEXT("UFieldModeComponent: 몬스터 %s와 충돌!"), *HitMonster->GetName());
            return HitMonster; // 충돌한 몬스터 반환
        }
    }
    return nullptr; // 몬스터와 충돌하지 않음
}

void UFieldModeComponent::StartBattleTransition(AMonsterCharacter* HitMonster)
{
    APlayerCharacter* OwningPlayer = Cast<APlayerCharacter>(GetOwner());
    if (!OwningPlayer || !OwningPlayer->GetCombatData() || !HitMonster || !HitMonster->GetCombatData())
    {
        UE_LOG(LogTemp, Warning, TEXT("UFieldModeComponent: 전투 전환에 필요한 정보가 부족합니다."));
        return;
    }

    // UGameInstance를 가져오는 올바른 방법 (UActorComponent에서)
    UMyGameInstance* MyGameInstance = nullptr;
    if (GetWorld()) // 컴포넌트가 유효한 월드에 속해 있는지 확인
    {
        MyGameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
    }

    if (MyGameInstance)
    {
        FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
        MyGameInstance->StartBattleTransition(
            OwningPlayer,      // 플레이어 액터 자체를 넘겨 클래스와 데이터를 추출하도록 함
            HitMonster,        // 몬스터 액터 자체를 넘겨 클래스와 데이터를 추출하도록 함
            CurrentLevelName
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UFieldModeComponent: UMyGameInstance를 찾을 수 없습니다!"));
    }
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

