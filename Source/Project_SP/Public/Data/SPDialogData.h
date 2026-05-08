#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SPDialogData.generated.h"

USTRUCT(BlueprintType)
struct FDialogLineData : public FTableRowBase
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FName SpeakerID; //

    // 화자 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
    FText SpeakerName;

    // 대사 내용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog", meta = (MultiLine = true))
    FText DialogText;
};

USTRUCT(BlueprintType)
struct FDialogAssetData : public FTableRowBase
{
	GENERATED_BODY()

public:

    // 말하는 화자 일러스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
    TSoftObjectPtr<UTexture2D> StandinIllustration;

    // 타자기 효과와 함께 출력될 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
    TSoftObjectPtr<class USoundBase> TypingSound;

    // 나중에 특정 대사에서 이벤트를 발생시키고 싶을 때를 대비한 태그 - 안들어 갈 수도 있음
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
    FGameplayTag OptionalEventTag;
};