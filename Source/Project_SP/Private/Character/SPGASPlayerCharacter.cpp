// Character/PlayerCharacter.cpp

#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"
#include "Component/SPInteractionComponent.h"
#include "Character/SPGASPlayerController.h"



ASPGASPlayerCharacter::ASPGASPlayerCharacter()
{
	InteractionComponent = CreateDefaultSubobject<USPInteractionComponent>(TEXT("InteractComponent"));
}

void ASPGASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASPGASPlayerState* SPGAS = GetPlayerState<ASPGASPlayerState>();
	if (SPGAS)
	{
		ASC = SPGAS->GetAbilitySystemComponent();
		AttributeSet = SPGAS->GetAttributeSet();
		ASC->InitAbilityActorInfo(SPGAS, this);
		GiveAbilities();

		UE_LOG(LogTemp, Warning, TEXT("[Server] GAS Initialized & Abilities Given"));
	}

	APlayerController* PlayerController = CastChecked<ASPGASPlayerController>(NewController);
	PlayerController->ConsoleCommand(TEXT("showdebug abilitysystem"));
}

void ASPGASPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// [클라이언트] 초기화 진입
	ASPGASPlayerState* SPGAS = GetPlayerState<ASPGASPlayerState>();
	if (SPGAS)
	{
		ASC = SPGAS->GetAbilitySystemComponent();
		AttributeSet = SPGAS->GetAttributeSet();

		// 클라이언트 쪽 ASC 연결 (이게 있어야 컨트롤러가 ASC를 찾음)
		ASC->InitAbilityActorInfo(SPGAS, this);

		UE_LOG(LogTemp, Warning, TEXT("[Client] GAS Initialized for %s"), *GetName());
	}
}

void ASPGASPlayerCharacter::GiveAbilities()
{
	// 1. 권한 확인 로그
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("GiveAbilities Failed: Not Authority (Client tried to run this?)"));
		return;
	}
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("GiveAbilities Failed: ASC is NULL"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== GiveAbilities Start: %s ==="), *GetName());

	// 2. 필드 입력 GA 확인
	UE_LOG(LogTemp, Log, TEXT("FieldInputAbilities Count: %d"), FieldInputAbilities.Num());
	for (const auto& Pair : FieldInputAbilities)
	{
		if (Pair.Value)
		{
			FGameplayAbilitySpec Spec(Pair.Value);
			ASC->GiveAbility(Spec);

			UE_LOG(LogTemp, Log, TEXT("   -> Given Ability: %s (Tag: %s)"), *Pair.Value->GetName(), *Pair.Key.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("   -> Failed: Value(Class) is None for Tag: %s"), *Pair.Key.ToString());
		}
	}

	// 3. 전투 입력 GA 확인
	UE_LOG(LogTemp, Log, TEXT("BattleInputAbilities Count: %d"), BattleInputAbilities.Num());
	for (const auto& Pair : BattleInputAbilities)
	{
		if (Pair.Value)
		{
			FGameplayAbilitySpec Spec(Pair.Value);
			ASC->GiveAbility(Spec);
			UE_LOG(LogTemp, Log, TEXT("   -> Given Battle Ability: %s"), *Pair.Value->GetName());
		}
	}

	// 4. 필드 패시브 GA 확인
	UE_LOG(LogTemp, Log, TEXT("FieldPassiveAbilities Count: %d"), FieldPassiveAbilities.Num());
	FGameplayTag FieldTag = FSPGameplayTags::Get().Ability_Type_Field; // 이름 수정 확인 필요 (Ability_Type_Auto_Field 인지 확인)
	for (const auto& AbilityClass : FieldPassiveAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass);
			ASC->GiveAbility(Spec);
			UE_LOG(LogTemp, Log, TEXT("   -> Given Field Passive: %s"), *AbilityClass->GetName());
		}
	}

	// 5. 전투 패시브 GA 확인
	UE_LOG(LogTemp, Log, TEXT("BattlePassiveAbilities Count: %d"), BattlePassiveAbilities.Num());
	FGameplayTag BattleTag = FSPGameplayTags::Get().Ability_Type_Battle;
	for (const auto& AbilityClass : BattlePassiveAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass);
			ASC->GiveAbility(Spec);
			UE_LOG(LogTemp, Log, TEXT("   -> Given Battle Passive: %s"), *AbilityClass->GetName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== GiveAbilities End ==="));

	// 초기 상태 활성화
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FieldTag));
}
