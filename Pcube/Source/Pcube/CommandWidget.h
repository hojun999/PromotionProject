#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CommandWidget.generated.h"

UCLASS()
class PCUBE_API UCommandWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// meta = (BindWidget)은 블루프린트의 버튼 이름과 C++ 변수 이름을 일치시켜 자동으로 연결
	UPROPERTY(meta = (BindWidget))
	UButton* AttackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* DefendButton;

	UPROPERTY(meta = (BindWidget))
	UButton* EndTurnButton;

	virtual void NativeConstruct() override;

	// 각 버튼의 클릭 이벤트 함수
	UFUNCTION()
	void OnAttackClicked();

	UFUNCTION()
	void OnDefendClicked();

	UFUNCTION()
	void OnEndTurnClicked();
};