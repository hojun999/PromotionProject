// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterSpawner.generated.h"

class USpawnDataAsset;
class UUnitDataAsset;
class AWorldEnemyUnit;

UCLASS()
class PCUBE_API AEncounterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEncounterSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// 이 스포너(=Encounter 트리거)의 고유 ID
	UPROPERTY(EditInstanceOnly, Category="Encounter")
	FName EncounterID = NAME_None;
	
	// 월드에서 스폰할 적 트리거 액터 - BP 가능
	UPROPERTY(EditInstanceOnly, Category="Encounter")
	TSubclassOf<AWorldEnemyUnit> WorldEnemyClass;
	
	UPROPERTY(EditInstanceOnly, Category="Encounter")
	TSoftObjectPtr<UUnitDataAsset> UnitDataAsset;
	
	UPROPERTY(EditInstanceOnly, Category="Encounter")
	TSoftObjectPtr<USpawnDataAsset> SpawnDataAsset;
	
	// 이미 처치한 Encounter에 대해서 스폰 제외
	UPROPERTY(EditInstanceOnly, Category="Encounter")
	bool bSkipSpawnIfDefeated = false;
	
};
