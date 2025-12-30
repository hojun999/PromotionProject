// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUnit.h"
#include "AllyUnit.generated.h"

/**
 * 
 */
UCLASS()
class PCUBE_API AAllyUnit : public ABaseUnit
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> CommandWidgetClass;
	
protected:
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	
private:
	UPROPERTY()
	UUserWidget* CurrentCommandWidget;
	
};
