#include "MagicEffect.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/CombatComponent.h"
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

AMagicEffect::AMagicEffect()
{
	PrimaryActorTick.bCanEverTick = true;
  
}
void AMagicEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();
  
	// 1. 获取 Root 并尝试转换为 PrimitiveComponent (所有碰撞体的基类)
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
 
 

		EffectAnchor = RootPrim;
	
		AudioComp = Cast<UAudioComponent>(GetComponentByClass(UAudioComponent::StaticClass()));

	 
		// 默认设置为不自动播放，由我们逻辑控制
		if (AudioComp)
		{
			AudioComp->bAutoActivate = false;
			AudioComp->SetupAttachment(RootPrim);
		}
		// 2. 尝试转换
		MainCollision = Cast<UPrimitiveComponent>(RootPrim);
	 
		if (MainCollision)
		{
			// 3. 执行逻辑配置
			MainCollision->SetNotifyRigidBodyCollision(true);
			MainCollision->SetCanEverAffectNavigation(false); // 弹丸不需要影响寻路
			MainCollision->OnComponentHit.AddDynamic(this, &AMagicEffect::OnFlySphereHit);
		}else
		{
			// 如果转换失败，在屏幕左上角显示红字，持续 10 秒
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,                 // Key: -1 表示每次都新增一行，不覆盖旧的
					10.0f,              // 持续时间（秒）
					FColor::Red,        // 颜色
					FString::Printf(TEXT("错误: [%s] 的 Root 不是碰撞体！当前是: %s"), 
						*GetName(), 
						GetRootComponent() ? *GetRootComponent()->GetClass()->GetName() : TEXT("NULL"))
				);
			}
			
		}
		// 4. 挂载其他 C++ 创建的组件
		 
	      
	}
	else 
	{
		// 健壮性检查：如果 Root 只是个 SceneComponent 而不是碰撞体，这里会报错或记录日志
		UE_LOG(LogTemp, Warning, TEXT("AMagicEffect: RootComponent is not a PrimitiveComponent!"));
	}
}


AMagicEffect* AMagicEffect::SpawnMagicEffect(const UObject* WorldContextObject, const FEffectContext& InContext,const FVector& location,const FQuat& rotation)
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
	TSubclassOf<AMagicEffect> ClassToSpawn = Config->ActorClass;
	if (!ClassToSpawn)  return nullptr;

	// 2. 计算生成位置 (如果没传 AttachComp，默认在 Instigator 位置)
	FTransform SpawnTransform = FTransform::Identity;
	//没有初始化挂点 需要指定坐标与朝向
	if (InContext.VfxConfig->SpawnSpace==EVfxSpawnSpace::WorldSpace)
	{
		SpawnTransform.SetLocation(location);
		SpawnTransform.SetRotation(rotation);
	}
	
	
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
if (InContext.VfxConfig->LifeSpan>0) SetLifeSpan(InContext.VfxConfig->LifeSpan);
	// 如果需要吸附，则将容器本身挂载过去
	if (InAttachComp && (MyContext.VfxConfig->SpawnSpace == EVfxSpawnSpace::AttachToInstigator || MyContext.VfxConfig->
		SpawnSpace == EVfxSpawnSpace::AttachToVictim))
	{
		AttachToComponent(InAttachComp, FAttachmentTransformRules::SnapToTargetIncludingScale,
		                  MyContext.VfxConfig->SocketName);
	}
	if (IsValid(MainCollision))
	{
		MainCollision->IgnoreActorWhenMoving(InContext.Instigator,true);
	}
	


	if (IsValid( InContext.VfxConfig->Sound)&&IsValid(AudioComp))
	{
		AudioComp->SetSound(InContext.VfxConfig->Sound);
        
		// 关键设置：当绑定的 Actor 销毁时，音效是否停止
		AudioComp->bStopWhenOwnerDestroyed = true;
        
		AudioComp->Play();
		 
	}
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
	if (IsValid(MainCollision)) MainCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	FVector initLocation=FVector::ZeroVector;
	
    auto effect=SpawnMagicEffect(this,MyContext, Hit.ImpactPoint,UKismetMathLibrary::MakeRotFromX(Hit.ImpactNormal).Quaternion());
	 
	 
}