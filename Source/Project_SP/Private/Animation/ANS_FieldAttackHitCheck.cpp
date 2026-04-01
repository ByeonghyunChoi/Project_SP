#include "Animation/ANS_FieldAttackHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h" // 모듈 추가 필요할 수 있음 (아래 설명 참고)
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
		// 내 본체 메쉬(MeshComp)는 무시하고, 지정한 무기 소켓(WeaponSocketName)을 가진 메쉬를 찾습니다!
		if (Comp != MeshComp && Comp->DoesSocketExist(WeaponSocketName))
		{
			TargetWeaponMesh = Comp;
			break;
		}
	}

	// 판정 범위 설정 (캐릭터 정면)
	FVector TraceLocation;
	if (TargetWeaponMesh)
	{
		// 무기 메쉬를 찾았다면 그 무기의 소켓 위치를 가져옴 (가장 정확!)
		TraceLocation = TargetWeaponMesh->GetSocketLocation(WeaponSocketName);
	}
	else
	{
		// 무기나 소켓을 못 찾았을 때의 안전장치 (기존처럼 캐릭터 정면 사용)
		FVector Forward = Owner->GetActorForwardVector();
		TraceLocation = Owner->GetActorLocation() + (Forward * 100.0f);
	}

	// 충돌 검사 (Sphere Trace)
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner); // 나는 때리지 않음

	bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		TraceLocation, // 소켓 위치
		FQuat::Identity,
		ECC_GameTraceChannel2, // 몬스터 채널 감지
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (bShowDebug)
	{
		DrawDebugSphere(World, TraceLocation, AttackRadius, 12, FColor::Red, false, FrameDeltaTime);
	}

	// 3. 맞은 대상에게 이벤트 전송
	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && HitActor != Owner)
			{
				// ★ 핵심: GA_FieldAttack에게 "맞췄어!"라고 신호 보냄
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