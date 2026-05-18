// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_Parry.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerController.h"
#include "GA/SPGA_BattleActionBase.h"
#include "Tag/SPGameplayTags.h"
#include "Sound/SoundBase.h"     
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"


bool USPGA_Parry::CheckWeaponMatch(ASPGASMonsterCharacter* TargetMonster)
{
	if (!TargetMonster) return false;

	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = TargetMonster->GetAbilitySystemComponent();
	if (!PlayerASC || !TargetASC) return false;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// 패링 창이 열려있는지 검사 (태그 확인)
	if (!TargetMonster->GetAbilitySystemComponent()->HasMatchingGameplayTag(SPTags.State_ParryWindow))
	{
		UE_LOG(LogTemp, Warning, TEXT("패링 실패: 몬스터가 패링 가능 상태(창문)가 아닙니다."));
		return false;
	}

	// 2. 무기와 약점 상성 확인 (선생님의 태그 이름에 맞춰 수정하세요!)
	// [펜리르]
	bool bIsMatch = false;
	if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Fenrir)) bIsMatch = true;
	else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Surtr)) bIsMatch = true;
	else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Jormungandr)) bIsMatch = true;

	// 상성이 맞다면, 그 즉시 몬스터의 패링 창을 닫아버립니다!
	if (bIsMatch)
	{
		// 이 코드를 통과하는 첫 번째 패링만 정상 작동하며, 
		// 0.001초 뒤에 발동된 두 번째 패링은 위 1번 검사에서 무조건 튕겨 나갑니다!
		TargetASC->RemoveLooseGameplayTag(SPTags.State_ParryWindow);
		TargetASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(SPTags.State_ParryWindow));

		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("패링 실패: 무기와 몬스터의 약점이 일치하지 않습니다."));
	return false;
}

bool USPGA_Parry::CheckCounterConditions()
{
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerASC) return false;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (PlayerASC->HasMatchingGameplayTag(SPTags.State_Buff_JadeClock))
	{
		UE_LOG(LogTemp, Warning, TEXT("[옥시계] 버프가 존재하여 쿨타임 검사를 패스합니다!"));
		return true;
	}

	if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerChar->GetController()))
		{
			FGameplayTag CurrentWeapon = PC->GetCurrentWeaponTag();
			UWeaponAbilityData* WeaponData = PlayerChar->GetWeaponData(CurrentWeapon);

			if (WeaponData && WeaponData->ParrySkillAbility)
			{
				// 기본 UGameplayAbility가 아니라, 베이스 클래스로 캐스팅합니다!
				if (USPGA_BattleActionBase* AbilityCDO = WeaponData->ParrySkillAbility->GetDefaultObject<USPGA_BattleActionBase>())
				{
					// CooldownTag 변수를 가져옵니다.
					FGameplayTag TargetCooldownTag = AbilityCDO->GetCooldownTag();

					// 태그가 유효하고, 내 몸(ASC)에 그 태그가 붙어있다면 쿨타임 중인 것!
					if (TargetCooldownTag.IsValid() && PlayerASC->HasMatchingGameplayTag(TargetCooldownTag))
					{
						UE_LOG(LogTemp, Warning, TEXT("패링 성공: 하지만 반격 스킬(%s)이 쿨타임 중이라 추가 턴을 얻지 못합니다!"), *WeaponData->ParrySkillAbility->GetName());
						return false; // ❌ 쿨타임 컷!
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("패링 성공 & 쿨타임 통과! 즉시 반격 턴을 획득합니다."));
	return true;
}

void USPGA_Parry::SendParriedEventToMonster(AActor* TargetMonster)
{
	if (ParrySuccessSounds.Num() > 0)
	{
		int index = FMath::RandRange(0, ParrySuccessSounds.Num() - 1);
		UGameplayStatics::PlaySound2D(this, ParrySuccessSounds[index]);
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetMonster))
	{
		// 몬스터에게 "너 패링당했어!" (Event.Combat.Parried) 무전을 날립니다.
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo(); // 내가 때렸다
		ASI->GetAbilitySystemComponent()->HandleGameplayEvent(FSPGameplayTags::Get().Event_Battle_Parried, &Payload);
		UE_LOG(LogTemp, Warning, TEXT("몬스터에게 패링 이벤트를 성공적으로 전송했습니다."));
	}

	// 내 몸의 유물들에게 패링 성공했다고 알림
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayEventData ParryPayload;
		ParryPayload.Instigator = GetAvatarActorFromActorInfo();
		ParryPayload.Target = TargetMonster;

		ASC->HandleGameplayEvent(FSPGameplayTags::Get().Event_Combat_ParrySuccess, &ParryPayload);
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetMonster))
	{
		TargetASC->AddLooseGameplayTag(FSPGameplayTags::Get().State_Status_DamageDisabled);
	}
}