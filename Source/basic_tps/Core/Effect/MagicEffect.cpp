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
  
	 
 
 

		EffectAnchor = GetRootComponent();
	
		AudioComp = Cast<UAudioComponent>(GetComponentByClass(UAudioComponent::StaticClass()));

	 
		// 默认设置为不自动播放，由我们逻辑控制
		if (AudioComp)
		{
			AudioComp->bAutoActivate = false;
			AudioComp->SetupAttachment(EffectAnchor);
		}
	
	MainCollision=nullptr;
	// 获取所有带 Tag 的组件（通常我们只需要第一个）
	TArray<UActorComponent*> Components = GetComponentsByTag(UPrimitiveComponent::StaticClass(), FName("MainCollision"));

	if (Components.Num() > 0)
	{
		MainCollision = Cast<UPrimitiveComponent>(Components[0]);
        
		 
	}
	 
		if (MainCollision)
		{
			// 3. 执行逻辑配置
			MainCollision->SetNotifyRigidBodyCollision(true);
			MainCollision->SetCanEverAffectNavigation(false); // 弹丸不需要影响寻路
			
			// 1. 核心设置：开启重叠事件产生
			MainCollision->SetGenerateOverlapEvents(true);

		 
			MainCollision->OnComponentHit.AddDynamic(this, &AMagicEffect::OnFlySphereHit);
			MainCollision->OnComponentBeginOverlap.AddDynamic(this, &AMagicEffect::OnEffectOverlap);
			
		} 
		// 4. 挂载其他 C++ 创建的组件
		 
	      
	 
}


AMagicEffect* AMagicEffect::SpawnMagicEffect(const UObject* WorldContextObject,TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext& InContext,const FVector& location,const FQuat& rotation)
{
	if (!WorldContextObject || ClassToSpawn==nullptr) return nullptr;
  
	GEngine->AddOnScreenDebugMessage(-1,10,FColor::Yellow,FString::Printf( TEXT("SpawnMagicEffect")));

	
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

	if (!AttachToComp&&Config->SpawnSpace != EVfxSpawnSpace::WorldSpace ) return nullptr;




	// 2. 计算生成位置 (如果没传 AttachComp，默认在 Instigator 位置)
	FTransform SpawnTransform = FTransform::Identity;
	//没有初始化挂点 需要指定坐标与朝向
	if (Config->SpawnSpace==EVfxSpawnSpace::WorldSpace)
	{
		SpawnTransform.SetLocation(location);
		SpawnTransform.SetRotation(rotation);
	}
	
	
	if (Config->SpawnSpace == EVfxSpawnSpace::WorldSpaceInstigator ||Config->SpawnSpace ==
		EVfxSpawnSpace::WorldSpaceVictim)
	{
		SpawnTransform.SetLocation(AttachToComp->GetSocketLocation(Config->SocketName));
		if (Config->InitRotationWithSpaceActor)
		SpawnTransform.SetRotation(AttachToComp->GetOwner()->GetActorRotation().Quaternion());
	}

	FActorSpawnParameters Params;
	Params.Instigator = InContext.Instigator;

	// 1. 开启延迟生成
	AMagicEffect* NewEffect = World->SpawnActorDeferred<AMagicEffect>(
		ClassToSpawn, 
		SpawnTransform, 
		nullptr, 
		nullptr, 
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (NewEffect)
	{
		// 2. 先填充你的 Context 和 Config（此时 Overlap 绝对不会触发）
		NewEffect->InitializeEffect(InContext, AttachToComp, Config);

		// 3. 最后完成生成（此时才会跑 OnConstruction -> PostInitializeComponents -> 触发 Overlap）
		NewEffect->FinishSpawning(SpawnTransform);
	}

	return NewEffect;
}


void AMagicEffect::InitializeEffect(FEffectContext InContext, USceneComponent* InAttachComp,FSkillVfxConfig* Config)
{
	MyContext = InContext;
if (Config->LifeSpan>0) SetLifeSpan(Config->LifeSpan);
	// 如果需要吸附，则将容器本身挂载过去
	if (InAttachComp && (Config->SpawnSpace == EVfxSpawnSpace::AttachToInstigator || Config->
		SpawnSpace == EVfxSpawnSpace::AttachToVictim))
	{
		AttachToComponent(InAttachComp, FAttachmentTransformRules::SnapToTargetIncludingScale,
		                 Config->SocketName);
	}
	if (IsValid(MainCollision))
	{
		MainCollision->IgnoreActorWhenMoving(InContext.Instigator,true);
	}
	


	if (IsValid( Config->Sound)&&IsValid(AudioComp))
	{
		AudioComp->SetSound(Config->Sound);
        
		// 关键设置：当绑定的 Actor 销毁时，音效是否停止
		AudioComp->bStopWhenOwnerDestroyed = true;
        
		AudioComp->Play();
		 
	}
}

 
AMagicEffect* AMagicEffect::SpawnNextMagicEffect()
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
	if (EffectConfig.ChildMode != ECreateChildMode::Hit) return;
	if (OtherActor==MyContext.Instigator)return;
	
//	GEngine->AddOnScreenDebugMessage(-1,5,FColor::Green,FString::Printf( TEXT("hit by c++,%s"),*OtherActor->GetName()));
 
 
	//没有设置生命的 都算碰撞消失
	if (EffectConfig.LifeSpan<=0)
	{
		SetLifeSpan(0.3f);
		if (IsValid(MainCollision)) MainCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	}
	
	auto targetActor = Cast<ACombatCharacter>(OtherActor);
	if (IsValid(targetActor) && IsValid(MyContext.Instigator))
	{
		MyContext.TargetActor = targetActor;
		MyContext.Instigator->CombatComp->TryHurtTarget(targetActor, MyContext.SkillBaseVo->ID);
	}

	FVector ImpactPoint=SweepResult.ImpactPoint;
	 
	// 获取敌人碰撞体上距离我（子弹/火球）中心最近的点
	OtherComp->GetClosestPointOnCollision(GetActorLocation(), ImpactPoint, SweepResult.BoneName);
	
	
    auto effect=SpawnMagicEffect(this,EffectConfig.NextEffect,MyContext, ImpactPoint,UKismetMathLibrary::MakeRotFromX(SweepResult.ImpactNormal).Quaternion());
	 
	 
}

void AMagicEffect::OnFlySphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//没有设置生命的 都算碰撞消失
	if (EffectConfig.LifeSpan<=0)
	{
		SetLifeSpan(0.3f);
	}
	auto effect=SpawnMagicEffect(this,EffectConfig.NextEffect,MyContext, Hit.ImpactPoint,UKismetMathLibrary::MakeRotFromX(Hit.ImpactNormal).Quaternion());
	
}