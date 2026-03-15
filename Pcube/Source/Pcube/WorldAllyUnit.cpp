// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldAllyUnit.h"

#include "BattleInfoTransferSubsystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AWorldAllyUnit::AWorldAllyUnit()
{
	// 월드에서 조작하는 플레이어는 기본적으로 파티 0번으로 간주
	PartyIndex = 1;

	// 카메라 설정 (쿼터뷰 시점)
	PlayerSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("PlayerSpringArm"));
	PlayerSpringArm->SetupAttachment(RootComponent);
	PlayerSpringArm->TargetArmLength = 700.0f;
	PlayerSpringArm->SetRelativeLocation(FVector(25.0f, 0.0f, 50.f));
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

void AWorldAllyUnit::BeginPlay()
{
	Super::BeginPlay();
	
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UBattleInfoTransferSubsystem* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>())
		{
			FVector ReturnLoc;
			FRotator ReturnRot;
			if (Transfer->ConsumeReturnPoint(ReturnLoc, ReturnRot))
			{
				SetActorLocationAndRotation(ReturnLoc, ReturnRot, false, nullptr, ETeleportType::TeleportPhysics);
				
				// 텔레포트 후 오버랩 갱신(겹침 상태 정리)
				GetCapsuleComponent()->UpdateOverlaps();
			}
		}
	}
}

void AWorldAllyUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AWorldAllyUnit::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AWorldAllyUnit::MoveRight);
}

void AWorldAllyUnit::MoveForward(float Value)
{
	if (!Controller || Value == 0.f) return;

	// 카메라 컴포넌트의 실제 월드 회전 기준으로 이동 방향 계산
	// Controller->GetControlRotation()은 초기화 타이밍에 따라 폰 초기 회전을 반환할 수 있어 불안정
	FRotator CamRot;
	if (UCameraComponent* Cam = FindComponentByClass<UCameraComponent>())
	{
		CamRot = Cam->GetComponentRotation();
	}
	else
	{
		CamRot = Controller->GetControlRotation(); // 카메라 없으면 폴백
	}

	const FRotator YawRotation(0, CamRot.Yaw, 0);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}


void AWorldAllyUnit::MoveRight(float Value)
{
	if (!Controller || Value == 0.f) return;

	FRotator CamRot;
	if (UCameraComponent* Cam = FindComponentByClass<UCameraComponent>())
	{
		CamRot = Cam->GetComponentRotation();
	}
	else
	{
		CamRot = Controller->GetControlRotation(); // 카메라 없으면 폴백
	}

	const FRotator YawRotation(0, CamRot.Yaw, 0);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Value);
}

