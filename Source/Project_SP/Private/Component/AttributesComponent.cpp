// AttributesComponent.cpp

#include "Component/AttributesComponent.h"
#include "Items/OpartsBase.h"

UAttributesComponent::UAttributesComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAttributesComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeAttributes();
}

void UAttributesComponent::InitializeAttributes()
{
    if (!AttributesDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: AttributesDataTable이 지정되지 않았습니다!"), *GetName());
        return;
    }

    const FCharacterStatsData* FoundRow = AttributesDataTable->FindRow<FCharacterStatsData>(CharacterID, TEXT("Loading Attributes"));
    if (FoundRow)
    {
        BaseStats = *FoundRow;
        CurrentStats = *FoundRow;
        CurrentStats.fCurrentHealth = CurrentStats.fMaxHealth;
        SkillPoints = 3;

        UE_LOG(LogTemp, Log, TEXT("%s가 %s의 스탯으로 초기화되었습니다."), *GetOwner()->GetName(), *CharacterID.ToString());

        // 초기화된 값들을 외부에 브로드캐스트
        OnHealthChanged.Broadcast(CurrentStats.fCurrentHealth, 0.f, nullptr);
        OnSPChanged.Broadcast(SkillPoints, 0);
        OnExperienceChanged.Broadcast(Experience, NextLevelExperience);
        OnLevelChanged.Broadcast(Level);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AttributesDataTable에서 ID '%s'를 찾을 수 없습니다."), *CharacterID.ToString());
    }
}

void UAttributesComponent::ApplyHealthChange(float Delta, AActor* Instigator)
{
    const float OldHealth = CurrentStats.fCurrentHealth;
    CurrentStats.fCurrentHealth = FMath::Clamp(CurrentStats.fCurrentHealth + Delta, 0.f, CurrentStats.fMaxHealth);
    const float ActualDelta = CurrentStats.fCurrentHealth - OldHealth;

    if (ActualDelta != 0.f)
    {
        OnHealthChanged.Broadcast(CurrentStats.fCurrentHealth, ActualDelta, Instigator);
    }

    if (OldHealth > 0.f && CurrentStats.fCurrentHealth <= 0.f)
    {
        OnHealthDepleted.Broadcast(GetOwner(), Instigator);
    }
}

void UAttributesComponent::ApplySPChange(int32 Delta)
{
    const int32 OldSP = SkillPoints;
    SkillPoints = FMath::Clamp(SkillPoints + Delta, 0, MAX_SP);
    const int32 ActualDelta = SkillPoints - OldSP;

    if (ActualDelta != 0)
    {
        OnSPChanged.Broadcast(SkillPoints, ActualDelta);
    }
}

void UAttributesComponent::GainExperience(int32 Amount)
{
    if (Level >= MAX_LEVEL || Amount <= 0) return;

    Experience += Amount;
    UE_LOG(LogTemp, Log, TEXT("%s gained %d EXP! (Current: %d / %d)"), *GetOwner()->GetName(), Amount, Experience, NextLevelExperience);

    // 레벨업에 필요한 경험치를 충족했는지 반복해서 확인 (한 번에 여러 레벨업 가능)
    while (Experience >= NextLevelExperience && Level < MAX_LEVEL)
    {
        LevelUp();
    }

    // 경험치 변경 사항 브로드캐스트 (레벨업 후 남은 경험치 반영)
    OnExperienceChanged.Broadcast(Experience, NextLevelExperience);
}


// 오파츠 스탯 적용 함수
void UAttributesComponent::ApplyOpartsStats(const FOpartStats& OpartsStats)
{
    OpartsBonusStats.Health = OpartsStats.Health;
    OpartsBonusStats.Attack = OpartsStats.Attack;
    OpartsBonusStats.Speed = OpartsStats.Speed;

    RecalculateFinalStats();
}

// 오파츠 스탯 제거 함수
void UAttributesComponent::RemoveOpartsStats(const FOpartStats& OpartsStats)
{
    OpartsBonusStats.Health = FMath::Max(0.f, OpartsBonusStats.Health - OpartsStats.Health);
    OpartsBonusStats.Attack = FMath::Max(0.f, OpartsBonusStats.Attack - OpartsStats.Attack);
    OpartsBonusStats.Speed = FMath::Max(0.f, OpartsBonusStats.Speed - OpartsStats.Speed);

    RecalculateFinalStats();
}

