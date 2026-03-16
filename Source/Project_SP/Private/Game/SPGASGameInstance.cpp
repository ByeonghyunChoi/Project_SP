// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SPGASGameInstance.h"

USPGASGameInstance::USPGASGameInstance()
{
	bIsReturnFromGame = false;
}

void USPGASGameInstance::SetbIsReturnFromGame(bool IsReturn)
{
	bIsReturnFromGame = IsReturn;
}

bool USPGASGameInstance::GetbIsReturnFromGame() const
{
	return bIsReturnFromGame;
}
