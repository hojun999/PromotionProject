#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleActionIntentWidget.generated.h"

class UImage;
class UItemDataAsset;
class USkillDataAsset;
class UTextBlock;
class UTexture2D;

UCLASS()
class PCUBE_API UBattleActionIntentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Battle|UI")
	void ShowSkillIntent(USkillDataAsset* Skill);

	UFUNCTION(BlueprintCallable, Category="Battle|UI")
	void ShowItemIntent(UItemDataAsset* Item);

	UFUNCTION(BlueprintCallable, Category="Battle|UI")
	void ClearIntent();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Action = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_ActionName = nullptr;

private:
	void ApplyIntent(UTexture2D* Icon, const FText& Label);
};
