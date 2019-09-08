// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "LastimMenuHUD.h"
#include "LastimLocalPlayer.h"
#include "SWindow.h"
#include "SOptionsScreenWidget.h"
#include "SlateOptMacros.h"
#include "SKeyBind.h"

#include "UserWidget.h"

FControlBind* FControlBind::AddMapping(const FString& Mapping, float Scale)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	bool bFound = false;
	// Find any found action mappings.
	for (int32 i = 0; i < InputSettings->ActionMappings.Num(); i++)
	{
		FInputActionKeyMapping Action = InputSettings->ActionMappings[i];
		if (Mapping.Compare(Action.ActionName.ToString()) == 0 && !Action.Key.IsGamepadKey())
		{
			ActionMappings.Add(Action);

			// Fill in the first 2 keys we find from the ini
			if (*Key == FKey())
			{
				*Key = Action.Key;
			}
			else if (*AltKey == FKey() && Action.Key != *Key)
			{
				*AltKey = Action.Key;
			}
			bFound = true;
		}
	}
	// Find any found axis mappings.
	for (int32 i = 0; i < InputSettings->AxisMappings.Num(); i++)
	{
		FInputAxisKeyMapping Axis = InputSettings->AxisMappings[i];
		if (Mapping.Compare(Axis.AxisName.ToString()) == 0 && Axis.Scale == Scale && !Axis.Key.IsGamepadKey())
		{
			AxisMappings.Add(Axis);

			// Fill in the first 2 keys we find from the ini
			if (*Key == FKey())
			{
				*Key = Axis.Key;
			}
			else if (*AltKey == FKey() && Axis.Key != *Key)
			{
				*AltKey = Axis.Key;
			}
			bFound = true;
		}
	}

	//Special Console case
	if (Mapping.Compare(TEXT("Console")) == 0)
	{
		*Key = InputSettings->ConsoleKeys[0];
	}

	return this;
}

void FControlBind::WriteBind()
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	// Collapse the keys if the main key is missing.
	if (*Key == FKey() && *AltKey != FKey())
	{
		Key = AltKey;
		AltKey = MakeShareable(new FKey());
	}

	//Remove the original bindings
	for (auto& Bind : ActionMappings)
	{
		InputSettings->RemoveActionMapping(Bind);
	}
	for (auto& Bind : AxisMappings)
	{
		InputSettings->RemoveAxisMapping(Bind);
	}
	//Set our new keys and readd them
	for (auto Bind : ActionMappings)
	{
		Bind.Key = *Key;
		InputSettings->AddActionMapping(Bind);
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddActionMapping(Bind);
		}
	}
	for (auto Bind : AxisMappings)
	{
		Bind.Key = *Key;
		InputSettings->AddAxisMapping(Bind);
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddAxisMapping(Bind);
		}
	}
}

void SOptionsScreenWidget::WindowSetup()
{
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "MoveForwardBind", "Move Forward")))
		->AddMapping("MoveForward", 1.0f)
		->AddDefaults(EKeys::E)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "MoveBackwardBind", "Move Backward")))
		->AddMapping("MoveForward", -1.0f)
		->AddDefaults(EKeys::D)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "MoveLeftBind", "Move Left")))
		->AddMapping("MoveRight", -1.0f)
		->AddDefaults(EKeys::S)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "MoveRightBind", "Move Right")))
		->AddMapping("MoveRight", 1.0f)
		->AddDefaults(EKeys::F)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "FireBind", "Fire")))
		->AddMapping("Fire")
		->AddDefaults(EKeys::LeftMouseButton)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "AimBind", "Aim")))
		->AddMapping("Aim")
		->AddDefaults(EKeys::RightMouseButton)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "ReloadBind", "Reload")))
		->AddMapping("Reload")
		->AddDefaults(EKeys::R)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "SwitchFireModeBind", "Switch Fire Mode")))
		->AddMapping("SwitchFireMode")
		->AddDefaults(EKeys::A, EKeys::Enter)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "UseBind", "Use")))
		->AddMapping("Use")
		->AddDefaults(EKeys::Q, EKeys::Enter)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "JumpBind", "Jump")))
		->AddMapping("Jump")
		->AddDefaults(EKeys::SpaceBar)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "CrouchBind", "Crouch")))
		->AddMapping("Crouch")
		->AddDefaults(EKeys::LeftControl)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "SprintBind", "Sprint")))
		->AddMapping("Sprint")
		->AddDefaults(EKeys::LeftShift)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "PrevWeaponBind", "Select Previous Weapon")))
		->AddMapping("PrevWeapon")
		->AddDefaults(EKeys::One)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "NextWeaponBind", "Select Next Weapon")))
		->AddMapping("NextWeapon")
		->AddDefaults(EKeys::Two)));
	Binds.Add(MakeShareable((new FControlBind(NSLOCTEXT("Lastim.HUD.Menu", "ShowScoreboardBind", "Show Scoreboard")))
		->AddMapping("ShowScoreboard")
		->AddDefaults(EKeys::Tab)));
}

