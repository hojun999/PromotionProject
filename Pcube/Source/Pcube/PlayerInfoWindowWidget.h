// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoWindowWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

class UEquipmentSubsystem;
class UBattleInfoTransferSubsystem;
class UUnitDataAsset;

/**
 * PlayerInfoWindow (World UI)
 * - 파티원이 2명인 전제에 맞춰: 초상화 버튼(Btn_Ally0/1)로 선택
 * - 3D 프리뷰 대신 이미지(Img_Illustration)로 표시
 * - 스탯 + 무기부품 합산 효과(Proj/Hit/DmgMul 등) 표시
 *
 * NOTE:
 * - 무기/방어구는 인벤 아이템이 아니고, "부품"만 인벤/드랍 대상.
 * - 무기 슬롯/모딩창은 별도 위젯으로 띄우는 구조를 권장.
 */
UCLASS()
class PCUBE_API UPlayerInfoWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Open();

	UFUNCTION(BlueprintCallable)
	void Close();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Blueprint에서 모딩창을 띄우고 싶으면 여기 이벤트를 구현해서 사용
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenWeaponModWindow(int32 PartyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenArmorModWindow(int32 PartyIndex);
	
private:
	// --- UI 이벤트 ---
	UFUNCTION() void HandleAlly0Clicked();
	UFUNCTION() void HandleAlly1Clicked();

	// 빨간 박스(무기/방어구) 버튼이 추가될 경우 사용
	UFUNCTION() void HandleWeaponSlotClicked();
	UFUNCTION() void HandleArmorSlotClicked();

	// --- Subsystem 이벤트 ---
	UFUNCTION() void HandleEquipmentChanged(int32 PartyIndex);

private:
	void SelectPartyMember(int32 PartyIndex);
	void RefreshPortraitButtons();
	void RefreshSelectedMemberPanel();
	void RefreshStatsPanel();

	int32 GetPartyCount() const;
	UUnitDataAsset* GetPartyUnitData(int32 PartyIndex) const;

	

private:
	// --- Portrait Buttons (HB_Portraits) ---
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Ally0 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Ally1 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Portrait0 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Portrait1 = nullptr;

	// --- 중앙 일러스트(3D 대신 이미지) ---
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Illustration = nullptr;

	// --- 스탯/효과 표시 (VB_StatTexts 내부) ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_HP = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_SP = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_ATK = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_SPD = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Effects = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Proj = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Hit = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_DmgMul = nullptr;

	// --- (선택) 빨간 박스에 추가할 버튼/아이콘 ---
	// WBP에 실제로 추가했을 때만 BindWidgetOptional로 연결됨
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_WeaponSlot = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_ArmorSlot = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_WeaponIcon = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_ArmorIcon = nullptr;

private:
	UPROPERTY() TObjectPtr<UEquipmentSubsystem> EquipmentSubsystem = nullptr;
	UPROPERTY() TObjectPtr<UBattleInfoTransferSubsystem> TransferSubsystem = nullptr;

	int32 SelectedPartyIndex = 0;
};
