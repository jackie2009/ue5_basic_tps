#include "VirtualSpringComponent.h"

UVirtualSpringComponent::UVirtualSpringComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVirtualSpringComponent::BeginPlay()
{
	Super::BeginPlay();

	Position = FVector(0, 0, 0);
	Target = Position;
}

void UVirtualSpringComponent::SetTarget(FVector InTarget)
{
	Target = InTarget;
}

void UVirtualSpringComponent::AddImpulse(FVector Impulse)
{
	Velocity += Impulse / Mass;
}

void UVirtualSpringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ====== 计算弹簧力 ======
	FVector ToTarget = Target - Position;

	FVector SpringForce = Stiffness * ToTarget;
	FVector DampingForce = -Damping * Velocity;

	FVector Acceleration = (SpringForce + DampingForce) / Mass;

	// ====== 积分 ======
	Velocity += Acceleration * DeltaTime;
	Position += Velocity * DeltaTime;

	
}