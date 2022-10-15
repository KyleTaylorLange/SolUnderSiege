// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SolHUD.h"
#include "Sol.h"
#include "SolCharacter.h"
#include "SolGameState.h"
#include "SolGameMode.h"
#include "SolPlayerController.h"
#include "SolWorldSettings.h"
#include "UserWidget.h"
#include "SScoreboardWidget.h"
#include "SInGameMenuWidget.h"
#include "SolPlayerState.h"
#include "TeamState.h"
#include "Firearm.h"
#include "InteractableComponent.h"
#include "Math/UnitConversion.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "InventoryComponent.h"

ASolHUD::ASolHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> LowHealthOverlayTextureObj(TEXT("/Game/UI/HUD/LowHealthOverlay"));
	LowHealthOverlayTexture = LowHealthOverlayTextureObj.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDAssetsTextureObj(TEXT("/Game/UI/HUD/T_HUDAssets"));
	UTexture2D* HUDAssetsTexture = HUDAssetsTextureObj.Object;

	Crosshair = UCanvas::MakeIcon(HUDAssetsTexture, 2, 2, 12, 12);

	// Icon taken from Wikipedia's Star of Life article; slightly modified to be recolourable.
	static ConstructorHelpers::FObjectFinder<UTexture2D> HealthIconTextureObj(TEXT("/Game/UI/HUD/WikipediaStarOfLifeMono"));
	HealthIconTexture = HealthIconTextureObj.Object;

	static ConstructorHelpers::FObjectFinder<UFont> LargeFontObj(TEXT("/Game/UI/HUD/Roboto51"));
	LargeFont = LargeFontObj.Object;
	static ConstructorHelpers::FObjectFinder<UFont> MediumFontObj(TEXT("/Game/UI/HUD/Roboto18"));
	MediumFont = MediumFontObj.Object;
	static ConstructorHelpers::FObjectFinder<UFont> SmallFontObj(TEXT("/Game/UI/HUD/Roboto09"));
	SmallFont = SmallFontObj.Object; //GEngine->GetSmallFont()

	// Just draw the HUD in canvas for now. Too many changes to keep the UMG one updated.
	static ConstructorHelpers::FClassFinder<UUserWidget> FoundHUDWidgetClass(TEXT("/Game/UI/HUD/HUDWidget"));
	HUDWidgetClass = FoundHUDWidgetClass.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> FoundInventoryWidgetClass(TEXT("/Game/UI/HUD/InventoryDisplayWidget"));
	InventoryWidgetClass = FoundInventoryWidgetClass.Class;

	//SlateScoreboardWidgetClass = class<SScoreboardWidget>;
	//SlateInGameMenuWidgetClass = SInGameMenuWidget::StaticClass();

	HUDDrawScale = 1.f;

	MaxDeathMessages = 10;
	DeathMessageDuration = 10.0f;
	MaxHeaderMessages = 3;
	HeaderMessageDuration = 5.0f;
	bShowInventory = false;
	bShowScoreboard = false;
	bHUDMessagesChanged = false;

	bDrawLivesInScoreboard = false;
}

void ASolHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (const ASolPlayerController* PCOwner = Cast<ASolPlayerController>(PlayerOwner))
	{
		// Create the HUD widget.
		if (HUDWidgetClass)
		{
			if ((HUDWidget = CreateWidget<UUserWidget, APlayerController>(PlayerOwner, HUDWidgetClass)) != nullptr)
			{
				HUDWidget->AddToViewport();
			}
		}

		// Create a UMG scoreboard widget if we have one.
		if (ScoreboardWidgetClass)
		{
			if ((ScoreboardWidget = CreateWidget<UUserWidget, APlayerController>(PlayerOwner, ScoreboardWidgetClass)) != nullptr)
			{
				ScoreboardWidget->AddToViewport();
			}
		}
		// Otherwise, just make the old Slate widget for now.
		else if (!SlateScoreboardWidget.IsValid())
		{
			//SAssignNew(SlateScoreboardWidget, SlateScoreboardWidgetClass)
			SAssignNew(SlateScoreboardWidget, SScoreboardWidget)
				.OwnerHUD(this)
				.PCOwner(TWeakObjectPtr<APlayerController>(PlayerOwner));

			if (SlateScoreboardWidget.IsValid())
			{

				GEngine->GameViewport->AddViewportWidgetContent(
					SNew(SWeakWidget)
					.PossiblyNullContent(SlateScoreboardWidget.ToSharedRef())
					);
				//MyHUDMenuWidget->ActionButtonsWidget->SetVisibility(EVisibility::Visible);
				//MyHUDMenuWidget->ActionWidgetPosition.BindUObject(this, &AStrategyHUD::GetActionsWidgetPos);
			}
		}

		// Create a UMG in-game menu if we have one.
		if (InGameMenuWidgetClass)
		{
			if ((InGameMenuWidget = CreateWidget<UUserWidget, APlayerController>(PlayerOwner, InGameMenuWidgetClass)) != nullptr)
			{
				InGameMenuWidget->AddToViewport();
			}
		}
		// Otherwise, just make the old Slate version for now.
		else if (!SlateInGameMenuWidget.IsValid())
		{
			//SAssignNew(SlateInGameMenuWidget, SlateInGameMenuWidgetClass)

			ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);

			SAssignNew(SlateInGameMenuWidget, SInGameMenuWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(MyPlayerOwner)
			.OwnerHUD(this);

			if (SlateInGameMenuWidget.IsValid())
			{

				GEngine->GameViewport->AddViewportWidgetContent(
					SNew(SWeakWidget)
					.PossiblyNullContent(SlateInGameMenuWidget.ToSharedRef())
					);
				ShowInGameMenu(false);
				//MyHUDMenuWidget->ActionButtonsWidget->SetVisibility(EVisibility::Visible);
				//MyHUDMenuWidget->ActionWidgetPosition.BindUObject(this, &AStrategyHUD::GetActionsWidgetPos);
			}
		}

		// Setup the widget to forward focus to when the viewport receives focus.
		TSharedPtr<SViewport> GameViewportWidget = GEngine->GetGameViewportWidget();
		if (GameViewportWidget.IsValid())
		{
			//GameViewportWidget->SetWidgetToFocusOnActivate(SlateScoreboardWidget);
			//GameViewportWidget->SetWidgetToFocusOnActivate(SlateInGameMenuWidget);
		}
	}
	if (GEngine)
	{
		//TinyFont = GEngine->GetTinyFont();
		SmallFont = GEngine->GetSmallFont();
		MediumFont = GEngine->GetMediumFont();
		//LargeFont = GEngine->GetLargeFont();
	}
}

