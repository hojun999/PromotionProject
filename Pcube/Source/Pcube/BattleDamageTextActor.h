#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleDamageTextActor.generated.h"

class UTextRenderComponent;

UCLASS()
class PCUBE_API ABattleDamageTextActor : public AActor
{
	GENERATED_BODY()

public:
	ABattleDamageTextActor();

	// DamageAmount: 표시할 값(양수). bIsHeal=true면 회복 텍스트로 표시
	UFUNCTION(BlueprintCallable)
	void InitText(float DamageAmount, bool bIsHeal);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> TextComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="DamageText")
	float LifeTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="DamageText")
	float RiseSpeed = 80.0f;

	float Elapsed = 0.f;
};
