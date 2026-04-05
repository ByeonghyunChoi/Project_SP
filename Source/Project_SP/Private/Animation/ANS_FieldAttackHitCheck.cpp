#include "Animation/ANS_FieldAttackHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Tag/SPGameplayTags.h"

void UANS_FieldAttackHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	UMeshComponent* TargetWeaponMesh = nullptr;
	TArray<UMeshComponent*> AllMeshComponents;

	Owner->GetComponents<UMeshComponent>(AllMeshComponents);

	for (UMeshComponent* Comp : AllMeshComponents)
	{
		// 내 본체 메쉬는 무시하고, 지정한 무기 소켓을 가진 메쉬를 찾음.
		if (Comp != MeshComp && Comp->DoesSocketExist(WeaponSocketName))
		{
			TargetWeaponMesh = Comp;
			break;
		}
	}

	// 판정 범위 설정
	FVector TraceLocation;
	if (TargetWeaponMesh)
	{
		// 무기 메쉬를 찾았다면 그 무기의 소켓 위치를 가져옴
		TraceLocation = TargetWeaponMesh->GetSocketLocation(WeaponSocketName);
	}
	else
	{
		// 무기나 소켓을 못 찾았을 때는 캐릭터의 중앙
		FVector Forward = Owner->GetActorForwardVector();
		TraceLocation = Owner->GetActorLocation() + (Forward * 100.0f);
	}

	// 충돌 검사
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner); // 나는 무시

	bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		TraceLocation,
		FQuat::Identity,
		ECC_GameTraceChannel2, // 몬스터 채널
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (bShowDebug)
	{
		DrawDebugSphere(World, TraceLocation, AttackRadius, 12, FColor::Red, false, FrameDeltaTime);
	}

	// 타격 신호 보냄
	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && HitActor != Owner)
			{
				FGameplayEventData Payload;
				Payload.Instigator = Owner;
				Payload.Target = HitActor;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					Owner,
					SPTags.Event_Field_Hit,
					Payload
				);
			}
		}
	}
}