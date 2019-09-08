// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Lastim.h"
#include "LastimCharacter.h"
#include "LastimGameState.h"
#include "LastimGameMode.h"
#include "LastimPlayerController.h"
#include "UserWidget.h"
#include "SScoreboardWidget.h"
#include "SInGameMenuWidget.h"
#include "LastimPlayerState.h"
#include "TeamState.h"
#include "LastimFirearm.h"
#include "LastimAmmo.h"
#include "LastimHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

ALastimHUD::ALastimHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> LowHealthOverlayTextureObj(TEXT("/Game/UI/HUD/LowHealthOverlay"));
	LowHealthOverlayTexture = LowHealthOverlayTextureObj.Object;
	//static ConstructorHelpers::FObjectFinder<UFont> DFontObj(TEXT("/Game/UI/HUD/RobotoDistanceField"));
	//DFFont = SmallFontObj.Object; //GEngine->GetSmallFont()
	static ConstructorHelpers::FObjectFinder<UFont> LargeFontObj(TEXT("/Game/UI/HUD/Roboto51"));
	LargeFont = LargeFontObj.Object;
	static ConstructorHelpers::FObjectFinder<UFont> MediumFontObj(TEXT("/Game/UI/HUD/Roboto18"));
	MediumFont = MediumFontObj.Object;
	static ConstructorHelpers::FObjectFinder<UFont> SmallFontObj(TEXT("/Game/UI/HUD/Roboto09"));
	SmallFont = SmallFontObj.Object; //GEngine->GetSmallFont()
	static ConstructorHelpers::FClassFinder<UUserWidget> FoundHUDWidgetClass(TEXT("/Game/UI/HUD/TestHUDWidget"));
	UMGHUDWidgetClass = FoundHUDWidgetClass.Class;
	//ScoreboardWidgetClass = class<SScoreboardWidget>;
	//InGameMenuWidgetClass = SInGameMenuWidget::StaticClass();

	HUDDrawScale = 1.f;

	MaxDeathMessages = 10;
	DeathMessageDuration = 10.0f;
	MaxHeaderMessages = 3;
	HeaderMessageDuration = 5.0f;
	bShowInventory = false;
	bShowScoreboard = false;
	bHUDMessagesChanged = false;
}

void ALastimHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	/* Disabled for now. */
	if (0 > 1 && UMGHUDWidgetClass && PlayerOwner)
	{
		UMGHUDWidget = CreateWidget<UUserWidget>(PlayerOwner, UMGHUDWidgetClass);
		if (UMGHUDWidget)
		{
			UMGHUDWidget->AddToViewport();
		}
	}
	const ALastimPlayerController* PCOwner = Cast<ALastimPlayerController>(PlayerOwner);

	if (PCOwner)
	{
		if (!ScoreboardWidget.IsValid())
		{
			//SAssignNew(ScoreboardWidget, ScoreboardWidgetClass)
			SAssignNew(ScoreboardWidget, SScoreboardWidget)
				.OwnerHUD(this)
				.PCOwner(TWeakObjectPtr<APlayerController>(PlayerOwner));

			if (ScoreboardWidget.IsValid())
			{

				GEngine->GameViewport->AddViewportWidgetContent(
					SNew(SWeakWidget)
					.PossiblyNullContent(ScoreboardWidget.ToSharedRef())
					);
				//MyHUDMenuWidget->ActionButtonsWidget->SetVisibility(EVisibility::Visible);
				//MyHUDMenuWidget->ActionWidgetPosition.BindUObject(this, &AStrategyHUD::GetActionsWidgetPos);
			}
		}

		if (!InGameMenuWidget.IsValid())
		{
			//SAssignNew(InGameMenuWidget, InGameMenuWidgetClass)

			ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);

			SAssignNew(InGameMenuWidget, SInGameMenuWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(MyPlayerOwner)
			.OwnerHUD(this);

			if (InGameMenuWidget.IsValid())
			{

				GEngine->GameViewport->AddViewportWidgetContent(
					SNew(SWeakWidget)
					.PossiblyNullContent(InGameMenuWidget.ToSharedRef())
					);

				//MyHUDMenuWidget->ActionButtonsWidget->SetVisibility(EVisibility::Visible);
				//MyHUDMenuWidget->ActionWidgetPosition.BindUObject(this, &AStrategyHUD::GetActionsWidgetPos);
			}
		}

		// Setup the widget to forward focus to when the viewport receives focus.
		TSharedPtr<SViewport> GameViewportWidget = GEngine->GetGameViewportWidget();
		if (GameViewportWidget.IsValid())
		{
			//GameViewportWidget->SetWidgetToFocusOnActivate(ScoreboardWidget);
			//GameViewportWidget->SetWidgetToFocusOnActivate(InGameMenuWidget);
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

void ALastimHUD::DrawHUD()
{
	Super::DrawHUD();
	HUDDrawScale = FMath::Max(Canvas->ClipY / 1024.f, 0.5f);

	ALastimCharacter* MyPawn = Cast<ALastimCharacter>(GetOwningPawn());
	ALastimGameState* MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
	// Draw the Scoreboard if toggled, the game is over, or if the game hasn't begun yet.
	if (bShowScoreboard || GetMatchState() == "WaitingPostMatch" || GetMatchState() == "WaitingToStart")
	{
		if (UMGHUDWidget && UMGHUDWidget->GetVisibility() != ESlateVisibility::Hidden)
		{
			UMGHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		if (ScoreboardWidget.IsValid())
		{
			ScoreboardWidget->SetVisibility(EVisibility::Visible);
		}
	}
	else
	{
		if (UMGHUDWidget && UMGHUDWidget->GetVisibility() != ESlateVisibility::Visible)
		{
			UMGHUDWidget->SetVisibility(ESlateVisibility::Visible);
		}
		if (ScoreboardWidget.IsValid())
		{
			ScoreboardWidget->SetVisibility(EVisibility::Hidden);
		}
	}

	if (bShowInGameMenu)
	{
		if (InGameMenuWidget.IsValid())
		{
			InGameMenuWidget->SetVisibility(EVisibility::Visible);
		}
	}
	else
	{
		if (InGameMenuWidget.IsValid())
		{
			InGameMenuWidget->SetVisibility(EVisibility::Hidden);
		}
	}
	// Some HUD stuff is still in canvas for now.
	UpdateHUDMessages();
	if (MyPawn)
	{
		// Low health overlay.
		if (MyPawn->GetHealth() < MyPawn->GetMaxHealth() ) //if (MyPawn && MyPawn->IsAlive() && MyPawn->Health < MyPawn->GetMaxHealth() * MyPawn->GetLowHealthPercentage())
		{
			//Was 1.0f + 5.0f *
			const float AnimSpeedModifier = 3.14f + 9.42f * FMath::Square((1.0f - MyPawn->GetHealth() / (100 * 1.0))); // (1.0f - MyPawn->Health / (MyPawn->GetMaxHealth() * MyPawn->GetLowHealthPercentage()));
			// Was 32 + 72 *
			int32 EffectValue = 127 * (1.0f - MyPawn->GetHealth() / (MyPawn->GetMaxHealth() * 1.0)); // (1.0f - MyPawn->Health / (MyPawn->GetMaxHealth() * MyPawn->GetLowHealthPercentage()));
			LowHealthPulseValue += GetWorld()->GetDeltaSeconds() * AnimSpeedModifier;
			float EffectAlpha = 0.25f + 0.75f * FMath::Abs(FMath::Sin(LowHealthPulseValue));
			float HitNotifyValue = 127 * (NotifyHitDamage / MyPawn->GetMaxHealth());
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
		// Draw pickup message if necessary.
		APickup* UsablePickup = Cast<APickup>(MyPawn->GetUsableObject());
		if (UsablePickup)
		{
			FString ItemName = "No Held Item!";
			if (UsablePickup->GetHeldItem())
			{
				ItemName = UsablePickup->GetHeldItem()->GetDisplayName();
			}
			FString TextString = FString::Printf(TEXT("[USE] Pick up %s"), *ItemName);
			FCanvasTextItem TextItem = GetDefaultTextItem();
			TextItem.Text = FText::FromString(TextString);
			TextItem.Scale = FVector2D(HUDDrawScale, HUDDrawScale);
			float SizeX, SizeY;
			Canvas->StrLen(TextItem.Font, TextItem.Text.ToString(), SizeX, SizeY);
			SizeX *= TextItem.Scale.X;
			SizeY *= TextItem.Scale.Y;
			FVector2D DrawPosition = FVector2D((Canvas->ClipX * 0.5) - (SizeX * 0.5), (Canvas->ClipY * 0.625) - (SizeY * 0.5));
			TextItem.SetColor(FColor::Green);
			TextItem.Position = DrawPosition;
			Canvas->DrawItem(TextItem);
		}
	}
	// This should be transferred to the HUD.
	//  In addition, the text should change if the player cannot respawn.
	else if (!bShowScoreboard && MyGameState && GetMatchState() == "InProgress")
	{
		FCanvasTextItem TextItem = GetDefaultTextItem();
		FText DeathText = NSLOCTEXT("HUD", "DeathText", "Press [FIRE] to respawn.");
		ALastimPlayerState* MyPlayerState = PlayerOwner ? Cast<ALastimPlayerState>(PlayerOwner->PlayerState) : NULL;
		if (MyPlayerState && MyPlayerState->RespawnTime > 0.0f)
		{
			DeathText = FText::FromString(FString::Printf(TEXT("Can respawn in: %d"), FMath::CeilToInt(MyPlayerState->RespawnTime)));
		}
		TextItem.Text = DeathText;
		TextItem.Font = LargeFont;
		TextItem.Scale = FVector2D(HUDDrawScale, HUDDrawScale);
		float SizeX, SizeY;
		Canvas->StrLen(LargeFont, TextItem.Text.ToString(), SizeX, SizeY);
		SizeX *= TextItem.Scale.X;
		SizeY *= TextItem.Scale.Y;
		FVector2D DrawPosition = FVector2D((Canvas->ClipX * 0.5) - (SizeX * 0.5), (Canvas->ClipY * 0.75) - (SizeY * 0.5));
		TextItem.SetColor(FColor::White);
		TextItem.Position = DrawPosition;
		Canvas->DrawItem(TextItem);
	}
	if (!bShowScoreboard)
	{
		DrawDeathMessages();
		DrawHeaderMessages();
		if (MyGameState)
		{
			DrawObjectiveInfo(MyGameState);
		}
		if (MyPawn)
		{
			DrawPlayerInfo(MyPawn);
			ALastimWeapon* MyWeapon = MyPawn->GetEquippedWeapon();
			if (MyWeapon)
			{
				DrawWeaponInfo(MyPawn, MyWeapon);
			}
			if (bShowWeaponList)
			{
				DrawWeaponList(MyPawn);
			}
			if (bShowInventory)
			{
				DrawInventory(MyPawn);
			}
		}
	}
}

void ALastimHUD::DrawPlayerInfo(ALastimCharacter* InPlayer)
{
	/* Just health for now. */
	if (InPlayer)
	{
		FVector2D Size(0.0f, 0.0f);
		FVector2D DrawPosition(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.9f);
		FCanvasTextItem TextItem = GetDefaultTextItem();
		TextItem.Font = MediumFont;
		FString TextString = FString::Printf(TEXT("Health: %d | Shield: %d | Energy: %d"), FMath::CeilToInt(InPlayer->GetHealth()), FMath::CeilToInt(InPlayer->GetShield()), FMath::CeilToInt(InPlayer->GetEnergy()));
		TextItem.Text = FText::FromString(TextString);
		GetTextSize(TextString, Size.X, Size.Y, MediumFont);
		DrawPosition = FVector2D((Canvas->ClipX - Size.X) * 0.5f, Canvas->ClipY - Size.Y);
		Canvas->DrawItem(TextItem, DrawPosition);
	}
}

void ALastimHUD::DrawWeaponInfo(ALastimCharacter* InPlayer, ALastimWeapon* InWeapon)
{
	if (InWeapon)
	{
		FVector2D Size(0.0f, 0.0f);
		FVector2D DrawPosition(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.4f);
		FCanvasTextItem TextItem = GetDefaultTextItem();
		TextItem.Font = MediumFont;
		ALastimFirearm* InFirearm = Cast<ALastimFirearm>(InWeapon);
		if (InFirearm)
		{
			/* Draw ammo string. */
			FString TextString = FString::Printf(TEXT("Energy: %d - Shots: %d/%d - Percent: %d%%"), InFirearm->GetAmmo(), 
				InFirearm->GetAmmoForFireMode(InFirearm->GetCurrentFireMode()), InFirearm->GetMaxAmmoForFireMode(InFirearm->GetCurrentFireMode()),
				FMath::CeilToInt(100 * InFirearm->GetAmmoPct()));
			TextItem.Text = FText::FromString(TextString);
			GetTextSize(TextString, Size.X, Size.Y, MediumFont);
			DrawPosition = FVector2D(Canvas->ClipX - Size.X, Canvas->ClipY - Size.Y);
			Canvas->DrawItem(TextItem, DrawPosition);
			/* Draw "clips" string (reserve ammo or magazines). */
			/*if (InFirearm->GetCurrentClips() > 0)
			{
				TextString = FString::Printf(TEXT("Clips: %d"), InFirearm->GetCurrentClips());
				TextItem.Text = FText::FromString(TextString);
				int32 OldDrawHeight = DrawPosition.Y;
				GetTextSize(TextString, Size.X, Size.Y, MediumFont);
				DrawPosition = FVector2D(Canvas->ClipX - Size.X, OldDrawHeight - Size.Y);
				Canvas->DrawItem(TextItem, DrawPosition);
			}*/
		}
	}
}

void ALastimHUD::DrawWeaponList(ALastimCharacter* InPlayer)
{
	if (InPlayer)
	{
		FVector2D Size(0.0f, 0.0f);
		FVector2D DrawPosition(Canvas->ClipX * 0.5f, Canvas->ClipX * 0.4f);
		FCanvasTextItem TextItem = GetDefaultTextItem();
		TextItem.Font = MediumFont;
		TextItem.bCentreX = true;
		ALastimWeapon* EquippedWeapon = InPlayer->GetEquippedWeapon();
		ALastimWeapon* TestWeapon = NULL; // MyPawn->GetSpecificWeapon(i);
		int32 EquippedWeaponIndex = -1;

		/* Roundabout way to get EquippedWeaponIndex. */
		for (int32 i = 0; i < InPlayer->GetInventoryCount(); i++)
		{
			TestWeapon = InPlayer->GetSpecificWeapon(i);
			if (TestWeapon == EquippedWeapon)
			{
				EquippedWeaponIndex = i;
				break;
			}
		}

		/* Draw the weapon list. */
		for (int32 i = 0; i < InPlayer->GetInventoryCount(); i++)
		{
			TestWeapon = InPlayer->GetSpecificWeapon(i);
			if (TestWeapon)
			{
				TextItem.Text = FText::FromString(InPlayer->GetSpecificWeapon(i)->GetDisplayName());
				TextItem.SetColor(FColor(127, 127, 127));
				// Highlight equipped weapon and selected weapon.
				if (FMath::Abs(EquippedWeaponIndex + DeltaWeapSelectIndex) % InPlayer->GetInventoryCount() == i)
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

void ALastimHUD::DrawInventory(ALastimCharacter* InPlayer)
{
	if (InPlayer)
	{
		//Currently draws inventory in the upper-left corner of the screen.
		FVector2D Size(0.0f, 0.0f);
		FVector2D DrawPosition(Canvas->ClipX * 0.15f, Canvas->ClipY * 0.15f);
		FCanvasTextItem TextItem = GetDefaultTextItem();
		TextItem.Font = MediumFont;
		int32 EquippedWeaponIndex = -1;


		TArray<ALastimInventory*> InvList = InPlayer->ItemInventory;
		TArray<FText> TextItemsToDraw, WeaponsToDraw, AmmoToDraw, OthersToDraw;
		// Get a weapon list.
		WeaponsToDraw.Add(FText::FromString("Weapons:"));
		AmmoToDraw.Add(FText::FromString(""));
		AmmoToDraw.Add(FText::FromString("Ammo:"));
		OthersToDraw.Add(FText::FromString(""));
		OthersToDraw.Add(FText::FromString("Other:"));

		for (int32 i = 0; i < InvList.Num(); i++)
		{
			if (InvList[i] != nullptr)
			{
				ALastimWeapon* WeaponItem = Cast<ALastimWeapon>(InvList[i]);
				ALastimAmmo* AmmoItem = Cast<ALastimAmmo>(InvList[i]);
				if (WeaponItem)
				{
					WeaponsToDraw.Add(FText::FromString(WeaponItem->GetDisplayName()));
				}
				else if (AmmoItem)
				{
					AmmoToDraw.Add(FText::FromString(AmmoItem->GetDisplayName()));
				}
				else
				{
					OthersToDraw.Add(FText::FromString(InvList[i]->GetDisplayName()));
				}
			}
			else
			{
				TextItemsToDraw.Add(FText::FromString("NULLPTR"));
			}
		}

		TextItemsToDraw.Append(WeaponsToDraw);
		TextItemsToDraw.Append(AmmoToDraw);
		TextItemsToDraw.Append(OthersToDraw);

		for (int32 i = 0; i < TextItemsToDraw.Num(); i++)
		{
			TextItem.Text = TextItemsToDraw[i];
			TextItem.SetColor(FColor(127, 127, 127));
			Canvas->DrawItem(TextItem, DrawPosition);
			DrawPosition.Y += 20.0f;
		}
	}
}

void ALastimHUD::DrawDeathMessages()
{
	FVector2D Size(0.0f, 0.0f);
	FVector2D BoxSize(0.0f, 0.0f);
	FVector2D DrawPosition(0.f, 0.f);
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
		DrawPosition.X = 0;
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

void ALastimHUD::DrawHeaderMessages()
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

void ALastimHUD::DrawObjectiveInfo(ALastimGameState* InGameState)
{
	// Nothing here by default.
}

void ALastimHUD::UpdateHUDMessages()
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

void ALastimHUD::SetShowWeaponList(bool bNewValue)
{
	bShowWeaponList = bNewValue;
}

void ALastimHUD::SetWeapSelectIndex(int32 NewIndex)
{
	DeltaWeapSelectIndex = NewIndex;
}

void ALastimHUD::AddDeathMessage(class ALastimPlayerState* KillerPlayerState, class ALastimPlayerState* VictimPlayerState, const UDamageType* KillerDamageType)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Purple, FString::Printf(TEXT("AddDeathMessage: 1")));
	if (GetWorld()->GetGameState() && GetWorld()->GetGameState()->GameModeClass)
	{
		const ALastimGameMode* DefGame = GetWorld()->GetGameState()->GameModeClass->GetDefaultObject<ALastimGameMode>();
		ALastimPlayerState* MyPlayerState = PlayerOwner ? Cast<ALastimPlayerState>(PlayerOwner->PlayerState) : NULL;

		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Purple, FString::Printf(TEXT("AddDeathMessage: 2")));
		if (DefGame && KillerPlayerState && VictimPlayerState && MyPlayerState)
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
		}
	}
}

void ALastimHUD::NotifyPlayerHit(float InDamage)
{
	NotifyHitDamage += InDamage;
	LastNotifyHitDamage = NotifyHitDamage;
}

void ALastimHUD::ToggleInventory()
{
	ShowInventory(!bShowInventory);
}

void ALastimHUD::ShowInventory(bool bEnable)
{
	bShowInventory = bEnable;
}

void ALastimHUD::ToggleInGameMenu()
{
	ShowInGameMenu(!bShowInGameMenu);
}

void ALastimHUD::ShowInGameMenu(bool bEnable)
{
	bShowInGameMenu = bEnable;
	// Temporary: show cursor. 
	ALastimPlayerController* MyPC = Cast<ALastimPlayerController>(PlayerOwner);
	if (MyPC)
	{
		MyPC->bShowMouseCursor = bShowInGameMenu;
		MyPC->bEnableMouseOverEvents = bShowInGameMenu;
		MyPC->bEnableClickEvents = bShowInGameMenu;
	}
}

void ALastimHUD::ToggleScoreboard()
{
	ShowScoreboard(!bShowScoreboard);
}

void ALastimHUD::ShowScoreboard(bool bEnable)
{
	bShowScoreboard = bEnable;
}

FName ALastimHUD::GetMatchState()
{
	ALastimGameState* MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		return MyGameState->GetMatchState();
	}
	else
	{
		return "NO GAME STATE";
	}
}

FCanvasTextItem ALastimHUD::GetDefaultTextItem()
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

FString ALastimHUD::GetTimeString(float TimeSeconds, bool bUseHours)
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

FLinearColor ALastimHUD::GetTeamColor(class ATeamState* InTeam)
{
	/** Return black if no team. **/
	FLinearColor DrawColor = FLinearColor(0.0f, 0.0f, 0.0f);

	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
	if (InTeam)
	{
		DrawColor = InTeam->GetTeamColor();
	}

	return DrawColor;
}

FLinearColor ALastimHUD::GetHUDDrawColor()
{
	/**Return black if no team. **/
	FLinearColor DrawColor = FLinearColor(0.0f, 0.0f, 0.0f);

	ALastimPlayerController* MyPC = Cast<ALastimPlayerController>(PlayerOwner);
	if (MyPC)
	{
		ALastimPlayerState* MyPS = Cast<ALastimPlayerState>(MyPC->PlayerState);
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

FLinearColor ALastimHUD::GetPlayerNameDrawColor(ATeamState* InTeam, bool bIsSelf)
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

FText ALastimHUD::GetHeaderText()
{
	FText OutText;
	
	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		ALastimGameMode* MyGameMode = MyGameState->GameModeClass->GetDefaultObject<ALastimGameMode>();
		if (MyGameMode)
		{
			OutText = MyGameMode->DisplayName;
		}
	}
	return OutText;
}

FText ALastimHUD::GetVictoryConditionsText()
{
	FText OutText;

	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
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

FText ALastimHUD::GetTeamScoreText(int32 TeamIndex)
{
	FText OutText;

	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		if (MyGameState->TeamArray.Num() > TeamIndex)
		{
			OutText = FText::FromString(FString::Printf(TEXT("Team Score: %d"), MyGameState->TeamArray[TeamIndex]->GetScore()));
		}
	}

	return OutText;
}

FString ALastimHUD::GetHealthString()
{
	FString OutString;
	
	ALastimCharacter* MyPawn = Cast<ALastimCharacter>(GetOwningPawn());
	if (MyPawn)
	{
		OutString = FString::Printf(TEXT("Health: %d"), FMath::CeilToInt(MyPawn->GetHealth()));
	}
	return OutString;
}

FString ALastimHUD::GetAmmoString()
{
	return FString::Printf(TEXT("GetAmmoString() IS DEPRECIATED"));
}

FString ALastimHUD::GetClipsString()
{
	return FString::Printf(TEXT("GetClipsString() IS DEPRECIATED"));
}

FString ALastimHUD::GetTimerString()
{
	FString OutString;

	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GetWorld()->GetGameState());
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

int32 ALastimHUD::GetDeathMessageCount()
{
	return DeathMessages.Num();
}

FComplexString ALastimHUD::GetDeathMessage(int32 MessageNum)
{
	return DeathMessages[MessageNum];
}