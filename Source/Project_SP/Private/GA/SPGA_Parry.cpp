// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_Parry.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Tag/SPGameplayTags.h"


bool USPGA_Parry::CheckWeaponMatch(ASPGASMonsterCharacter* TargetMonster)
{
	if (!TargetMonster) return false;

	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerASC) return false;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// 1. 패링 창문이 열려있는지 검사 (태그 확인)
	if (!TargetMonster->GetAbilitySystemComponent()->HasMatchingGameplayTag(SPTags.State_ParryWindow))
	{
		UE_LOG(LogTemp, Warning, TEXT("패링 실패: 몬스터가 패링 가능 상태(창문)가 아닙니다."));
		return false;
	}

	// 2. 무기와 약점 상성 확인 (선생님의 태그 이름에 맞춰 수정하세요!)
	// [펜리르]
	if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir) &&
		TargetMonster->WeaknessTags.HasTag(SPTags.Weakness_Fenrir))
	{
		return true;
	}

	// [수르트]
	if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr) &&
		TargetMonster->WeaknessTags.HasTag(SPTags.Weakness_Surtr))
	{
		return true;
	}

	// [요르문간드]
	if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr) &&
		TargetMonster->WeaknessTags.HasTag(SPTags.Weakness_Jormungandr))
	{
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("패링 실패: 무기와 몬스터의 약점이 일치하지 않습니다."));
	return false;
}

bool USPGA_Parry::CheckCounterConditions()
{
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerASC) return false;

	// 1. 반격 모드인지 확인
	if (!PlayerASC->HasMatchingGameplayTag(CounterModeTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("패링 성공: 하지만 반격 모드가 아니므로 턴만 종료합니다."));
		return false;
	}

	// 2. 쿨타임 검사 (CounterSkillTag를 통해 쿨타임 체크)
	// (기본적으로 태그로 쿨다운을 검사하는 GAS 로직을 추가하시면 됩니다. 지금은 일단 true!)

	UE_LOG(LogTemp, Warning, TEXT("반격 조건 올 클리어! 반격 턴을 준비합니다."));
	return true;
}

void USPGA_Parry::SendParriedEventToMonster(AActor* TargetMonster)
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetMonster))
	{
		// 몬스터에게 "너 패링당했어!" (Event.Combat.Parried) 무전을 날립니다.
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo(); // 내가 때렸다
		ASI->GetAbilitySystemComponent()->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_Parried, &Payload);
		UE_LOG(LogTemp, Warning, TEXT("몬스터에게 패링 이벤트를 성공적으로 전송했습니다."));
	}
}