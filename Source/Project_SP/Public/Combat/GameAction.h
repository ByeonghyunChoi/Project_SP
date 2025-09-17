#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/ActionData.h" 
#include "GameAction.generated.h"

class ACombatPawn;
class UActionComponent;
class UAttributesComponent;

/**
 *
 */
UCLASS(Blueprintable, Abstract)
class PROJECT_SP_API UGameAction : public UObject
{
	GENERATED_BODY()
public:
	//필요 정보 초기화
	void Initialize(UActionComponent* InOwningComponent, FName InActionID);

	//실행 여부 체크
	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStartAction(ACombatPawn* instigator);
	virtual bool CanStartAction_Implementation(ACombatPawn* Instigator);

	//액션 실행
	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction(ACombatPawn* Instigator, const TArray<ACombatPawn*>& Targets);
	virtual void StartAction_Implementation(ACombatPawn* Instigator, const TArray<ACombatPawn*>& Targets);

	//해당 액션의 모든 데이터 반환
	UFUNCTION(BlueprintPure, Category = "Action")
	const FActionData& GetData() const { return Data; }

	//해당 액션의 ID 반환
	UFUNCTION(BlueprintPure, Category = "Action")
	FName GetActionID() const { return ActionID; }

	//액션 완료, 턴 종료
	UFUNCTION(BlueprintCallable, Category = "Action")
	void EndAction(ACombatPawn* Instigator);

protected:
	//해당 액션을 소유한 컴포넌트 참조
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UActionComponent> OwningComponent;

	//해당 액션의 모든 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	FActionData Data;

	//해당 액션의 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	FName ActionID;

	
};