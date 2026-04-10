#include "MagicEffect.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/CombatComponent.h"
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "basic_tps/Core/Utils/CombatCameraUtils.h"
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
  
	 
 
 

		EffectAnchor = GetRootComponent();
	
		AudioComp = Cast<UAudioComponent>(GetComponentByClass(UAudioComponent::StaticClass()));

	 
		// 默认设置为不自动播放，由我们逻辑控制
		if (AudioComp)
		{
			AudioComp->bAutoActivate = false;
	 
		}
	
	MainCollision=nullptr;
	// 获取所有带 Tag 的组件（通常我们只需要第一个）
	TArray<UActorComponent*> Components = GetComponentsByTag(UPrimitiveComponent::StaticClass(), FName("MainCollision"));

	if (Components.Num() > 0)
	{
		MainCollision = Cast<UPrimitiveComponent>(Components[0]);
        
		 
	}
	 auto Config=&EffectConfig;
	if (IsValid( Config->Sound)&&IsValid(AudioComp))
	{
		AudioComp->SetSound(Config->Sound);
        
		// 关键设置：当绑定的 Actor 销毁时，音效是否停止
		AudioComp->bStopWhenOwnerDestroyed = true;
        
		AudioComp->Play();
		 
	}

	if (MainCollision)
	{
		 
		MainCollision->IgnoreActorWhenMoving(MyContext.Instigator,true);
		for (auto ignoreTarget : 	MyContext.IgnoreTargetSet)
		{
			MainCollision->IgnoreActorWhenMoving(ignoreTarget,true);
		}
		
		
		// 3. 执行逻辑配置
		MainCollision->SetNotifyRigidBodyCollision(true);
		MainCollision->SetCanEverAffectNavigation(false); // 弹丸不需要影响寻路
			
		// 1. 核心设置：开启重叠事件产生
		MainCollision->SetGenerateOverlapEvents(true);

		
		MainCollision->OnComponentHit.AddDynamic(this, &AMagicEffect::OnFlySphereHit);
		MainCollision->OnComponentBeginOverlap.AddDynamic(this, &AMagicEffect::OnEffectOverlap);

		if (Config->AlwaysRotationWithSpaceActor)
		{
			EffectAnchor->SetUsingAbsoluteRotation(true);
		}
			
	} 
		 
		 
	      
	 
}
void AMagicEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 关键点 2：实时同步 Owner（角色）的旋转
	if (EffectConfig.AlwaysRotationWithSpaceActor)
	{
		if(AActor* MyOwner = GetOwner())
		{
			// 直接让特效的世界旋转等于角色的世界旋转
			// 这样角色转身，特效就跟着转；角色摆手，特效方向不动
			SetActorRotation(MyOwner->GetActorRotation());
		}
	}
}

AMagicEffect* AMagicEffect::SpawnMagicEffect(const UObject* WorldContextObject,TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext& InContext,const FVector& location,const FQuat& rotation,const bool ForceUseParamsTransform)
{
	if (!WorldContextObject || ClassToSpawn==nullptr) return nullptr;
  
	//GEngine->AddOnScreenDebugMessage(-1,10,FColor::Yellow,FString::Printf( TEXT("SpawnMagicEffect")));

	
	UWorld* World = WorldContextObject->GetWorld();

	UClass* EffectClass = ClassToSpawn;
	FSkillVfxConfig* Config=&EffectClass->GetDefaultObject<AMagicEffect>()->EffectConfig;

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

	if (!AttachToComp&&(Config->SpawnSpace != EVfxSpawnSpace::WorldSpace&&Config->SpawnSpace != EVfxSpawnSpace::CameraViewHitLocation
	&&Config->SpawnSpace != EVfxSpawnSpace::CameraViewHitGroundLocation
	) ) return nullptr;




	// 2. 计算生成位置 (如果没传 AttachComp，默认在 Instigator 位置)
	FTransform SpawnTransform = FTransform::Identity;
	//没有初始化挂点 需要指定坐标与朝向
	if (ForceUseParamsTransform||Config->SpawnSpace==EVfxSpawnSpace::WorldSpace)
	{
		SpawnTransform.SetLocation(location);
		SpawnTransform.SetRotation(rotation);
	}else if (Config->SpawnSpace== EVfxSpawnSpace::CameraViewHitLocation)
	{
	 
		FVector CameraHitPoint;
		// 2. MaxRange: 20米 = 2000厘米
		float MaxRange = 2000.0f;
		if (!UCombatCameraUtils::GetCameraViewHitLocation(WorldContextObject,MaxRange,CameraHitPoint))return nullptr;
		SpawnTransform.SetLocation(CameraHitPoint);
		SpawnTransform.SetRotation( InContext.Instigator->GetActorRotation().Quaternion());
		
	}
	else if (Config->SpawnSpace== EVfxSpawnSpace::CameraViewHitGroundLocation)
	{
	 
		FVector CameraHitPoint;
		// 2. MaxRange: 20米 = 2000厘米
		float MaxRange = 2000.0f;
		if (!UCombatCameraUtils::ProjectLocationToGround(WorldContextObject,MaxRange,CameraHitPoint))return nullptr;
		SpawnTransform.SetLocation(CameraHitPoint);
		SpawnTransform.SetRotation( InContext.Instigator->GetActorRotation().Quaternion());
	}
	
	if (ForceUseParamsTransform==false)
	{
		if (Config->SpawnSpace == EVfxSpawnSpace::WorldSpaceInstigator ||Config->SpawnSpace ==
			EVfxSpawnSpace::WorldSpaceVictim
			||Config->SpawnSpace == EVfxSpawnSpace::AttachToInstigator ||Config->SpawnSpace == EVfxSpawnSpace::AttachToVictim)
		{
			auto CombatComp=Cast<UCombatComponent>(AttachToComp->GetOwner()->GetComponentByClass(UCombatComponent::StaticClass()));
			SpawnTransform.SetLocation(  CombatComp->GetEffectAttachSource(Config->SocketName)->GetSocketLocation(Config->SocketName));
		
		}
		if (Config->InitRotationWithSpaceActor&&AttachToComp!=nullptr)
			SpawnTransform.SetRotation(AttachToComp->GetOwner()->GetActorRotation().Quaternion());
	}
	FActorSpawnParameters Params;
	Params.Instigator = InContext.Instigator;

	// 1. 开启延迟生成
	AMagicEffect* NewEffect = World->SpawnActorDeferred<AMagicEffect>(ClassToSpawn, SpawnTransform);

	if (NewEffect)
	{
	 
		NewEffect->InitializeEffect(InContext, AttachToComp,SpawnTransform);
	
		 
	}

	return NewEffect;
}

