// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInfoWindowWidget.h"

#include "PlayerInfoEquipButtonWidget.h"

#include "EquipmentSubsystem.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"
#include "WeaponDataAsset.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UPlayerInfoWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		EquipmentSubsystem = GI->GetSubsystem<UEquipmentSubsystem>();
		TransferSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	}

	// Subsystem 이벤트
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
		EquipmentSubsystem->OnEquipmentChanged.AddDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);

		EquipmentSubsystem->EnsurePartySize(GetPartyCount());
	}

	// Portrait 버튼
	if (Btn_Ally0)
	{
		Btn_Ally0->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleAlly0Clicked);
		Btn_Ally0->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleAlly0Clicked);
	}
	if (Btn_Ally1)
	{
		Btn_Ally1->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleAlly1Clicked);
		Btn_Ally1->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleAlly1Clicked);
	}

	// (선택) 무기/방어구 슬롯 버튼
	if (Btn_WeaponSlot)
	{
		Btn_WeaponSlot->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleWeaponSlotClicked);
		Btn_WeaponSlot->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleWeaponSlotClicked);
	}
	if (Btn_ArmorSlot)
	{
		Btn_ArmorSlot->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleArmorSlotClicked);
		Btn_ArmorSlot->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleArmorSlotClicked);
	}

	// 초기 선택
	RefreshPortraitButtons();
	SelectPartyMember(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::NativeDestruct()
{
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
	}
	Super::NativeDestruct();
}

