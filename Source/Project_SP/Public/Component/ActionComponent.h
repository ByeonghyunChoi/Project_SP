// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UGameAction;
class UDataTable;
class ACombatPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionListChanged, UActionComponent*, ActionComp);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionComponent();

protected:
	// 참조할 액션 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	TObjectPtr<UDataTable> ActionDataTable;

	// 캐릭터 생성 시 기본적으로 부여할 행동	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions")
	TArray<TObjectPtr<UGameAction>> GrantedActions;

	// 사용하고 있는 행동
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UGameAction> ActiveAction;

public:
	// 캐릭터의 기본 액션들을 생성하고 초기화합니다. (몬스터용) 
	void InitializeDefaultActions(const TArray<FName>& DefaultActionIDs);

	// ID를 이용해 새로운 액션을 부여합니다. (플레이어의 무기 교체 시 사용)
	void GrantAction(FName ActionID);

	// ID를 이용해 기존 액션을 제거합니다. (플레이어의 무기 교체 시 사용) 
	void RemoveAction(FName ActionID);

	// ID를 이용해 특정 액션의 실행을 시작합니다.
	bool StartActionByID(ACombatPawn* Instigator, FName ActionID, const TArray<ACombatPawn*>& Targets);

	// 현재 실행 중인 액션을 종료 처리합니다. 
	void EndActiveAction(ACombatPawn* Instigator);

	// 접근자
	const TArray<TObjectPtr<UGameAction>>& GetGrantedActions() const { return GrantedActions; }
	UDataTable* GetActionDataTable() const { return ActionDataTable; }
	UGameAction* GetActiveAction() const { return ActiveAction; }

	// 행동 목록이 변경되었음을 알리는 이벤트 
	UPROPERTY(BlueprintAssignable)
	FOnActionListChanged OnActionListChanged;
};
