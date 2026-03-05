// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LootTypes.h"
#include "Blueprint/UserWidget.h"
#include "LootSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLootSlotClicked, int32, SlotIndex);

UCLASS()
class PCUBE_API ULootSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitFilled(const FLootStack& InStack, int32 InSlotIndex); // 아이템 있는 슬롯 초기화
	void InitEmpty(int32 InSlotIndex); // 빈 슬롯 초기화
	
	UPROPERTY(BlueprintAssignable)
	FOnLootSlotClicked OnLootSlotClicked; // 슬롯 클릭 이벤트 - 인덱스 전달
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void HandleClicked(); // 버튼 클릭 처리
	
private:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Select = nullptr; // 슬롯 클릭 버튼
	
	UPROPERTY(meta=(BindWidget))
	UImage* Img_Icon = nullptr; // 아이콘
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Text_Count = nullptr; // 수량 텍스트
	
private:
	int32 SlotIndex = -1; // 루팅 그리드 인덱스 (0~7)
	bool bHasItem = false; // 빈 슬롯인지 판단 여부
};
