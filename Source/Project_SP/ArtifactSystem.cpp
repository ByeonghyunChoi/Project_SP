#include "ArtifactSystem.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"

UArtifactSystem::UArtifactSystem()
{
    // 컴포넌트 기본 설정
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;

    // 기본 스탯 초기화
    TotalStatBonus = FStatBonus();

    // 장착된 아티팩트 맵 초기화
    EquippedArtifacts.Empty();

    // 모든 아티팩트 타입에 대해 빈 슬롯 생성
    for (int32 i = 1; i <= 5; ++i)
    {
        EArtifactType Type = static_cast<EArtifactType>(i);
        FEquippedArtifact EmptyArtifact;
        EmptyArtifact.bIsEquipped = false;
        EquippedArtifacts.Add(Type, EmptyArtifact);
    }

    // 오파츠 슬롯 초기화
    EquippedOpartz.bIsEquipped = false;
}

void UArtifactSystem::BeginPlay() // 게임시작
{
    Super::BeginPlay();

    if (!ArtifactDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("ArtifactDataTable is NULL at BeginPlay!"));
    }

    if (!SetBonusDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("SetBonusDataTable is NULL at BeginPlay!"));
    }

    // 게임 시작 시 초기 스탯 계산
    RecalculateStats(); //326
    RecalculateSetBonuses(); //357
}

bool UArtifactSystem::EquipArtifact(FName ArtifactID) //일반 아티팩트 장착
{
    // 아티팩트 데이터 로드
    FArtifactData* ArtifactData = LoadArtifactData(ArtifactID);
    if (!ArtifactData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load artifact data: %s"), *ArtifactID.ToString());
        return false;
    }

    // 오파츠인지 확인
    if (ArtifactData->bIsOpartz)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot equip Opartz using EquipArtifact. Use EquipOpartz instead."));
        return false;
    }

    // 장착 가능 여부 확인
    if (!CanEquipArtifact(ArtifactID))
    {
        return false;
    }

    // 해당 타입 슬롯에 장착
    EArtifactType Type = ArtifactData->Type;
    FEquippedArtifact& EquippedSlot = EquippedArtifacts[Type];

    // 기존 아티팩트가 있다면 해제
    if (EquippedSlot.bIsEquipped)
    {
        UnequipArtifact(Type);
    }

    // 새 아티팩트 장착
    EquippedSlot.ArtifactID = ArtifactID;
    EquippedSlot.bIsEquipped = true;
    EquippedSlot.ArtifactData = *ArtifactData;

    // 스탯 재계산
    RecalculateStats();
    RecalculateSetBonuses();

    // 이벤트 브로드캐스트
    OnArtifactEquipped.Broadcast(Type, ArtifactID, true);

    UE_LOG(LogTemp, Log, TEXT("Artifact equipped: %s"), *ArtifactData->Name);
    return true;
}

bool UArtifactSystem::UnequipArtifact(EArtifactType ArtifactType) //특정 타입의 아틱팩트 해제
{
    // 유효한 타입인지 확인
    if (ArtifactType == EArtifactType::None)
    {
        return false;
    }

    // 해당 슬롯에 장착된 아티팩트가 있는지 확인
    FEquippedArtifact& EquippedSlot = EquippedArtifacts[ArtifactType];
    if (!EquippedSlot.bIsEquipped)
    {
        return false;
    }

    // 아티팩트 해제
    FName RemovedArtifactID = EquippedSlot.ArtifactID;
    EquippedSlot.ArtifactID = NAME_None;
    EquippedSlot.bIsEquipped = false;
    EquippedSlot.ArtifactData = FArtifactData();

    // 스탯 재계산
    RecalculateStats();
    RecalculateSetBonuses();

    // 이벤트 브로드캐스트
    OnArtifactEquipped.Broadcast(ArtifactType, RemovedArtifactID, false);

    UE_LOG(LogTemp, Log, TEXT("Artifact unequipped from slot: %d"), static_cast<int32>(ArtifactType));
    return true;
}

bool UArtifactSystem::EquipOpartz(FName OpartzID) // 오파츠 장착
{
    // 오파츠 데이터 로드
    FArtifactData* OpartzData = LoadArtifactData(OpartzID);
    if (!OpartzData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load opartz data: %s"), *OpartzID.ToString());
        return false;
    }

    // 오파츠인지 확인
    if (!OpartzData->bIsOpartz)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item is not an Opartz: %s"), *OpartzID.ToString());
        return false;
    }

    // 기존 오파츠가 있다면 해제
    if (EquippedOpartz.bIsEquipped)
    {
        UnequipOpartz();
    }

    // 새 오파츠 장착
    EquippedOpartz.ArtifactID = OpartzID;
    EquippedOpartz.bIsEquipped = true;
    EquippedOpartz.ArtifactData = *OpartzData;

    // 스탯 재계산
    RecalculateStats();

    UE_LOG(LogTemp, Log, TEXT("Opartz equipped: %s"), *OpartzData->Name);
    return true;
}

