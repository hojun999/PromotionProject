// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInfoWindowWidget.h"

#include "PlayerInfoEquipButtonWidget.h"
#include "EquipPartsWindowWidget.h"

#include "EquipmentSubsystem.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"
#include "WeaponDataAsset.h"
#include "WorldHUD.h"
#include "WorldPlayerController.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UPlayerInfoWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		EquipmentSubsystem = GI->GetSubsystem<UEquipmentSubsystem>();
		TransferSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	}

	// 기본 장비 상태 준비
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->EnsurePartySize(GetPartyCount());
		EnsureSelectedLoadoutInitialized();
		
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
		EquipmentSubsystem->OnEquipmentChanged.AddDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
	}

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

	if (EquipPartsWindowClass && GetOwningPlayer())
	{
		EquipPartsWindow = CreateWidget<UEquipPartsWindowWidget>(GetOwningPlayer(), EquipPartsWindowClass);
		if (EquipPartsWindow)
		{
			EquipPartsWindow->AddToViewport(55);
			EquipPartsWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	RefreshPortraitButtons();
	SelectPartyMember(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::NativeDestruct()
{
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
	}

	ClosePartsWindow();
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
	HideEquipPartsWindow();
	SetVisibility(ESlateVisibility::Collapsed);
	
	if (AWorldPlayerController* PC = Cast<AWorldPlayerController>(GetOwningPlayer()))
	{
		if (AWorldHUD* WHUD = Cast<AWorldHUD>(PC->GetHUD()))
		{
			WHUD->CloseAnyOpenPanel();
			WHUD->ApplyInputMode_GameOnly();
			WHUD->SetWorldInputBlocked(false);
		}
	}
}

int32 UPlayerInfoWindowWidget::GetPartyCount() const
{
	if (!TransferSubsystem) return 0;
	return TransferSubsystem->GetPartyCount(); // AlliesToSpawn 대신 영구 보존 리스트 사용
}


UUnitDataAsset* UPlayerInfoWindowWidget::GetPartyUnitData(int32 PartyIndex) const
{
	if (!TransferSubsystem) return nullptr;
	return TransferSubsystem->GetPartyUnitData(PartyIndex); // AlliesToSpawn 대신 영구 보존 리스트 사용
}

void UPlayerInfoWindowWidget::EnsureSelectedLoadoutInitialized()
{
	if (!EquipmentSubsystem) return;
	const int32 PartyCount = GetPartyCount();
	EquipmentSubsystem->EnsurePartySize(PartyCount);
	for (int32 i = 0; i < PartyCount; ++i) // 전체 파티 초기화
	{
		if (UUnitDataAsset* UnitDA = GetPartyUnitData(i))
		{
			EquipmentSubsystem->InitializeUnitLoadoutIfMissing(i, UnitDA);
		}
	}
}


void UPlayerInfoWindowWidget::EnsureDefaultWeaponsInitialized()
{
	if (!EquipmentSubsystem) return;

	const int32 PartyCount = GetPartyCount();
	EquipmentSubsystem->EnsurePartySize(PartyCount);

	for (int32 PartyIndex = 0; PartyIndex < PartyCount; ++PartyIndex)
	{
		if (EquipmentSubsystem->GetEquippedWeapon(PartyIndex))
		{
			continue;
		}

		if (UUnitDataAsset* UnitDA = GetPartyUnitData(PartyIndex))
		{
			if (UnitDA->DefaultWeapon)
			{
				EquipmentSubsystem->EquipWeapon(PartyIndex, UnitDA->DefaultWeapon);
			}
		}
	}
}

const FPlayerInfoEquipmentButtonDef* UPlayerInfoWindowWidget::FindEquipmentButtonDef(FName EquipmentKey) const
{
	return EquipmentButtonDefsByKey.Find(EquipmentKey);
}

UWeaponDataAsset* UPlayerInfoWindowWidget::ResolveCurrentWeaponForUI(const FPlayerInfoEquipmentButtonDef& Def, UUnitDataAsset* UnitDA) const
{
	UWeaponDataAsset* Weapon = EquipmentSubsystem ? EquipmentSubsystem->GetEquippedWeapon(SelectedPartyIndex) : nullptr;
	if (!Weapon && Def.WeaponOverride)
	{
		Weapon = Def.WeaponOverride;
	}
	if (!Weapon && UnitDA)
	{
		Weapon = UnitDA->DefaultWeapon;
	}
	return Weapon;
}

UArmorDataAsset* UPlayerInfoWindowWidget::ResolveCurrentArmorForUI(const FPlayerInfoEquipmentButtonDef& Def, UUnitDataAsset* UnitDA) const
{
	// ArmorID(EquipmentKey) 기준으로 조회
	UArmorDataAsset* Armor = (EquipmentSubsystem && Def.EquipmentKey != NAME_None)
		? EquipmentSubsystem->GetEquippedArmor(SelectedPartyIndex, Def.EquipmentKey) : nullptr;
	if (!Armor && Def.ArmorOverride)
	{
		Armor = Def.ArmorOverride;
	}
	if (!Armor && UnitDA)
	{
		Armor = UnitDA->DefaultArmor;
	}
	return Armor;
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
	BP_OpenWeaponModWindow(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::HandleArmorSlotClicked()
{
	BP_OpenArmorModWindow(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::HandleEquipmentButtonClicked(FName EquipmentKey)
{
	OpenEquipmentPartsWindow(EquipmentKey);
}

bool UPlayerInfoWindowWidget::OpenFirstEquipmentOfKind(EPlayerInfoEquipmentKind Kind)
{
	if (UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex))
	{
		for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
		{
			if (Def.Kind == Kind && !Def.EquipmentKey.IsNone())
			{
				return OpenEquipmentPartsWindow(Def.EquipmentKey);
			}
		}
	}
	return false;
}

bool UPlayerInfoWindowWidget::OpenEquipmentPartsWindow(FName EquipmentKey)
{
	if (!EquipPartsWindow) return false;
	const FPlayerInfoEquipmentButtonDef* Def = FindEquipmentButtonDef(EquipmentKey);
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);
	if (!Def || !UnitDA) return false;

	if (Def->Kind == EPlayerInfoEquipmentKind::Weapon)
	{
		if (UWeaponDataAsset* Weapon = ResolveCurrentWeaponForUI(*Def, UnitDA))
		{
			EquipPartsWindow->OpenForWeapon(SelectedPartyIndex, EquipmentKey, Weapon);
			PositionEquipPartsWindowNextToButton(EquipmentKey);
			return true;
		}
	}
	else
	{
		if (UArmorDataAsset* Armor = ResolveCurrentArmorForUI(*Def, UnitDA))
		{
			EquipPartsWindow->OpenForArmor(SelectedPartyIndex, EquipmentKey, Armor);
			PositionEquipPartsWindowNextToButton(EquipmentKey);
			return true;
		}
	}

	return false;
}

void UPlayerInfoWindowWidget::PositionEquipPartsWindowNextToButton(FName EquipmentKey)
{
	if (!EquipPartsWindow) return;
	if (TObjectPtr<UPlayerInfoEquipButtonWidget>* Found = EquipmentButtonWidgetsByKey.Find(EquipmentKey))
	{
		if (!IsValid(*Found)) return;
		const FGeometry& Geo = (*Found)->GetCachedGeometry();
		const FVector2D AbsPos = Geo.GetAbsolutePosition();
		const FVector2D LocalSize = Geo.GetLocalSize();
		FVector2D PixelPos;
		FVector2D ViewportPos;
		USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), AbsPos, PixelPos, ViewportPos);
		EquipPartsWindow->SetPositionInViewport(ViewportPos + FVector2D(LocalSize.X + 24.f, 0.f), false);
	}
}

void UPlayerInfoWindowWidget::HandleEquipmentChanged(int32 PartyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[PIW] HandleEquipmentChanged called. PartyIndex=%d SelectedPartyIndex=%d"),
		PartyIndex, SelectedPartyIndex);
	if (PartyIndex != SelectedPartyIndex) return;

	// 장비 변경 시 SavedMaxHP 기준으로 SavedHP 증가분 반영
	if (EquipmentSubsystem && TransferSubsystem)
	{
		UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);
		if (UnitDA)
		{
			const float NewMaxHP = EquipmentSubsystem->ResolveFinalStats(
				SelectedPartyIndex, UnitDA->BaseStats).MaxHP;
			const float OldMaxHP = TransferSubsystem->GetSavedMaxHP(SelectedPartyIndex);
			const float SavedHP  = TransferSubsystem->GetSavedHP(SelectedPartyIndex);

			if (OldMaxHP >= 0.f && SavedHP >= 0.f)
			{
				// MaxHP 증가분만큼 CurHP도 올림
				const float Delta = NewMaxHP - OldMaxHP;
				if (Delta > 0.f)
				{
					TransferSubsystem->SetSavedHP(
						SelectedPartyIndex,
						FMath::Clamp(SavedHP + Delta, 0.f, NewMaxHP));
				}
				else
				{
					// MaxHP 감소 시 CurHP가 초과하지 않게만 클램프
					TransferSubsystem->SetSavedHP(
						SelectedPartyIndex,
						FMath::Clamp(SavedHP, 0.f, NewMaxHP));
				}
			}
			else if (SavedHP < 0.f)
			{
				// SavedHP 없으면 MaxHP 풀피로 초기화
				TransferSubsystem->SetSavedHP(SelectedPartyIndex, NewMaxHP);
			}

			// 현재 MaxHP를 기록해둠 (다음 장비 변경 시 OldMaxHP로 사용)
			TransferSubsystem->SetSavedMaxHP(SelectedPartyIndex, NewMaxHP);
			UE_LOG(LogTemp, Warning, TEXT("[PIW] OldMaxHP=%.1f NewMaxHP=%.1f Delta=%.1f SavedHP=%.1f -> NewSavedHP=%.1f"),
				OldMaxHP, NewMaxHP, NewMaxHP - OldMaxHP,
				TransferSubsystem->GetSavedHP(SelectedPartyIndex),
				TransferSubsystem->GetSavedHP(SelectedPartyIndex));
		}
	}

	RefreshSelectedMemberPanel();
	if (EquipPartsWindow && EquipPartsWindow->GetVisibility() == ESlateVisibility::Visible)
	{
		EquipPartsWindow->Refresh();
	}
}


