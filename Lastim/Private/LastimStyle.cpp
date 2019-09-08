// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimStyle.h"
#include "SlateGameResources.h"

/** Pretty well taken verbatim from ShooterGame because I have no clue what I'm doing with slate. **/

TSharedPtr< FSlateStyleSet > FLastimStyle::LastimStyleInstance = NULL;

void FLastimStyle::Initialize()
{
	if (!LastimStyleInstance.IsValid())
	{
		LastimStyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*LastimStyleInstance);
	}
}

void FLastimStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*LastimStyleInstance);
	ensure(LastimStyleInstance.IsUnique());
	LastimStyleInstance.Reset();
}

FName FLastimStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LastimStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FPaths::GameContentDir() / "Slate"/ RelativePath + TEXT(".png"), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( FPaths::GameContentDir() / "Slate"/ RelativePath + TEXT(".png"), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( FPaths::GameContentDir() / "Slate"/ RelativePath + TEXT(".png"), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "Slate"/ RelativePath + TEXT(".ttf"), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "Slate"/ RelativePath + TEXT(".otf"), __VA_ARGS__ )

TSharedRef< FSlateStyleSet > FLastimStyle::Create()
{
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FLastimStyle::GetStyleSetName(), "/Game/UI/Styles", "/Game/UI/Styles");
	FSlateStyleSet& Style = StyleRef.Get();

	// Load the speaker icon to be used for displaying when a user is talking
	//Style.Set("Lastim.Speaker", new IMAGE_BRUSH("/Images/SoundCue_SpeakerIcon", FVector2D(32, 32)));

	// Fonts still need to be specified in code for now
	Style.Set("Lastim.TestStyle", FTextBlockStyle()
		//.SetFont(TTF_FONT("Fonts/Roboto-Black", 24))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	/**
	Style.Set("Lastim.MenuServerListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Black", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	Style.Set("Lastim.ScoreboardListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Black", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	Style.Set("Lastim.MenuProfileNameStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Black", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	Style.Set("Lastim.MenuTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Black", 20))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	Style.Set("Lastim.MenuHeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Black", 26))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	Style.Set("Lastim.WelcomeScreen.WelcomeTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Medium", 32))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
		);

	Style.Set("Lastim.DefaultScoreboard.Row.HeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Black", 24))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
		);

	Style.Set("Lastim.DefaultScoreboard.Row.StatTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Regular", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
		);

	Style.Set("Lastim.SplitScreenLobby.StartMatchTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("Fonts/Roboto-Regular", 16))
		.SetColorAndOpacity(FLinearColor::Green)
		.SetShadowOffset(FVector2D(0, 1))
		);
	**/
	return StyleRef;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FLastimStyle::ReloadTextures()
{
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle& FLastimStyle::Get()
{
	return *LastimStyleInstance;
}