TSharedRef<SWidget> SOptionsScreenWidget::ConstructWindow()
{
	return MakeOptionsWindow();
}

TSharedRef<SWidget> SOptionsScreenWidget::MakeOptionsWindow()
{
	TSharedPtr<SVerticalBox> OptionsWindowWidget = SNew(SVerticalBox);

	OptionsWindowWidget->AddSlot().AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(250.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("Lastim.HUD.Menu", "PlayerNameOption", "Player Name"))
			]
		]
		+ SHorizontalBox::Slot()
		[
			SAssignNew(PlayerNameTextBox, SEditableTextBox)
			.Text(FText::FromString(PlayerOwner->GetNickname()))
			.OnTextCommitted(this, &SOptionsScreenWidget::OnPlayerNameTextCommited)
		]
	];

	OptionsWindowWidget->AddSlot().AutoHeight()
	[
		MakeControlBinds()
	];

	return OptionsWindowWidget.ToSharedRef();
}

TSharedRef<SWidget> SOptionsScreenWidget::MakeControlBinds()
{
	TSharedPtr<SVerticalBox> BindList = SNew(SVerticalBox);

	//for (int32 i = 0; i < Binds.Num(); i++)
	for (const auto& Bind : Binds)
	{
		BindList->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(250.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Bind->DisplayName))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(Bind->KeyWidget, SKeyBind)
				.Key(Bind->Key)
				.DefaultKey(Bind->DefaultKey)
				.OnKeyBindingChanged(this, &SOptionsScreenWidget::OnCommitBind, Bind)
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(Bind->AltKeyWidget, SKeyBind)
				.Key(Bind->AltKey)
				.DefaultKey(Bind->DefaultAltKey)
				.OnKeyBindingChanged(this, &SOptionsScreenWidget::OnCommitBind, Bind)
			]
		];
	}

	return BindList.ToSharedRef();
}



TSharedRef<SWidget> SOptionsScreenWidget::MakeBind(FControlBind& InBind)
{
	TSharedPtr<SVerticalBox> BindWidget = SNew(SVerticalBox);

	return BindWidget.ToSharedRef();
}

void SOptionsScreenWidget::OnCommitBind(FKey OldKey, FKey NewKey, TSharedPtr<FControlBind> ChangedBinding)
{
	// Note to self: Key and ActionName have to be the same for actions to be equal.
	// There are other options (for if Ctrl or Shift are held), but they're normally false.
	ChangedBinding->WriteBind();
	UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>()->SaveConfig();
}

void SOptionsScreenWidget::OnPlayerNameTextCommited(const FText& NewText, ETextCommit::Type CommitType)
{
	// if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus || CommitType == ETextCommit::Default || CommitType == ETextCommit::OnCleared)
	ULastimLocalPlayer* LLP = Cast<ULastimLocalPlayer>(PlayerOwner.Get());
	if (LLP && !PlayerNameTextBox->GetText().ToString().IsEmpty())
	{
		LLP->SetPlayerName(PlayerNameTextBox->GetText().ToString());
	}
}