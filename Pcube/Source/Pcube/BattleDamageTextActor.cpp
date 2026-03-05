#include "BattleDamageTextActor.h"
#include "Components/TextRenderComponent.h"

ABattleDamageTextActor::ABattleDamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	TextComp->SetupAttachment(Root);

	TextComp->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextComp->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	TextComp->SetWorldSize(64.f);
	TextComp->SetTextRenderColor(FColor::Red);

	// 기본 수명
	SetLifeSpan(LifeTime);
}

void ABattleDamageTextActor::InitText(float DamageAmount, bool bIsHeal)
{
	if (!TextComp) return;

	const int32 Value = FMath::RoundToInt(DamageAmount);
	TextComp->SetText(FText::FromString(FString::Printf(TEXT("%d"), Value)));

	TextComp->SetTextRenderColor(bIsHeal ? FColor::Green : FColor::Red);

	// Init 이후에도 LifeTime이 바뀔 수 있으니 재설정
	SetLifeSpan(LifeTime);
	Elapsed = 0.f;
}

void ABattleDamageTextActor::BeginPlay()
{
	Super::BeginPlay();

	// BeginPlay 시점에도 LifeSpan 보장
	SetLifeSpan(LifeTime);
}

void ABattleDamageTextActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Elapsed += DeltaSeconds;

	// 위로 상승
	AddActorWorldOffset(FVector(0.f, 0.f, RiseSpeed * DeltaSeconds), false);

	// 아주 간단한 페이드(알파만)
	if (TextComp && LifeTime > 0.f)
	{
		const float Alpha01 = 1.f - FMath::Clamp(Elapsed / LifeTime, 0.f, 1.f);
		FColor C = TextComp->TextRenderColor;
		C.A = (uint8)FMath::Clamp(FMath::RoundToInt(Alpha01 * 255.f), 0, 255);
		TextComp->SetTextRenderColor(C);
	}
}
