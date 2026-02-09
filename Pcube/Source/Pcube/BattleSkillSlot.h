// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "BattleAllyUnit.h"
#include "BattleSkillSlot.generated.h"


class USkillDataAsset;

UCLASS()
class PCUBE_API UBattleSkillSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetSkillSlotInfo(USkillDataAsset* TargetSkillDataInfo);
	
protected:
	UPROPERTY(meta=(BindWidget))
	UImage* SkillIcon;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* SkillNameText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battle|UI")
	USkillDataAsset* SkillDataInfo;
	
	// TODO:
	// 1. 스킬 설명 TextBlock
	// 2. 위의 내용을 담을 BackgroundImage
	
};
