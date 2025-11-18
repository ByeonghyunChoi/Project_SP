
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelicBase.generated.h"

UCLASS()
class PROJECT_SP_API URelicBase : public UObject
{
	GENERATED_BODY()
	
protected:
	// 유물 효과 함수 (파생 클래스에서 구현)
	virtual void ApplyRelicEffect();

	virtual void RemoveRelicEffect();


};
