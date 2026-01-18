// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/OpartsComponent.h"
#include "Data/Asset/OpartsDefinition.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

UOpartsComponent::UOpartsComponent()
{

}

void UOpartsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Owner(캐릭터)로부터 ASC(Ability System Component) 가져오기
	// 캐릭터가 IAbilitySystemInterface를 상속받았다고 가정합니다.
	if (IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		ASC = Interface->GetAbilitySystemComponent();
	}

	// 만약 인터페이스로 못 찾았다면 컴포넌트 검색으로 한 번 더 시도 (안전장치)
	if (!ASC)
	{
		ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	}
}

void UOpartsComponent::EquipOparts(const UOpartsDefinition* NewOparts)
{
	// 안전장치: ASC가 없거나, 장착할 오파츠 정보가 없으면 중단
	if (!ASC || !NewOparts) return;

	// 이미 같은 것을 끼고 있다면 무시
	if (CurrentOparts == NewOparts) return;

	// 1. 기존에 끼고 있던 오파츠가 있다면 먼저 해제 (능력/스탯 제거)
	UnequipOparts();

	// 2. 새 오파츠 정보 등록
	CurrentOparts = NewOparts;

	// 3. GAS: 패시브 능력(Gameplay Ability) 부여
	if (NewOparts->PassiveAbilityClass)
	{
		// 능력 스펙 생성 (레벨 1, InputID -1)
		FGameplayAbilitySpec Spec(NewOparts->PassiveAbilityClass, 1, -1, this);

		// 능력을 부여하고 즉시 활성화 시도 (패시브이므로)
		ActiveAbilityHandle = ASC->GiveAbilityAndActivateOnce(Spec);
	}

	// 4. GAS: 스탯 이펙트(Gameplay Effect) 적용
	if (NewOparts->StatsEffectClass)
	{
		// 이펙트 컨텍스트 생성 (이 효과를 누가 줬는지 = OpartsComponent)
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		// 적용할 이펙트 스펙 생성
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(NewOparts->StatsEffectClass, 1.0f, EffectContext);

		if (SpecHandle.IsValid())
		{
			// 자신에게 이펙트 적용
			ActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// 5. [UI 알림] 델리게이트 방송 -> UI가 구독 중이라면 아이콘이 바뀜
	if (OnOpartsChanged.IsBound())
	{
		OnOpartsChanged.Broadcast(CurrentOparts);
	}

	UE_LOG(LogTemp, Log, TEXT("오파츠 장착 완료: %s"), *NewOparts->DisplayName.ToString());
}

void UOpartsComponent::UnequipOparts()
{
	if (!ASC) return;

	// 1. 기존 능력 제거
	if (ActiveAbilityHandle.IsValid())
	{
		ASC->ClearAbility(ActiveAbilityHandle);
		ActiveAbilityHandle = FGameplayAbilitySpecHandle(); // 핸들 초기화
	}

	// 2. 기존 스탯 이펙트 제거
	if (ActiveEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveEffectHandle);
		ActiveEffectHandle = FActiveGameplayEffectHandle(); // 핸들 초기화
	}

	// 데이터 비우기
	CurrentOparts = nullptr;

	// 빈 상태로 UI 알림
	if (OnOpartsChanged.IsBound())
	{
		OnOpartsChanged.Broadcast(nullptr);
	}
}