void ASolHUD::DrawHUD()
{
	Super::DrawHUD();
	HUDDrawScale = FMath::Max(Canvas->ClipY / 1080.f, 0.5f);

	// Temporary way to show network players that the game is loading.
	// Not sure if this will display anything.
	if (GetMatchState() == MatchState::EnteringMap || GetMatchState() == MatchState::LeavingMap || GetMatchState() == MatchState::Aborted)
	{
		FCanvasTextItem TextItem = GetDefaultTextItem();
		TextItem.Text = NSLOCTEXT("HUD", "EnteringMap", "Entering Map");
		if (GetMatchState() == MatchState::LeavingMap)
		{
			TextItem.Text = NSLOCTEXT("HUD", "LeavingMap", "Leaving Map");
		}
		else if (GetMatchState() == MatchState::Aborted)
		{
			TextItem.Text = NSLOCTEXT("HUD", "ConnectionAborted", "Connection Aborted");
		}
		TextItem.Font = LargeFont;
		TextItem.bCentreX = true;
		TextItem.bCentreY = true;
		TextItem.Scale = FVector2D(HUDDrawScale, HUDDrawScale) * 2;
		TextItem.SetColor(FColor::White);
		TextItem.Position = FVector2D((Canvas->ClipX / 2), (Canvas->ClipY / 2));
		Canvas->DrawItem(TextItem);
		return;
	}

	ASolCharacter* MyPawn = Cast<ASolCharacter>(GetOwningPawn());
	ASolGameState* MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	// Draw the Scoreboard if toggled, the game is over, or if the game hasn't begun yet.
	if (bShowScoreboard || GetMatchState() == "WaitingPostMatch" || GetMatchState() == "WaitingToStart")
	{
		if (HUDWidget && HUDWidget->GetVisibility() != ESlateVisibility::Hidden)
		{
			HUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		if (ScoreboardWidget)
		{
			ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else if (SlateScoreboardWidget.IsValid())
		{
			SlateScoreboardWidget->SetVisibility(EVisibility::Visible);
		}
	}
	else
	{
		if (HUDWidget && HUDWidget->GetVisibility() != ESlateVisibility::Visible)
		{
			HUDWidget->SetVisibility(ESlateVisibility::Visible);
		}
		if (ScoreboardWidget)
		{
			ScoreboardWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		else if (SlateScoreboardWidget.IsValid())
		{
			SlateScoreboardWidget->SetVisibility(EVisibility::Hidden);
		}
	}
	// Most HUD stuff is still in canvas for now.
	UpdateHUDMessages();
	if (MyPawn)
	{
		DrawObjectLabels();
		// Low health overlay.
		if (MyPawn->GetHealth() < MyPawn->GetFullHealth() ) //if (MyPawn && MyPawn->IsAlive() && MyPawn->Health < MyPawn->GetFullHealth() * MyPawn->GetLowHealthPercentage())
		{
			//Was 1.0f + 5.0f *
			const float AnimSpeedModifier = 3.14f + 9.42f * FMath::Square((1.0f - MyPawn->GetHealth() / (100 * 1.0))); // (1.0f - MyPawn->Health / (MyPawn->GetFullHealth() * MyPawn->GetLowHealthPercentage()));
			// Was 32 + 72 *
			int32 EffectValue = 127 * (1.0f - MyPawn->GetHealth() / (MyPawn->GetFullHealth() * 1.0)); // (1.0f - MyPawn->Health / (MyPawn->GetFullHealth() * MyPawn->GetLowHealthPercentage()));
			LowHealthPulseValue += GetWorld()->GetDeltaSeconds() * AnimSpeedModifier;
			float EffectAlpha = 0.25f + 0.75f * FMath::Abs(FMath::Sin(LowHealthPulseValue));
			float HitNotifyValue = 127 * (NotifyHitDamage / MyPawn->GetFullHealth());
			NotifyHitDamage -= GetWorld()->GetDeltaSeconds() * LastNotifyHitDamage;
			if (NotifyHitDamage < 0)
			{
				NotifyHitDamage = 0;
			}

			float AlphaValue = (1.0f / 255.0f) * ((EffectAlpha * EffectValue) + HitNotifyValue);

			// Full screen low health overlay
			Canvas->PopSafeZoneTransform();
			FCanvasTileItem TileItem(FVector2D(0, 0), LowHealthOverlayTexture->Resource, FVector2D(Canvas->ClipX, Canvas->ClipY), FLinearColor(1.0f, 0.0f, 0.0f, AlphaValue));
			TileItem.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(TileItem);
			Canvas->ApplySafeZoneTransform();
		}
		// Draw use object message if necessary.
		TSubclassOf<UInteractionEvent> Interaction = nullptr;
		if (UInteractableComponent* Interactable = MyPawn->FindInteractable(Interaction))
		{
			FString TextString = FString::Printf(TEXT("[USE] ~Unknown Use~"));
			if (Interaction)
			{
				FString InteractionName = Interaction->GetDefaultObject<UInteractionEvent>()->GetActionName(MyPawn, Interactable);
				TextString = FString::Printf(TEXT("[USE] %s"), *InteractionName);
			}
			FCanvasTextItem TextItem = GetDefaultTextItem();
			TextItem.bCentreX = true;
			TextItem.bCentreY = true;
			TextItem.Text = FText::FromString(TextString);
			TextItem.Scale = FVector2D(HUDDrawScale, HUDDrawScale);
			FVector2D DrawPosition = FVector2D((Canvas->ClipX * 0.5), (Canvas->ClipY * 0.625));
			TextItem.SetColor(FColor::Green);
			TextItem.Position = DrawPosition;
			Canvas->DrawItem(TextItem);
		}
	}
	// This should be transferred to the HUD.
	//  In addition, the text should change if the player cannot respawn.
	else
	{
		// Hide inventory if we have no pawn.
		ShowInventory(false);
		if (!bShowScoreboard && MyGameState && GetMatchState() == "InProgress")
		{
			ASolPlayerState* MyPlayerState = PlayerOwner ? Cast<ASolPlayerState>(PlayerOwner->PlayerState) : NULL;
			FText DeathText;
			if (PlayerOwner->CanRestartPlayer())
			{
				DeathText = NSLOCTEXT("HUD", "DeathText", "Press [FIRE] to respawn.");
			}
			else if (MyPlayerState && MyPlayerState->RespawnTime > 0.0f)
			{
				DeathText = FText::FromString(FString::Printf(TEXT("Can respawn in: %d"), FMath::CeilToInt(MyPlayerState->RespawnTime)));
			}
			if (!DeathText.IsEmpty())
			{
				FCanvasTextItem TextItem = GetDefaultTextItem();
				TextItem.Text = DeathText;
				TextItem.Font = LargeFont;
				TextItem.bCentreX = true;
				TextItem.bCentreY = true;
				TextItem.Scale = FVector2D(HUDDrawScale, HUDDrawScale);
				FVector2D DrawPosition = FVector2D((Canvas->ClipX * 0.5), (Canvas->ClipY * 0.75));
				TextItem.SetColor(FColor::White);
				TextItem.Position = DrawPosition;
				Canvas->DrawItem(TextItem);
			}
		}
	}
	if (!bShowScoreboard && GetMatchState() != MatchState::EnteringMap && GetMatchState() != MatchState::WaitingToStart)
	{
		FVector2D DrawPosition = FVector2D(5.f, 5.f);
		DrawDeathMessages(DrawPosition);
		DrawGameData(DrawPosition, MyGameState);
		DrawHeaderMessages();

		if (MyPawn)
		{
			AWeapon* MyWeapon = Cast<AWeapon>(MyPawn->GetEquippedItem());
			if (MyWeapon)
			{
				DrawCrosshair(MyPawn, MyWeapon);
			}
			if (bShowWeaponList)
			{
				DrawWeaponList(MyPawn);
			}
		}
	}
}

void ASolHUD::DrawCrosshair(ASolCharacter* InPlayer, AWeapon* InWeapon)
{
	if (InWeapon)
	{
		FVector AimPoint = InWeapon->GetAimPoint();
		if (AimPoint != FVector::ZeroVector)
		{
			FVector2D ScreenPos = FVector2D(0.f, 0.f);
			if (UGameplayStatics::ProjectWorldToScreen(PlayerOwner, AimPoint, ScreenPos))
			{
				float Distance = (AimPoint - PlayerOwner->GetPawn()->GetActorLocation()).Size();
				const float FurthestScalingDistance = 10000.f;
				float CrosshairScale = FMath::Min(HUDDrawScale * 0.25f, 0.25f) * FMath::Max(1.f, 1.f + 1.f * ((FurthestScalingDistance - Distance) / FurthestScalingDistance));
				Canvas->SetDrawColor(255, 255, 255, 192);
				Canvas->DrawIcon(Crosshair, 
					ScreenPos.X - (Crosshair.VL * CrosshairScale / 2.f),
					ScreenPos.Y - (Crosshair.VL * CrosshairScale / 2.f),
					CrosshairScale);
			}
		}
	}
}

void ASolHUD::DrawWeaponList(ASolCharacter* InPlayer)
{
	if (InPlayer)
	{
		FVector2D Size(0.0f, 0.0f);
		FVector2D DrawPosition(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.625f);
		FCanvasTextItem TextItem = GetDefaultTextItem();
		TextItem.Font = MediumFont;
		TextItem.bCentreX = true;
		AInventoryItem* EquippedWeapon = InPlayer->GetEquippedItem();
		AInventoryItem* TestWeapon = nullptr;
		int32 EquippedWeaponIndex = -1;

		/* Roundabout way to get EquippedWeaponIndex. */
		TArray<AInventoryItem*> Inventory = InPlayer->GetInventoryComponent()->GetInventory();
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			TestWeapon = Cast<AInventoryItem>(Inventory[i]);
			if (TestWeapon == EquippedWeapon)
			{
				EquippedWeaponIndex = i;
				break;
			}
		}

		/* Draw the weapon list. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			TestWeapon = Inventory[i];
			if (TestWeapon)
			{
				TextItem.Text = FText::FromString(Inventory[i]->GetDisplayName());
				TextItem.SetColor(FColor(127, 127, 127));
				// Highlight equipped weapon and selected weapon.
				if (FMath::Abs(EquippedWeaponIndex + DeltaWeapSelectIndex) % Inventory.Num() == i)
				{
					TextItem.SetColor(FColor::Blue);
				}
				else if (EquippedWeaponIndex == i)
				{
					TextItem.SetColor(FColor::White);
				}
				DrawPosition.Y += 20.0f;
				Canvas->DrawItem(TextItem, DrawPosition);
			}
		}
	}
}

void ASolHUD::DrawDeathMessages(FVector2D &DrawPosition)
{
	float LeftMargin = DrawPosition.X;
	FVector2D Size(0.0f, 0.0f);
	FVector2D BoxSize(0.0f, 0.0f);
	FCanvasTextItem TextItem = GetDefaultTextItem();
	TArray<FCanvasTextItem> TextItems;
	TextItem.Font = SmallFont;
	for (int32 i = 0; i < DeathMessages.Num(); i++)
	{
		FComplexString CompString = DeathMessages[i];
		for (int32 j = 0; j < CompString.MessageText.Num(); j++)
		{
			TextItem.Text = FText::FromString(CompString.MessageText[j]);
			if (CompString.MessageColor.IsValidIndex(j))
			{
				TextItem.SetColor(CompString.MessageColor[j]);

			}
			//UFont* TinyFont = GEngine->GetTinyFont();
			//TextItem.Depth = i;
			Canvas->StrLen(SmallFont, TextItem.Text.ToString(), Size.X, Size.Y);
			TextItem.Position = DrawPosition;
			TextItems.Add(TextItem);
			//Canvas->DrawItem(TextItem);

			DrawPosition.X += Size.X;
		}
		BoxSize.X = FMath::Max(BoxSize.X, DrawPosition.X + Size.X);
		BoxSize.Y = FMath::Max(BoxSize.Y, DrawPosition.Y + Size.Y);
		DrawPosition.X = LeftMargin;
		DrawPosition.Y += Size.Y;
	}
	FCanvasTileItem BoxItemTest(FVector2D(0.f, 0.f), BoxSize, FLinearColor(0.f, 0.f, 0.f, 0.25f));
	//BoxItemTest.SetColor(FLinearColor(0.f, 0.f, 0.f, 0.25f));
	//BoxItemTest.Size = BoxSize;
	//BoxItemTest.StereoDepth = 1;
	BoxItemTest.BlendMode = SE_BLEND_Translucent;
	//BoxItemTest.Z = 1;
	Canvas->DrawItem(BoxItemTest);
	for (int32 i = 0; i < TextItems.Num(); i++)
	{
		Canvas->DrawItem(TextItems[i]);
	}
}

void ASolHUD::DrawHeaderMessages()
{
	FVector2D Size(0.0f, 0.0f);
	FCanvasTextItem TextItem = GetDefaultTextItem();
	FVector2D DrawPosition = FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.15f);
	TextItem.Font = MediumFont;
	for (int32 i = 0; i < HeaderMessages.Num(); i++)
	{
		FComplexString CompString = HeaderMessages[i];
		FString CombinedString;
		for (int32 j = 0; j < CompString.MessageText.Num(); j++)
		{
			CombinedString += CompString.MessageText[j];
		}
		Canvas->StrLen(MediumFont, CombinedString, Size.X, Size.Y);
		float CombinedStringOffset = Size.X * 0.5f;
		DrawPosition.X -= CombinedStringOffset;
		for (int32 j = 0; j < CompString.MessageText.Num(); j++)
		{
			TextItem.Text = FText::FromString(CompString.MessageText[j]);
			if (CompString.MessageColor.IsValidIndex(j))
			{
				TextItem.SetColor(CompString.MessageColor[j]);

			}
			Canvas->StrLen(MediumFont, TextItem.Text.ToString(), Size.X, Size.Y);
			//TextItem.bCentreX = true;
			//TextItem.bCentreY = true;
			Canvas->DrawItem(TextItem, DrawPosition);
			DrawPosition.X += Size.X;
		}
		DrawPosition.X = Canvas->ClipX * 0.5f;
		DrawPosition.Y += Size.Y;
	}
}

void ASolHUD::DrawObjectLabels()
{
	// Nothing here by default.
}

void ASolHUD::DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState)
{
	bool bDrawTeamData = InGameState && InGameState->TeamArray.Num() != 0;
	// Draw team scores if it's a team game by default.
	if (bDrawTeamData)
	{
		ASolPlayerState* MyPlayerState = PlayerOwner ? Cast<ASolPlayerState>(PlayerOwner->PlayerState) : NULL;
		int32 TeamNum = MyPlayerState->GetTeamNum();

		FCanvasTextItem TextItem = GetDefaultTextItem();
		Canvas->DrawItem(TextItem, DrawPosition);
		DrawPosition.Y += 15.f;
		// Draw list of team scores - starting with our team.
		for (int32 i = 0; i < InGameState->TeamArray.Num(); i++)
		{
			int32 AdjTeamIdx = (i + TeamNum) % InGameState->TeamArray.Num();
			if (InGameState->TeamArray.IsValidIndex(AdjTeamIdx))
			{
				TextItem.Text = FText::FromString(FString::Printf(TEXT("%s: %d"), *InGameState->TeamArray[AdjTeamIdx]->GetTeamName().ToString(),
					FMath::FloorToInt(InGameState->TeamArray[AdjTeamIdx]->GetScore())));
				TextItem.SetColor(InGameState->TeamArray[AdjTeamIdx]->GetTeamColor());
				Canvas->DrawItem(TextItem, DrawPosition);
				DrawPosition.Y += 15.f;
			}
		}
	}
}

void ASolHUD::UpdateHUDMessages()
{
	for (int32 i = 0; i < DeathMessages.Num(); i++)
	{
		if (DeathMessages[i].HideTime <= GetWorld()->GetTimeSeconds())
		{
			DeathMessages.RemoveAt(i, 1, true);
			bHUDMessagesChanged = true;
			i--; // Check this index again.
		}
	}
	for (int32 i = 0; i < HeaderMessages.Num(); i++)
	{
		if (HeaderMessages[i].HideTime <= GetWorld()->GetTimeSeconds())
		{
			HeaderMessages.RemoveAt(i, 1, true);
			bHUDMessagesChanged = true;
			i--; // Check this index again.
		}
	}
}

void ASolHUD::SetShowWeaponList(bool bNewValue)
{
	bShowWeaponList = bNewValue;
}

void ASolHUD::SetWeapSelectIndex(int32 NewIndex)
{
	DeltaWeapSelectIndex = NewIndex;
}

void ASolHUD::AddChatMessage(const ASolPlayerState* Sender, const FString Message, FName Type)
{
	ASolPlayerState* MyPlayerState = PlayerOwner ? Cast<ASolPlayerState>(PlayerOwner->PlayerState) : NULL;
	if (Sender && MyPlayerState && !Message.IsEmpty())
	{
		FLinearColor MessageDrawColor = FLinearColor(0.75f, 0.75f, 0.75f);
		FComplexString NewChatMessage;
		bool bTeamSay = Type == TEXT("TeamSay");
		bTeamSay ? NewChatMessage.MessageText.Add(Sender->GetPlayerName() + FString(TEXT(" [TEAM]: "))) 
				 : NewChatMessage.MessageText.Add(Sender->GetPlayerName() + FString(TEXT(": ")));
		NewChatMessage.MessageColor.Add(GetPlayerNameDrawColor(Sender->GetTeam(), MyPlayerState == Sender));

		NewChatMessage.MessageText.Add(Message);
		NewChatMessage.MessageColor.Add(MessageDrawColor);

		NewChatMessage.HideTime = GetWorld()->GetTimeSeconds() + DeathMessageDuration;

		if (DeathMessages.Num() >= MaxDeathMessages)
		{
			DeathMessages.RemoveAt(0, 1, true);
		}
		DeathMessages.Add(NewChatMessage);
		bHUDMessagesChanged = true;
	}
}

void ASolHUD::AddDeathMessage(ASolPlayerState* KillerPlayerState, ASolPlayerState* VictimPlayerState, const UDamageType* KillerDamageType)
{
	if (GetWorld()->GetGameState() && GetWorld()->GetGameState()->GameModeClass)
	{
		ASolPlayerState* MyPlayerState = PlayerOwner ? Cast<ASolPlayerState>(PlayerOwner->PlayerState) : NULL;

		if (KillerPlayerState && VictimPlayerState && MyPlayerState)
		{
			FString KillerName = KillerPlayerState->GetPlayerName();
			FString VictimName = VictimPlayerState->GetPlayerName();
			bool bIsSuicide = KillerPlayerState == VictimPlayerState;
			FLinearColor KillerColor = GetPlayerNameDrawColor(KillerPlayerState->GetTeam(), MyPlayerState == KillerPlayerState);
			FLinearColor VictimColor = GetPlayerNameDrawColor(VictimPlayerState->GetTeam(), MyPlayerState == VictimPlayerState);
			FLinearColor MessageDrawColor = FLinearColor(0.75f, 0.75f, 0.75f);

			FComplexString NewDeathMessage;
			//FMessageComponent NewMessComp;
			if (bIsSuicide)
			{
				NewDeathMessage.MessageText.Add(VictimName);
				NewDeathMessage.MessageColor.Add(VictimColor);

				NewDeathMessage.MessageText.Add(FString(TEXT(" died.")));
				NewDeathMessage.MessageColor.Add(MessageDrawColor);
			}
			else
			{
				NewDeathMessage.MessageText.Add(KillerName);
				NewDeathMessage.MessageColor.Add(KillerColor);

				NewDeathMessage.MessageText.Add(FString(TEXT(" killed ")));
				NewDeathMessage.MessageColor.Add(MessageDrawColor);

				NewDeathMessage.MessageText.Add(VictimName);
				NewDeathMessage.MessageColor.Add(VictimColor);

				NewDeathMessage.MessageText.Add(FString(TEXT(".")));
				NewDeathMessage.MessageColor.Add(MessageDrawColor);
			}

			//NewDeathMessage.DamageType = Cast<const UShooterDamageType>(KillerDamageType);
			NewDeathMessage.HideTime = GetWorld()->GetTimeSeconds() + DeathMessageDuration;

			if (DeathMessages.Num() >= MaxDeathMessages)
			{
				DeathMessages.RemoveAt(0, 1, true);
			}
			DeathMessages.Add(NewDeathMessage);
			bHUDMessagesChanged = true;

			// Draw header messages for people we kill (or people who kill us).
			if (KillerPlayerState == MyPlayerState && !bIsSuicide)
			{
				FComplexString NewHeaderMessage;
				NewHeaderMessage.MessageText.Add(FString(TEXT("You killed ")));
				NewHeaderMessage.MessageColor.Add(MessageDrawColor);

				NewHeaderMessage.MessageText.Add(VictimName);
				NewHeaderMessage.MessageColor.Add(VictimColor);

				NewHeaderMessage.MessageText.Add(FString(TEXT(".")));
				NewHeaderMessage.MessageColor.Add(MessageDrawColor);

				NewHeaderMessage.HideTime = GetWorld()->GetTimeSeconds() + HeaderMessageDuration;

				if (HeaderMessages.Num() >= MaxHeaderMessages)
				{
					HeaderMessages.RemoveAt(0, 1, true);
				}
				HeaderMessages.Add(NewHeaderMessage);
				bHUDMessagesChanged = true;
			}
			else if (VictimPlayerState == MyPlayerState && !bIsSuicide)
			{
				FComplexString NewHeaderMessage;
				NewHeaderMessage.MessageText.Add(KillerName);
				NewHeaderMessage.MessageColor.Add(KillerColor);

				NewHeaderMessage.MessageText.Add(FString(TEXT(" killed you.")));
				NewHeaderMessage.MessageColor.Add(MessageDrawColor);

				NewHeaderMessage.HideTime = GetWorld()->GetTimeSeconds() + HeaderMessageDuration;

				if (HeaderMessages.Num() >= MaxHeaderMessages)
				{
					HeaderMessages.RemoveAt(0, 1, true);
				}
				HeaderMessages.Add(NewHeaderMessage);
				bHUDMessagesChanged = true;
			}
		}
	}
}

void ASolHUD::NotifyPlayerHit(float InDamage)
{
	NotifyHitDamage += InDamage;
	LastNotifyHitDamage = NotifyHitDamage;
}

void ASolHUD::ToggleInventory()
{
	ShowInventory(!bShowInventory);
}

void ASolHUD::ShowInventory(bool bEnable)
{
	// Only show if we have a pawn.
	ASolCharacter* SolPawn = Cast<ASolCharacter>(GetOwningPawn());
	if (SolPawn && bEnable && InventoryWidgetClass)
	{
		bShowInventory = true;
		// Create the Inventory widget if it is not already created.
		if (!InventoryWidget)
		{
			if ((InventoryWidget = CreateWidget<UUserWidget, APlayerController>(PlayerOwner, InventoryWidgetClass)) != nullptr)
			{
				InventoryWidget->AddToViewport();
				InventoryWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
	// Delete widget if we're not showing the inventory.
	else
	{
		bShowInventory = false;
		if (InventoryWidget)
		{
			InventoryWidget->RemoveFromViewport();
			InventoryWidget = nullptr;
		}
	}

	// Handle showing mouse and allowing input.
	if (ASolPlayerController* MyPC = Cast<ASolPlayerController>(PlayerOwner))
	{
		MyPC->SetShowMouseCursor(bShowInventory);
		MyPC->bEnableMouseOverEvents = bShowInventory;
		MyPC->bEnableClickEvents = bShowInventory;
		MyPC->SetIgnoreLookInput(bShowInventory);
		//MyPC->SetIgnoreMoveInput(bShowInventory);
	}
}

void ASolHUD::ToggleInGameMenu()
{
	ShowInGameMenu(!bShowInGameMenu);
}

void ASolHUD::ShowInGameMenu(bool bEnable)
{
	bShowInGameMenu = bEnable;
	if (InGameMenuWidget)
	{
		InGameMenuWidget->SetVisibility(bShowInGameMenu ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	else if (SlateInGameMenuWidget.IsValid())
	{
		SlateInGameMenuWidget->SetVisibility(bShowInGameMenu ? EVisibility::Visible : EVisibility::Hidden);
	}
	if (ASolPlayerController* MyPC = Cast<ASolPlayerController>(PlayerOwner))
	{
		MyPC->SetShowMouseCursor(bShowInGameMenu);
		if (bShowInGameMenu)
		{
			// TODO: Centre mouse over menu once it's pulled up.
			//MyPC->SetMouseLocation(Canvas->ClipX / 2, Canvas->ClipY / 2);
		}
		MyPC->bEnableMouseOverEvents = bShowInGameMenu;
		MyPC->bEnableClickEvents = bShowInGameMenu;
		MyPC->SetIgnoreLookInput(bShowInGameMenu);
		MyPC->SetIgnoreMoveInput(bShowInGameMenu);
		// This currently prevents player from closing menu (since the menu key is ignored).
		/*if (bShowInGameMenu)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(SlateInGameMenuWidget);
			MyPC->SetInputMode(InputMode);
		}
		else
		{
			MyPC->SetInputMode(FInputModeGameOnly());
		}*/
	}
}

void ASolHUD::ToggleScoreboard()
{
	ShowScoreboard(!bShowScoreboard);
}

void ASolHUD::ShowScoreboard(bool bEnable)
{
	bShowScoreboard = bEnable;
}

FName ASolHUD::GetMatchState()
{
	ASolGameState* MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		return MyGameState->GetMatchState();
	}
	else
	{
		return "NO GAME STATE";
	}
}

FCanvasTextItem ASolHUD::GetDefaultTextItem()
{
	FCanvasTextItem TextItem(FVector2D::ZeroVector, FText::GetEmpty(), SmallFont, FColor::White);
	//TextItem.EnableShadow(FColor::Black);
	TextItem.bOutlined = true;
	TextItem.OutlineColor = FLinearColor::Black;
	//TextItem.Scale = HUDDrawScale;
	return TextItem;
}

/////////////////////////////////
// Scoreboard Draw Details

FString ASolHUD::GetTimeString(float TimeSeconds, bool bUseHours)
{
	/** Actually, no hours for now. This just allows the minutes field to exceed two digits. */
	if (bUseHours)
	{
		const int32 TotalSeconds = FMath::Max(0, FMath::TruncToInt(TimeSeconds));
		const int32 NumMinutes = TotalSeconds / 60;
		const int32 NumSeconds = TotalSeconds % 60;

		FString TimeDesc = FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
		if (NumMinutes >= 100)
		{
			TimeDesc = FString::Printf(TEXT("%d:%02d"), NumMinutes, NumSeconds);
		}
		return TimeDesc;
	}
	else
	{
		const int32 TotalSeconds = FMath::Max(0, FMath::TruncToInt(TimeSeconds) % 3600);
		const int32 NumMinutes = TotalSeconds / 60;
		const int32 NumSeconds = TotalSeconds % 60;

		const FString TimeDesc = FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
		return TimeDesc;
	}
}

FLinearColor ASolHUD::GetTeamColor(class ATeamState* InTeam)
{
	/** Return black if no team. **/
	FLinearColor DrawColor = FLinearColor(0.0f, 0.0f, 0.0f);

	ASolGameState* const MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	if (InTeam)
	{
		DrawColor = InTeam->GetTeamColor();
	}

	return DrawColor;
}

FLinearColor ASolHUD::GetHUDDrawColor()
{
	/**Return black if no team. **/
	FLinearColor DrawColor = FLinearColor(0.0f, 0.0f, 0.0f);

	ASolPlayerController* MyPC = Cast<ASolPlayerController>(PlayerOwner);
	if (MyPC)
	{
		ASolPlayerState* MyPS = Cast<ASolPlayerState>(MyPC->PlayerState);
		if (MyPS)
		{
			ATeamState* Team = MyPS->GetTeam();
			if (Team)
			{
				DrawColor = Team->GetTeamColor();
			}
		}
	}
	return DrawColor;
}

FLinearColor ASolHUD::GetPlayerNameDrawColor(ATeamState* InTeam, bool bIsSelf)
{
	FLinearColor DrawColor = FLinearColor(0.5f, 0.5f, 0.5f);
	if (bIsSelf)
	{
		DrawColor = FLinearColor::Yellow;
	}
	else if (InTeam)
	{
		DrawColor = (InTeam->GetTeamColor() * 0.5f) + (FLinearColor::White * 0.5f);
	}

	return DrawColor;
}

FText ASolHUD::GetHeaderText()
{
	FText OutText;
	
	ASolGameState* const MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		ASolGameMode* MyGameMode = MyGameState->GameModeClass->GetDefaultObject<ASolGameMode>();
		if (MyGameMode)
		{
			OutText = MyGameMode->DisplayName;
		}
	}
	return OutText;
}

