// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WorldBaseUnit.generated.h"

class UUnitDataAsset;

UCLASS()
class PCUBE_API AWorldBaseUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWorldBaseUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 유닛의 정보를 담은 데이터 에셋 (월드 - 전투 연결을 위함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data")
	UUnitDataAsset* UnitData;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
