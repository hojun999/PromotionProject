// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalDataInstance.h"
#include "Engine/GameInstance.h"
#include "SpawnComponent.h"

#include "BattleBaseUnit.h"
#include "UnitDataAsset.h"
#include "Kismet/GameplayStatics.h"

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
	if (!GI || GI->BattleInfo.EnemySpawnMap.Num() == 0)
	{
		return;
	}
	
	// TMap 순회 (Key: SpawnLotation, Value: UnitData)
	for (auto& Elem : GI->BattleInfo.EnemySpawnMap)
	{
		const FString& SpawnLotation = Elem.Key;
		UUnitDataAsset* UnitData = Elem.Value;
		
		if (!UnitData)
		{
			continue;
		}
		
		AActor* SpawnPoint = nullptr;
		TArray<AActor*> SpawnPoints;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*SpawnLotation), SpawnPoints);
		
		if (SpawnPoints.Num() > 0)
		{
			SpawnPoint = SpawnPoints[0];
		}
		
		if (SpawnPoint)
		{
			GetWorld()->SpawnActor<ABattleBaseUnit>(
				UnitData->BattleUnitClass,
				SpawnPoint->GetActorLocation(),
				SpawnPoint->GetActorRotation()
			);
		}
	}
	
}
