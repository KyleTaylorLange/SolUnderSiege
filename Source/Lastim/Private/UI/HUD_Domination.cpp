// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "DominationControlPoint.h"
#include "TeamState.h"
#include "GameState_Domination.h"
#include "Math/UnitConversion.h"
#include "Kismet/GameplayStatics.h"
#include "HUD_Domination.h"

void AHUD_Domination::DrawObjectLabels()
{
	Super::DrawObjectLabels();
	// Draw overlay text for each CP.
	AGameState_Domination* DomGS = Cast<AGameState_Domination>(GetWorld()->GetGameState());
	if (DomGS && DomGS->ControlPoints.Num() > 0)
	{
		int32 i = 0;
		for (ADominationControlPoint* ControlPoint : DomGS->ControlPoints)
		{
			FVector WorldPos = ControlPoint->GetActorLocation();
			FVector2D ScreenPos = FVector2D(0.f, 0.f);
			if (UGameplayStatics::ProjectWorldToScreen(PlayerOwner, WorldPos, ScreenPos) && PlayerOwner->GetPawn())
			{
				float Distance = FMath::Sqrt((WorldPos - PlayerOwner->GetPawn()->GetActorLocation()).SizeSquared());
				if (ScreenPos.X > 0 && ScreenPos.X < Canvas->ClipX && ScreenPos.Y > 0 && ScreenPos.Y < Canvas->ClipY && Distance > 1000)
				{
					FCanvasTextItem LabelItem = GetDefaultTextItem();
					EUnit UnitType = EUnit::Centimeters;
					if (DomGS->ControlPoints.Num() % 2 == 1)
					{
						Distance = FUnitConversion::Convert(Distance, EUnit::Centimeters, EUnit::Inches);
						UnitType = EUnit::Inches;
					}
					FNumericUnit<float> DisplayDistance = FUnitConversion::QuantizeUnitsToBestFit(Distance, UnitType);
					UnitType = DisplayDistance.Units;
					FString DistToPrint = FString::FromInt(FMath::CeilToInt(DisplayDistance.Value));
					FString CPName = FString(" CP") + FString::FromInt(0 + i) + FString(" ") + DistToPrint + FString(" ") + FUnitConversion::GetUnitDisplayString(DisplayDistance.Units);
					//const FString CPName = FString::Printf(TEXT(" CP%d                %s %s"), i, *DistToPrint, FUnitConversion::GetUnitDisplayString(DisplayDistance.Units));
					LabelItem.bCentreX = true;
					LabelItem.bCentreY = true;
					LabelItem.Text = FText::FromString(CPName);
					LabelItem.SetColor(ControlPoint->GetOwningTeam() ? ControlPoint->GetOwningTeam()->GetTeamColor() : FLinearColor::White);
					Canvas->DrawItem(LabelItem, ScreenPos);
					i++;
				}
			}
		}
	}
}

void AHUD_Domination::DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState)
{
	Super::DrawGameData(DrawPosition, InGameState);
	// Draw all Control Points
	AGameState_Domination* DomGS = Cast<AGameState_Domination>(InGameState);
	if (DomGS && DomGS->ControlPoints.Num() > 0)
	{
		for (int32 i = 0; i < DomGS->ControlPoints.Num(); i++)
		{
			FCanvasTextItem TextItem = GetDefaultTextItem();
			const FString CPName = FString::Printf(TEXT(" CP%d "), i);
			TextItem.Text = FText::FromString(CPName);
			TextItem.SetColor(DomGS->ControlPoints[i]->GetOwningTeam() ? DomGS->ControlPoints[i]->GetOwningTeam()->GetTeamColor() : FLinearColor::White);
			Canvas->DrawItem(TextItem, DrawPosition);
			FVector2D Size;
			Canvas->StrLen(TextItem.Font, CPName, Size.X, Size.Y);
			DrawPosition.X += Size.X;
			// Attempt to draw label on screen?
			FVector WorldPos = DomGS->ControlPoints[i]->GetActorLocation();
			FVector2D ScreenPos = FVector2D(0.f, 0.f);
			if (UGameplayStatics::ProjectWorldToScreen(PlayerOwner, WorldPos, ScreenPos)&& PlayerOwner->GetPawn())
			{
				float Distance = FMath::Sqrt((WorldPos - PlayerOwner->GetPawn()->GetActorLocation()).SizeSquared());
				if (ScreenPos.X > 0 && ScreenPos.X < Canvas->ClipX && ScreenPos.Y > 0 && ScreenPos.Y < Canvas->ClipY && Distance > 1000)
				{
					FCanvasTextItem LabelItem = GetDefaultTextItem();
					LabelItem.Text = FText::FromString(CPName);
					LabelItem.SetColor(DomGS->ControlPoints[i]->GetOwningTeam() ? DomGS->ControlPoints[i]->GetOwningTeam()->GetTeamColor() : FLinearColor::White);
					LabelItem.bCentreX = true;
					LabelItem.bCentreY = true;
					Canvas->DrawItem(LabelItem, ScreenPos);
				}
			}
		}
	}
}