bool UArtifactSystem::UnequipOpartz() // 장착된 오파츠 해제하고 스탯 재계산
{
    // 장착된 오파츠가 있는지 확인
    if (!EquippedOpartz.bIsEquipped)
    {
        return false;
    }

    // 오파츠 해제
    EquippedOpartz.ArtifactID = NAME_None;
    EquippedOpartz.bIsEquipped = false;
    EquippedOpartz.ArtifactData = FArtifactData();

    // 스탯 재계산
    RecalculateStats();

    UE_LOG(LogTemp, Log, TEXT("Opartz unequipped"));
    return true;
}

FEquippedArtifact UArtifactSystem::GetEquippedArtifact(EArtifactType ArtifactType) const // 특정 타입의 아티팩트 정보 가져오기
{
    // 유효한 타입이고 장착된 아티팩트가 있는지 확인
    if (ArtifactType != EArtifactType::None && EquippedArtifacts.Contains(ArtifactType))
    {
        return EquippedArtifacts[ArtifactType];
    }

    // 빈 아티팩트 반환
    return FEquippedArtifact();
}

FEquippedArtifact UArtifactSystem::GetEquippedOpartz() const // 장착된 오파츠 정보 가져오기
{
    return EquippedOpartz;
}

FStatBonus UArtifactSystem::GetTotalStatBonus() const // 현재 총 스탯 보너스 가져오기
{
    return TotalStatBonus;
}

int32 UArtifactSystem::GetSetPieceCount(EArtifactSetType SetType) const // 특정 세트 타입의 장착된 피스 수 가져오기
{
    // 활성화된 세트 카운트에서 해당 세트의 피스 수 반환
    if (ActiveSetCounts.Contains(SetType))
    {
        return ActiveSetCounts[SetType];
    }
    return 0;
}

TArray<FSetBonusData> UArtifactSystem::GetActiveSetBonuses() const // 현재 활성화된 세트 보너스 목록 가져오기
{
    TArray<FSetBonusData> ActiveBonuses;

    // 각 세트 타입에 대해 활성화된 보너스 확인
    for (const auto& SetCount : ActiveSetCounts)
    {
        EArtifactSetType SetType = SetCount.Key;
        int32 PieceCount = SetCount.Value;

        // 2피스, 4피스 보너스 확인
        if (PieceCount >= 2)
        {
            FSetBonusData* BonusData = LoadSetBonusData(SetType, 2);
            if (BonusData)
            {
                ActiveBonuses.Add(*BonusData);
            }
        }

        if (PieceCount >= 4)
        {
            FSetBonusData* BonusData = LoadSetBonusData(SetType, 4);
            if (BonusData)
            {
                ActiveBonuses.Add(*BonusData);
            }
        }
    }

    return ActiveBonuses;
}

bool UArtifactSystem::GetArtifactData(FName ArtifactID, FArtifactData& OutArtifactData) const // 특정 아티팩트 ID에 대한 데이터 가져오기
{
    // 데이터 테이블에서 아티팩트 데이터 로드
    FArtifactData* Data = LoadArtifactData(ArtifactID);
    if (Data)
    {
        OutArtifactData = *Data;
        return true;
    }
    return false;
}

void UArtifactSystem::UnequipAllArtifacts() // 모든 아티팩트 해제
{
    // 모든 아티팩트 슬롯 해제
    for (auto& EquippedPair : EquippedArtifacts)
    {
        if (EquippedPair.Value.bIsEquipped)
        {
            UnequipArtifact(EquippedPair.Key);
        }
    }

    // 오파츠도 해제
    if (EquippedOpartz.bIsEquipped)
    {
        UnequipOpartz();
    }
}

void UArtifactSystem::UnequipArtifactSet(EArtifactSetType SetType) // 특정 세트 타입의 아티팩트 해제
{
    // 해당 세트의 모든 아티팩트 해제
    for (auto& EquippedPair : EquippedArtifacts)
    {
        if (EquippedPair.Value.bIsEquipped &&
            EquippedPair.Value.ArtifactData.SetType == SetType)
        {
            UnequipArtifact(EquippedPair.Key);
        }
    }
}

bool UArtifactSystem::CanEquipArtifact(FName ArtifactID) const // 아티팩트 장착 가능 여부 확인
{
    // 아티팩트 데이터 로드
    FArtifactData* ArtifactData = LoadArtifactData(ArtifactID);
    if (!ArtifactData)
    {
        return false;
    }

    // 오파츠는 따로 처리
    if (ArtifactData->bIsOpartz)
    {
        return false;
    }

    // 플레이어 레벨 확인 (여기서는 간단히 true 반환)
    // 실제 게임에서는 플레이어 레벨과 비교
    return true;
}

