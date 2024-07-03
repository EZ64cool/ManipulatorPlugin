#include "ManipulatorPluginCommands.h"

#define LOCTEXT_NAMESPACE "FManipulatorPluginModule"

void FManipulatorPluginCommands::RegisterCommands()
{
    UI_COMMAND(MoveToAction, "ManipulatorPlugin", "Execute Move To action", EUserInterfaceActionType::Button, FInputChord(EKeys::T, EModifierKey::Alt));
    UI_COMMAND(CopyTransformAction, "ManipulatorPlugin", "Execute Copy Transform action", EUserInterfaceActionType::Button, FInputChord(EKeys::W, EModifierKey::Alt));
}

#undef LOCTEXT_NAMESPACE
