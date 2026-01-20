#include "CoreMinimal.h"
#include "MapInfo.generated.h"

UENUM(BlueprintType)
enum class EMapState :uint8
{

};

UENUM(BlueprintType)
enum class EMapType :uint8
{
	NormalBattle,
	StrongEnemyBattle,
	BossBattle,
	Jester,
	Rest,
	Prepare
};