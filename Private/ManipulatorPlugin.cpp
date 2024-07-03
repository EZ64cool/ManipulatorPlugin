// Copyright Epic Games, Inc. All Rights Reserved.

#include "ManipulatorPlugin.h"
#include "ManipulatorPluginStyle.h"
#include "ManipulatorPluginCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include <EditorModeManager.h>
#include <Engine/Selection.h>
#include <EngineUtils.h>
#include <Editor/MainFrame/Public/Interfaces/IMainFrameModule.h>
#include <Editor/UnrealEd/Private/Editor/ActorPositioning.h>
#include <Editor/UnrealEd/Public/LevelEditorViewport.h>
#include <LevelEditor.h>
#include <ActorFactories/ActorFactory.h>

static const FName ManipulatorPluginTabName("ManipulatorPlugin");

#define LOCTEXT_NAMESPACE "FManipulatorPluginModule"

void FManipulatorPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FManipulatorPluginStyle::Initialize();
	FManipulatorPluginStyle::ReloadTextures();

	FManipulatorPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FManipulatorPluginCommands::Get().MoveToAction,
		FExecuteAction::CreateStatic(&FManipulatorPluginModule::PlaceAtCursorExec),
		FCanExecuteAction());

    PluginCommands->MapAction(
        FManipulatorPluginCommands::Get().CopyTransformAction,
        FExecuteAction::CreateStatic(&FManipulatorPluginModule::CopyTransformExec),
        FCanExecuteAction());

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());
}

void FManipulatorPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FManipulatorPluginStyle::Shutdown();

	FManipulatorPluginCommands::Unregister();
}

static FTransform RotateAroundPoint(const FTransform& original, const FVector& pivot, const FQuat& rotation)
{
    FTransform pivotTransform(pivot);
    FTransform pivotToActor(original * pivotTransform.Inverse());
    FTransform deltaTransform(rotation);

    return FTransform(pivotToActor * deltaTransform * pivotTransform);
}

void FManipulatorPluginModule::PlaceAtCursorExec()
{
    const int32 HitX = GEditor->GetActiveViewport()->GetMouseX();
    const int32 HitY = GEditor->GetActiveViewport()->GetMouseY();

    // Compute a view.
    FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
        GEditor->GetActiveViewport(),
        GCurrentLevelEditingViewportClient->GetScene(),
        GCurrentLevelEditingViewportClient->EngineShowFlags)
        .SetRealtimeUpdate(GCurrentLevelEditingViewportClient->IsRealtime()));
    FSceneView* View = GCurrentLevelEditingViewportClient->CalcSceneView(&ViewFamily);

    const FViewportCursorLocation Cursor(View, GCurrentLevelEditingViewportClient, HitX, HitY);
    const FActorPositionTraceResult TraceResult = FActorPositioning::TraceWorldForPositionWithDefault(Cursor, *View);

    FVector worldPosition = TraceResult.Location;

    USelection* SelectedActors = GEditor->GetSelectedActors();

    GEditor->BeginTransaction(FText::FromString("Move To"));

    FEditorModeTools * mode_tools = GCurrentLevelEditingViewportClient->GetModeTools();

    FVector relative = worldPosition - mode_tools->PivotLocation;

    AActor* mainActor = SelectedActors->GetBottom<AActor>();

    const UActorFactory *factory = GEditor->FindActorFactoryForActorClass(mainActor->GetClass());
    FQuat targetRot;
    if (factory != nullptr)
    {
        targetRot = factory->AlignObjectToSurfaceNormal(TraceResult.SurfaceNormal, mainActor->GetActorQuat());
    }
    else
    {
        FVector up = TraceResult.SurfaceNormal;
        FVector forward = FVector::ZeroVector;
        FVector right = GCurrentLevelEditingViewportClient->GetViewRotation().RotateVector(FVector::RightVector);
        right *= -1.0f;

        FVector::CreateOrthonormalBasis(forward, right, up);

        FMatrix rotationMatrix(forward, right, up, FVector::ZeroVector);
        targetRot = rotationMatrix.ToQuat();
    }

    FQuat rotationDiff = targetRot * mainActor->GetTransform().GetRotation().Inverse();

    for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
    {
        AActor *Actor = Cast<AActor, UObject>(*Iter);
        if (Actor)
        {
            Actor->Modify();

            Actor->SetActorLocation(Actor->GetActorLocation() + relative);
            Actor->SetActorTransform(RotateAroundPoint(Actor->GetTransform(), worldPosition, rotationDiff));
        }
    }


    mode_tools->PivotLocation = worldPosition;
    mode_tools->SnappedLocation = worldPosition;

    GEditor->EndTransaction();
}

void FManipulatorPluginModule::CopyTransformExec()
{
    const int32 HitX = GEditor->GetActiveViewport()->GetMouseX();
    const int32 HitY = GEditor->GetActiveViewport()->GetMouseY();

    const auto proxy = GEditor->GetActiveViewport()->GetHitProxy(HitX, HitY);

    if (proxy == nullptr || proxy->IsA(HActor::StaticGetType()) == false)
    {
        return;
    }

    const auto actor_prox = static_cast<const HActor*>(proxy);

    const auto &transfrom = actor_prox->Actor->GetActorTransform();

    USelection* SelectedActors = GEditor->GetSelectedActors();

    GEditor->BeginTransaction(FText::FromString("Copy Transform"));

    for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
    {
        AActor* Actor = Cast<AActor, UObject>(*Iter);
        if (Actor)
        {
            //Actor->AddActorWorldTransform(RelativeTransform);
            Actor->Modify();
            Actor->SetActorTransform(transfrom);
        }
    }

    FEditorModeTools* mode_tools = GCurrentLevelEditingViewportClient->GetModeTools();
    mode_tools->PivotLocation = transfrom.GetLocation();
    mode_tools->SnappedLocation = transfrom.GetLocation();
    mode_tools->TranslateRotateXAxisAngle = transfrom.Rotator().Yaw;
    mode_tools->TranslateRotate2DAngle = transfrom.Rotator().Pitch;

    GEditor->EndTransaction();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FManipulatorPluginModule, ManipulatorPlugin)