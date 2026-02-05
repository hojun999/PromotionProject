// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "BattleActionOrderSlot.generated.h"
	
UCLASS()
class PCUBE_API UBattleActionOrderSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	void SetUnitSlotInfo(class ABattleBaseUnit* TargetUnit);
	
protected:
	// 블루프린트의 Image 위젯과 연결 - 이름 같아야됨 ***
	UPROPERTY(meta=(BindWidget))
	UImage* UnitIcon;
};
