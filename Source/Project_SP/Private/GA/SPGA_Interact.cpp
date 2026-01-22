// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_Interact.h"
#include "Interface/InteractableInterface.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

USPGA_Interact::USPGA_Interact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USPGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Error, TEXT("GA_Interact: ActivateAbility 진입 성공!"));
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ASPGASPlayerCharacter* Player = Cast<ASPGASPlayerCharacter>(ActorInfo->AvatarActor);
	if (!Player)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	
	FVector Forward = Player->GetActorForwardVector(); 
	FVector Start = Player->GetActorLocation() + (Forward * 50.0f);
	FVector End = Start + (Forward * TraceDistance);
	FRotator Rotation = Player->GetActorRotation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractionRadius);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,// 나중에 상호작용 전용 채널을 만들어서 사용하기
		SphereShape,
		Params
	);

	if (bShowDebug)
	{
		FColor DebugColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, (TraceDistance * 0.5f) + InteractionRadius, InteractionRadius, Rotation.Quaternion() * FRotator(90, 0, 0).Quaternion(), DebugColor, false, 2.0f);
	}

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		// 4. 인터페이스 확인 및 실행
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			IInteractableInterface* Interactable = Cast<IInteractableInterface>(HitActor);
			if (Interactable && Interactable->CanInteract(Player))
			{
				Interactable->ExecuteInteraction(Player);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
