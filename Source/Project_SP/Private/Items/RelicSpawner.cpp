// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RelicSpawner.h"
#include "Items/RelicBase.h"
#include "Engine/DataTable.h"

URelicSpawner::URelicSpawner()
{

}

TArray<FRelicData> URelicSpawner::GenerateRandomRelicChoices(UDataTable* RelicDataTable, const TArray<TSubclassOf<URelicBase>>& EquippedRelicClasses)
{
    TArray<FRelicData> ResultChoices;

	// 1. 데이터 테이블 유효성 검사
	if (!RelicDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("RelicSpawner: 데이터 테이블이 유효하지 않습니다!"));
		return ResultChoices;
	}

	// 2. 모든 Row Name 가져오기 (데이터 전체를 복사하는 것보다 이름만 가져오는 게 효율적)
	TArray<FName> AllRowNames = RelicDataTable->GetRowNames();
	TArray<FName> ValidRowNames;

	// 3. 필터링 (장착된 유물 제외)
	static const FString ContextString(TEXT("RelicGeneration"));

	for (const FName& RowName : AllRowNames)
	{
		FRelicData* RowData = RelicDataTable->FindRow<FRelicData>(RowName, ContextString);
		if (RowData)
		{
			// 유효한 클래스가 있고, 아직 장착하지 않은 유물만 후보에 추가
			if (RowData->RelicClass && !EquippedRelicClasses.Contains(RowData->RelicClass))
			{
				ValidRowNames.Add(RowName);
			}
		}
	}

	// 4. 셔플 및 선택 (유효한 후보가 3개 미만이면 전부 다 선택)
	int32 CandidatesCount = ValidRowNames.Num();
	int32 SelectionCount = FMath::Min(CandidatesCount, 3); // 최대 3개

	// 셔플 로직 (후보 배열의 앞부분 3개만 랜덤으로 섞어 뽑음)
	for (int32 i = 0; i < SelectionCount; ++i)
	{
		int32 RandomIndex = FMath::RandRange(i, CandidatesCount - 1);
		if (i != RandomIndex)
		{
			ValidRowNames.Swap(i, RandomIndex);
		}

		// 5. 선택된 RowName으로 실제 데이터 가져와서 결과 배열에 추가
		FRelicData* SelectedData = RelicDataTable->FindRow<FRelicData>(ValidRowNames[i], ContextString);
		if (SelectedData)
		{
			ResultChoices.Add(*SelectedData);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RelicSpawner: %d 개의 후보 중 %d 개를 선택했습니다."), CandidatesCount, ResultChoices.Num());

	return ResultChoices;
}

