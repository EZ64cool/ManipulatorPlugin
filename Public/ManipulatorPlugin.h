#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FManipulatorPluginModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command. */
	static void PlaceAtCursorExec();
	static void CopyTransformExec();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
