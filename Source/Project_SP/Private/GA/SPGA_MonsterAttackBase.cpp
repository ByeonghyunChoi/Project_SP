// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_MonsterAttackBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/SPGASCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"

USPGA_MonsterAttackBase::USPGA_MonsterAttackBase()
{

}

void USPGA_MonsterAttackBase::SetupActionEventListeners()
{
	Super::SetupActionEventListeners();

	UAbilityTask_WaitGameplayEvent* WaitParriedTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FSPGameplayTags::Get().Event_Battle_Parried
		);

	if (WaitParriedTask)
	{
		WaitParriedTask->EventReceived.AddDynamic(
			this,
			&USPGA_MonsterAttackBase::OnParriedEventCallback
		);

		WaitParriedTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FSPGameplayTags::Get().Event_Battle_ApplyDamage
		);

	if (WaitDamageEventTask)
	{
		WaitDamageEventTask->EventReceived.AddDynamic(
			this,
			&USPGA_MonsterAttackBase::OnDamageEventReceived
		);

		WaitDamageEventTask->ReadyForActivation();
	}
}


void USPGA_MonsterAttackBase::OnParriedEventCallback(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[Monster GA] 패링 무전 수신! 하던 공격을 멈추고 복귀합니다."));

	OnParried();
}

void USPGA_MonsterAttackBase::ApplyDamageToTarget(AActor* TargetActor, float Damage)
{
	if (!TargetActor || !DamageEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Monster GA] TargetActor가 없거나 DamageEffectClass가 할당되지 않았습니다!"));
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (SourceASC && SourceASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_DamageDisabled))
	{
		UE_LOG(LogTemp, Warning, TEXT("[시스템] 패링된 공격입니다! 데미지를 주지 않고 스킵합니다."));
		return;
	}

	if (SourceASC && TargetASC)
	{
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

		if (SpecHandle.IsValid())
		{
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
				SpecHandle,
				SPTags.Data_Damage,
				DamageMultiplier
			);

			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

			FHitResult HitResult;
			AActor* AvatarActor = GetAvatarActorFromActorInfo();

			if (AvatarActor)
			{
				FVector StartLoc = AvatarActor->GetActorLocation(); // 몬스터 위치
				FVector EndLoc = TargetActor->GetActorLocation();   // 플레이어 위치

				// 레이저를 쏴서 타격점 찾기
				GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility);

				if (!HitResult.bBlockingHit || HitResult.GetActor() != TargetActor)
				{
					HitResult.ImpactPoint = TargetActor->GetActorLocation();
				}
				HitResult.HitObjectHandle = FActorInstanceHandle(TargetActor);

				if (ASPGASCharacterBase* TargetBaseChar = Cast<ASPGASCharacterBase>(TargetActor))
				{
					TargetBaseChar->PlayHitReact(HitResult.ImpactPoint);
				}
			}
		}
	}
}

void USPGA_MonsterAttackBase::OnDamageEventReceived(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[Monster GA] 애님 노티파이 타격 신호 수신! 플레이어를 공격합니다."));

	AActor* TargetActor = nullptr;

	if (Payload.Target)
	{
		TargetActor = const_cast<AActor*>(Payload.Target.Get());
	}
	else
	{
		TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}

	if (TargetActor)
	{
		ApplyDamageToTarget(TargetActor, DamageMultiplier);
	}
}
