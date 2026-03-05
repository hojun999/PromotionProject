#include "WorldCameraZoneTrigger.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "WorldCCTVCameraDirector.h"

AWorldCameraZoneTrigger::AWorldCameraZoneTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TriggerBox->SetGenerateOverlapEvents(true);
}

void AWorldCameraZoneTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldCameraZoneTrigger::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AWorldCameraZoneTrigger::OnTriggerEndOverlap);

	EnsureDirector();

	// If user didn't set an anchor, default to this actor's transform.
	// (Makes it convenient to place this actor at the desired camera anchor.)
	if (CameraAnchor.GetLocation().IsNearlyZero() && CameraAnchor.GetRotation().IsIdentity())
	{
		CameraAnchor = GetActorTransform();
	}
}

void AWorldCameraZoneTrigger::EnsureDirector()
{
	if (Director)
	{
		return;
	}

	// Auto-find first director in the world.
	Director = Cast<AWorldCCTVCameraDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldCCTVCameraDirector::StaticClass()));
}

bool AWorldCameraZoneTrigger::IsPlayerActor(AActor* Actor) const
{
	if (!Actor) return false;

	APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn) return false;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	return Pawn == PlayerPawn;
}

void AWorldCameraZoneTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsPlayerActor(OtherActor))
	{
		return;
	}

	EnsureDirector();
	if (!Director)
	{
		return;
	}

	// Optional per-zone rotation speed.
	if (bOverrideRotationSpeed)
	{
		Director->SetRotationInterpSpeed(RotationInterpSpeedOverride);
	}

	Director->RequestAnchorTransform(CameraAnchor, BlendTime, Priority);
	Director->SetTargetActor(OtherActor);
}

void AWorldCameraZoneTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bIgnoreEndOverlap)
	{
		return;
	}

	// Optional: when leaving, drop priority back to 0 (or a default zone).
	// For now, do nothing by default.
}