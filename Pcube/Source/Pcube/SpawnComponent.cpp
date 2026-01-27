// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalDataInstance.h"
#include "Engine/GameInstance.h"
#include "SpawnComponent.h"

// Sets default values for this component's properties
USpawnComponent::USpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USpawnComponent::SpawnUnitsFromGI()
{
	UGlobalDataInstance* GI = Cast<UGlobalDataInstance>(GetWorld()->GetGameInstance());
	if (!GI)
	{
		return;
	}
	
	for (FEncounterUnit& Unit : GI->BattleInfo.EnemyGroup)
	{
		//TODO:
		// PositionName을 통해 스폰 위치 찾기
		// UnitData를 통해 폰 스폰
		UE_LOG(LogTemp, Log, TEXT("Spawning %s at slot %s"), *Unit.UnitData->UnitName, *Unit.SlotName);
	}
}
