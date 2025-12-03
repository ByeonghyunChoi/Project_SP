// Component/ActionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UGameAction;
class UDataTable;
class ACombatPawn;
struct FActionData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionListChanged, UActionComponent*, ActionComp);
/**
 * @class UActionComponent
 * @brief 캐릭터에 부착되는 '공장'.
 * 데이터 테이블('설계도')을 읽어 UGameAction('제품')을 생산하고 관리하며,
 * 외부의 요청에 따라 액션을 실행시킵니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionComponent();

	UPROPERTY(BlueprintAssignable)
	FOnActionListChanged OnActionListChanged;

	void InitializeDefaultActions(const TArray<FName>& DefaultActionIDs);
	void GrantAction(FName ActionID);
	void RemoveAction(FName ActionID);
	bool StartActionByID(ACombatPawn* Instigator, FName ActionID, const TArray<ACombatPawn*>& Targets);
	void EndActiveAction(ACombatPawn* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByName(ACombatPawn* Instigator, FName ActionName);

	UFUNCTION(BlueprintPure, Category = "Actions")
	UGameAction* GetActiveAction() const { return ActiveAction; }

	UDataTable* GetActionDataTable() const { return ActionDataTable; }

	UFUNCTION(BlueprintPure)
	bool GetActionData(FName ActionID, FActionData& OutActionData) const;
	const TArray<TObjectPtr<UGameAction>>& GetGrantedActions() const { return GrantedActions; }

	//액션 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ResetActiveAction();

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ReduceCooldowns();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TMap<FName, int32> CooldownMap;

protected:
	// 참조할 '설계도' 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	TObjectPtr<UDataTable> ActionDataTable;

	// '생산'해서 '보관'하고 있는 액션(제품) 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions")
	TArray<TObjectPtr<UGameAction>> GrantedActions;

	//현재 사용하고 있는 행동을 보관할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UGameAction> ActiveAction;
};