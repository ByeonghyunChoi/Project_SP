// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterCharacter.h"



// Sets default values
AMonsterCharacter::AMonsterCharacter()
{

}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();
    Faction = EFaction::Enemy;
}


