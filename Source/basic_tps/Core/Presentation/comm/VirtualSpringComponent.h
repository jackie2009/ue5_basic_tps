#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VirtualSpringComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UVirtualSpringComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVirtualSpringComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;

	// ====== 外部控制 ======
	UFUNCTION(BlueprintCallable)
	void AddImpulse(FVector Impulse);

	UFUNCTION(BlueprintCallable)
	void SetTarget(FVector InTarget);
	UFUNCTION(BlueprintCallable)
	FVector GetBasePosition() const { return Position; }

	UFUNCTION(BlueprintCallable)
	FVector GetBaseVelocity() const { return Velocity; }

	UFUNCTION(BlueprintCallable)
	FVector GetBaseTarget() const { return Target; }

protected:

	// ====== 虚拟状态 ======
	FVector Position;
	FVector Velocity;
	FVector Target;

	// ====== 参数 ======
	UPROPERTY(EditAnywhere)
	float Stiffness = 80.f;

	UPROPERTY(EditAnywhere)
	float Damping = 20.f;

	UPROPERTY(EditAnywhere)
	float Mass = 1.f;

	


};