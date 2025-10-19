// Combat/GameAction.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/ActionData.h" 
#include "GameAction.generated.h"

class UActionComponent;
class ACombatPawn;
class UCombatTask;

DECLARE_DELEGATE_OneParam(FOnCombatEvent, FName);

UCLASS(Blueprintable, Abstract)
class PROJECT_SP_API UGameAction : public UObject
{
	GENERATED_BODY()

public:
	// '공장(ActionComponent)'이 '제품'을 만들 때 호출하여 필요한 정보를 주입합니다.
	void Initialize(UActionComponent* InOwningComponent, FName InActionID);

	// 이 액션을 시작할 수 있는지 조건을 확인합니다. (예: SP가 충분한가?)
	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStartAction(ACombatPawn* Instigator);

	// 이 액션의 실제 로직을 수행합니다.
	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction(ACombatPawn* Instigator, const TArray<ACombatPawn*>& Targets);

	// 액션이 모두 끝나면, BattleManager에 보고하여 턴을 종료시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Action")
	void EndAction(ACombatPawn* Instigator);

	// Getter 함수
	UFUNCTION(BlueprintPure, Category = "Action")
	FName GetActionID() const { return ActionID; }

	UFUNCTION(BlueprintPure, Category = "Action")
	const FActionData& GetData() const { return Data; }

	FOnCombatEvent OnCombatEvent;
protected:
	// 자신을 소유한 '공장' 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UActionComponent> OwningComponent;

	// 자신의 '설계도' 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	FActionData Data;

	// 자신의 ID (데이터 테이블의 Row Name)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	FName ActionID;

	//실행할 작업 순서
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Action Sequence")
	TArray<TObjectPtr<UCombatTask>> Tasks;

};