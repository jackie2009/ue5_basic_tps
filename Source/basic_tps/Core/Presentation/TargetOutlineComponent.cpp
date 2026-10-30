#include "TargetOutlineComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"

UTargetOutlineComponent::UTargetOutlineComponent()
{
	bAutoActivate = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	OutlineMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Combat/Targeting/M_TargetOutline.M_TargetOutline")));
}

void UTargetOutlineComponent::ClearHighlight()
{
	for (const FMeshState& State : PreviousStates)
	{
		if (auto* Mesh = State.Mesh.Get())
		{
			// Don't overwrite another system that has claimed the stencil since our selection.
			if (Mesh->CustomDepthStencilValue == 250)
			{
				Mesh->SetRenderCustomDepth(State.bCustomDepth);
				Mesh->SetCustomDepthStencilValue(State.Stencil);
				Mesh->SetCustomDepthStencilWriteMask(State.WriteMask);
			}
		}
	}
	PreviousStates.Reset();
	Highlighted.Reset();
}

void UTargetOutlineComponent::Deactivate()
{
	ClearHighlight();
	RemoveBlendable();
	Super::Deactivate();
}

void UTargetOutlineComponent::RemoveBlendable()
{
	if (Camera.IsValid() && MaterialInstance) Camera->RemoveBlendable(MaterialInstance);
	Camera.Reset();
}

void UTargetOutlineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTick)
{
	Super::TickComponent(DeltaTime, TickType, ThisTick);
	const auto* Pawn = Cast<APawn>(GetOwner());
	const auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	auto* Selector = GetOwner()->FindComponentByClass<UCombatTargetingComponent>();
	const auto* Instance = GetWorld()->GetGameInstance();
	// Shared CustomDepth is not per-view. Disable in split-screen instead of leaking highlights across views.
	const bool bLocalView = bShowOutline && IsActive() && PC && PC->IsLocalController() && PC->GetViewTarget() == GetOwner()
		&& Instance && Instance->GetNumLocalPlayers() == 1 && GetNetMode() != NM_DedicatedServer;
	UCameraComponent* ActiveCamera = nullptr;
	if (bLocalView)
	{
		TInlineComponentArray<UCameraComponent*> Cameras(GetOwner());
		for (auto* Candidate : Cameras) if (Candidate->IsActive()) { ActiveCamera = Candidate; break; }
	}
	if (Camera.Get() != ActiveCamera) { ClearHighlight(); RemoveBlendable(); }
	if (!ActiveCamera) return;
	if (!MaterialInstance && !bAttemptedMaterialLoad)
	{
		bAttemptedMaterialLoad = true;
		if (auto* Material = OutlineMaterial.LoadSynchronous()) MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
		else UE_LOG(LogTemp, Error, TEXT("Target outline material missing. Run Scripts/create_target_outline.py in UE editor."));
	}
	if (!MaterialInstance) return;
	if (!Camera.IsValid())
	{
		Camera = ActiveCamera;
		Camera->AddOrUpdateBlendable(MaterialInstance, 1.f);
	}
	MaterialInstance->SetScalarParameterValue(TEXT("OutlineWidth"), FMath::Clamp(WidthPixels, 1.f, 6.f));
	AActor* Target = Selector ? Selector->GetActionTarget() : nullptr;
	if (Highlighted.Get() == Target) return;
	ClearHighlight();
	if (!Target) return;
	Highlighted = Target;
	TInlineComponentArray<UMeshComponent*> Meshes;
	Target->GetComponents(Meshes, true);
	for (auto* Mesh : Meshes)
	{
		if (!Mesh->IsVisible() || !Mesh->bRenderInMainPass) continue;
		PreviousStates.Add({ Mesh, Mesh->bRenderCustomDepth != 0, Mesh->CustomDepthStencilValue, Mesh->CustomDepthStencilWriteMask });
		Mesh->SetCustomDepthStencilWriteMask(ERendererStencilMask::ERSM_Default);
		Mesh->SetCustomDepthStencilValue(250);
		Mesh->SetRenderCustomDepth(true);
	}
}

void UTargetOutlineComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	ClearHighlight();
	RemoveBlendable();
	Super::EndPlay(Reason);
}
