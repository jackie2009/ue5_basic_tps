#include "MagicEffect.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

#include "basic_tps/Core/Character/CombatCharacter.h"


AMagicEffect::AMagicEffect()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建默认根组件和挂点
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	EffectAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("EffectAnchor"));
	EffectAnchor->SetupAttachment(RootComponent);
}

AMagicEffect* AMagicEffect::SpawnMagicEffect(const UObject* WorldContextObject, const FEffectContext& InContext)
{
	if (!WorldContextObject || !InContext.VfxConfig) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();


	USkillVfxDataAsset* Config = InContext.VfxConfig;

	// 计算初始化位置的参考对象
	USceneComponent* AttachToComp = nullptr;
	if (Config->SpawnSpace == EVfxSpawnSpace::AttachToInstigator || Config->SpawnSpace ==
		EVfxSpawnSpace::WorldSpaceInstigator)
	{
		AttachToComp = InContext.Instigator ? InContext.Instigator->GetMesh() : nullptr;
	}
	else if (Config->SpawnSpace == EVfxSpawnSpace::AttachToVictim || Config->SpawnSpace ==
		EVfxSpawnSpace::WorldSpaceVictim)
	{
		AttachToComp = InContext.TargetActor ? InContext.TargetActor->GetMesh() : nullptr;
	}

	if (!AttachToComp) return nullptr;


	// 1. 自动选择容器类（如果是 Niagara 且没配容器，可以用默认类）
	TSubclassOf<AMagicEffect> ClassToSpawn = Config->ContainerClass;
	if (!ClassToSpawn) ClassToSpawn = AMagicEffect::StaticClass();

	// 2. 计算生成位置 (如果没传 AttachComp，默认在 Instigator 位置)
	FTransform SpawnTransform = FTransform::Identity;
	if (InContext.VfxConfig->SpawnSpace == EVfxSpawnSpace::WorldSpaceInstigator || InContext.VfxConfig->SpawnSpace ==
		EVfxSpawnSpace::WorldSpaceVictim)
	{
		SpawnTransform.SetLocation(AttachToComp->GetSocketLocation(InContext.VfxConfig->SocketName));
		SpawnTransform.SetRotation(AttachToComp->GetOwner()->GetActorRotation().Quaternion());
	}

	FActorSpawnParameters Params;
	Params.Instigator = InContext.Instigator;

	AMagicEffect* NewEffect = World->SpawnActor<AMagicEffect>(ClassToSpawn, SpawnTransform, Params);
	if (NewEffect)
	{
		NewEffect->InitializeEffect(InContext, AttachToComp);
	}
	return NewEffect;
}


void AMagicEffect::InitializeEffect(FEffectContext InContext, USceneComponent* InAttachComp)
{
	MyContext = InContext;

	// 如果需要吸附，则将容器本身挂载过去
	if (InAttachComp && (MyContext.VfxConfig->SpawnSpace == EVfxSpawnSpace::AttachToInstigator || MyContext.VfxConfig->
		SpawnSpace == EVfxSpawnSpace::AttachToVictim))
	{
		AttachToComponent(InAttachComp, FAttachmentTransformRules::SnapToTargetIncludingScale,
		                  MyContext.VfxConfig->SocketName);
	}

	Internal_SetupVisuals();
}

void AMagicEffect::Internal_SetupVisuals()
{
	USkillVfxDataAsset* Config = MyContext.VfxConfig;
	if (!Config) return;

	// 这一段替代了你原来的 switch(ObjectType)
	if (Config->ObjectType == EVfxObjectType::Niagara && Config->NiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			Config->NiagaraSystem,
			EffectAnchor, // 挂在容器的锚点上，而不是直接挂在 Mesh 上
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}
	// 如果是 Actor 类型，ContainerClass 本身已经生成了，这里可以处理额外的子物体挂载
}
AMagicEffect* AMagicEffect::SpawnNextMagicEffect()
{
	if (MyContext.VfxConfig->ChildMode!=ECreateChildMode::Notify) return nullptr;
	MyContext.VfxConfig=MyContext.VfxConfig->NextEffect.LoadSynchronous();
	return SpawnMagicEffect(this,MyContext);
}
