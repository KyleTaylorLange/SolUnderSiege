// Copyright Kyle Taylor Lange

// TODO

#include "SolCore.h"

// These colours can go somewhere else in the future, but this works for now.
#define SOL_COLOR_RED			FLinearColor(357.f, 1.0f, 0.625f).HSVToLinearRGB()
#define SOL_COLOR_BLUE			FLinearColor(235.f, 1.0f, 0.4f).HSVToLinearRGB()
#define SOL_COLOR_GREEN			FLinearColor(110.f, 1.0f, 0.5f).HSVToLinearRGB()
#define SOL_COLOR_GOLD			FLinearColor(50.f, 1.0f, 0.75f).HSVToLinearRGB() //45 1.0 .78
#define SOL_COLOR_PURPLE		FLinearColor(290.f, 1.0f, 0.5f).HSVToLinearRGB()
#define SOL_COLOR_ORANGE		FLinearColor(30.f, 1.0f, 0.875f).HSVToLinearRGB()
#define SOL_COLOR_CYAN			FLinearColor(160.f, 1.0f, 0.75f).HSVToLinearRGB()
#define SOL_COLOR_SILVER		FLinearColor(210.f, 0.625f, 0.75f).HSVToLinearRGB()
#define SOL_COLOR_WHITE			FLinearColor::White
#define SOL_COLOR_BLACK			FLinearColor(0.125f, 0.125f, 0.125f)
#define SOL_COLOR_LIME			FLinearColor(80.f, 1.0f, 0.875f).HSVToLinearRGB()
#define SOL_COLOR_TEAL			FLinearColor(180.f, 1.0f, 0.50f).HSVToLinearRGB()
#define SOL_COLOR_PINK			FLinearColor(320.f, 0.5f, 1.0f).HSVToLinearRGB()
#define SOL_COLOR_BROWN			FLinearColor(30.f, 0.75f, 0.5f).HSVToLinearRGB()
#define SOL_COLOR_GREY			FLinearColor(200.f, 0.625f, 0.375f).HSVToLinearRGB()
