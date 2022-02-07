// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "UserWidget.h"
#include "SolHUD.generated.h"

// String that draws different portions with different colours.
USTRUCT(BlueprintType)
struct FComplexString
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = ComplexString)
	TArray<FString> MessageText;

	UPROPERTY(BlueprintReadOnly, Category = ComplexString)
	TArray<FLinearColor> MessageColor;

	/* Time until this message should disappear. */
	UPROPERTY(BlueprintReadOnly, Category = ComplexString)
	float HideTime;

	/* Init defaults. */
	FComplexString() : HideTime(0.f)
	{
	}

public:

	//UFUNCTION(BlueprintCallable, Category = ComplexString)
	FString ToRegularString()
	{
		FString FinalString;
		for (int32 i = 0; i < MessageText.Num(); i++)
		{
			FinalString += MessageText[i];
		}
		return FinalString;
	}
};

UCLASS()
class ASolHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	// Toggles visibility of the inventory screen.
	void ToggleInventory();

	/** Sets visibility of the in-game menu. **/
	void ShowInventory(bool bEnable);

	/** Toggles visibility of the in-game menu. **/
	void ToggleInGameMenu();

	/** Sets visibility of the in-game menu. **/
	void ShowInGameMenu(bool bEnable);

	/** Toggles visibility of the scoreboard. **/
	void ToggleScoreboard();

	/** Sets visibility of the scoreboard. **/
	void ShowScoreboard(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = HUD)
	FLinearColor GetTeamColor(class ATeamState* InTeam);

	/* Gets the main HUD color. */
	UFUNCTION(BlueprintCallable, Category = HUD)
	FLinearColor GetHUDDrawColor();

	/* Gets the color for player names in messages. */
	UFUNCTION(BlueprintCallable, Category = HUD)
	FLinearColor GetPlayerNameDrawColor(class ATeamState* InTeam, bool bIsSelf);

	/* Set bShowWeaponList. */
	void SetShowWeaponList(bool bNewValue);

	/* Sets the */
	void SetWeapSelectIndex(int32 NewIndex);

	/* Adds a chat message to the screen. */
	void AddChatMessage(const class ASolPlayerState* User, const FString Message, FName Type);

	/* Adds a death message to the list of death messages. */
	void AddDeathMessage(class ASolPlayerState* KillerPlayerState, class ASolPlayerState* VictimPlayerState, const UDamageType* KillerDamageType);

	/** Notifies the HUD that damage was taken. **/
	virtual void NotifyPlayerHit(float InDamage);

	/* Draws the player's information (e.g. health). */
	virtual void DrawPlayerInfo(class ASolCharacter* InPlayer);

	/** Draws the crosshair for the selected weapon. */
	virtual void DrawCrosshair(ASolCharacter* InPlayer, class AWeapon* InWeapon);

	/* Draws information for the player's current weapon. */
	virtual void DrawWeaponInfo(class ASolCharacter* InPlayer, class AWeapon* InWeapon);

	/* Draws list of the player's weapons. */
	virtual void DrawWeaponList(class ASolCharacter* InPlayer);

	// Draws the player's inventory screen.
	virtual void DrawInventory(class ASolCharacter* InPlayer);

	/* Draws death messages in Canvas. */
	virtual void DrawDeathMessages(FVector2D &DrawPosition);

	/* Draws header messages in Canvas. */
	virtual void DrawHeaderMessages();

	/* Draws labels for objects in the world (e.g. player names, objective locations, etc). Blank in the default class. */
	virtual void DrawObjectLabels();

	/* Draws game-specific data such as objectives or player scores. Blank in the default class. */
	virtual void DrawGameData(FVector2D &DrawPosition, class ASolGameState* InGameState);

	/* Gets a FCanvasTextItem with our preferred settings. */
	virtual FCanvasTextItem GetDefaultTextItem();

	////////////////////////////////
	// BEGIN UMG PROPERTIES

	UFUNCTION(BlueprintCallable, Category = HUD)
	FText GetHeaderText();

	UFUNCTION(BlueprintCallable, Category = HUD)
	FText GetVictoryConditionsText();

	UFUNCTION(BlueprintCallable, Category = HUD)
	FText GetTeamScoreText(int32 TeamIndex);

	UFUNCTION(BlueprintCallable, Category = HUD)
	FString GetHealthString();

	UFUNCTION(BlueprintCallable, Category = HUD)
	FString GetTimerString();

	UFUNCTION(BlueprintCallable, Category = HUD)
	int32 GetDeathMessageCount();

	//UFUNCTION(BlueprintCallable, Category = HUD)
	FComplexString GetDeathMessage(int32 MessageNum);

	////////////////////////////////
	// BEGIN SLATE STUFF

	/* Class for the In-Game Menu Widget. Subclassable for gametype-specific options (e.g. changing teams). */
	//UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class SInGameMenuWidget> InGameMenuWidgetClass;

	/* Pointer to the HUD's In Game Menu Widget. */
	TSharedPtr<class SInGameMenuWidget> InGameMenuWidget;

	/* Class for the Scoreboard Widget. The default scoreboard can handle many (but not all) gametypes. */
	//UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class SInGameMenuWidget> ScoreboardWidgetClass;

	/* Pointer to the HUD's Scoreboard Widget. */
	TSharedPtr<class SScoreboardWidget> ScoreboardWidget;

	/* Draw columns for lives/deaths only, e.g. for Last One Standing/Last Team Standing.
		Not a great implementation, but it works for now. */
	bool bDrawLivesInScoreboard;

	////////////////////////////////
	// END SLATE STUFF

	/* Array of death messages. */
	UPROPERTY(BlueprintReadOnly, Category = GameState)
	TArray<FComplexString> DeathMessages;

	/* Maximum amount of death messages visible. */
	UPROPERTY(BlueprintReadOnly, Category = HUD)
	int32 MaxDeathMessages;

	/* Time death messages last. */
	UPROPERTY(BlueprintReadOnly, Category = HUD)
	float DeathMessageDuration;

	UPROPERTY(BlueprintReadOnly, Category = GameState)
	TArray<FComplexString> HeaderMessages;

	/* Maximum amount of death messages visible. */
	UPROPERTY(BlueprintReadOnly, Category = HUD)
	int32 MaxHeaderMessages;

	/* Time death messages last. */
	UPROPERTY(BlueprintReadOnly, Category = HUD)
	float HeaderMessageDuration;

