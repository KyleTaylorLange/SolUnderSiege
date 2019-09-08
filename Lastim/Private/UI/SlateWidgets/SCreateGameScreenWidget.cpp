// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "LastimMenuHUD.h"
#include "LastimLocalPlayer.h"
#include "LastimGameInstance.h"
#include "SCreateGameScreenWidget.h"
#include "SlateOptMacros.h"

#include "UserWidget.h"

void SCreateGameScreenWidget::WindowSetup()
{
	CreateGameOptions();
}

void SCreateGameScreenWidget::CreateGameOptions()
{
	MapListArray.Add(MakeShareable(new FString(TEXT("Flora"))));
	MapListArray.Add(MakeShareable(new FString(TEXT("Urbania"))));
	MapListArray.Add(MakeShareable(new FString(TEXT("UndergroundOutpost"))));

	// List of game modes; first line is friendly name, second line is the actual class name.
	// These have to be in pairs.
	GameModeListArray.Add(MakeShareable(new FString(TEXT("Anarchy"))));
	GameModeClassNameArray.Add(MakeShareable(new FString(TEXT("Lastim.GameMode_Anarchy"))));
	GameModeListArray.Add(MakeShareable(new FString(TEXT("Team Anarchy"))));
	GameModeClassNameArray.Add(MakeShareable(new FString(TEXT("Lastim.GameMode_TeamAnarchy"))));
	GameModeListArray.Add(MakeShareable(new FString(TEXT("Last One Standing"))));
	GameModeClassNameArray.Add(MakeShareable(new FString(TEXT("Lastim.GameMode_LastOneStanding"))));
	GameModeListArray.Add(MakeShareable(new FString(TEXT("Last Team Standing"))));
	GameModeClassNameArray.Add(MakeShareable(new FString(TEXT("Lastim.GameMode_LastTeamStanding"))));
	GameModeListArray.Add(MakeShareable(new FString(TEXT("Elimination"))));
	GameModeClassNameArray.Add(MakeShareable(new FString(TEXT("Lastim.GameMode_Elimination"))));
	GameModeListArray.Add(MakeShareable(new FString(TEXT("Domination"))));
	GameModeClassNameArray.Add(MakeShareable(new FString(TEXT("Lastim.GameMode_Domination"))));
	
	GameOptions.Empty();
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "ScoreLimit", "Score Limit"), FString("ScoreLimit")));
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "TimeLimit", "Time Limit"), FString("TimeLimit")));
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "BotCount", "Bots"), FString("MaxBots")));
	GameOptions.Add(FGameOption(NSLOCTEXT("Lastim.HUD.Menu", "TeamCount", "Teams"), FString("MaxTeams")));
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
				SAssignNew(GameModeListBox, SComboBox< TSharedPtr<FString> >)
				//SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&GameModeListArray)
				.InitiallySelectedItem(GameModeListArray[0])
				.OnGenerateWidget(this, &SCreateGameScreenWidget::GenerateStringListWidget)
				.OnSelectionChanged(this, &SCreateGameScreenWidget::OnGameModeSelected)
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(10.0f, 0.0f, 10.0f, 0.0f)
					[
						SAssignNew(SelectedGameMode, STextBlock)
						//SNew(STextBlock)
						.Text(FText::FromString(FString(TEXT("Anarchy"))))
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
						.Text(FText::FromString(FString(TEXT("UndergroundOutpost"))))
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
			//SNew(SEditableTextBox)
			.Text(FText::FromString(FString("900")))
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
		ALastimMenuHUD* TestHUD = MainMenuHUD.Get();
		if (TestHUD)
		{
			FString StartStr = FString("/Game/Maps/");
			FString MapName = *MapListBox->GetSelectedItem().Get();
			StartStr += MapName;
			FString GameModeName = *GameModeListBox->GetSelectedItem().Get();
			FString GameModeClassName = FString("");
			for (int32 i = 0; i < GameModeClassNameArray.Num() && i < GameModeListArray.Num(); i++)
			{
				FString TestedName = *GameModeListArray[i];
				if (GameModeName == TestedName)
				{
					GameModeClassName = *GameModeClassNameArray[i];
				}
			}
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

			ULastimGameInstance* const GI = Cast<ULastimGameInstance>(PlayerOwner->GetGameInstance());
			if (GI)
			{
				GI->HostGame(PlayerOwner.Get(), GameModeClassName, StartStr);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("No Game Instance!")));
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
		//.TextStyle(SUWindowsStyle::Get(), "UT.ContextMenu.TextStyle")
		];
}

void SCreateGameScreenWidget::OnMapSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedMap->SetText(*NewSelection.Get());
}

void SCreateGameScreenWidget::OnGameModeSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedGameMode->SetText(*NewSelection.Get());
}