// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitDataAsset.h"
#include "Engine/DataAsset.h"
#include "AllyPartyDataAsset.generated.h"


UCLASS()
class PCUBE_API UAllyPartyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 초기 파티에 포함될 유닛들
	UPROPERTY(EditAnywhere, Category="Battle")
	TArray<UUnitDataAsset*> DefaultMembers;
};
