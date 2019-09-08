// Copyright Kyle Taylor Lange

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

/**
 * 
 */
class LASTIM_API SLastimScoreboardWidget : public SBorder
{
public:
	/** Needed for every widget. */
	void Construct(const FArguments& InArgs);
	
protected:

	/** Holds player info rows. */
	TSharedPtr<SVerticalBox> ScoreboardData;
};
