// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_FieldAttack.h"
#include "Character/SPGASPlayerCharacter.h"  
#include "Character/SPGASMonsterCharacter.h" 
#include "Map/MapManagerSubSystem.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" 
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"  
#include "Kismet/GameplayStatics.h"

USPGA_FieldAttack::USPGA_FieldAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USPGA_FieldAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. 자원 확인 및 커밋 (쿨타임, 코스트 등)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = nullptr;
	if (ASPGASCharacterBase* Character = Cast<ASPGASCharacterBase>(ActorInfo->AvatarActor))
	{
		MontageToPlay = Character->FieldAttackMontage;
	}

	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("FieldAttackMontage가 설정되지 않았습니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 몽타주 재생 태스크 생성
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay
	);

	// 몽타주 종료/취소 시 처리 연결
	MontageTask->OnCompleted.AddDynamic(this, &USPGA_FieldAttack::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USPGA_FieldAttack::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USPGA_FieldAttack::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &USPGA_FieldAttack::OnMontageEnded);
	MontageTask->ReadyForActivation();

	// 3. 타격 이벤트 대기 태스크 생성 ("Event.Field.Hit")
	// * 중요: 애니메이션 몽타주에 AnimNotify_SendGameplayEvent를 심어서 이 태그를 보내줘야 함
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("Event.Field.Hit"))
	);

	WaitEventTask->EventReceived.AddDynamic(this, &USPGA_FieldAttack::OnEventReceived);
	WaitEventTask->ReadyForActivation();
}

void USPGA_FieldAttack::OnEventReceived(FGameplayEventData Payload)
{
	// Payload 안에 Instigator와 Target이 들어있습니다.
	AActor* Attacker = const_cast<AActor*>(Payload.Instigator.Get());
	AActor* Victim = const_cast<AActor*>(Payload.Target.Get());

	if (Attacker && Victim)
	{
		ResolveBattleEncounter(Attacker, Victim);
	}
}

void USPGA_FieldAttack::OnMontageEnded()
{
	// 몽타주 끝나면 능력 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USPGA_FieldAttack::ResolveBattleEncounter(AActor* Attacker, AActor* Victim)
{
	const UCombatEncounterData* EncounterData = nullptr;
	ECombatAdvantage Advantage = ECombatAdvantage::PlayerAdvantage;
	APawn* PlayerPawn = nullptr;

	// --- Case A: 플레이어 -> 몬스터 (선공) ---
	if (Attacker->IsA(ASPGASPlayerCharacter::StaticClass()))
	{
		if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(Victim))
		{
			Advantage = ECombatAdvantage::PlayerAdvantage;
			EncounterData = Monster->EncounterData;
		}
		PlayerPawn = Cast<APawn>(Attacker);
	}
	// --- Case B: 몬스터 -> 플레이어 (기습) ---
	else if (Attacker->IsA(ASPGASMonsterCharacter::StaticClass()))
	{
		ASPGASMonsterCharacter* MonsterAttacker = Cast<ASPGASMonsterCharacter>(Attacker);
		if (MonsterAttacker && Victim->IsA(ASPGASPlayerCharacter::StaticClass()))
		{
			Advantage = ECombatAdvantage::EnemyAdvantage;
			EncounterData = MonsterAttacker->EncounterData;
		}
		PlayerPawn = Cast<APawn>(Victim);
	}

	// --- 전투 진입 ---
	if (EncounterData && PlayerPawn)
	{
		UGameInstance* GI = GetWorld()->GetGameInstance();
		if (UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>())
		{
			// "야, 전투 시작해!" 한마디면 끝
			MapManager->StartBattleEncounter(PlayerPawn, EncounterData, Advantage);
		}
	}
}