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
#include "Blueprint/UserWidget.h" 
#include "Animation/WidgetAnimation.h" 
#include "TimerManager.h" 
#include "Blueprint/WidgetBlueprintGeneratedClass.h" 
#include "Component/CombatCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

// FieldModeComponent.cpp

void UFieldModeComponent::StartBattleTransition(AMonsterCharacter* HitMonster)
{
    if (bIsInBattle || !HitMonster) return;
    bIsInBattle = true;
    MonsterToBattle = HitMonster;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;
    LastFieldLocation = OwnerCharacter->GetActorLocation();
    OwnerCharacter->SetActorHiddenInGame(true);
    OwnerCharacter->SetActorEnableCollision(false);
    OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();

    if (TransitionWidgetClass)
    {
        TransitionWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TransitionWidgetClass);
        if (TransitionWidgetInstance)
        {
            TransitionWidgetInstance->AddToViewport();

            UWidgetAnimation* FadeInAnimation = nullptr;
            UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(TransitionWidgetInstance->GetClass());

            // --- 디버깅 코드 시작 ---
            if (WidgetClass)
            {
                UE_LOG(LogTemp, Warning, TEXT("--- Checking Animations in WBP_BattleTransition ---"));

                // 위젯에 있는 모든 애니메이션의 이름을 출력합니다.
                for (UWidgetAnimation* Anim : WidgetClass->Animations)
                {
                    if (Anim)
                    {
                        UE_LOG(LogTemp, Display, TEXT("Found Animation in Widget: [%s]"), *Anim->GetFName().ToString());
                    }
                }
                UE_LOG(LogTemp, Warning, TEXT("--- Finished Checking Animations ---"));

                // 기존의 애니메이션 찾는 로직
                for (UWidgetAnimation* Anim : WidgetClass->Animations)
                {
                    if (Anim && Anim->GetFName().ToString().StartsWith(TEXT("FadeIn")))
                    {
                        FadeInAnimation = Anim;
                        break;
                    }
                }
            }
            // --- 디버깅 코드 끝 ---

            if (FadeInAnimation)
            {
                UE_LOG(LogTemp, Warning, TEXT("FadeIn Animation FOUND! Playing animation."));
                TransitionWidgetInstance->PlayAnimation(FadeInAnimation);
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(
                    TimerHandle, this, &UFieldModeComponent::StartLoadingBattleMap, FadeInAnimation->GetEndTime(), false
                );
                return;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("CRITICAL ERROR: 'FadeIn' Animation NOT FOUND in WBP_BattleTransition!"));
                StartLoadingBattleMap();
            }
        }
    }
    else
    {
        StartLoadingBattleMap();
    }
}

void UFieldModeComponent::OnBattleArenaLoaded()
{
    UE_LOG(LogTemp, Warning, TEXT("======= OnBattleArenaLoaded HAS BEEN CALLED! ======="));
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // 태그를 이용해 '전투 무대' Target Point 액터 찾기
    TArray<AActor*> BattleStageActors;
    UGameplayStatics::GetAllActorsWithTag(this, BattleStageTag, BattleStageActors);

    if (BattleStageActors.Num() > 0)
    {
        // 플레이어를 전투 무대로 순간이동시키고 다시 보이게 함
        OwnerCharacter->SetActorLocation(BattleStageActors[0]->GetActorLocation());
        const FRotator TargetRotation = FRotator::ZeroRotator;

        OwnerCharacter->SetActorRotation(TargetRotation);
        OwnerCharacter->SetActorHiddenInGame(false);
    }

    // BattleManager를 찾아 전투 시작 명령
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(this, ABattleManager::StaticClass()));
    if (BattleManager && MonsterToBattle.IsValid())
    {
        if (UCombatCameraComponent* CameraComp = BattleManager->GetCameraComponent())
        {
            // 1. "Battle_Default_Cam" 태그를 가진 카메라를 찾아 제어권을 넘겨받으라고 명령
            CameraComp->InitializeCamera(TEXT("Battle_Default_Cam"));
            CameraComp->PlayDefaultShot(OwnerCharacter, MonsterToBattle.Get());
        }

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

            //몬스터 배치 숫자를 계산하여 중심 인덱스 위치를 구함.
            const int32 TotalMonsters = MonstersToSpawn.Num();
            const float CenterIndex = (TotalMonsters - 1) / 2.0f;

            const float SideSpacing = 300.0f;       // 몬스터 간의 '좌우' 간격
            const float DepthSpacing = 50.0f;      // 몬스터 간의 '앞뒤' 간격
            const float BaseForwardDistance = 650.0f; // 기본 전방 거리

            for (int32 i = 0; i < MonstersToSpawn.Num(); ++i)
            {
                const float Y_Offset = (i - CenterIndex) * SideSpacing;
                const float X_Offset = BaseForwardDistance + FMath::Abs(i - CenterIndex) * DepthSpacing;

                FVector SpawnLocation = SpawnOrigin + FVector(X_Offset, Y_Offset, 0.f);

                // 3. 몬스터를 월드에 스폰합니다.
                const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, OwnerCharacter->GetActorLocation());
                AMonsterCharacter* SpawnedMonster = GetWorld()->SpawnActor<AMonsterCharacter>(MonstersToSpawn[i].MonsterClass, SpawnLocation, LookAtRotation);
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
        if (BattleManager && MonsterToBattle.IsValid())
        {
            // ...
            BattleManager->StartBattle(PlayerParty, EnemyParty);
        }
    }

    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(OwnerCharacter))
    {
        PlayerChar->OnEnterBattleMode();
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle, this, &UFieldModeComponent::StartFadeOut, 2.0f, false
    );
}

