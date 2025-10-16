// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_DealDamage.h"
#include "Character/CombatPawn.h"
#include "Component/AttributesComponent.h"
#include "Combat/GameAction.h" 
#include "Combat/CombatStatics.h"
#include "Kismet/GameplayStatics.h"

void UTask_DealDamage::ExecuteTask_Implementation()
{
	UGameAction* OwningAction = Cast<UGameAction>(GetOuter());
	if (Instigator && OwningAction && Instigator->GetAttributesComponent() && Targets.Num() > 0)
	{
		const float SkillCoefficient = OwningAction->GetData().SkillCoefficient;
		for (ACombatPawn* Target : Targets)
		{
			if (Target && Target->GetAttributesComponent())
			{
				float FinalDamage = UCombatStatics::CalculateDamage(Instigator->GetAttributesComponent(), Target->GetAttributesComponent(), SkillCoefficient);
				UGameplayStatics::ApplyDamage(Target, FinalDamage, Instigator->GetController(), Instigator, UDamageType::StaticClass());
			}
		}
	}
	FinishTask();
}