//不用依赖组件 这时候还没准备好组件 
void AMagicEffect::InitializeEffect(FEffectContext InContext, USceneComponent* InAttachComp,FTransform & SpawnTransform)
{
	MyContext = InContext;
	MyContext.Generation++;
	// 先做组件初始化处理 在做 依赖组件的逻辑
	 FinishSpawning(SpawnTransform);
	auto Config = &EffectConfig;
	
   if (Config->LifeSpan>0) SetLifeSpan(Config->LifeSpan);
	
	// 如果需要吸附，则将容器本身挂载过去
	if (InAttachComp && (Config->SpawnSpace == EVfxSpawnSpace::AttachToInstigator || Config->
		SpawnSpace == EVfxSpawnSpace::AttachToVictim))
	{
		auto CombatComp=Cast<UCombatComponent>(InAttachComp->GetOwner()->GetComponentByClass(UCombatComponent::StaticClass()));
		AttachToComponent(CombatComp->GetEffectAttachSource( Config->SocketName), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                 Config->SocketName);
		SetOwner(InAttachComp->GetOwner());
 
	}
	 
	 

}

 
AMagicEffect* AMagicEffect::SpawnNextMagicEffect_Implementation()
{
 
	if (EffectConfig.ChildMode!=ECreateChildMode::Notify) return nullptr;
	 
	return SpawnMagicEffect(this,EffectConfig.NextEffect,MyContext);
}


void AMagicEffect::OnEffectOverlap(UPrimitiveComponent* OverlappedComponent, 
						 AActor* OtherActor, 
						 UPrimitiveComponent* OtherComp, 
						 int32 OtherBodyIndex, 
						 bool bFromSweep, 
						 const FHitResult& SweepResult)
{
	if (MyContext.Instigator==nullptr)return;
	if (MyContext.IgnoreTargetSet.Contains(OtherActor))return;
	 
	if (OtherActor==MyContext.Instigator)return;
	 
    
	// 调用抽离后的核心逻辑
	ProcessImpact(OtherActor, OtherComp, SweepResult);
//	GEngine->AddOnScreenDebugMessage(-1,5,FColor::Green,FString::Printf( TEXT("hit by c++,%s"),*OtherActor->GetName()));
 
 
	
	 
	 
}
void   AMagicEffect::ProcessImpact(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& HitResult)
{

	 
	if (EffectConfig.HitFirstTarget== EHitFirstTargetHandle::Disable||EffectConfig.HitFirstTarget== EHitFirstTargetHandle::Destroy)
	{
		
		if (IsValid(MainCollision)) MainCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (EffectConfig.HitFirstTarget== EHitFirstTargetHandle::Destroy)SetLifeSpan(0.3f);

	}
	auto targetActor = Cast<ACombatCharacter>(OtherActor);
	if (EffectConfig.HurtTargetWhenHit)
	{
		
		if (IsValid(targetActor) && IsValid(MyContext.Instigator))
		{
			MyContext.TargetActor = targetActor;
			MyContext.Instigator->CombatComp->TryHurtTarget(targetActor, MyContext.SkillBaseVo->ID);
		}
	}
	if (EffectConfig.ChildMode == ECreateChildMode::Hit)
	{
		FVector ImpactPoint=HitResult.ImpactPoint;
	 
		// 获取敌人碰撞体上距离我（子弹/火球）中心最近的点
		OtherComp->GetClosestPointOnCollision(GetActorLocation(), ImpactPoint, HitResult.BoneName);
	
	
		auto effect=SpawnMagicEffect(this,EffectConfig.NextEffect,MyContext, ImpactPoint,UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal).Quaternion());
	}
	OnValidHit(targetActor);
}
void AMagicEffect::OnFlySphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (MyContext.Instigator==nullptr)return;
 
		
	 if (IsValid(MainCollision)) MainCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	 SetLifeSpan(0.3f);

 
	auto effect=SpawnMagicEffect(this,EffectConfig.NextEffect,MyContext, Hit.ImpactPoint,UKismetMathLibrary::MakeRotFromX(Hit.ImpactNormal).Quaternion());
	
}