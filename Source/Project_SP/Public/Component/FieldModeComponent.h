#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FieldModeComponent.generated.h"

class APlayerCharacter;
class AMonsterCharacter;


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UFieldModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFieldModeComponent();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 공격 범위
	UPROPERTY(EditAnywhere, Category = "FieldMode")
	float AttackRange = 300.0f;

	//공격 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldMode")
	bool bIsAttacking = false;

	//이동 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldMode")
	bool bIsMoving = true;

	// 블루프린트에서 스트리밍할 전투 맵의 이름을 지정합니다.
	UPROPERTY(EditAnywhere, Category = "Battle Transition")
	FName BattleArenaMapName;

	// 필드 맵에 배치된 '전투 무대' Target Point 액터에 지정할 태그입니다.
	UPROPERTY(EditAnywhere, Category = "Battle Transition")
	FName BattleStageTag;

	// 전투 시작 전 플레이어의 위치를 저장할 변수
	UPROPERTY()
	FVector LastFieldLocation;

	// 전투 중인지 상태를 나타내는 플래그
	UPROPERTY()
	bool bIsInBattle = false;

	// 전투를 시작할 몬스터 정보를 임시로 저장할 변수
	UPROPERTY()
	TWeakObjectPtr<AMonsterCharacter> MonsterToBattle;

	// 블루프린트에서 지정할 전환 효과 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "Battle Transition")
	TSubclassOf<UUserWidget> TransitionWidgetClass;

	// 생성된 전환 효과 위젯의 인스턴스를 저장할 변수
	UPROPERTY()
	TObjectPtr<UUserWidget> TransitionWidgetInstance;


public:

	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void StartAttackSequence();

	//공격 판정 로직 분리
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	AMonsterCharacter* PerformAttackHitDetection();

	//전투 시작
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void StartBattleTransition(AMonsterCharacter* HitMonster);

	//전투 종료
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void EndBattleTransition();

	// 공격 애니메이션 종료 시 호출될 함수 (애니메이션 노티파이에서 호출)
	UFUNCTION(BlueprintCallable, Category = "FieldMode")
	void OnAttackAnimationFinished();

protected:
	// 전투 맵 로딩이 완료되면 호출될 콜백 함수
	UFUNCTION()
	void OnBattleArenaLoaded();

	// 전투 맵 언로딩이 완료되면 호출될 콜백 함수
	UFUNCTION()
	void OnBattleArenaUnloaded();
private:
	void StartLoadingBattleMap();
	void UnloadBattleMap();
	void StartFadeOut();
};