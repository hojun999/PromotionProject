// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleManager.h"

#include "SpawnComponent.h"
#include "TurnComponent.h"

// Sets default values
ABattleManager::ABattleManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnComp = CreateDefaultSubobject<USpawnComponent>(TEXT("SpawnManager"));
	TurnComp = CreateDefaultSubobject<UTurnComponent>(TEXT("TurnManager"));
}

// Called when the game starts or when spawned
void ABattleManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

