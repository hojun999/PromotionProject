// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandWidget.h"

void UCommandWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (AttackButton) AttackButton->OnClicked.AddDynamic(this, &UCommandWidget::OnAttackClicked);
	if (DefendButton) DefendButton->OnClicked.AddDynamic(this, &UCommandWidget::OnDefendClicked);
	if (EndTurnButton) EndTurnButton->OnClicked.AddDynamic(this, &UCommandWidget::OnEndTurnClicked);
}

void UCommandWidget::OnAttackClicked() { UE_LOG(LogTemp, Warning, TEXT("Attack Selected*****")); }
void UCommandWidget::OnDefendClicked() { UE_LOG(LogTemp, Warning, TEXT("Defend Selected*****")); }
void UCommandWidget::OnEndTurnClicked() { UE_LOG(LogTemp, Warning, TEXT("End Turn Selected*****")); }