protected:

	virtual void PostInitializeComponents() override;

	// Should the inventory be shown?
	bool bShowInventory;

	// Should the scoreboard be shown?
	bool bShowScoreboard;

	// Should the in-game menu be shown?
	bool bShowInGameMenu;

	/* Should we show the weapon list? */
	bool bShowWeaponList;

	/* Scales HUD elements. */
	float HUDDrawScale;
	
	////////////////////////////////
	// BEGIN UMG STUFF

	/** Test UMG HUD Class. **/
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf< class UUserWidget> UMGHUDWidgetClass;

	/** Test UMG HUD Class. **/
	UPROPERTY(BlueprintReadWrite)
	UUserWidget* UMGHUDWidget;

	////////////////////////////////
	// END UMG STUFF

	/** Returns input seconds converted into MM:SS or H:MM:SS formats **/
	FString GetTimeString(float TimeSeconds, bool bUseHours);

	/** Fonts used by this HUD. **/
	UPROPERTY()
	UFont* LargeFont;
	UPROPERTY()
	UFont* MediumFont;
	UPROPERTY()
	UFont* SmallFont;

	/** Texture used when player is injured. **/
	UPROPERTY()
	UTexture2D* LowHealthOverlayTexture;

	/** Texture used for player's health icon. **/
	UPROPERTY()
	UTexture2D* HealthIconTexture;

	UPROPERTY()
	FCanvasIcon Crosshair;

	/** The current point of the low health overlay's "animation." **/
	float LowHealthPulseValue;

	/** Damage recently taken. Reduced every frame. **/
	float NotifyHitDamage;

	/** The amount of NotifyHitDamage after last hit. **/
	float LastNotifyHitDamage;

	/* The weapon we have selected in relation to the currently equipped one. */
	int32 DeltaWeapSelectIndex;

	// Internal fetch of the match state.
	virtual FName GetMatchState();

	/* Removes any old death messages. */
	void UpdateHUDMessages();

	/* Have the death messages changed? */
	UPROPERTY(BlueprintReadWrite, Category = HUD)
	bool bHUDMessagesChanged;

};

