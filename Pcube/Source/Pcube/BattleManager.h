// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleManager.generated.h"

UCLASS()
class PCUBE_API ABattleManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattleManager();

	// BattleGameMode에서 호출할 전투 시작 함수
	void InitiateBattle();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class USpawnComponent* SpawnComp;
	
	UPROPERTY(VisibleAnywhere)
	class UTurnComponent* TurnComp;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
