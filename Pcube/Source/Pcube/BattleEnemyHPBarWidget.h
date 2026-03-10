#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleEnemyHPBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UBorder;
class UHorizontalBox;

UCLASS()
class PCUBE_API UBattleEnemyHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void InitForUnitName(const FString& InName);
	void SetHP(float CurrentHP, float MaxHP);

private:
	void EnsureWidgetTreeBuilt();

private:
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HPBar = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText = nullptr;
};
