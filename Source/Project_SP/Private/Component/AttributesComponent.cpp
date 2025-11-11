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

    const FCombatStats* FoundRow = AttributesDataTable->FindRow<FCombatStats>(CharacterID, TEXT("Loading Attributes"));
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
        OnMoneyChanged.Broadcast(Money, 0);
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

void UAttributesComponent::ApplyMoneyChange(int32 Delta)
{
    const int32 OldMoney = Money;
    Money = FMath::Max(0, Money + Delta); // 돈이 음수가 되지 않도록 함
    const int32 ActualDelta = Money - OldMoney;

    if (ActualDelta != 0)
    {
        OnMoneyChanged.Broadcast(Money, ActualDelta);
    }
}

// 오파츠 스탯 적용 함수
void UAttributesComponent::ApplyOpartsStats(const FOpartStats& OpartsStats)
{
    OpartsBonusStats.fMaxHealth += OpartsStats.Health;
    OpartsBonusStats.fAttackPower += OpartsStats.Attack;
    OpartsBonusStats.fMovementSpeed += OpartsStats.Speed;

    RecalculateFinalStats();
}

// 오파츠 스탯 제거 함수
void UAttributesComponent::RemoveOpartsStats(const FOpartStats& OpartsStats)
{
    OpartsBonusStats.fMaxHealth = FMath::Max(0.f, OpartsBonusStats.fMaxHealth - OpartsStats.Health);
    OpartsBonusStats.fAttackPower = FMath::Max(0.f, OpartsBonusStats.fAttackPower - OpartsStats.Attack);
    OpartsBonusStats.fMovementSpeed = FMath::Max(0.f, OpartsBonusStats.fMovementSpeed - OpartsStats.Speed);

    RecalculateFinalStats();
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
    CurrentStats.fDefensePower = FMath::Lerp(BaseStats.fDefensePower, BaseStats.DefensePowerCap, Progress);

    // 다른 스탯들도 같은 방식으로 재계산...
}

// 오파츠의 스탯 보너스를 반영하여 최종 스탯 재계산 - 만든 이유는 플레이어가 레벨업 시 오파츠 보너스가 반영되지 않는 문제 해결
void UAttributesComponent::RecalculateFinalStats()
{
    RecalculateStatsForLevel(Level);

    // 2. 오파츠 보너스 합산 (CurrentStats에 OpartsBonusStats를 더함)
    CurrentStats.fMaxHealth += OpartsBonusStats.fMaxHealth;
    CurrentStats.fAttackPower += OpartsBonusStats.fAttackPower;
    CurrentStats.fMovementSpeed += OpartsBonusStats.fMovementSpeed;

	CurrentStats.fCurrentHealth += OpartsBonusStats.fMaxHealth; // 현재 체력도 오파츠 보너스만큼 증가시킴

    // 3. 체력 및 이벤트 브로드캐스트
    // CurrentStats.fCurrentHealth는 fMaxHealth를 초과하지 않도록 Clamp
    CurrentStats.fCurrentHealth = FMath::Clamp(CurrentStats.fCurrentHealth, 0.f, CurrentStats.fMaxHealth);
    OnHealthChanged.Broadcast(CurrentStats.fCurrentHealth, 0.f, nullptr); // MaxHealth가 변했음을 알림
}
