// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AllyStatusPanelWidget.generated.h"

class UHorizontalBox;
class UAllyStatusEntryWidget;
class ABattleBaseUnit;

UCLASS()
class PCUBE_API UAllyStatusPanelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	// TSubclassOf<UAllyStatusEntryWidget> EntryClass;
	
	// HUD에서 OnBattleUnitsSpawned 받을 때 호출
	UFUNCTION(BlueprintCallable)
	void InitParty(const TArray<AActor*>& AllyActors);
	
	// HUD에서 HandleTurnUnitChanged에서 호출 (현재 턴 아군 유닛 강조)
	UFUNCTION(BlueprintCallable)
	void SetActiveUnit(ABattleBaseUnit* ActiveUnit);
	
protected:
	UPROPERTY(meta=(BindWidget))
	class UHorizontalBox* AllyStatusPanel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<UAllyStatusEntryWidget> AllyStatusEntryClass;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UAllyStatusEntryWidget>> Entries;
	
	// Active 표시 빠르게 찾기용
	TMap<TWeakObjectPtr<ABattleBaseUnit>, TWeakObjectPtr<UAllyStatusEntryWidget>> EntryByUnit;
};
