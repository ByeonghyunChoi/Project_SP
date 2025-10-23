#include "Character/MonsterCharacter.h"
#include "Combat/MonsterAIController.h" 
#include "Component/ActionComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/WidgetComponent.h"
#include "Component/GameEventComponent.h"
#include "Combat/MonsterGroupObject.h"


AMonsterCharacter::AMonsterCharacter()
{
    AIControllerClass = AMonsterAIController::StaticClass();
}

void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    SetFaction(EFaction::Enemy);

    if (ActionComponent)
    {
        ActionComponent->InitializeDefaultActions(DefaultActionIDs);
    }

    if (GameEventComponent)
    {
        // UGameAction이 방송하는 '패링 창 열림' 이벤트를 구독합니다.
        GameEventComponent->OnParryWindowOpened.AddDynamic(this, &AMonsterCharacter::HandleParryWindowOpened);
        GameEventComponent->OnParryWindowClosed.AddDynamic(this, &AMonsterCharacter::HandleParryWindowClosed);
    }
}

void AMonsterCharacter::OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets)
{
    if (GetCombatPawnState() == ECombatPawnState::Defeated) return;

    UE_LOG(LogTemp, Log, TEXT("Monster '%s' Turn Began."), *GetCharacterDisplayName().ToString());
    SetCombatPawnState(ECombatPawnState::PerformingAction);

    AMonsterAIController* AICon = GetController<AMonsterAIController>();
    if (AICon)
    {
        AICon->OnTurnBegan(PotentialTargets);
    }
}

void AMonsterCharacter::PlayActionMontage(FName ActionID)
{
    const TSoftObjectPtr<UAnimMontage>* FoundMontage = ActionMontageMap.Find(ActionID);
    if (FoundMontage && FoundMontage->IsValid()) // .IsValid()로 포인터가 null이 아닌지 확인
    {
        // Get()은 이미 로드된 애셋을 즉시 반환합니다.
        if (UAnimMontage* MontageToPlay = FoundMontage->Get())
        {
            PlayAnimMontage(MontageToPlay);
        }
        else
        {
            // 만약 Get()이 실패했다면, CriticalAssets에 이 몽타주가 빠졌다는 의미입니다.
            UE_LOG(LogTemp, Error, TEXT("Montage '%s' was not preloaded! Check FMonsterData::CriticalAssets."), *FoundMontage->ToSoftObjectPath().ToString());
        }
    }
}

void AMonsterCharacter::HandleParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration)
{
    // 이 이벤트를 발생시킨 장본인이 '나' 자신일 경우에만 연출을 재생합니다.
    if (Attacker == this)
    {
        K2_OnParryWindowOpened();
    }
}

void AMonsterCharacter::HandleParryWindowClosed(ACombatPawn* Attacker)
{
    if (Attacker == this)
    {
        K2_OnParryWindowClosed();
    }
}