void UPlayerInfoWindowWidget::SelectPartyMember(int32 PartyIndex)
{
	ClosePartsWindow();

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
		}
		Img_Portrait0->SetRenderOpacity(SelectedPartyIndex == 0 ? 1.f : 0.35f);
	}

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
		}
		Img_Portrait1->SetRenderOpacity(SelectedPartyIndex == 1 ? 1.f : 0.35f);
	}
}

void UPlayerInfoWindowWidget::RefreshSelectedMemberPanel()
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);

	if (Img_Illustration)
	{
		if (UnitDA)
		{
			if (UnitDA->UnitIcon) Img_Illustration->SetBrushFromTexture(UnitDA->UnitIcon);
			else if (UnitDA->PortraitTexture) Img_Illustration->SetBrushFromTexture(UnitDA->PortraitTexture);
		}
	}

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
		if (!WeaponIcon && UnitDA && UnitDA->DefaultWeapon)
		{
			WeaponIcon = UnitDA->DefaultWeapon->Icon;
		}

		if (WeaponIcon)
		{
			Img_WeaponIcon->SetBrushFromTexture(WeaponIcon);
			Img_WeaponIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_WeaponIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (Img_ArmorIcon)
	{
		Img_ArmorIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	RebuildEquipmentButtons();
	RefreshStatsPanel();
}

bool UPlayerInfoWindowWidget::ResolveEquipmentButtonDef(FName EquipmentKey, FPlayerInfoEquipmentButtonDef& OutDef) const
{
	if (UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex))
	{
		for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
		{
			if (Def.EquipmentKey == EquipmentKey)
			{
				OutDef = Def;
				return true;
			}
		}
	}

	return false;
}