FText ASolHUD::GetVictoryConditionsText()
{
	FText OutText;

	ASolGameState* const MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		if (MyGameState->ScoreLimit > 0)
		{
			OutText = FText::FromString(FString::Printf(TEXT("Score Limit: %d"), MyGameState->ScoreLimit));
		}
		else if (MyGameState->TimeLimit > 0)
		{
			OutText = FText::FromString(FString::Printf(TEXT("Time Limit: %d"), MyGameState->TimeLimit));
		}
	}

	return OutText;
}

FText ASolHUD::GetTeamScoreText(int32 TeamIndex)
{
	FText OutText;

	ASolGameState* const MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		if (MyGameState->TeamArray.Num() > TeamIndex)
		{
			OutText = FText::FromString(FString::Printf(TEXT("Team Score: %d"), MyGameState->TeamArray[TeamIndex]->GetScore()));
		}
	}

	return OutText;
}

FString ASolHUD::GetHealthString()
{
	FString OutString;
	
	ASolCharacter* MyPawn = Cast<ASolCharacter>(GetOwningPawn());
	if (MyPawn)
	{
		OutString = FString::Printf(TEXT("Health: %d"), FMath::CeilToInt(MyPawn->GetHealth()));
	}
	return OutString;
}

FString ASolHUD::GetTimerString()
{
	FString OutString;

	ASolGameState* const MyGameState = Cast<ASolGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		FString TimerText;
		FString TimerNumbers = FString::Printf(TEXT("%d"), MyGameState->RemainingTime);
		if (GetMatchState() == "WaitingToStart")
		{
			TimerText = FString::Printf(TEXT("Match Start: "));
		}
		else if (GetMatchState() == "InProgress")
		{
			TimerText = FString::Printf(TEXT("Remaining Time: "));
			TimerNumbers = GetTimeString(MyGameState->RemainingTime, true);
			// If no time limit, show elapsed time.
			if (MyGameState->RemainingTime <= 0)
			{
				TimerText = FString::Printf(TEXT("Elapsed Time: "));
				TimerNumbers = GetTimeString(MyGameState->ElapsedTime, true);
			}
		}
		else if (GetMatchState() == "WaitingPostMatch")
		{
			TimerText = FString::Printf(TEXT("Next Match: "));
		}
		OutString = TimerText + TimerNumbers;
		if (GetMatchState() == "LeavingMap")
		{
			OutString = FString::Printf(TEXT("Leaving Map..."));
		}
	}
	return OutString;
}

int32 ASolHUD::GetDeathMessageCount()
{
	return DeathMessages.Num();
}

FComplexString ASolHUD::GetDeathMessage(int32 MessageNum)
{
	return DeathMessages[MessageNum];
}
