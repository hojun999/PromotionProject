#include "AllyUnit.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"

void AAllyUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	// 이미 UI가 켜져 있다면 닫기
	if (CurrentCommandWidget) 
	{
		CurrentCommandWidget->RemoveFromParent();
		CurrentCommandWidget = nullptr;
		return;
	}

	if (CommandWidgetClass)
	{
		CurrentCommandWidget = CreateWidget<UUserWidget>(GetWorld(), CommandWidgetClass);
		if (CurrentCommandWidget)
		{
			CurrentCommandWidget->AddToViewport();

			// 유닛 우측에 위치시키기 (World to Screen 변환)
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				FVector2D ScreenPosition;
				// 유닛 위치에서 오른쪽으로 100 유닛만큼 떨어진 지점 계산
				FVector WorldLocation = GetActorLocation() + GetActorRightVector() * 100.0f;
                
				if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, ScreenPosition, false))
				{
					CurrentCommandWidget->SetPositionInViewport(ScreenPosition);
				}
			}
		}
	}
}