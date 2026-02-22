// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyStatusPanelWidget.h"
#include "AllyStatusEntryWidget.h"
#include "BattleBaseUnit.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Tests/ToolMenusTestUtilities.h"

void UAllyStatusPanelWidget::InitParty(const TArray<AActor*>& AllyActors)
{
	UE_LOG(LogTemp, Warning, TEXT("[AllyStatusPanel] InitParty called Allies=%d"), AllyActors.Num());
	
	if (!AllyStatusPanel)
	{
		UE_LOG(LogTemp, Error, TEXT("[AllyStatusPanel] BindWidget failed: AllyStatusPanel is null"));
		return;
	}

	if (!AllyStatusEntryClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[AllyStatusPanel] AllyStatusEntryClass is null. Set it in WBP defaults."));
		return;
	}
	
	// 기존 엔트리 제거
	AllyStatusPanel->ClearChildren();
	Entries.Empty();
	EntryByUnit.Empty();
	
	// CreateWidget에 사용할 PC
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	}
	
	int32 Created = 0;
	
	for (AActor* A : AllyActors)
	{
		ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(A);
		if (!IsValid(Unit)) continue;
		
		UAllyStatusEntryWidget* Entry = CreateWidget<UAllyStatusEntryWidget>(PC, AllyStatusEntryClass);
		if (!Entry) continue;
		
		Entry->InitWithUnit(Unit);
		
		// if (UHorizontalBoxSlot* Slot = AllyStatusPanel->AddChildToHorizontalBox(Entry))
		// {
		// 	// 원하는 간격/정렬 설정
		// }
		
		Entries.Add(Entry);
		EntryByUnit.Add(Unit, Entry);
		AllyStatusPanel->AddChild(Entry);
		Created++;
	}
	
	SetActiveUnit(nullptr);
	
	UE_LOG(LogTemp, Log, TEXT("[AllyStatusPanel] InitParty: Allies=%d CreatedEntries=%d"), AllyActors.Num(), Created);
}

void UAllyStatusPanelWidget::SetActiveUnit(ABattleBaseUnit* ActiveUnit)
{
	for (UAllyStatusEntryWidget* E : Entries)
	{
		if (IsValid(E))
		{
			E->SetActive(false);
		}
	}

	if (!IsValid(ActiveUnit) || ActiveUnit->IsDead()) return;

	if (TWeakObjectPtr<UAllyStatusEntryWidget>* Found = EntryByUnit.Find(ActiveUnit))
	{
		if (Found && Found->IsValid())
		{
			Found->Get()->SetActive(true);
		}
	}
}
