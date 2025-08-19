#include "Combat/MonsterGroupObject.h"

TArray<FMonsterData> UMonsterGroupObject::GetAllMonsterDataInGroup() const
{
    TArray<FMonsterData> Result;
    if (!MonsterDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("MonsterDataTable is not set in UMonsterGroupObject."));
        return Result;
    }

    for (const FName& MonsterID : MonsterIDsInGroup)
    {
        FMonsterData* FoundData = MonsterDataTable->FindRow<FMonsterData>(MonsterID, TEXT("Looking for Monster Data in Group"));
        if (FoundData)
        {
            Result.Add(*FoundData);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find MonsterID '%s' in MonsterDataTable."), *MonsterID.ToString());
        }
    }
    return Result;
}

FMonsterData UMonsterGroupObject::GetMonsterData(FName MonsterID) const
{
    if (MonsterDataTable)
    {
        FMonsterData* FoundData = MonsterDataTable->FindRow<FMonsterData>(MonsterID, TEXT("Looking for specific Monster Data"));
        if (FoundData)
        {
            return *FoundData;
        }
    }
    return FMonsterData(); // 데이터 없으면 기본 생성자 반환
}

void UMonsterGroupObject::CopyMonterGroup(UMonsterGroupObject* OtherGroup)
{
    if (OtherGroup)
    {
        MonsterDataTable = OtherGroup->MonsterDataTable;
        MonsterIDsInGroup = OtherGroup->MonsterIDsInGroup;
    }
}