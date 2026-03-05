// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LootWindowWidget.generated.h"

class UUniformGridPanel;
class UButton;
class ULootSlotWidget;
class ALootCorpseActor;

UCLASS()
class PCUBE_API ULootWindowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 시체를 대상으로 루팅창 열기
	UFUNCTION(BlueprintCallable)
	void OpenForCorpse(ALootCorpseActor* InCorpse);
	
	UFUNCTION(BlueprintCallable)
	void CloseSelf(); // 닫기 버튼용 -> HUD에 Hide 요청
	
protected:
	virtual void NativeConstruct() override;
	
private:
	void RebuildGrid(); // 2x4 (= 8칸) 그리드 재생성
	void HandleLootChangedAfterTake(); // 루팅 후 UI&시체 처리
	
	UFUNCTION()
	void HandleTakeAll(); // TakeAll 버튼 클릭
	
	UFUNCTION()
	void HandleClose(); // Close 버튼 클릭
	
	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex); // 슬롯 클릭 -> 해당 아이템 인벤토리로 이동
	
private:
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* Grid_Loot = nullptr; // 2x4 루팅 그리드
	
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_TakeAll = nullptr; // 전부 획득
	
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Close = nullptr; // 닫기
	
	UPROPERTY(EditAnywhere, Category="Loot|UI")
	TSubclassOf<ULootSlotWidget> LootSlotClass; // 슬롯 위젯 클래스 (WBP_LootSlot)
	
private:
	UPROPERTY()
	TObjectPtr<ALootCorpseActor> Corpse = nullptr; // 현재 루팅 대상
};
