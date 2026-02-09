// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleActionOrderWidget.h"

#include "BattleBaseUnit.h"
#include "BattleActionOrderSlot.h"
#include "Components/VerticalBox.h"

void UBattleActionOrderWidget::RefreshList(const TArray<AActor*>& NewOrder)
{
	if (!ActionOrderBox) {
		UE_LOG(LogTemp, Error, TEXT("Widget: TurnOrderBox(VerticalBox) is NULL!"));
		return;
	}
	
	if (!ActionOrderBox || !SlotClass) return;
	
	ActionOrderBox->ClearChildren();
	UE_LOG(LogTemp, Warning, TEXT("Widget: VerticalBox Cleared."));
	
	for (AActor* Actor : NewOrder)
	{
		if (auto* Unit = Cast<ABattleBaseUnit>(Actor))
		{
			auto* NewSlot = CreateWidget<UBattleActionOrderSlot>(this, SlotClass);
			NewSlot->SetUnitSlotInfo(Unit); // 슬롯에 데이터 세팅
			ActionOrderBox->AddChildToVerticalBox(NewSlot);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Widget: VerticalBox Refreshed with %d slots."), NewOrder.Num());
}

void UBattleActionOrderWidget::ExceptDeadUnitOnList(const AActor* DeadUnit)
{
	// TODO: 죽은 유닛을 찾아서 verticalbox의 컴포넌트에서 제외
}
