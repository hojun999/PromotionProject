// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataAsset.h"
#include "WeaponModTypes.h"
#include "WeaponPartDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWeaponPartEffect
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	int32 BonusProjectileCount = 0;	// 투사체 개수 + N
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	int32 BonusHitCount = 0; // 타수 + N
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	int32 BonusSkillPointGain = 0; // 기본공격 SP 획득량 +N
	
	// 파츠 장착 스탯 보너스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	float BonusMaxHP = 0.f; // MaxHP +N

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	float BonusAttackPower = 0.f; // ATK +N
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	float BonusSpeed = 0.f; // SPD +N
};

UCLASS()
class PCUBE_API UWeaponPartDataAsset : public UItemDataAsset
{
	GENERATED_BODY()
	
public:
	UWeaponPartDataAsset()
	{
		ItemType = EItemType::WeaponPart;
		bStackable = false;
		MaxStack = 1;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponPart")
	FWeaponPartEffect Effect; // 장착 효과 스펙
	
	// 기존 SlotType / AttachmentSocketName은 레거시
	// 실제 장착 가능 여부는 CompatibleEquipmentKeys 기반으로 판정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponPart")
	EWeaponModSlotType SlotType = EWeaponModSlotType::Custom;
	
	// 이 파츠를 장착할 수 있는 장비의 Key들
	// 예: Weapon_Main, Weapon_Core, Armor_Body
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponPart|Compatibility")
	TArray<FName> CompatibleEquipmentKeys;
	
	UFUNCTION(BlueprintPure, Category="WeaponPart|Compatibility")
	bool IsCompatibleWithEquipmentKey(FName EquipmentKey) const
	{
		return EquipmentKey != NAME_None && CompatibleEquipmentKeys.Contains(EquipmentKey);
	}
	
	// 캐릭터 정보 확인창에서 볼 수 있는 적용 중인 효과 텍스트
	UFUNCTION(BlueprintCallable, Category="WeaponPart")
	FText BuildEffectText() const; // ex - 투사체 + 1, 공격 횟수 +2
	
	// 무기 부품 외형 변경용 데이터 (현재 장착 판정에는 사용하지 않음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeponPart|Visual")
	TObjectPtr<UStaticMesh> AttachmentMesh = nullptr; // 무기에 붙일 메시
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Visual")
	FName AttachmentSocketName = NAME_None;
};
