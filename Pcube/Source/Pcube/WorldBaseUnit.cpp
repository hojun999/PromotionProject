// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldBaseUnit.h"

// Sets default values
AWorldBaseUnit::AWorldBaseUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWorldBaseUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWorldBaseUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AWorldBaseUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

