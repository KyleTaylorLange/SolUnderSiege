// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "MainMenuHUD.h"
#include "SolLocalPlayer.h"
#include "SolGameInstance.h"
#include "SCreateGameScreenWidget.h"
#include "SlateOptMacros.h"

#include "UserWidget.h"

void SCreateGameScreenWidget::WindowSetup()
{
	CreateGameOptions();
}

void SCreateGameScreenWidget::CreateGameOptions()
{
	MapListArray.Add(MakeShareable(new FString(TEXT("Athena"))));
	MapListArray.Add(MakeShareable(new FString(TEXT("DualArena"))));
	MapListArray.Add(MakeShareable(new FString(TEXT("ParkHaus"))));
	MapListArray.Add(MakeShareable(new FString(TEXT("Urbania"))));

	// List of game modes; first line is friendly name, second line is the actual class name.
	// These have to be in pairs.
	GameModeListArray.Add(MakeShareable(new FString("Anarchy")));
	GameModeClassNameArray.Add(MakeShareable(new FString("Lastim.GameMode_Anarchy")));
	GameModeListArray.Add(MakeShareable(new FString("Team Anarchy")));
	GameModeClassNameArray.Add(MakeShareable(new FString("Lastim.GameMode_TeamAnarchy")));
	GameModeListArray.Add(MakeShareable(new FString("Last One Standing")));
	GameModeClassNameArray.Add(MakeShareable(new FString("Lastim.GameMode_LastOneStanding")));
	GameModeListArray.Add(MakeShareable(new FString("Last Team Standing")));
	GameModeClassNameArray.Add(MakeShareable(new FString("Lastim.GameMode_LastTeamStanding")));
	GameModeListArray.Add(MakeShareable(new FString("Elimination")));
	GameModeClassNameArray.Add(MakeShareable(new FString("Lastim.GameMode_Elimination")));
	GameModeListArray.Add(MakeShareable(new FString("Domination")));
	GameModeClassNameArray.Add(MakeShareable(new FString("Lastim.GameMode_Domination")));
	GameModes.Add(MakeShareable(new FGameMode("Anarchy", "Lastim.GameMode_Anarchy")));
	GameModes.Add(MakeShareable(new FGameMode("Team Anarchy", "Lastim.GameMode_TeamAnarchy")));
	GameModes.Add(MakeShareable(new FGameMode("Last One Standing", "Lastim.GameMode_LastOneStanding")));
	GameModes.Add(MakeShareable(new FGameMode("Last Team Standing", "Lastim.GameMode_LastTeamStanding")));
	GameModes.Add(MakeShareable(new FGameMode("Domination", "Lastim.GameMode_Domination")));
	
	GameOptions.Empty();
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "ScoreLimit", "Score Limit"), FString("ScoreLimit"), FText::FromString("50")));
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "TimeLimit", "Time Limit (seconds)"), FString("TimeLimit"), FText::FromString("900")));
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "BotCount", "Bots"), FString("MaxBots"), FText::FromString("6")));
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "TeamCount", "Teams"), FString("MaxTeams"), FText::FromString("2")));
}

TSharedRef<SWidget> SCreateGameScreenWidget::MakeGameOptionsList()
{
	TSharedPtr<SVerticalBox> GameOptionsList = SNew(SVerticalBox);

	return GameOptionsList.ToSharedRef();
}

