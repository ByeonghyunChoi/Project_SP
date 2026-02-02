#include "Animation/ANS_FieldAttackHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h" // 모듈 추가 필요할 수 있음 (아래 설명 참고)
#include "Engine/OverlapResult.h"
#include "GameFramework/Actor.h"

void UANS_FieldAttackHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	// 1. 판정 범위 설정 (캐릭터 정면)
	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();
	FVector End = Start + (Forward * AttackRange);

	// 2. 충돌 검사 (Sphere Trace)
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner); // 나는 때리지 않음

	// 몬스터는 보통 Pawn 채널입니다. (필요시 ECC_Interaction으로 변경)
	bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		Start + (Forward * (AttackRange * 0.5f)), // 구체 중심
		FQuat::Identity,
		ECC_Pawn, // Pawn 채널 감지
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	// 디버그 그리기 (테스트 할 때만 켜세요)
	DrawDebugSphere(World, Start + (Forward * (AttackRange * 0.5f)), AttackRadius, 12, FColor::Red, false, -1.0f);

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
					FGameplayTag::RequestGameplayTag(FName("Event.Field.Hit")),
					Payload
				);

				// (중요) 한 프레임에 여러 번 보내도 되지만, 보통 전투 진입은 한 번이면 충분하므로
				// 첫 번째 유효 타격에서 바로 return 하는 것도 방법입니다.
				// 하지만 'Overlap' 특성상 매 프레임 호출되므로 GA쪽에서 한번 처리하면 끝나는 구조여야 합니다.
			}
		}
	}
}