// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ManipulatorPluginStyle.h"

class FManipulatorPluginCommands : public TCommands<FManipulatorPluginCommands>
{
public:

	FManipulatorPluginCommands()
		: TCommands<FManipulatorPluginCommands>(TEXT("ManipulatorPlugin"), NSLOCTEXT("Contexts", "ManipulatorPlugin", "ManipulatorPlugin Plugin"), NAME_None, FManipulatorPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > MoveToAction;
    TSharedPtr< FUICommandInfo > CopyTransformAction;
};
