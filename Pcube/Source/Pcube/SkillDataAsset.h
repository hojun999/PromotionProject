// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PCUBE_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Info")
	FString SkillName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Info")
	float DamageMultiplier;
	
	
	// 스킬 속성, 애니메이션, 사용 행동력 등 정의
};
