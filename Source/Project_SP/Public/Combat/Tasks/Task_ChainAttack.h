// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_ChainAttack.generated.h"


class UAnimMontage;
/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_ChainAttack : public UCombatTask
{
	GENERATED_BODY()

public:
	virtual void ExecuteTask_Implementation() override;

	// 시간이 걸리는 태스크이므로 Latent로 설정합니다.
	virtual bool IsLatent() const override { return true; }

	// [핵심] BattleManager가 보내주는 노티파이 신호를 받는 함수 (부모 함수 오버라이드)
	virtual void OnNotifyReceived(FName NotifyName) override;

	// 점프 이동 Tick
	virtual void TickTask(float DeltaTime) override;

protected:
	// 1. 시작 시 재생할 점프 몽타주 (하늘로 솟구치는 모션)
	UPROPERTY(EditAnywhere, Category = "Chain Attack")
	TObjectPtr<UAnimMontage> JumpMontage;
	// 점프 이동 좌표
	UPROPERTY(EditAnywhere, Category = "ChainAttack|Jump")
	FVector JumpOffset = FVector(0.f, 0.f, 500.f);
	// 점프 중 공격 전환 노티파이
	UPROPERTY(EditAnywhere, Category = "Chain Attack|Jump")
	FName JumpEndNotifyName = TEXT("JumpEnd");
	// 점프 속도
	UPROPERTY(EditAnywhere, Category = "Chain Attack|Jump")
	float JumpMoveSpeed = 1000.0f;

	// 2. 각 적에게 반복 재생할 공격 몽타주 (나타나서 찌르는 모션)
	UPROPERTY(EditAnywhere, Category = "Chain Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 데미지 적용을 위해 기다릴 노티파이 이름 (기본값: "Hit")
	UPROPERTY(EditAnywhere, Category = "Chain Attack")
	FName HitNotifyName = TEXT("Hit");

	// 데미지 배율 (기본 공격력 * 이 값)
	UPROPERTY(EditAnywhere, Category = "Chain Attack")
	float DamageCoefficient = 1.0f;

	// 공격 위치 오프셋 (적 기준, X=150이면 적 1.5m 앞)
	UPROPERTY(EditAnywhere, Category = "Chain Attack")
	FVector AttackOffset = FVector(150.f, 0.f, 0.f);

private:
	// 공격할 적들의 목록을 저장할 배열
	UPROPERTY()
	TArray<ACombatPawn*> SortedTargets;

	// 현재 몇 번째 적을 공격 중인지 추적하는 인덱스
	int32 CurrentTargetIndex = 0;

	// 점프 중인지 확인하는 플래그
	bool bIsJumping = false;

	//점프 목표 위치 저장
	FVector JumpTargetLocation;

	// --- 내부 로직 함수들 ---

	// 1단계: 점프 시작
	void PlayJumpAnim();

	// 점프 몽타주가 끝났을 때 호출되는 콜백
	UFUNCTION()
	void OnJumpAnimEnded(UAnimMontage* Montage, bool bInterrupted);

	// 2단계: 다음 타겟 공격 시작 (루프)
	void PlayNextAttack();

	// 공격 몽타주가 끝났을 때 호출되는 콜백
	UFUNCTION()
	void OnAttackAnimEnded(UAnimMontage* Montage, bool bInterrupted);

	// 실제 데미지를 주는 함수
	void ApplyDamageToCurrentTarget();

	// 점프 마무리 공통 함수
	void FinalizeJumpPhase();

};
