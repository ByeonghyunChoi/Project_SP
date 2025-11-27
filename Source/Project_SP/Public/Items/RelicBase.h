
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelicBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROJECT_SP_API URelicBase : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Relic")
	virtual void OnEquip(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Relic")
	virtual void OnUnequip(AActor* Instigator);

protected:
	// 유물 효과 함수 (파생 클래스에서 구현)
	virtual void ApplyRelicEffect(AActor* Target);

	virtual void RemoveRelicEffect(AActor* Target);


};