TArray<FEquippedArtifact> UArtifactSystem::GetAllEquippedArtifacts() const // 현재 장착된 모든 아티팩트 정보 가져오기
{
    TArray<FEquippedArtifact> Result;

    // 모든 장착된 아티팩트 추가
    for (const auto& EquippedPair : EquippedArtifacts)
    {
        if (EquippedPair.Value.bIsEquipped)
        {
            Result.Add(EquippedPair.Value);
        }
    }

    // 오파츠도 추가
    if (EquippedOpartz.bIsEquipped)
    {
        Result.Add(EquippedOpartz);
    }

    return Result;
}

void UArtifactSystem::RecalculateStats() // 현재 장착된 아티팩트들의 스탯 보너스 재계산
{
    // 스탯 보너스 초기화
    TotalStatBonus = FStatBonus();

    // 장착된 모든 아티팩트의 스탯 합산
    for (const auto& EquippedPair : EquippedArtifacts)
    {
        if (EquippedPair.Value.bIsEquipped)
        {
            TotalStatBonus = TotalStatBonus + EquippedPair.Value.ArtifactData.StatBonus;
        }
    }

    // 오파츠 스탯 합산
    if (EquippedOpartz.bIsEquipped)
    {
        TotalStatBonus = TotalStatBonus + EquippedOpartz.ArtifactData.StatBonus;
    }

    // 세트 보너스 추가
    TArray<FSetBonusData> ActiveBonuses = GetActiveSetBonuses();
    for (const FSetBonusData& BonusData : ActiveBonuses)
    {
        TotalStatBonus = TotalStatBonus + BonusData.SetBonus;
    }

    // 스탯 변경 이벤트 브로드캐스트
    OnStatsChanged.Broadcast(TotalStatBonus);
}

void UArtifactSystem::RecalculateSetBonuses() // 현재 장착된 아티팩트들의 세트 보너스 재계산
{
    // 세트 카운트 업데이트
    UpdateSetCounts();

    // 활성화된 세트 보너스 이벤트 브로드캐스트
    for (const auto& SetCount : ActiveSetCounts)
    {
        if (SetCount.Value > 0)
        {
            OnSetBonusActivated.Broadcast(SetCount.Key, SetCount.Value);
        }
    }
}

void UArtifactSystem::UpdateSetCounts() // 현재 장착된 아티팩트들의 세트 카운트 업데이트
{
    // 세트 카운트 초기화
    ActiveSetCounts.Empty();

    // 장착된 아티팩트들의 세트 카운트
    for (const auto& EquippedPair : EquippedArtifacts)
    {
        if (EquippedPair.Value.bIsEquipped)
        {
            EArtifactSetType SetType = EquippedPair.Value.ArtifactData.SetType;
            if (SetType != EArtifactSetType::None)
            {
                if (ActiveSetCounts.Contains(SetType))
                {
                    ActiveSetCounts[SetType]++;
                }
                else
                {
                    ActiveSetCounts.Add(SetType, 1);
                }
            }
        }
    }
}

//FArtifactData* UArtifactSystem::LoadArtifactData(FName ArtifactID) const // 특정 아티팩트 ID에 대한 데이터 로드
//{
//    // 데이터 테이블이 유효한지 확인
//    if (!ArtifactDataTable)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("ArtifactDataTable is not set"));
//        return nullptr;
//    }
//
//    // 데이터 테이블에서 아티팩트 데이터 찾기
//    FArtifactData* ArtifactData = ArtifactDataTable->FindRow<FArtifactData>(ArtifactID, TEXT(""));
//    if (!ArtifactData)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Artifact data not found: %s"), *ArtifactID.ToString());
//        return nullptr;
//    }
//
//    return ArtifactData;
//}

FSetBonusData* UArtifactSystem::LoadSetBonusData(EArtifactSetType SetType, int32 PieceCount) const // 특정 세트 타입과 피스 수에 대한 세트 보너스 데이터 로드
{
    // 데이터 테이블이 유효한지 확인
    if (!SetBonusDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetBonusDataTable is not set"));
        return nullptr;
    }

    // 세트 보너스 ID 생성 (예: "Crystal_2", "Emerald_4" 등)
    FString EnumString = UEnum::GetValueAsString(SetType);
    int32 Index;
    if (EnumString.FindLastChar(':', Index))
    {
        EnumString = EnumString.RightChop(Index + 1);
    }
    FString SetBonusID = FString::Printf(TEXT("%s_%d"), *EnumString, PieceCount);

    // 데이터 테이블에서 세트 보너스 데이터 찾기
    FSetBonusData* SetBonusData = SetBonusDataTable->FindRow<FSetBonusData>(FName(*SetBonusID), TEXT(""));
    if (!SetBonusData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Set bonus data not found: %s"), *SetBonusID);
        return nullptr;
    }

    return SetBonusData;
}

