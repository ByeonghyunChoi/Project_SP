#include "Combat/Tasks/Task_ChainAttack.h"
#include "Character/CombatPawn.h"
#include "Core/BattleManager.h"
#include "Component/AttributesComponent.h"
#include "Component/GameEventComponent.h" // 데미지 플로터용
#include "Combat/CombatStatics.h"       // 데미지 공식용
#include "Combat/GameAction.h"
#include "Animation/AnimInstance.h"

void UTask_ChainAttack::ExecuteTask_Implementation()
{
	// 안전 장치: 배틀매니저나 시전자가 없으면 바로 종료
	if (!BattleManager || !Instigator)
	{
		FinishTask();
		return;
	}

	// 1. 타겟 목록 확보하기
	SortedTargets.Empty();

	// 나의 진영을 확인하여 적대 진영을 결정 (플레이어 <-> 적)
	EFaction MyFaction = Instigator->GetFaction();
	EFaction TargetFaction = (MyFaction == EFaction::Player) ? EFaction::Enemy : EFaction::Player;

	// BattleManager에 등록된 모든 전투원 중 '살아있는 적'만 골라내기
	// (BattleManager의 리스트 순서를 그대로 따르므로, 배치된 순서대로 때리게 됨)
	for (ACombatPawn* Pawn : BattleManager->GetAllCombatants())
	{
		if (Pawn && Pawn->GetFaction() == TargetFaction && Pawn->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			SortedTargets.Add(Pawn);
		}
	}

	// 공격할 대상이 하나도 없으면 종료
	if (SortedTargets.Num() == 0)
	{
		FinishTask();
		return;
	}

	// 인덱스 초기화 후 첫 단계(점프) 시작
	CurrentTargetIndex = 0;

	// 점프 위치 계산
	if (Instigator)
	{
		JumpTargetLocation = Instigator->GetActorLocation() + Instigator->GetActorRotation().RotateVector(JumpOffset);
		bIsJumping = true; // 점프 모드 시작
	}

	PlayJumpAnim();
}

void UTask_ChainAttack::PlayJumpAnim()
{
	if (Instigator && JumpMontage)
	{
		UAnimInstance* AnimInst = Instigator->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			Instigator->PlayAnimMontage(JumpMontage);

			// 몽타주 종료 델리게이트 (노티파이가 없어서 끝까지 재생됐을 때를 대비)
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UTask_ChainAttack::OnJumpAnimEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, JumpMontage);
			return;
		}
	}
	// 몽타주 없으면 바로 다음으로
	FinalizeJumpPhase();
}

void UTask_ChainAttack::OnJumpAnimEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 이미 노티파이로 끝났다면 무시
	if (!bIsJumping) return;

	FinalizeJumpPhase();
}

void UTask_ChainAttack::PlayNextAttack()
{
	// [종료 조건] 리스트의 모든 적을 다 공격했는지 확인
	if (!SortedTargets.IsValidIndex(CurrentTargetIndex))
	{
		// 모든 공격이 끝났으니 복귀를 위해 캐릭터를 다시 보이게 만듭니다.
		if (Instigator)
		{
			Instigator->SetActorHiddenInGame(false);
		}

		// 태스크 종료 (이후 MoveToHome 태스크가 실행됨)
		FinishTask();
		return;
	}

	ACombatPawn* Target = SortedTargets[CurrentTargetIndex];

	// 타겟이 그새 죽었거나 유효하지 않다면 스킵하고 다음 적으로 넘어감
	if (!Target || Target->GetCombatPawnState() == ECombatPawnState::Defeated)
	{
		CurrentTargetIndex++;
		PlayNextAttack(); // 재귀 호출 (Loop)
		return;
	}

	// 1. 위치 이동 (순간이동)
	if (Instigator)
	{
		FVector TargetLoc = Target->GetActorLocation();

		// 적의 정면 방향(Forward)으로 AttackOffset만큼 떨어진 위치 계산
		// 예: 적 위치 + (적 정면 * 150) = 적 1.5m 앞
		FVector AttackPos = TargetLoc + (Target->GetActorForwardVector() * AttackOffset.X);

		// 회전: 공격자가 적을 바라보도록 설정
		FVector DirToTarget = TargetLoc - AttackPos;
		FRotator LookAtRot = DirToTarget.Rotation();
		LookAtRot.Pitch = 0.f; // 기울어지지 않게

		// 텔레포트!
		Instigator->SetActorLocation(AttackPos);
		Instigator->SetActorRotation(LookAtRot);

		// 공격 모션을 보여주기 위해 캐릭터 표시 (짠! 하고 나타남)
		Instigator->SetActorHiddenInGame(false);
	}

	// 2. 공격 몽타주 재생
	if (Instigator && AttackMontage)
	{
		UAnimInstance* AnimInst = Instigator->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			Instigator->PlayAnimMontage(AttackMontage);

			// 종료 델리게이트 연결 (끝나면 OnAttackAnimEnded 호출)
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UTask_ChainAttack::OnAttackAnimEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, AttackMontage);

			// *주의: 여기서는 타이머를 쓰지 않습니다. 
			// 대신 아래 OnNotifyReceived 함수가 노티파이를 기다립니다.
		}
	}
	else
	{
		// 몽타주가 없으면 즉시 데미지만 주고 다음으로 넘어감 (안전장치)
		ApplyDamageToCurrentTarget();
		OnAttackAnimEnded(nullptr, false);
	}
}

