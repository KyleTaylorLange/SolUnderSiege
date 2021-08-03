// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "MainMenuHUD.h"
#include "SolLocalPlayer.h"
#include "SolGameInstance.h"
#include "SolGameState.h"
#include "SolGameMode.h"
#include "SCreateGameScreenWidget.h"
#include "SlateOptMacros.h"

#include "UserWidget.h"

void SCreateGameScreenWidget::WindowSetup()
{
	CreateGameOptions();
}

void SCreateGameScreenWidget::CreateGameOptions()
{
	ASolGameState* GameState = Cast<ASolGameState>(PlayerOwner->GetWorld()->GetGameState());
	if (GameState)
	{
		GameState->GetGameModes(GameModesList);
		for (int32 i = 0; i < GameState->TempMapNames.Num(); i++)
		{
			MapListArray.Add(MakeShareable(new FString(GameState->TempMapNames[i])));
		}
	}
}

TSharedRef<SWidget> SCreateGameScreenWidget::MakeGameOptionsList()
{
	TSharedPtr<SVerticalBox> GameOptionsList = SNew(SVerticalBox);

	return GameOptionsList.ToSharedRef();
}

TSharedRef<SWidget> SCreateGameScreenWidget::ConstructWindow()
{
	TSharedPtr<SVerticalBox> GameOptionsWindow = SNew(SVerticalBox);
	TSubclassOf<ASolGameMode> InitiallySelectedGameMode = GameModesList[0];

	GameOptionsWindow->AddSlot().AutoHeight()
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
				SAssignNew(GameModeListBox, SComboBox<UClass*>)
				.OptionsSource(&GameModesList)
				.InitiallySelectedItem(InitiallySelectedGameMode)
				.OnGenerateWidget(this, &SCreateGameScreenWidget::GenerateGameModesList)
				.OnSelectionChanged(this, &SCreateGameScreenWidget::OnGameModeSelected)
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(5.0f, 0.0f, 5.0f, 0.0f)
					[
						SAssignNew(SelectedGameModeBox, STextBlock)
						.Text(FText::FromString(*InitiallySelectedGameMode->GetDefaultObject<ASolGameMode>()->DisplayName.ToString()))
					]
				]
			]
		]
	];

	GameOptionsWindow->AddSlot().AutoHeight()
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
						SAssignNew(SelectedMapBox, STextBlock)
						.Text(FText::FromString(*MapListArray[0].Get()))
					]
				]
			]
		]
	];
	
	GameOptionsWindow->AddSlot().AutoHeight()
	[
		SAssignNew(GameOptionsBox, SVerticalBox)
	];

	// Make start game button.
	GameOptionsWindow->AddSlot().AutoHeight()
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

	RefreshGameOptionsList();

	return GameOptionsWindow.ToSharedRef();
}

void SCreateGameScreenWidget::RefreshGameOptionsList()
{
	GameOptions.Empty();
	GameModeListBox->GetSelectedItem()->GetDefaultObject<ASolGameMode>()->GetGameOptions(GameOptions);
	
	GameOptionsBox->ClearChildren();
	for (int32 i = 0; i < GameOptions.Num(); i++)
	{
		GameOptionsBox->AddSlot().AutoHeight()
			[
				SNew(SBorder)

				.Padding(1)
			[
				MakeGameOptionWidget(GameOptions[i])
			]
			];
	}
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
	// Hacky way to implement checkboxes: assume any variable starting with "b" is a boolean.
	if (InOption.URLString.StartsWith("b"))
	{
		GameOptionWidget->AddSlot()
		[
			SNew(SBox)
			.WidthOverride(150.f)
			[
				SAssignNew(InOption.OptionWidget, SCheckBox)
				.IsChecked(InOption.DefaultValue.EqualTo(FText::FromString("1")) || InOption.DefaultValue.EqualTo(FText::FromString("true")) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			]
		];
	}
	else 
	{
		GameOptionWidget->AddSlot()
		[
			SNew(SBox)
			.WidthOverride(150.f)
			[
				SAssignNew(InOption.OptionWidget, SEditableTextBox)
				.Text(InOption.DefaultValue)
			]
		];
	}

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
			FString GameModeClassName = *GameModeListBox->GetSelectedItem()->GetPathName();
			StartStr += FString("?game=") + GameModeClassName;
			// Add LAN option if LAN game.
			if (bIsLAN)
			{
				StartStr += FString("?bIsLanMatch");
			}
			// Add URL details for each option.
			for (int32 i = 0; i < GameOptions.Num(); i++)
			{
				if (GameOptions[i].OptionWidget.IsValid())
				{
					TSharedPtr<SEditableTextBox> TextBox = StaticCastSharedPtr<SEditableTextBox>(GameOptions[i].OptionWidget);
					TSharedPtr<SCheckBox> CheckBox = StaticCastSharedPtr<SCheckBox>(GameOptions[i].OptionWidget);
					// Temp hack: assume any variable starting with "b" is a boolean. The casting is not working.
					if (GameOptions[i].URLString.StartsWith("b") && CheckBox.IsValid())
					{
						FString OptionName = GameOptions[i].URLString;
						FString Value = CheckBox->IsChecked() ? "1" : "0";
						StartStr += FString("?") + OptionName + FString("=") + Value;
					}
					else if (TextBox.IsValid() && !TextBox->GetText().ToString().IsEmpty())
					{
						FString OptionName = GameOptions[i].URLString;
						FString Value = TextBox->GetText().ToString();
						StartStr += FString("?") + OptionName + FString("=") + Value;
					}
				}
			}
			StartStr += FString("?listen"); //ESSENTIAL FOR PLAYER TO CONNECT TO SERVER!

			USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
			if (GI)
			{
				GI->HostGame(PlayerOwner.Get(), GameModeClassName, StartStr);
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

TSharedRef<SWidget> SCreateGameScreenWidget::GenerateGameModesList(UClass* InItem)
{
	return SNew(SBox)
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InItem->GetDefaultObject<ASolGameMode>()->DisplayName.ToString()))
		];
}

void SCreateGameScreenWidget::OnMapSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedMapBox->SetText(FText::FromString(*NewSelection.Get()));
}

void SCreateGameScreenWidget::OnGameModeSelected(UClass* NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedGameModeBox->SetText(FText::FromString(*NewSelection->GetDefaultObject<ASolGameMode>()->DisplayName.ToString()));
	RefreshGameOptionsList();
}