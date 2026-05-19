// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_MonsterAttackBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Tag/SPGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/SPGASCharacterBase.h"
#include "Kismet/GameplayStatics.h"

USPGA_MonsterAttackBase::USPGA_MonsterAttackBase()
{
	// 이벤트 리스너(Task)를 쓰려면 무조건 InstancedPerActor 여야 합니다!
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USPGA_MonsterAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 🌟 어빌리티가 시작되자마자 '패링 무전'을 기다리는 잠복 근무(Task)를 시작합니다!
	UAbilityTask_WaitGameplayEvent* WaitParriedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FSPGameplayTags::Get().Event_Battle_Parried
	);

	if (WaitParriedTask)
	{
		// 무전이 들어오면 OnParriedEventCallback 함수를 실행하라고 바인딩합니다.
		WaitParriedTask->EventReceived.AddDynamic(this, &USPGA_MonsterAttackBase::OnParriedEventCallback);

		// 잠복 근무 시작!
		WaitParriedTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FSPGameplayTags::Get().Event_Battle_ApplyDamage);
	if (WaitDamageEventTask)
	{
		// 무전이 오면 OnDamageEventReceived 함수를 실행해라!
		WaitDamageEventTask->EventReceived.AddDynamic(this, &USPGA_MonsterAttackBase::OnDamageEventReceived);
		WaitDamageEventTask->ReadyForActivation();
	}
}

void USPGA_MonsterAttackBase::OnParriedEventCallback(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[Monster GA] 패링 무전 수신! 하던 공격을 멈추고 복귀합니다."));

	// 블루프린트에서 구현한 연출(튕겨 나가기, 제자리 복귀 등)을 실행하도록 이벤트 호출!
	OnParried();
}

void USPGA_MonsterAttackBase::ApplyDamageToTarget(AActor* TargetActor, float Damage)
{
	// 1. 타겟이 없거나, 데미지 이펙트가 세팅되어 있지 않으면 무시합니다.
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
		// 2. 데미지 이펙트 명세서(Spec) 생성
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

		if (SpecHandle.IsValid())
		{
			// 🌟 [핵심!] 플레이어와 똑같이 Data.Damage 태그로 데미지 배율(계수)을 동봉합니다!
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
				SpecHandle,
				SPTags.Data_Damage,
				DamageMultiplier
			);

			// 3. 타겟(플레이어)에게 데미지 발사! (이 순간 데미지 계산기가 돌아갑니다)
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

			// ------------------------------------------------------------------
			// 🎬 [연출] 플레이어처럼 몬스터도 피격 이펙트와 애니메이션을 발생시킵니다!
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

				// 💥 타겟(플레이어)의 ASC에 Hit VFX(GameplayCue) 실행 명령!
				// (헤더에 HitVFXTag 변수를 추가해두셨다면 그대로 쓰시면 됩니다)
				// FGameplayCueParameters CueParams;
				// CueParams.EffectContext = TargetASC->MakeEffectContext();
				// CueParams.EffectContext.AddHitResult(HitResult); 
				// TargetASC->ExecuteGameplayCue(HitVFXTag, CueParams);

				// 🩸 타겟(플레이어)에게 피격 애니메이션(Hit React) 재생 명령!
				if (ASPGASCharacterBase* TargetBaseChar = Cast<ASPGASCharacterBase>(TargetActor))
				{
					TargetBaseChar->PlayHitReact(HitResult.ImpactPoint);
				}

				// 📸 (선택) 몬스터 공격이 묵직하다면 카메라 쉐이크를 줘도 좋습니다.
				// if (HitCameraShakeClass && GetWorld())
				// {
				// 	UGameplayStatics::PlayWorldCameraShake(GetWorld(), HitCameraShakeClass, TargetActor->GetActorLocation(), 0.0f, 1000.0f, 1.0f);
				// }
			}
		}
	}
}

void USPGA_MonsterAttackBase::OnDamageEventReceived(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[Monster GA] 애님 노티파이 타격 신호 수신! 플레이어를 공격합니다."));

	// 타겟 찾기: 싱글 플레이/턴제 RPG라면 보통 0번 플레이어가 타겟이 됩니다.
	// (만약 AI가 Payload.Target에 타겟을 넣어줬다면 그걸 써도 됩니다.)
	AActor* TargetActor = nullptr;

	if (Payload.Target)
	{
		TargetActor = const_cast<AActor*>(Payload.Target.Get());
	}
	else
	{
		TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}

	// 타겟을 찾았다면, 이전에 만든 만능 데미지 함수를 호출! (계수는 기본 1.0f)
	if (TargetActor)
	{
		ApplyDamageToTarget(TargetActor, DamageMultiplier);
	}
}
