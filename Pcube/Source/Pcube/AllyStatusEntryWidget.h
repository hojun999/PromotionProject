// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleBaseUnit.h"
#include "AllyStatusEntryWidget.generated.h"

class UHorizontalBox;

UCLASS()
class PCUBE_API UAllyStatusEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitWithUnit(ABattleBaseUnit* InUnit);
	
	UFUNCTION(BlueprintCallable)
	void SetActive(bool bActive);
	
protected:
	virtual void NativeDestruct() override; // 위젯 파괴 시 델리게이트 정리용
	
	UPROPERTY(meta=(BindWidget)) class UImage* PortraitImage;
	UPROPERTY(meta=(BindWidget)) class UProgressBar* HPBar;
	UPROPERTY(meta=(BindWidget)) class UTextBlock* HPText;
	//UPROPERTY(meta=(BindWidgetOptional)) class UBorder* ActiveBorder;
	UPROPERTY(meta=(BindWidgetOptional)) class UImage* ActiveImage;
	
	// --- Skill Point UI ---
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* HB_SkillPoints = nullptr; // SP 아이콘들이 들어갈 컨테이너
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SkillPoints|UI")
	TObjectPtr<UTexture2D> SkillPointIconTexture = nullptr; // SP 1칸 아이콘 텍스쳐
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SkillPoints|UI")
	FLinearColor SkillPointFilledColor = FLinearColor(1.f, 1.f, 1.f, 1.f); // 보유 중인 SP 색
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SkillPoints|UI")
	FLinearColor SkillPointEmptyColor = FLinearColor(0.25f, 0.25f, 0.25f, 0.26f); // 미보유 SP 색
	
private:
	UPROPERTY() TObjectPtr<ABattleBaseUnit> Unit; // 이 Widget이 표시하는 유닛
	
	// HP 이벤트
	UFUNCTION()
	void HandleHPChanged(float CurrentHP, float MaxHP);
	
	UFUNCTION()
	void HandleDied();
	
	// SP 이벤트
	UFUNCTION()
	void HandleSkillPointsChanged(int32 Current, int32 Max); // SP 변동 시 UI 갱신
	void RebuildSkillPointsIcons(int32 Max); // MaxSP만큼 아이콘 생성
	void UpdateSkillPointFill(int32 Current); // CurrentSP만큼 앞에서부터 채우기
	
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> SkillPointIcons; // 생성된 SP 아이콘 캐시 (색만 변경)
	
	int32 CachedMaxSP = -1; // 마지막으로 할당한 MaxSP
};
