// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPawn.h"
#include "BattleManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACombatPawn::ACombatPawn()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CombatData = CreateDefaultSubobject<UCharacterBase>(TEXT("CombatData"));
}

UCharacterBase* ACombatPawn::GetCombatData() const
{
	return CombatData;
}

void ACombatPawn::Attack(ACombatPawn* Target)
{
	if (!Target || !Target->GetCombatData() || !CombatData)
	{
		UE_LOG(LogTemp, Log, TEXT("Attack 실패 - 대상 또는 CombatData 유효하지 않음."));
		return;
	}

	//공격 애니메이션 재생
	PlayAttackAnimation();

	// 데미지 계산(나중에 수정 할 예정)
	float Damage = CombatData->GetStats().fAttackPower;

	Target->ReceiveDamage(Damage);
}

void ACombatPawn::UseSkill(int32 SkillID, ACombatPawn* Target)
{
	if (!Target || !Target->GetCombatData() || !CombatData)
	{
		UE_LOG(LogTemp, Log, TEXT("Attack 실패 - 대상 또는 CombatData 유효하지 않음."));
		return;
	}

	//스킬 애니메이션 재생
	PlaySkillAnimation(SkillID);

	// 데미지 계산(나중에 수정 할 예정)
	float Damage = CombatData->GetStats().fAttackPower;

	Target->ReceiveDamage(Damage);

}

void ACombatPawn::ReceiveDamage(float DamageAmount)
{
	if (!CombatData)
	{
		UE_LOG(LogTemp, Error, TEXT("TakeDamage 실패 - CombatData 유효하지 않음."));
		return;
	}

	CombatData->SetStats(EStat::CurrentHealth, CombatData->GetStats().fCurrentHealth - DamageAmount);

	//피격 애니메이션 재생
	PlayHitAnimation();

	//사망 처리
	if (CombatData->GetStats().fCurrentHealth <= 0.0f)
	{
		// TODO: 사망 로직 (사망 애니메이션, 전투에서 제거 등)
	}
}

void ACombatPawn::PlayAttackAnimation_Implementation()
{
	// 기본적으로 아무것도 하지 않음 (블루프린트에서 구현될 예정)
}

void ACombatPawn::PlayHitAnimation_Implementation()
{
	// 기본적으로 아무것도 하지 않음 (블루프린트에서 구현될 예정)
}

void ACombatPawn::PlaySkillAnimation_Implementation(int32 SkillID)
{
	// 기본적으로 아무것도 하지 않음 (블루프린트에서 구현될 예정)
}

TArray<ACombatPawn*> ACombatPawn::GetAllAliveCombatantsOfFaction(EFaction TargetFaction) const
{
	TArray<ACombatPawn*> FoundCombatants;
	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));

	if (BattleManager)
	{
		for (ACombatPawn* Combatant : BattleManager->AllCombatants)
		{
			if (Combatant && Combatant->GetCombatData() && Combatant->GetCombatData()->GetStats().fCurrentHealth > 0 && Combatant->GetCombatData()->GetFaction() == TargetFaction)
			{
				FoundCombatants.Add(Combatant);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllAliveCombatantsOfFaction: BattleManager를 찾을 수 없습니다."));
	}
	return FoundCombatants;
}


