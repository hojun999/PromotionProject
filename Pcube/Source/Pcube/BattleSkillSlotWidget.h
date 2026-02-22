// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleSkillSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class USkillDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillSlotclicked, USkillDataAsset*, Skill);

UCLASS()
class PCUBE_API UBattleSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category="Skill")
	FOnSkillSlotclicked OnSkillClicked;
	
	UFUNCTION(BlueprintCallable)
	void Init(USkillDataAsset* InSkill, int32 InSkillIndex);
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void HandleClicked();
	
	// --- 컴포넌트 ---
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Select;
	
	// UPROPERTY(meta=(BindWidget))
	// UImage* Img_Background;
	//
	// UPROPERTY(meta=(BindWidget))
	// UImage* Img_Line;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* SkillNameText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* SkillDescriptionText;
	
	// // --- 할당할 자료 ---
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	// UTexture2D* SkillBtnBackground;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	
	
private:
	UPROPERTY()
	TObjectPtr<USkillDataAsset> Skill;
	
	int32 SkillIndex = INDEX_NONE;
};