void UFieldModeComponent::EndBattleTransition()
{
    if (!bIsInBattle) return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // 전투 공간에 있는 플레이어 캐릭터 다시 숨기기
    OwnerCharacter->SetActorHiddenInGame(true);

    if (TransitionWidgetClass)
    {
        TransitionWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TransitionWidgetClass);
        if (TransitionWidgetInstance)
        {
            TransitionWidgetInstance->AddToViewport();

            UWidgetAnimation* FadeInAnimation = nullptr;
            UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(TransitionWidgetInstance->GetClass());
            if (WidgetClass)
            {
                for (UWidgetAnimation* Anim : WidgetClass->Animations)
                {
                    if (Anim && Anim->GetFName().ToString().StartsWith(TEXT("FadeIn")))
                    {
                        FadeInAnimation = Anim;
                        break;
                    }
                }
            }

            if (FadeInAnimation)
            {
                TransitionWidgetInstance->PlayAnimation(FadeInAnimation);

                // 애니메이션이 끝날 때까지 기다린 후 레벨 언로딩 시작
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(
                    TimerHandle, this, &UFieldModeComponent::UnloadBattleMap, FadeInAnimation->GetEndTime(), false
                );
                return; // 타이머가 설정되었으므로 함수를 빠져나감
            }
        }
    }

    // 2. 위젯이 없다면 즉시 언로딩 시작
    UnloadBattleMap();
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

    if (TransitionWidgetInstance)
    {
        UWidgetAnimation* FadeOutAnimation = nullptr;
        UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(TransitionWidgetInstance->GetClass());
        if (WidgetClass)
        {
            for (UWidgetAnimation* Anim : WidgetClass->Animations)
            {
                if (Anim && Anim->GetFName().ToString().StartsWith(TEXT("FadeOut")))
                {
                    FadeOutAnimation = Anim;
                    break;
                }
            }
        }

        if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(OwnerCharacter))
        {
            PlayerChar->OnEnterFieldMode();
        }

        if (FadeOutAnimation)
        {
            TransitionWidgetInstance->PlayAnimation(FadeOutAnimation);

            // 애니메이션이 끝나면 위젯을 제거
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                [this]() {
                    if (TransitionWidgetInstance)
                    {
                        TransitionWidgetInstance->RemoveFromParent();
                        TransitionWidgetInstance = nullptr;
                    }
                },
                FadeOutAnimation->GetEndTime(),
                false
            );
        }
    }
}

void UFieldModeComponent::StartLoadingBattleMap()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnBattleArenaLoaded");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = FMath::Rand();

    UGameplayStatics::LoadStreamLevel(this, BattleArenaMapName, true, false, LatentInfo);
}

void UFieldModeComponent::UnloadBattleMap()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnBattleArenaUnloaded");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = FMath::Rand();

    UGameplayStatics::UnloadStreamLevel(this, BattleArenaMapName, LatentInfo, false);
}

void UFieldModeComponent::StartFadeOut()
{
    if (TransitionWidgetInstance)
    {
        UWidgetAnimation* FadeOutAnimation = nullptr;
        UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(TransitionWidgetInstance->GetClass());
        if (WidgetClass)
        {
            for (UWidgetAnimation* Anim : WidgetClass->Animations)
            {
                if (Anim && Anim->GetFName().ToString().StartsWith(TEXT("FadeOut")))
                {
                    FadeOutAnimation = Anim;
                    break;
                }
            }
        }

        if (FadeOutAnimation)
        {
            TransitionWidgetInstance->PlayAnimation(FadeOutAnimation);

            // 애니메이션이 끝나면 위젯을 제거
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                [this]() {
                    if (TransitionWidgetInstance)
                    {
                        TransitionWidgetInstance->RemoveFromParent();
                        TransitionWidgetInstance = nullptr;
                    }
                },
                FadeOutAnimation->GetEndTime(),
                false
            );
        }
        // 만약 FadeOut 애니메이션을 못찾으면, 위젯을 즉시 제거합니다.
        else if (TransitionWidgetInstance)
        {
            TransitionWidgetInstance->RemoveFromParent();
            TransitionWidgetInstance = nullptr;
        }
    }
}

