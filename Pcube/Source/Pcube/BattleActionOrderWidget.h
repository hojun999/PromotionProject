// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleActionOrderWidget.generated.h"

/**
 * 
 */
UCLASS()
class PCUBE_API UBattleActionOrderWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// HUD가 호출할 데이터 갱신 함수
	void RefreshList(const TArray<AActor*>& NewList);
	
	// 턴이 오지 않은 유닛이 죽은 경우 해당 유닛 표시 제외
	void ExceptDeadUnitOnList(const AActor* DeadUnit);
	
protected:
	// 블루프린트의 VerticalBox와 이름이 같아야 함
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UVerticalBox* ActionOrderBox;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<class UBattleActionOrderSlot> SlotClass;
	
};
