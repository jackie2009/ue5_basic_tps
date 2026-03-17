#include "MagicEffect.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/CombatComponent.h"
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"


AMagicEffect::AMagicEffect()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建默认根组件和挂点
	// 1. 创建球体碰撞并设为根
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	// 1. 设置半径
	CollisionSphere->InitSphereRadius(16.0f);

	// 2. 勾选 Simulation Generates Hit Events (对应蓝图里的同名选项)
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	// 3. 设置 Collision Presets
	// 方案 A: 使用预设名称（最简洁，推荐在 Project Settings 里定义好你的 Projectile 频道）
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetCanEverAffectNavigation(false); // 弹丸不需要影响寻路
	RootComponent = CollisionSphere;
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

	if (!AttachToComp&&Config->SpawnSpace != EVfxSpawnSpace::WorldSpace ) return nullptr;


	// 1. 自动选择容器类（如果是 Niagara 且没配容器，可以用默认类）
	TSubclassOf<AMagicEffect> ClassToSpawn = Config->ContainerClass;
	if (!ClassToSpawn) ClassToSpawn = AMagicEffect::StaticClass();

	// 2. 计算生成位置 (如果没传 AttachComp，默认在 Instigator 位置)
	FTransform SpawnTransform = FTransform::Identity;
	if (InContext.VfxConfig->SpawnSpace == EVfxSpawnSpace::WorldSpaceInstigator || InContext.VfxConfig->SpawnSpace ==
		EVfxSpawnSpace::WorldSpaceVictim)
	{
		SpawnTransform.SetLocation(AttachToComp->GetSocketLocation(InContext.VfxConfig->SocketName));
		if (InContext.VfxConfig->InitRotationWithSpaceActor)
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
	CollisionSphere->IgnoreActorWhenMoving(InContext.Instigator,true);
	Internal_SetupVisuals();
	
	if (CollisionSphere)
	{
		//GEngine->AddOnScreenDebugMessage(-1,5,FColor::Green,FString::Printf( TEXT("Bind by c++")));
		// AddDynamic 是一个宏，用于绑定函数
		CollisionSphere->OnComponentHit.AddDynamic(this, &AMagicEffect::OnFlySphereHit);
	}
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
	MyContext.VfxConfig=MyContext.VfxConfig->NextEffect;
	return SpawnMagicEffect(this,MyContext);
}


void AMagicEffect::OnFlySphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
		if (MyContext.VfxConfig->ChildMode!=ECreateChildMode::Hit) return;
	 CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GEngine->AddOnScreenDebugMessage(-1,5,FColor::Green,FString::Printf( TEXT("hit by c++,%s"),*OtherActor->GetName()));
		// 2. 获取碰撞点
		FVector HitLocation = Hit.ImpactPoint;
	SetLifeSpan(0.3f);
	auto targetActor=Cast<ACombatCharacter>( Hit.GetActor());
if (IsValid(targetActor)&&IsValid(MyContext.Instigator))
{
	MyContext.TargetActor=targetActor;
	MyContext.Instigator->CombatComp->TryHurtTarget(targetActor,MyContext.SkillBaseVo->ID);
}
	MyContext.VfxConfig=MyContext.VfxConfig->NextEffect;
    auto effect=SpawnMagicEffect(this,MyContext);
	if (effect!=nullptr&&MyContext.VfxConfig->SpawnSpace==EVfxSpawnSpace::WorldSpace)
	{
		effect->SetActorLocation( Hit.ImpactPoint);
		effect->SetActorRotation( UKismetMathLibrary::MakeRotFromX(Hit.ImpactNormal));
		//子弹需要考虑 normal
		
	}
	 
}