void UPlayerInfoWindowWidget::HideEquipPartsWindow()
{
	if (EquipPartsWindow)
	{
		EquipPartsWindow->Close();
	}
}

void UPlayerInfoWindowWidget::RebuildEquipmentButtons()
{
	if (!Canvas_EquipButtons) return;

	Canvas_EquipButtons->ClearChildren();
	EquipmentButtonDefsByKey.Empty();
	EquipmentButtonWidgetsByKey.Empty();

	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);
	if (!UnitDA || !EquipButtonWidgetClass) return;

	for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
	{
		if (Def.EquipmentKey.IsNone()) continue;

		EquipmentButtonDefsByKey.Add(Def.EquipmentKey, Def);

		UPlayerInfoEquipButtonWidget* ButtonWidget = CreateWidget<UPlayerInfoEquipButtonWidget>(GetOwningPlayer(), EquipButtonWidgetClass);
		if (!ButtonWidget) continue;

		UTexture2D* Icon = Def.IconOverride;
		if (!Icon)
		{
			if (Def.Kind == EPlayerInfoEquipmentKind::Weapon)
			{
				if (UWeaponDataAsset* Weapon = ResolveCurrentWeaponForUI(Def, UnitDA))
				{
					Icon = Weapon->Icon;
				}
			}
			else
			{
				if (UArmorDataAsset* Armor = ResolveCurrentArmorForUI(Def, UnitDA))
				{
					Icon = Armor->Icon;
				}
			}
		}

		ButtonWidget->Init(Def.EquipmentKey, Icon);
		ButtonWidget->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentButtonClicked);
		ButtonWidget->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentButtonClicked);
		EquipmentButtonWidgetsByKey.Add(Def.EquipmentKey, ButtonWidget);

		if (UCanvasPanelSlot* CanvasSlot = Canvas_EquipButtons->AddChildToCanvas(ButtonWidget))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(Def.UIAnchor.X, Def.UIAnchor.Y, Def.UIAnchor.X, Def.UIAnchor.Y));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(Def.UIPixelOffset);
			CanvasSlot->SetSize(Def.UISize);
		}
	}

}

