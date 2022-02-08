// Copyright Kyle Taylor Lange

#pragma once
#include "SlateBasics.h"
#include "AssetData.h"
#include "SKeyBind.h"
#include "SLastimWindow.h"

struct FControlBind
{
	FString DisplayName;
	TSharedPtr<FKey> Key;
	TSharedPtr<FKey> AltKey;
	FKey DefaultKey;
	FKey DefaultAltKey;
	TSharedPtr<SKeyBind> KeyWidget;
	TSharedPtr<SKeyBind> AltKeyWidget;

	TArray<FInputActionKeyMapping> ActionMappings;
	TArray<FInputAxisKeyMapping> AxisMappings;

	FControlBind* AddMapping(const FString& Mapping, float Scale = 0.0f);

	FControlBind* AddDefaults(FKey InDefaultKey, FKey InDefaultAltKey = FKey())
	{
		DefaultKey = InDefaultKey;
		DefaultAltKey = InDefaultAltKey;
		return this;
	}

	void WriteBind();

	FControlBind(const FText& InName)
	{
		DisplayName = InName.ToString();
		Key = MakeShareable(new FKey());
		AltKey = MakeShareable(new FKey());
	}
};

/**
 * Menu for game options: controls, graphics, and gameplay.
 */
class SOptionsScreenWidget : public SLastimWindow
{
public:

	TSharedRef<SWidget> MakeOptionsWindow();

	TSharedRef<SWidget> MakeControlBinds();

	TSharedRef<SWidget> MakeBind(FControlBind& InBind);

	TSharedPtr<class SEditableTextBox> PlayerNameTextBox;

	virtual void WindowSetup() override;

	virtual TSharedRef<SWidget> ConstructWindow() override;

	TArray<TSharedPtr<FControlBind>> Binds;

	TSharedPtr<class SSlider> MouseSensitivitySlider;

	TSharedPtr<class STextBlock> MouseSensitivityTextBlock;

	TArray<TSharedPtr<FLinearColor>> ColorOptions;

	TSharedPtr<SComboBox<TSharedPtr<FLinearColor>>> PrimaryColorPicker;

	TSharedPtr<SComboBox<TSharedPtr<FLinearColor>>> SecondaryColorPicker;

	TSharedPtr<class SImage> SelectedPrimaryColorImage;

	TSharedPtr<class SImage> SelectedSecondaryColorImage;

	TSharedRef<SWidget> GenerateColorBox(TSharedPtr<FLinearColor> InColor);

protected:

	void OnCommitBind(FKey OldKey, FKey NewKey, TSharedPtr<FControlBind> ChangedBinding);

	void OnPlayerNameTextCommited(const FText& NewText, ETextCommit::Type CommitType);

	void OnMouseSensitivityChanged(float NewSensitivity);

	void OnPrimaryColorSelected(TSharedPtr<FLinearColor> NewSelection, ESelectInfo::Type SelectInfo);

	void OnSecondaryColorSelected(TSharedPtr<FLinearColor> NewSelection, ESelectInfo::Type SelectInfo);


};