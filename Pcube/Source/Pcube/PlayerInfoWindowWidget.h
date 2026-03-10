// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitDataAsset.h"
#include "PlayerInfoWindowWidget.generated.h"

class UButton;
class UCanvasPanel;
class UImage;
class UTextBlock;
class UEquipmentSubsystem;
class UBattleInfoTransferSubsystem;
class UPlayerInfoEquipButtonWidget;
class UEquipPartsWindowWidget;
class UWeaponDataAsset;
class UArmorDataAsset;

/**
 * PlayerInfoWindow (World UI)
 * - 파티원이 2명인 전제에 맞춰: 초상화 버튼(Btn_Ally0/1)로 선택
 * - 3D 프리뷰 대신 이미지(Img_Illustration)로 표시
 * - 스탯 + 무기부품 합산 효과(Proj/Hit/DmgMul 등) 표시
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

	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenWeaponModWindow(int32 PartyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenArmorModWindow(int32 PartyIndex);

	// 레거시 BP 훅. 현재 기본 장착/교체 로직은 C++ 내부에서 직접 처리
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenEquipmentPartsSelectionWindow(int32 PartyIndex, FName EquipmentKey);

private:
	// --- UI 이벤트 ---
	UFUNCTION() void HandleAlly0Clicked();
	UFUNCTION() void HandleAlly1Clicked();
	
	// --- 무기/방어구 버튼 이벤트 ---
	UFUNCTION() void HandleWeaponSlotClicked();
	UFUNCTION() void HandleArmorSlotClicked();
	
	// --- Subsystem 이벤트 ---
	UFUNCTION() void HandleEquipmentChanged(int32 PartyIndex);

	// --- 동적 장비 버튼 ---
	UFUNCTION() void HandleEquipmentButtonClicked(FName EquipmentKey);
	
private:
	void SelectPartyMember(int32 PartyIndex);
	void RefreshPortraitButtons();
	void RefreshSelectedMemberPanel();
	void RefreshStatsPanel();
	void RebuildEquipmentButtons();
	void EnsureSelectedLoadoutInitialized();
	
	//void OpenPartsWindowForEquipment(FName EquipmentKey);
	bool OpenEquipmentPartsWindow(FName EquipmentKey);
	void PositionEquipPartsWindowNextToButton(FName EquipmentKey);

	void ClosePartsWindow();
	
	int32 GetPartyCount() const;
	UUnitDataAsset* GetPartyUnitData(int32 PartyIndex) const;
	const FPlayerInfoEquipmentButtonDef* FindEquipmentButtonDef(FName EquipmentKey) const;
	UWeaponDataAsset* ResolveCurrentWeaponForUI(const FPlayerInfoEquipmentButtonDef& Def, UUnitDataAsset* UnitDA) const;
	UArmorDataAsset* ResolveCurrentArmorForUI(const FPlayerInfoEquipmentButtonDef& Def, UUnitDataAsset* UnitDA) const;
	bool OpenFirstEquipmentOfKind(EPlayerInfoEquipmentKind Kind);
	
	void EnsureDefaultWeaponsInitialized();
	void HideEquipPartsWindow();
	//bool TryToggleEquipmentPartsWindow(FName EquipmentKey);
	bool ResolveEquipmentButtonDef(FName EquipmentKey, FPlayerInfoEquipmentButtonDef& OutDef) const;
	//FVector2D GetEquipButtonWindowPosition(FName EquipmentKey) const;

private:
	// --- 초상화 버튼 ---
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Ally0 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Ally1 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Portrait0 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Portrait1 = nullptr;

	// --- 중앙 일러스트 ---
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Illustration = nullptr;

	// --- 장비 버튼을 동적으로 배치할 캔버스 ---
	UPROPERTY(meta=(BindWidgetOptional))
	UCanvasPanel* Canvas_EquipButtons = nullptr;

	// --- 스탯 효과 표시 ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_HP = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_SP = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_ATK = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_SPD = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Effects = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Proj = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Hit = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_DmgMul = nullptr;

	// --- 착용 중인 방어구/무기 버튼/아이콘 ---
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_WeaponSlot = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_ArmorSlot = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_WeaponIcon = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_ArmorIcon = nullptr;

	// --- 동적 장비 버튼 위젯 클래스 ---
	UPROPERTY(EditAnywhere, Category="PlayerInfo|UI")
	TSubclassOf<UPlayerInfoEquipButtonWidget> EquipButtonWidgetClass;

	// --- 장착 가능한 파츠 2x2 선택 창 ---
	UPROPERTY(EditAnywhere, Category="PlayerInfo|UI")
	TSubclassOf<UEquipPartsWindowWidget> EquipPartsWindowClass;

private:
	UPROPERTY() TObjectPtr<UEquipmentSubsystem> EquipmentSubsystem = nullptr;
	UPROPERTY() TObjectPtr<UBattleInfoTransferSubsystem> TransferSubsystem = nullptr;
	UPROPERTY() TObjectPtr<UEquipPartsWindowWidget> EquipPartsWindow = nullptr;
	UPROPERTY(Transient) TMap<FName, FPlayerInfoEquipmentButtonDef> EquipmentButtonDefsByKey;
	UPROPERTY(Transient) TMap<FName, TObjectPtr<UPlayerInfoEquipButtonWidget>> EquipmentButtonWidgetsByKey;
	//UPROPERTY() TMap<FName, TObjectPtr<UPlayerInfoEquipButtonWidget>> EquipButtonsByKey;

	int32 SelectedPartyIndex = 0;
};
