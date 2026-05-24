#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SPDialogData.generated.h"

UENUM(BlueprintType)
enum class EClownChoiceType : uint8
{
    None			UMETA(DisplayName = "선택 안 함"),
    ReduceTP_Relic1		UMETA(DisplayName = "1. TP 20 감소 / 유물 1개 획득"),
    ReduceHP_Relic1		UMETA(DisplayName = "2. HP 50% 감소 / 유물 1개 획득"),
    ReduceATK_Relic2	UMETA(DisplayName = "3. 공격력 20% 감소 / 유물 2개 획득"),
    ReduceDEF_Relic2	UMETA(DisplayName = "4. 방어력 20% 감소 / 유물 2개 획득")
};

USTRUCT(BlueprintType)
struct FDialogChoiceData
{
    GENERATED_BODY()

    // 버튼에 표시될 텍스트 (예: "1. TP 20감소, 대신 유물 획득")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Choice")
    FText ChoiceText;

    // 이 버튼을 눌렀을 때 발동할 고정 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Choice")
    EClownChoiceType ChoiceType = EClownChoiceType::None;
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
    TArray<FDialogChoiceData> Choices;
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