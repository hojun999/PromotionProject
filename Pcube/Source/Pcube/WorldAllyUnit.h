// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldBaseUnit.h"
#include "WorldAllyUnit.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PCUBE_API AWorldAllyUnit : public AWorldBaseUnit
{
	GENERATED_BODY()
	
public:
	AWorldAllyUnit();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// 입력 함수
	// if: enhanced input system 사용하면 필요 없을 수도?
	void MoveForward(float Value);	// + -> 위, - -> 아래
	void MoveRight(float Value);  // + -> 오른쪽, - -> 왼쪽
	
	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere,  Category="Camera")
	USpringArmComponent* PlayerSpringArm;
	
	UPROPERTY(VisibleAnywhere, Category="Camera")
	UCameraComponent* PlayerCamera;
};