void UPlayerInfoWindowWidget::ClosePartsWindow()
{
	if (EquipPartsWindow)
	{
		EquipPartsWindow->Close();
	}
}

void UPlayerInfoWindowWidget::RefreshStatsPanel()
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);
	const FUnitBaseStats BaseStats = UnitDA ? UnitDA->BaseStats : FUnitBaseStats();

	FUnitBaseStats FinalStats = BaseStats;
	if (EquipmentSubsystem && UnitDA)
	{
		FinalStats = EquipmentSubsystem->ResolveFinalStats(SelectedPartyIndex, BaseStats);
	}

	UE_LOG(LogTemp, Warning, TEXT("[PlayerInfo] RefreshStats PartyIndex=%d UnitDA=%s BaseATK=%.1f FinalATK=%.1f BaseHP=%.1f FinalHP=%.1f"),
		SelectedPartyIndex, *GetNameSafe(UnitDA), BaseStats.AttackPower, FinalStats.AttackPower, BaseStats.MaxHP, FinalStats.MaxHP);
	UE_LOG(LogTemp, Warning, TEXT("[PlayerInfo] TextWidgets: Text_ATK=%s Text_HP=%s Text_SPD=%s"),
		Text_ATK ? TEXT("Valid") : TEXT("NULL"),
		Text_HP  ? TEXT("Valid") : TEXT("NULL"),
		Text_SPD ? TEXT("Valid") : TEXT("NULL"));

	float CurHP = FinalStats.MaxHP;
	int32 CurSP = UnitDA ? UnitDA->BaseSkillPoints : 0;
	if (TransferSubsystem)
	{
		// SavedMaxHP 미설정 시 현재 MaxHP로 초기화
		if (TransferSubsystem->GetSavedMaxHP(SelectedPartyIndex) < 0.f)
		{
			TransferSubsystem->SetSavedMaxHP(SelectedPartyIndex, FinalStats.MaxHP);
		}

		const float SavedHP = TransferSubsystem->GetSavedHP(SelectedPartyIndex);
		if (SavedHP >= 0.f)
		{
			CurHP = FMath::Clamp(SavedHP, 0.f, FinalStats.MaxHP);
		}
		const int32 SavedSP = TransferSubsystem->GetSavedSP(SelectedPartyIndex);
		if (SavedSP >= 0)
		{
			CurSP = SavedSP;
		}
	}

	if (Text_HP) Text_HP->SetText(FText::FromString(FString::Printf(TEXT("체력 %.0f / %.0f"), CurHP, FinalStats.MaxHP)));
	if (Text_SP) Text_SP->SetText(FText::FromString(FString::Printf(TEXT("SP %d"), CurSP)));
	if (Text_ATK) Text_ATK->SetText(FText::FromString(FString::Printf(TEXT("공격력 %.0f"), FinalStats.AttackPower)));
	if (Text_SPD) Text_SPD->SetText(FText::FromString(FString::Printf(TEXT("속도 %.0f"), FinalStats.Speed)));

	if (EquipmentSubsystem)
	{
		const FWeaponPartEffect Effect = EquipmentSubsystem->GetTotalEquippedPartEffect(SelectedPartyIndex);
		if (Text_Effects)
		{
			TArray<FString> Lines;
			if (Effect.BonusProjectileCount != 0) Lines.Add(FString::Printf(TEXT("투사체 개수 %+d"), Effect.BonusProjectileCount));
			if (Effect.BonusHitCount != 0) Lines.Add(FString::Printf(TEXT("타수 %+d"), Effect.BonusHitCount));
			if (Effect.BonusSkillPointGain != 0) Lines.Add(FString::Printf(TEXT("추가 SP %+d"), Effect.BonusSkillPointGain));
			if (Lines.Num() == 0) Lines.Add(TEXT("적용된 효과 없음"));
			Text_Effects->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
		}
		
		if (Text_Proj) Text_Proj->SetText(FText::FromString(FString::Printf(TEXT("투사체 개수 %+d"), Effect.BonusProjectileCount)));
		if (Text_Hit) Text_Hit->SetText(FText::FromString(FString::Printf(TEXT("타수 %+d"), Effect.BonusHitCount)));
		if (Text_DmgMul) Text_DmgMul->SetText(FText::FromString(FString::Printf(TEXT("추가 SP %d"), Effect.BonusSkillPointGain)));
	}
	else
	{
		if (Text_Effects) Text_Effects->SetText(FText::FromString(TEXT("적용된 효과 없음")));
		if (Text_Proj) Text_Proj->SetText(FText::FromString(TEXT("투사체 개수 +0")));
		if (Text_Hit) Text_Hit->SetText(FText::FromString(TEXT("타수 +0")));
		if (Text_DmgMul) Text_DmgMul->SetText(FText::FromString(TEXT("추가 SP +0")));
	}
}