// [핵심] BattleManager가 노티파이 신호를 보내주면 이 함수가 실행됩니다.
void UTask_ChainAttack::OnNotifyReceived(FName NotifyName)
{
	// 내가 기다리던 그 이름(예: "Hit")이 맞는지 확인
	if (NotifyName == HitNotifyName)
	{
		// 맞다면 데미지 적용!
		ApplyDamageToCurrentTarget();
	}
	else if (NotifyName == TEXT("End"))
	{
		// 몽타주를 강제로 정지시킵니다.
		if (Instigator && AttackMontage)
		{
			// 몽타주를 멈추면, 엔진이 자동으로 OnAttackAnimEnded 델리게이트를 호출해줍니다.
			// 그러면 자연스럽게 PlayNextAttack()으로 이어집니다.
			Instigator->StopAnimMontage(AttackMontage);
		}
	}
	else if (NotifyName == JumpEndNotifyName)
	{
		// 점프 단계를 강제로 끝내고 공격으로 넘어감
		FinalizeJumpPhase();
	}
}

void UTask_ChainAttack::TickTask(float DeltaTime)
{
	// 점프 중일 때만 위치 이동 수행
	if (bIsJumping && Instigator)
	{
		FVector CurrentLoc = Instigator->GetActorLocation();

		// 부드럽게 목표 지점으로 상승 (VInterpConstantTo 사용)
		FVector NewLoc = FMath::VInterpConstantTo(CurrentLoc, JumpTargetLocation, DeltaTime, JumpMoveSpeed);

		// 충돌 무시하고 이동 (하늘 뚫고 가야 하므로)
		Instigator->SetActorLocation(NewLoc, false);

		// (선택사항) 목표에 도달했는지 체크할 필요는 없음.
		// 왜냐하면 애니메이션 노티파이(JumpEnd)가 타이밍을 결정할 것이기 때문.
	}
}

void UTask_ChainAttack::ApplyDamageToCurrentTarget()
{
	// 1. 타겟 유효성 검사
	if (!SortedTargets.IsValidIndex(CurrentTargetIndex)) return;

	ACombatPawn* Target = SortedTargets[CurrentTargetIndex];

	// 2. 필수 컴포넌트 확인
	UGameAction* OwningAction = Cast<UGameAction>(GetOuter());
	if (Instigator && OwningAction && Instigator->GetAttributesComponent() && Target)
	{
		// 3. 스킬 계수 결정 (Task 자체의 DamageCoefficient 사용)
		// ChainAttack은 별도의 변수(DamageCoefficient)를 가지고 있으므로 그걸 씁니다.
		// 만약 ActionData의 기본값을 쓰고 싶다면 OwningAction->GetData().SkillCoefficient를 쓰면 됩니다.
		float SkillCoefficient = DamageCoefficient;

		UAttributesComponent* InstigatorStats = Instigator->GetAttributesComponent(); // 가해자 스탯

		if (Target->GetAttributesComponent() && Target->GetGameEventComponent())
		{
			UAttributesComponent* TargetStats = Target->GetAttributesComponent(); // 피해자 스탯

			// 4. 데미지 공식 계산
			float FinalDamage = UCombatStatics::CalculateDamage(InstigatorStats, TargetStats, SkillCoefficient);
			EDamageFloaterType DamageType = EDamageFloaterType::Normal;

			// 5. 크리티컬 계산
			if (FMath::FRand() < InstigatorStats->GetCurrentStats().fCriticalChance)
			{
				FinalDamage *= InstigatorStats->GetCurrentStats().fCriticalDamageMultiplier;
				DamageType = EDamageFloaterType::Critical;
			}

			const float RoundedDamage = FMath::RoundToFloat(FinalDamage);

			// 6. 체력 차감 적용
			Target->GetAttributesComponent()->ApplyHealthChange(-RoundedDamage, Instigator);

			// 7. 데미지 숫자(Floater) 방송
			Target->GetGameEventComponent()->BroadcastDamageFinalized(Target, RoundedDamage, DamageType, Instigator);
		}
	}
}

void UTask_ChainAttack::FinalizeJumpPhase()
{
	if (!bIsJumping) return; // 중복 호출 방지

	bIsJumping = false; // 점프 이동 멈춤

	// 몽타주 정지 (노티파이로 왔을 경우 아직 재생 중일 수 있으므로)
	if (Instigator && JumpMontage)
	{
		Instigator->StopAnimMontage(JumpMontage);
	}

	// 캐릭터 숨기기 (사라짐 연출)
	if (Instigator)
	{
		Instigator->SetActorHiddenInGame(true);
	}

	// 연타 루프 시작
	PlayNextAttack();
}

void UTask_ChainAttack::OnAttackAnimEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 공격 모션이 끝났으니 다시 사라집니다. (다음 적으로 이동하는 동안 안 보이게)
	if (Instigator)
	{
		Instigator->SetActorHiddenInGame(true);
	}

	// 인덱스를 증가시켜 다음 적을 가리킵니다.
	CurrentTargetIndex++;

	// 다음 공격 루프 실행 (재귀적 호출처럼 보이지만 꼬리를 무는 방식)
	PlayNextAttack();
}