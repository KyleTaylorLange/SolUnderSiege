// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "PlayerState_Assassination.h"
#include "HUD_Assassination.h"

void AHUD_Assassination::DrawObjectLabels()
{
	Super::DrawObjectLabels();
	// Draw overlay text for each CP.
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APlayerState_Assassination* PS = Cast<APlayerState_Assassination>(It->GetPlayerState());
		if (PS)
		{
			bool bAssassin = false;
			bool bHunter = false;
			FVector WorldPos = It->GetActorLocation();
			FVector2D ScreenPos;
			if (Cast<APlayerState_Assassination>(PlayerOwner->PlayerState)->AllowedKills.Contains(PS))
			{
				bAssassin = true;
			}
			else if (PS->AllowedKills.Contains(PlayerOwner->PlayerState))
			{
				bHunter = true;
			}
			if ((bAssassin || bHunter) && UGameplayStatics::ProjectWorldToScreen(PlayerOwner, WorldPos, ScreenPos) && PlayerOwner->GetPawn())
			{
				if (ScreenPos.X > 0 && ScreenPos.X < Canvas->ClipX && ScreenPos.Y > 0 && ScreenPos.Y < Canvas->ClipY)
				{
					FCanvasTextItem LabelItem = GetDefaultTextItem();
					LabelItem.Text = FText::FromString(*It->GetPlayerState()->GetPlayerName());
					LabelItem.SetColor(bAssassin ? FLinearColor::Red : FLinearColor::Yellow);
					LabelItem.bCentreX = true;
					LabelItem.bCentreY = true;
					Canvas->DrawItem(LabelItem, ScreenPos);
				}
			}
		}
	}
}

void AHUD_Assassination::DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState)
{
	Super::DrawGameData(DrawPosition, InGameState);
	// Draw all Control Points
	TArray<FString> Assassins, Hunters;
	for (TActorIterator<APlayerState_Assassination> It(GetWorld()); It; ++It)
	{
		//APlayerState_Assassination* PS = Cast<APlayerState_Assassination>(*It->GetPlayerState());
		//if (PS)
		//{
			if (Cast<APlayerState_Assassination>(PlayerOwner->PlayerState)->AllowedKills.Contains(*It))
			{
				Assassins.Add(*It->GetPlayerName());
			}
			else if (It->AllowedKills.Contains(PlayerOwner->PlayerState))
			{
				Hunters.Add(*It->GetPlayerName());
			}
		//}
	}
	for (int32 i = 0; i < Assassins.Num(); i++)
	{
		FCanvasTextItem NameText = GetDefaultTextItem();
		NameText.Text = FText::FromString(Assassins[i]);
		NameText.SetColor(FLinearColor::Red);
		Canvas->DrawItem(NameText, DrawPosition);
		DrawPosition.Y += 15.f;
	}
	for (int32 i = 0; i < Hunters.Num(); i++)
	{
		FCanvasTextItem NameText = GetDefaultTextItem();
		NameText.Text = FText::FromString(Hunters[i]);
		NameText.SetColor(FLinearColor::Yellow);
		Canvas->DrawItem(NameText, DrawPosition);
		DrawPosition.Y += 15.f;
	}
}