void UPlayerInfoWindowWidget::Open()
{
	SetVisibility(ESlateVisibility::Visible);
	RefreshPortraitButtons();
	SelectPartyMember(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

int32 UPlayerInfoWindowWidget::GetPartyCount() const
{
	if (!TransferSubsystem) return 0;
	return TransferSubsystem->BattleInfo.AlliesToSpawn.Num();
}

UUnitDataAsset* UPlayerInfoWindowWidget::GetPartyUnitData(int32 PartyIndex) const
{
	if (!TransferSubsystem) return nullptr;
	if (!TransferSubsystem->BattleInfo.AlliesToSpawn.IsValidIndex(PartyIndex)) return nullptr;

	const TSoftObjectPtr<UUnitDataAsset> Soft = TransferSubsystem->BattleInfo.AlliesToSpawn[PartyIndex].UnitDataAsset;
	if (Soft.IsNull()) return nullptr;

	return Soft.LoadSynchronous();
}

void UPlayerInfoWindowWidget::HandleAlly0Clicked()
{
	SelectPartyMember(0);
}

void UPlayerInfoWindowWidget::HandleAlly1Clicked()
{
	SelectPartyMember(1);
}

void UPlayerInfoWindowWidget::HandleWeaponSlotClicked()
{
	// 모딩창은 BP에서 띄우는 걸 기본으로 둠
	BP_OpenWeaponModWindow(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::HandleArmorSlotClicked()
{
	BP_OpenArmorModWindow(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::HandleEquipmentButtonClicked(FName EquipmentKey)
{
	// 동적 장비 버튼(빨간 박스) 클릭 -> BP에서 부품 장착 창을 띄우는 구조
	BP_OpenEquipmentPartsSelectionWindow(SelectedPartyIndex, EquipmentKey);
}

void UPlayerInfoWindowWidget::HandleEquipmentChanged(int32 PartyIndex)
{
	// 지금 UI는 선택된 파티원만 즉시 갱신
	if (PartyIndex != SelectedPartyIndex) return;
	RefreshSelectedMemberPanel();
}

void UPlayerInfoWindowWidget::SelectPartyMember(int32 PartyIndex)
{
	const int32 N = GetPartyCount();
	if (N <= 0)
	{
		SelectedPartyIndex = 0;
		RefreshPortraitButtons();
		RefreshSelectedMemberPanel();
		return;
	}

	SelectedPartyIndex = FMath::Clamp(PartyIndex, 0, N - 1);
	RefreshPortraitButtons();
	RefreshSelectedMemberPanel();
}

void UPlayerInfoWindowWidget::RefreshPortraitButtons()
{
	const int32 N = GetPartyCount();

	// Ally 0
	if (Btn_Ally0)
	{
		Btn_Ally0->SetVisibility(N >= 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Btn_Ally0->SetIsEnabled(N >= 1);
	}
	if (Img_Portrait0)
	{
		if (UUnitDataAsset* DA0 = GetPartyUnitData(0))
		{
			if (DA0->PortraitTexture) Img_Portrait0->SetBrushFromTexture(DA0->PortraitTexture);
			else if (DA0->UnitIcon) Img_Portrait0->SetBrushFromTexture(DA0->UnitIcon);
			else Img_Portrait0->SetBrushFromTexture(nullptr);
			Img_Portrait0->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_Portrait0->SetBrushFromTexture(nullptr);
			Img_Portrait0->SetVisibility(ESlateVisibility::Hidden);
		}
		Img_Portrait0->SetRenderOpacity(SelectedPartyIndex == 0 ? 1.f : 0.35f);
	}

	// Ally 1
	if (Btn_Ally1)
	{
		Btn_Ally1->SetVisibility(N >= 2 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Btn_Ally1->SetIsEnabled(N >= 2);
	}
	if (Img_Portrait1)
	{
		if (UUnitDataAsset* DA1 = GetPartyUnitData(1))
		{
			if (DA1->PortraitTexture) Img_Portrait1->SetBrushFromTexture(DA1->PortraitTexture);
			else if (DA1->UnitIcon) Img_Portrait1->SetBrushFromTexture(DA1->UnitIcon);
			else Img_Portrait1->SetBrushFromTexture(nullptr);
			Img_Portrait1->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_Portrait1->SetBrushFromTexture(nullptr);
			Img_Portrait1->SetVisibility(ESlateVisibility::Hidden);
		}
		Img_Portrait1->SetRenderOpacity(SelectedPartyIndex == 1 ? 1.f : 0.35f);
	}
}

void UPlayerInfoWindowWidget::RefreshSelectedMemberPanel()
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);

	// 중앙 이미지: 3D 대신 일러스트/아이콘
	if (Img_Illustration)
	{
		if (UnitDA)
		{
			if (UnitDA->UnitIcon) Img_Illustration->SetBrushFromTexture(UnitDA->UnitIcon);
			else if (UnitDA->PortraitTexture) Img_Illustration->SetBrushFromTexture(UnitDA->PortraitTexture);
			else Img_Illustration->SetBrushFromTexture(nullptr);
		}
		else
		{
			Img_Illustration->SetBrushFromTexture(nullptr);
		}
	}

	// (선택) 무기/방어구 슬롯 아이콘
	if (Img_WeaponIcon)
	{
		UTexture2D* WeaponIcon = nullptr;
		if (EquipmentSubsystem)
		{
			if (UWeaponDataAsset* Weapon = EquipmentSubsystem->GetEquippedWeapon(SelectedPartyIndex))
			{
				WeaponIcon = Weapon->Icon;
			}
		}
		Img_WeaponIcon->SetBrushFromTexture(WeaponIcon);
	}

	// Armor는 아직 별도 데이터가 없어서 기본은 비워둠(원하면 UnitDA 쪽에 ArmorIcon을 추가해서 연결)
	// Img_ArmorIcon은 WBP에만 넣어두고 나중에 확장 가능

	// 동적 장비 버튼(빨간 박스 영역) 재구성
	RebuildEquipmentButtons();

	RefreshStatsPanel();
}

void UPlayerInfoWindowWidget::RebuildEquipmentButtons()
{
	if (!Canvas_EquipButtons) return;

	Canvas_EquipButtons->ClearChildren();

	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);
	if (!UnitDA) return;
	if (!EquipButtonWidgetClass) return;

	for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
	{
		if (Def.EquipmentKey.IsNone()) continue;

		UPlayerInfoEquipButtonWidget* W = CreateWidget<UPlayerInfoEquipButtonWidget>(GetOwningPlayer(), EquipButtonWidgetClass);
		if (!W) continue;

		UTexture2D* Icon = Def.IconOverride;
		if (!Icon)
		{
			// Weapon icon auto-resolve
			if (Def.Kind == EPlayerInfoEquipmentKind::Weapon)
			{
				UWeaponDataAsset* Weapon = nullptr;
				if (EquipmentSubsystem)
				{
					Weapon = EquipmentSubsystem->GetEquippedWeapon(SelectedPartyIndex);
				}
				if (!Weapon && Def.WeaponOverride)
				{
					Weapon = Def.WeaponOverride;
				}
				// UnitDA에 DefaultWeapon 필드가 있는 프로젝트(고정 장비 정책)라면 여기서 추가 폴백을 걸어도 됨.
				// (없다면 컴파일 에러가 날 수 있으므로, 이 라인은 프로젝트 상황에 맞게 유지/제거)
				if (!Weapon)
				{
					// NOTE: DefaultWeapon은 별도 패치에서 추가된 필드
					// Weapon = UnitDA->DefaultWeapon;
				}
				if (Weapon) Icon = Weapon->Icon;
			}
		}

		W->Init(Def.EquipmentKey, Icon);
		W->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentButtonClicked);
		W->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentButtonClicked);

		UCanvasPanelSlot* CanvasPanelSlot = Canvas_EquipButtons->AddChildToCanvas(W);
		CanvasPanelSlot->SetAutoSize(false);
		CanvasPanelSlot->SetAnchors(FAnchors(Def.UIAnchor.X, Def.UIAnchor.Y, Def.UIAnchor.X, Def.UIAnchor.Y));
		CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasPanelSlot->SetPosition(Def.UIPixelOffset);
		CanvasPanelSlot->SetSize(Def.UISize);
	}
}

void UPlayerInfoWindowWidget::RefreshStatsPanel()
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);

	// Base Stats
	const float MaxHP = UnitDA ? UnitDA->BaseStats.MaxHP : 0.f;
	const float SPD = UnitDA ? UnitDA->BaseStats.Speed : 0.f;
	const float ATK = UnitDA ? UnitDA->BaseStats.AttackPower : 0.f;

	// Runtime HP/SP (Transfer에 저장된 값이 있으면 그걸 사용)
	float CurHP = MaxHP;
	int32 CurSP = UnitDA ? UnitDA->BaseSkillPoints : 0;

	if (TransferSubsystem)
	{
		const float SavedHP = TransferSubsystem->GetSavedHP(SelectedPartyIndex);
		if (SavedHP >= 0.f) CurHP = SavedHP;

		const int32 SavedSP = TransferSubsystem->GetSavedSP(SelectedPartyIndex);
		if (SavedSP >= 0) CurSP = SavedSP;
	}

	if (Text_HP) Text_HP->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f / %.0f"), CurHP, MaxHP)));
	if (Text_SP) Text_SP->SetText(FText::FromString(FString::Printf(TEXT("SP %d"), CurSP)));
	if (Text_ATK) Text_ATK->SetText(FText::FromString(FString::Printf(TEXT("ATK %.0f"), ATK)));
	if (Text_SPD) Text_SPD->SetText(FText::FromString(FString::Printf(TEXT("SPD %.0f"), SPD)));

	// 무기부품 합산 효과
	if (EquipmentSubsystem)
	{
		const FWeaponPartEffect E = EquipmentSubsystem->GetTotalWeaponPartEffect(SelectedPartyIndex);

		if (Text_Effects)
		{
			TArray<FString> Lines;
			if (E.BonusProjectileCount != 0) Lines.Add(FString::Printf(TEXT("Proj %+d"), E.BonusProjectileCount));
			if (E.BonusHitCount != 0) Lines.Add(FString::Printf(TEXT("Hit %+d"), E.BonusHitCount));
			if (!FMath::IsNearlyZero(E.DamageMulAdd)) Lines.Add(FString::Printf(TEXT("DmgAdd %+0.2f"), E.DamageMulAdd));
			if (!FMath::IsNearlyEqual(E.DamageMulMul, 1.f)) Lines.Add(FString::Printf(TEXT("DmgMul x%0.2f"), E.DamageMulMul));
			if (Lines.Num() == 0) Lines.Add(TEXT("Effects: None"));

			Text_Effects->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
		}

		if (Text_Proj) Text_Proj->SetText(FText::FromString(FString::Printf(TEXT("Proj %+d"), E.BonusProjectileCount)));
		if (Text_Hit) Text_Hit->SetText(FText::FromString(FString::Printf(TEXT("Hit %+d"), E.BonusHitCount)));
		if (Text_DmgMul)
		{
			Text_DmgMul->SetText(FText::FromString(
				FString::Printf(TEXT("DmgAdd %+0.2f / DmgMul x%.2f"), E.DamageMulAdd, E.DamageMulMul)));
		}
	}
	else
	{
		if (Text_Effects) Text_Effects->SetText(FText::FromString(TEXT("Effects: None")));
		if (Text_Proj) Text_Proj->SetText(FText::FromString(TEXT("Proj +0")));
		if (Text_Hit) Text_Hit->SetText(FText::FromString(TEXT("Hit +0")));
		if (Text_DmgMul) Text_DmgMul->SetText(FText::FromString(TEXT("DmgMul x1.00")));
	}
}