// 유물(렐릭) 스탯 적용 함수
void UAttributesComponent::ApplyRelicStats(ERelicStatType StatType, float Value)
{
    // [1] 변경 전 상태를 미리 저장 (체력 회복 계산용)
    float OldMaxHealth = CurrentStats.fMaxHealth;

    // 스위치 문으로 어떤 스탯을 건드릴지 결정
    switch (StatType)
    {
    case ERelicStatType::MaxHealth:
        // 최대 체력 % 증가 (기본 스탯 기준)
        CurrentStats.fMaxHealth += CurrentStats.fMaxHealth * Value;
        break;

    case ERelicStatType::AttackPower:
        // 공격력 % 증가
        CurrentStats.fAttackPower += CurrentStats.fAttackPower * Value;
        break;

    case ERelicStatType::MovementSpeed:
        // 이동 속도 % 증가
        CurrentStats.fMovementSpeed += CurrentStats.fMovementSpeed *  Value;
        break;

    case ERelicStatType::DamageIncrease:
        // 피해 증가율 합산 (예: 0.07)
        CurrentStats.fDamageIncreaseMultiplier += Value;
        break;

    case ERelicStatType::DamageReduction:
        // 피해 감소율 합산
        CurrentStats.fDamageReductionMultiplier += Value;
        break;
    }

    // 현재 체력 계산 함수 추가 하기 이거는 상태이상 관련 체력 게산 함수
    //// [4] 체력이 변했다면, 늘어난 만큼 현재 체력을 회복시켜 줍니다.
    //if (StatType == ERelicStatType::MaxHealth)
    //{
    //    float NewMaxHealth = CurrentStats.fMaxHealth;
    //    float HealthDelta = NewMaxHealth - OldMaxHealth;

    //    // 양수일 때(유물 장착 시)만 회복시킵니다. 
    //    // (해제 시에는 RecalculateFinalStats 내부의 Clamp에 의해 자동으로 깎입니다)
    //    if (HealthDelta > 0.f)
    //    {
    //        ApplyHealthChange(HealthDelta, nullptr);
    //    }
    //}
}

void UAttributesComponent::LevelUp()
{
    if (Level >= MAX_LEVEL) return;

    // 현재 레벨에 필요했던 경험치만큼 차감
    Experience -= NextLevelExperience;
    Level++;

    // 다음 레벨업에 필요한 경험치 재설정 (예: 1.2배씩 증가)
    NextLevelExperience = FMath::RoundToInt(NextLevelExperience * 1.2f);

    UE_LOG(LogTemp, Warning, TEXT("%s Leveled Up to %d!"), *GetOwner()->GetName(), Level);

    // 레벨에 맞춰 스탯 재계산
    RecalculateStatsForLevel(Level);

    // 레벨업 시 체력을 모두 회복
    CurrentStats.fCurrentHealth = CurrentStats.fMaxHealth;

    // 레벨업 이벤트 브로드캐스트
    OnLevelChanged.Broadcast(Level);
    // 체력 변경 이벤트도 브로드캐스트
    OnHealthChanged.Broadcast(CurrentStats.fCurrentHealth, 0.f, nullptr);
}

void UAttributesComponent::RecalculateStatsForLevel(int32 NewLevel)
{
    if (NewLevel <= 1)
    {
        CurrentStats = BaseStats;
        return;
    }

    // 레벨업 진행도 (0.0 ~ 1.0)
    float Progress = static_cast<float>(NewLevel - 1) / (MAX_LEVEL - 1);

    // 선형 보간(Lerp)을 사용하여 스탯을 부드럽게 증가시킴
    CurrentStats.fMaxHealth = FMath::Lerp(BaseStats.fMaxHealth, BaseStats.MaxHealthCap, Progress);
    CurrentStats.fAttackPower = FMath::Lerp(BaseStats.fAttackPower, BaseStats.AttackPowerCap, Progress);

    // 다른 스탯들도 같은 방식으로 재계산...
}

// 오파츠의 스탯 보너스를 반영하여 최종 스탯 재계산 - 만든 이유는 플레이어가 레벨업 시 오파츠 보너스가 반영되지 않는 문제 해결
void UAttributesComponent::RecalculateFinalStats()
{

    RecalculateStatsForLevel(Level);

    // 2. 오파츠 보너스 합산 (CurrentStats에 OpartsBonusStats를 더함)
    CurrentStats.fMaxHealth = CurrentStats.fMaxHealth + OpartsBonusStats.Health;
    CurrentStats.fAttackPower = CurrentStats.fAttackPower + OpartsBonusStats.Attack;
    CurrentStats.fMovementSpeed = CurrentStats.fMovementSpeed + OpartsBonusStats.Speed;

    // 현재 체력 부분인데 유물 추가하면서 수정해서 좀 이상한듯 나중에 수정 더 해야 할듯 일반 배틀에 들어가면 UI에 표기되는 체력이 0임
    //ApplyHealthChange(CurrentStats.fMaxHealth, nullptr);

    // 3. 체력 및 이벤트 브로드캐스트
    // CurrentStats.fCurrentHealth는 fMaxHealth를 초과하지 않도록 Clamp
    CurrentStats.fCurrentHealth = FMath::Clamp(CurrentStats.fCurrentHealth, 0.f, CurrentStats.fMaxHealth);
    OnHealthChanged.Broadcast(CurrentStats.fCurrentHealth, 0.f, nullptr); // MaxHealth가 변했음을 알림
}
