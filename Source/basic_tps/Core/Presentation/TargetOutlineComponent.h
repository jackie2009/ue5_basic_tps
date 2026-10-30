#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetOutlineComponent.generated.h"

class UCameraComponent;
class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** Local presentation only. Reserves stencil 250 while selected, restores previous mesh state. */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UTargetOutlineComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UTargetOutlineComponent();
	virtual void Deactivate() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Outline")
	bool bShowOutline = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Outline", meta=(ClampMin="1", ClampMax="6"))
	float WidthPixels = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Outline")
	TSoftObjectPtr<UMaterialInterface> OutlineMaterial;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTick) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void ClearHighlight();
	void RemoveBlendable();
	struct FMeshState
	{
		TWeakObjectPtr<UMeshComponent> Mesh;
		bool bCustomDepth = false;
		int32 Stencil = 0;
		ERendererStencilMask WriteMask = ERendererStencilMask::ERSM_Default;
	};
	TArray<FMeshState> PreviousStates;
	TWeakObjectPtr<AActor> Highlighted;
	TWeakObjectPtr<UCameraComponent> Camera;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;
	bool bAttemptedMaterialLoad = false;
};
