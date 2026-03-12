#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleEnemyHPBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class PCUBE_API UBattleEnemyHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitForUnitName(const FString& InName);
	void SetHP(float CurrentHP, float MaxHP);

private:
	// WBP_BattleEnemyHPBar 디자이너에서 이름을 정확히 맞춰야 바인딩됨
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HPBar = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText = nullptr;
};
