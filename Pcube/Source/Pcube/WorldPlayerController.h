#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WorldPlayerController.generated.h"

UCLASS()
class PCUBE_API AWorldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWorldPlayerController();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 카메라 전환 중 여부. 서브클래스에서 오버라이드 가능
	virtual bool IsCameraTransitionBusy() const;
	
protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	void Input_ToggleInventory(); // I
	void Input_TogglePlayerInfo(); // Tab
	void Input_CloseUI(); // ESC

	UPROPERTY()
	class AWorldCCTVCameraDirector* CachedDirector = nullptr;

	FTimerHandle TimerHandle_TrySetCCTV;

	void TrySetCCTVView();
};
