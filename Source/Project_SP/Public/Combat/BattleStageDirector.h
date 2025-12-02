// Combat/BattleStageDirector.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleStageDirector.generated.h"

class APlayerCharacter;
class UMonsterGroupObject;
class AMonsterCharacter;
class ACombatPawn;

UCLASS()
class PROJECT_SP_API ABattleStageDirector : public AActor
{
	GENERATED_BODY()

public:
	ABattleStageDirector();

	// BattleTransitionManager가 호출하여 전투 준비를 시작시키는 함수
	void PrepareBattleScene(APlayerCharacter* Player, UMonsterGroupObject* MonsterGroup);

protected:
	// 디자이너가 에디터에서 몬스터 배치 간격을 조절할 수 있도록 UPROPERTY로 노출합니다.
	UPROPERTY(EditAnywhere, Category = "Battle Stage | Formation")
	float SideSpacing = 300.0f; // 몬스터 간의 '좌우' 간격

	UPROPERTY(EditAnywhere, Category = "Battle Stage | Formation")
	float DepthSpacing = 50.0f; // 몬스터 간의 '앞뒤' 간격

	UPROPERTY(EditAnywhere, Category = "Battle Stage | Formation")
	float BaseForwardDistance = 750.0f; // 플레이어로부터의 기본 전방 거리

private:
	TArray<ACombatPawn*> SpawnEnemies(UMonsterGroupObject* MonsterGroup, APlayerCharacter* PlayerToFace);

	UFUNCTION()
	void OnAssetsLoaded();

	// 콜백에 전달할 임시 데이터
	UPROPERTY() 
	TObjectPtr<APlayerCharacter> TempPlayerRef;
	UPROPERTY() 
	TObjectPtr<UMonsterGroupObject> TempMonsterGroupRef;
};