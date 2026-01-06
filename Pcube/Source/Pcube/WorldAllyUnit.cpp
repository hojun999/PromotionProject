// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldAllyUnit.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AWorldAllyUnit::AWorldAllyUnit()
{
	// 카메라 설정 (쿼터뷰 시점)
	PlayerSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("PlayerSpringArm"));
	PlayerSpringArm->SetupAttachment(RootComponent);
	PlayerSpringArm->TargetArmLength = 1600.0f;
	PlayerSpringArm->SetRelativeRotation(FRotator(-25.0f, 0.0f, 0.0f)); // 시점 고정
	PlayerSpringArm->bDoCollisionTest = false; // Collision 충돌 시 카메라 줌인 방지
	PlayerSpringArm->bInheritYaw = false; // 컨트롤러 회전에 의한 카메라 회전 영향 방지
	
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(PlayerSpringArm);
	
	// 캐릭터 회전 설정 (마우스가 아닌 이동 방향으로 회전)
	bUseControllerRotationYaw = false; // 캐릭터가 카메라 방향을 따라가지 않음
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 자동 회전
	GetCharacterMovement() ->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // 회전 속도
}

void AWorldAllyUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AWorldAllyUnit::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AWorldAllyUnit::MoveRight);
}

void AWorldAllyUnit::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		// 카메라 기준 정면 방향 계산 (Yaw만 추출)
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		AddMovementInput(Direction, Value);
	}
}

void AWorldAllyUnit::MoveRight(float Value)
{
	if (Controller && Value != 0.f)
	{
		// 카메라 기준 오른쪽 방향 계산
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(Direction, Value);
	}
}