TSharedRef<SWidget> SCreateGameScreenWidget::ConstructWindow()
{
	TSharedPtr<SVerticalBox> GameOptionsList = SNew(SVerticalBox);

	GameOptionsList->AddSlot().AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(250.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("Lastim.HUD.Menu", "GameMode", "Game Mode"))
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(250.f)
			[
				SAssignNew(GameModeListBox, SComboBox< TSharedPtr<FGameMode> >)
				//SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&GameModes)
				.InitiallySelectedItem(GameModes[0])
				.OnGenerateWidget(this, &SCreateGameScreenWidget::GenerateGameModesList)
				.OnSelectionChanged(this, &SCreateGameScreenWidget::OnGameModeSelected)
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(10.0f, 0.0f, 10.0f, 0.0f)
					[
						SAssignNew(SelectedGameMode, STextBlock)
						//SNew(STextBlock)
						.Text(FText::FromString(*GameModes[0].Get()->DisplayName))
					]
				]
			]
		]
	];

	GameOptionsList->AddSlot().AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(250.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("Lastim.HUD.Menu", "Map", "Map"))
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(250.f)
			[
				SAssignNew(MapListBox, SComboBox< TSharedPtr<FString> >)
				//SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&MapListArray)
				.InitiallySelectedItem(MapListArray[0])
				.OnGenerateWidget(this, &SCreateGameScreenWidget::GenerateStringListWidget)
				.OnSelectionChanged(this, &SCreateGameScreenWidget::OnMapSelected)
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(5.0f, 0.0f, 10.0f, 0.0f)
					[
						SAssignNew(SelectedMap, STextBlock)
						//SNew(STextBlock)
						.Text(FText::FromString(*MapListArray[0].Get()))
					]
				]
			]
		]
	];
	
	for (int32 i = 0; i < GameOptions.Num(); i++)
	{
		GameOptionsList->AddSlot().AutoHeight()
		[
			SNew(SBorder)

			.Padding(1)
			[
				MakeGameOptionWidget(GameOptions[i])
			]
		];
	}

	// Make start game button.
	GameOptionsList->AddSlot().AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &SCreateGameScreenWidget::StartGame)
			//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::White)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(-1, 1))
				//.Font(LargeText)
				.Text(FText::FromString(FString("Start Game")))
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &SCreateGameScreenWidget::StartLANGame)
			//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::White)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(-1, 1))
				//.Font(LargeText)
				.Text(FText::FromString(FString("Start LAN")))
			]
		]
	];

	return GameOptionsList.ToSharedRef();
}

TSharedRef<SWidget> SCreateGameScreenWidget::MakeGameOptionWidget(FGameOption& InOption)
{
	TSharedPtr<SHorizontalBox> GameOptionWidget = SNew(SHorizontalBox);
	
	GameOptionWidget->AddSlot()
	[
		SNew(SBox)
		.WidthOverride(250.f)
		[
			SNew(STextBlock)
			.Text(InOption.OptionName)
		]
	];
	GameOptionWidget->AddSlot()
	[
		SNew(SBox)
		.WidthOverride(150.f)
		[
			SAssignNew(InOption.OptionWidget, SEditableTextBox)
			.Text(InOption.DefaultValue)
		]
	];

	return GameOptionWidget.ToSharedRef();
}

FReply SCreateGameScreenWidget::StartGame()
{
	HostGame(false);

	return FReply::Handled();
}

FReply SCreateGameScreenWidget::StartLANGame()
{
	HostGame(true);

	return FReply::Handled();
}


void SCreateGameScreenWidget::HostGame(bool bIsLAN)
{
	if (MainMenuHUD.IsValid())
	{
		AMainMenuHUD* TestHUD = MainMenuHUD.Get();
		if (TestHUD)
		{
			FString StartStr = FString("/Game/Maps/");
			FString MapName = *MapListBox->GetSelectedItem().Get();
			StartStr += MapName;
			FString GameModeClassName = *GameModeListBox->GetSelectedItem().Get()->ClassName;
			StartStr += FString("?game=") + GameModeClassName;
			// Add LAN option if LAN game.
			if (bIsLAN)
			{
				StartStr += FString("?bIsLanMatch");
			}
			// Add URL details for each option.
			for (int32 i = 0; i < GameOptions.Num(); i++)
			{
				if (GameOptions[i].OptionWidget.IsValid() && !GameOptions[i].OptionWidget->GetText().ToString().IsEmpty())
				{
					FString OptionName = GameOptions[i].URLString;
					FString Value = FString("=") + GameOptions[i].OptionWidget->GetText().ToString();
					StartStr += FString("?") + OptionName + Value;
				}
			}
			StartStr += FString("?listen"); //ESSENTIAL FOR PLAYER TO CONNECT TO SERVER!

			USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
			if (GI)
			{
				GI->HostGame(PlayerOwner.Get(), GameModeClassName, StartStr);
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("No Game Instance!")));
			}
		}
	}
}

TSharedRef<SWidget> SCreateGameScreenWidget::GenerateStringListWidget(TSharedPtr<FString> InItem)
{	
	return SNew(SBox)
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InItem.Get()))
		];
}

TSharedRef<SWidget> SCreateGameScreenWidget::GenerateGameModesList(TSharedPtr<FGameMode> InItem)
{
	return SNew(SBox)
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InItem.Get()->DisplayName))
		];
}

void SCreateGameScreenWidget::OnMapSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedMap->SetText(FText::FromString(*NewSelection.Get()));
}

void SCreateGameScreenWidget::OnGameModeSelected(TSharedPtr<FGameMode> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedGameMode->SetText(FText::FromString(*NewSelection.Get()->DisplayName));
}