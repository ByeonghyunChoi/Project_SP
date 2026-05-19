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
#include "Game/SPProjectileBase.h"


bool USPGA_Parry::CheckWeaponMatch(ASPGASMonsterCharacter* TargetMonster)
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (!PlayerASC) return false;

	// 눈앞에 날아오는 투사체(Projectile)가 있는지 먼저 검사합니다.
	if (MyAvatar)
	{
		FVector StartLoc = MyAvatar->GetActorLocation();
		FVector EndLoc = StartLoc + (MyAvatar->GetActorForwardVector() * 100.0f); // 패링 사거리

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(MyAvatar);

		// 플레이어 정면으로 구체 트레이스 발사 (반지름 80으로 넉넉하게 스캔)
		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult, StartLoc, EndLoc, FQuat::Identity,
			ECC_GameTraceChannel3, FCollisionShape::MakeSphere(80.0f), Params
		);

#if WITH_EDITOR
		FColor DebugColor = bHit ? FColor::Green : FColor::Red;
		// 출발지부터 도착지까지 스캔 경로 전체를 캡슐 형태로 그려줍니다.
		DrawDebugCapsule(
			GetWorld(),
			(StartLoc + EndLoc) * 0.5f,                  // 중심점
			(100.0f * 0.5f) + 80.0f,                     // 반만 펼쳐진 길이 + 반지름
			80.0f,                                       // 반지름
			FRotationMatrix::MakeFromZ(MyAvatar->GetActorForwardVector()).ToQuat(), // 방향
			DebugColor,                                  // 평소엔 빨간색, 투사체 감지하면 초록색
			false,                                       // 영구 지속 여부
			1.0f                                         // 화면에 잔상이 남을 시간 (1초 동안 유지)
		);
#endif

		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();
			UE_LOG(LogTemp, Warning, TEXT("[패링 레이더] 무언가 레이더에 걸렸습니다: %s"), HitActor ? *HitActor->GetName() : TEXT("NULL"));
			// 맞은 액터가 투사체 클래스(ASPProjectileBase)인지 확인!
			if (ASPProjectileBase* Projectile = Cast<ASPProjectileBase>(HitResult.GetActor()))
			{
				UE_LOG(LogTemp, Warning, TEXT("[패링 레이더] 잡힌 물체가 투사체입니다! (투사체 태그: %s)"), *Projectile->ElementTag.ToString());

				if (!Projectile->bIsParried) // 이미 튕겨낸 투사체가 아니라면
				{
					bool bIsProjMatch = false;
					FGameplayTag ProjElement = Projectile->ElementTag;

					// 투사체 속성과 내 무기 태그 매칭 검사
					if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir) && ProjElement == SPTags.Weakness_Fenrir) bIsProjMatch = true;
					else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr) && ProjElement == SPTags.Weakness_Surtr) bIsProjMatch = true;
					else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr) && ProjElement == SPTags.Weakness_Jormungandr) bIsProjMatch = true;

					if (bIsProjMatch)
					{
						// [투사체 패링 성공!]
						Projectile->bIsParried = true;
						Projectile->Destroy(); // 투사체를 즉시 화면에서 파괴합니다.

						UE_LOG(LogTemp, Warning, TEXT("[패링 성공] 투사체를 튕겨냈습니다!"));

						// true를 반환하면, 기존 블루프린트 로직이 알아서 TargetMonster(마녀)에게 
						// SendParriedEventToMonster를 호출하여 경직/스택 무전을 쏘게 됩니다!
						return true;
					}
					else
					{
						// 태그가 달라서 실패했음을 알림
						UE_LOG(LogTemp, Error, TEXT("[패링 실패] 투사체는 맞췄으나, 내 무기 태그와 투사체 태그가 다릅니다!"));
					}
				}
			}
			else
            {
                // 투사체가 아닌 다른 것(내 무기 등)을 때렸음을 알림
                UE_LOG(LogTemp, Error, TEXT("[패링 실패] 투사체가 아니라 이상한 물체(%s)가 레이더를 가로막고 있습니다!"), HitActor ? *HitActor->GetName() : TEXT("NULL"));
            }
		}
	}

	// 투사체가 없다면, 근접 공격(몬스터 몸통) 패링을 검사합니다.
	if (!TargetMonster) return false;

	UAbilitySystemComponent* TargetASC = TargetMonster->GetAbilitySystemComponent();
	if (!TargetASC) return false;

	// 패링 창이 열려있는지 검사 
	if (!TargetASC->HasMatchingGameplayTag(SPTags.State_ParryWindow))
	{
		UE_LOG(LogTemp, Warning, TEXT("패링 실패: 몬스터가 패링 가능 상태(창문)가 아닙니다."));
		return false;
	}

	// 무기와 약점 상성 확인 
	bool bIsMatch = false;
	if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Fenrir) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Fenrir)) bIsMatch = true;
	else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Surtr) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Surtr)) bIsMatch = true;
	else if (PlayerASC->HasMatchingGameplayTag(SPTags.Weapon_Jormungandr) && TargetASC->HasMatchingGameplayTag(SPTags.Weakness_Jormungandr)) bIsMatch = true;

	// 상성이 맞다면, 그 즉시 몬스터의 패링 창을 닫아버립니다!
	if (bIsMatch)
	{
		TargetASC->RemoveLooseGameplayTag(SPTags.State_ParryWindow);
		TargetASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(SPTags.State_ParryWindow));

		UE_LOG(LogTemp, Warning, TEXT("근접 패링 성공!"));
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