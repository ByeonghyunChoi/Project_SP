// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EquipmentSystemComponent.h"
#include "Component/AttributesComponent.h"
#include "Items/OpartsBase.h"
#include "Items/CrystalSkullOparts.h"
#include "Items/JadeClockOparts.h"
#include "Items/GoldBugOparts.h"

UEquipmentSystemComponent::UEquipmentSystemComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

}

void UEquipmentSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	// 1. 어트리뷰트 컴포넌트 찾기
	if (GetOwner())
	{
		AttributesComp = GetOwner()->FindComponentByClass<UAttributesComponent>();
		UE_LOG(LogTemp, Log, TEXT("능력치 컴포넌트 발견"));

		CrystalSkull = GetOwner()->FindComponentByClass<UCrystalSkullOparts>();
		JadeClock = GetOwner()->FindComponentByClass<UJadeClockOparts>();
		GoldBug = GetOwner()->FindComponentByClass<UGoldBugOparts>();
	}

	// 2. [추가] 게임 시작 시, 현재 설정된 오파츠(기본 0번 등)를 강제로 활성화해서 스탯을 적용시킵니다.
	// 이렇게 해야 시작하자마자 오파츠 스탯이 반영됩니다.
	// 만약 저장된 데이터가 있다면 로드 후 호출해도 됩니다.
	SetActiveOparts(ActiveOpartsIndex);
}

// PlayerCharacter에서 시작 시 포인터를 설정하는 함수
void UEquipmentSystemComponent::InitializeOpartsPointers(UCrystalSkullOparts* Skull, UJadeClockOparts* Clock, UGoldBugOparts* Bug)
{
	CrystalSkull = Skull;
	JadeClock = Clock;
	GoldBug = Bug;
}

void UEquipmentSystemComponent::SetActiveOparts(int32 NewIndex)
{
	// 1. 활성화 인덱스 변경
	ActiveOpartsIndex = NewIndex;

	// 2. 어트리뷰트 컴포넌트가 있는지 확인 (없으면 아무것도 못함)
	if (!AttributesComp)
	{
		// 만약 변수가 없다면 찾아옵니다.
		AttributesComp = GetOwner()->FindComponentByClass<UAttributesComponent>();
		if (!AttributesComp) return;
	}

	// 3. 현재 활성화된 오파츠 객체 가져오기
	// (GetActiveOparts 함수는 switch문으로 0:해골, 1:시계 등을 반환하도록 이미 구현하셨죠?)
	UOpartsBase* ActiveOparts = GetActiveOparts();

	// 4. 스탯 덮어쓰기 로직
	if (ActiveOparts)
	{
		// [케이스 A] 오파츠가 장착된 경우

		// 오파츠에게 "네 레벨이랑 아티팩트 계산해서 최종 스탯 줘" 라고 요청
		FStatModifiers NewStats = ActiveOparts->GetCalculatedModifiers();

		// 어트리뷰트 컴포넌트에게 "이걸로 덮어씌워!" 명령 (Update 함수 사용)
		AttributesComp->UpdateOpartsModifiers(NewStats);

		UE_LOG(LogTemp, Log, TEXT("오파츠 교체 완료: %s (HP+%f)"), *ActiveOparts->GetName(), NewStats.FlatHP);
	}
	else
	{
		// [케이스 B] 오파츠가 없는 경우 (NewIndex가 -1이거나 잘못된 경우)
		// 스탯을 0으로 초기화해줘야 합니다. 안 그러면 이전 오파츠 스탯이 남습니다.

		FStatModifiers EmptyStats; // 생성자가 모든 값을 0으로 초기화함
		AttributesComp->UpdateOpartsModifiers(EmptyStats);

		UE_LOG(LogTemp, Log, TEXT("오파츠 해제 완료 (스탯 초기화)"));
	}
}

UOpartsBase* UEquipmentSystemComponent::GetActiveOparts() const
{
	switch (ActiveOpartsIndex)
	{
	case 0:
		return CrystalSkull;
	case 1:
		return JadeClock;
	case 2:
		return GoldBug;
	default:
		return nullptr;
	}
}

void UEquipmentSystemComponent::TryUpgradeOparts()
{
	UOpartsBase* ActiveOparts = GetActiveOparts();

	// 1. 예외 처리 (오파츠가 없거나 이미 만렙이면 중단)
	if (!ActiveOparts) return;
	if (ActiveOparts->GetOpartsCurrentLevel() >= 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 최대 레벨입니다."));
		return;
	}

	// (여기서 모래 자원 소모 로직 체크)

	// =========================================================
	// 2. [수정] 이미 있는 함수 활용해서 레벨 업!
	// =========================================================
	ActiveOparts->LevelUpOparts();

	// =========================================================
	// 3. [중요] ★스탯 재적용★ (이게 꼭 있어야 합니다!)
	// =========================================================
	// 레벨이 올랐으니(Level_1 -> Level_2), 
	// 데이터 테이블에서 다시 값을 긁어와서 플레이어에게 덮어씌웁니다.
	SetActiveOparts(ActiveOpartsIndex);

	UE_LOG(LogTemp, Log, TEXT("오파츠 강화 완료! 현재 레벨: %d"), ActiveOparts->GetOpartsCurrentLevel());
}

void UEquipmentSystemComponent::SetOpartsActiveState(UOpartsBase* Oparts, bool bActive)
{
	if (Oparts)
	{
		// 1. 컴포넌트의 전체 활성화 상태 변경
		Oparts->SetActive(bActive);

		// 2. 오파츠의 장착/해제 로직 호출 (스탯 적용/제거)
		if (bActive)
		{
			Oparts->OnEquip(GetOwner());
		}
		else
		{
			Oparts->OnUnequip(GetOwner());
		}
	}
}

void UEquipmentSystemComponent::ActivateCrystalSkull()
{
	ActiveOpartsIndex = 0;
	// 수정 해골 활성화
	SetOpartsActiveState(CrystalSkull, true);

	// 나머지 두 개 비활성화
	SetOpartsActiveState(JadeClock, false);
	SetOpartsActiveState(GoldBug, false);

	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}

	SetActiveOparts(ActiveOpartsIndex);
}

void UEquipmentSystemComponent::ActivateJadeClock()
{
	ActiveOpartsIndex = 1;
	SetOpartsActiveState(JadeClock, true);

	// 나머지 두 개 비활성화
	SetOpartsActiveState(CrystalSkull, false);
	SetOpartsActiveState(GoldBug, false);

	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}

	SetActiveOparts(ActiveOpartsIndex);
}

void UEquipmentSystemComponent::ActivateGoldBug()
{
	ActiveOpartsIndex = 2;
	SetOpartsActiveState(GoldBug, true);

	// 나머지 두 개 비활성화
	SetOpartsActiveState(CrystalSkull, false);
	SetOpartsActiveState(JadeClock, false);

	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}

	SetActiveOparts(ActiveOpartsIndex);
}

void UEquipmentSystemComponent::ActivateLastOparts()
{
	switch (ActiveOpartsIndex)
	{
	case 0:
		ActivateCrystalSkull();
		break;
	case 1:
		ActivateJadeClock();
		break;
	case 2:
		ActivateGoldBug();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("No previously active Oparts to activate."));
		break;
	}
}



