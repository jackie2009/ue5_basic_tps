#pragma once

#include "CoreMinimal.h"
 

#include "FVfxSpawnConfig.generated.h"
class USkillLogicBase;
class AMagicEffect;
struct  FSkillBaseVo;
class ACombatCharacter;
 
 
// 定义特效生成的空间规则
UENUM(BlueprintType)
enum class EVfxSpawnSpace : uint8
{
	AttachToInstigator  UMETA(DisplayName = "挂载在释放者身上"),
	AttachToVictim      UMETA(DisplayName = "挂载在承受者身上"),
	WorldSpace          UMETA(DisplayName = "世界空间(不跟随,需设置坐标)"),
	WorldSpaceInstigator          UMETA(DisplayName = "世界空间(不跟随,用释放者做初始位置)"),
	WorldSpaceVictim          UMETA(DisplayName = "世界空间(不跟随,用承受者做初始位置)"),
	CameraViewHitLocation  UMETA(DisplayName = "鼠标射线点(世界空间)"),
	CameraViewHitGroundLocation  UMETA(DisplayName = "鼠标射线点对应的地面点(世界空间)"),
};

UENUM(BlueprintType)
enum class EHitFirstTargetHandle : uint8
{
	Keep     UMETA(DisplayName = "保持继续"),
	Disable       UMETA(DisplayName = "仅关闭碰撞"),
	Destroy     UMETA(DisplayName = "关闭碰撞与渲染")
	
};
UENUM(BlueprintType)
enum class ECreateChildMode : uint8
{
	None     UMETA(DisplayName = "不创建新特效"),
	Hit     UMETA(DisplayName = "碰撞时创建新特效"),
	Notify       UMETA(DisplayName = "SpawnFlyVfxNotify时创建新特效")
};


//技能创建的特效配置类型
USTRUCT(BlueprintType)
struct  FSkillVfxConfig
{
	GENERATED_BODY()

public:

  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	USoundBase* Sound;

 
	
	//   生成的空间规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	EVfxSpawnSpace SpawnSpace = EVfxSpawnSpace::WorldSpace;

	//  挂载点名字 (如果是 Attach 类型)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	FName SocketName = NAME_None;

	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config",meta = (DisplayName = "初始化时朝向与跟随者一致"))
	bool InitRotationWithSpaceActor = true;
	//  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config",meta = (DisplayName = "朝向与跟随者始终保持一致"))
	bool AlwaysRotationWithSpaceActor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config",meta = (DisplayName = "命中目标会造成伤害"))
	bool HurtTargetWhenHit=true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config",meta = (DisplayName = "命中首个目标处理"))
	EHitFirstTargetHandle HitFirstTarget;

	//   生命周期 (如果为 0 则由蓝图内部逻辑销毁)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	float LifeSpan = 0.5f;

	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Chain")
	ECreateChildMode ChildMode = ECreateChildMode::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Chain" ,meta = (EditCondition = "ChildMode != ECreateChildMode::None", EditConditionHides))
	TSubclassOf<AMagicEffect>  NextEffect;
	 
	
};
//技能表视觉字段的配置类型
UCLASS(BlueprintType)
class BASIC_TPS_API USkillVisualDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UAnimMontage> SkillMontage;
	
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<AMagicEffect> MagicEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<USkillLogicBase> SkillLogicBase;
	
	

	//衰减配置
	//  开关（控制下面所有字段显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Falloff", meta=(DisplayName="启用衰减配置"))
	bool bEnableFalloff = false;

	// 最大影响范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Falloff",
		meta=(DisplayName="最大影响范围",EditCondition="bEnableFalloff", EditConditionHides))
	float Radius = 15.0;
	// 满伤范围（内圈不衰减）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Falloff",
		meta=(DisplayName="满伤范围",EditCondition="bEnableFalloff", EditConditionHides))
	float FullDamageRadius = 4.0f;

	// 衰减指数（曲线形状）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Falloff",
		meta=(DisplayName="衰减指数",EditCondition="bEnableFalloff", EditConditionHides))
	float FalloffExponent = 1.0f;


 

	float CalWeightAfterFalloff(float DisToCenter) const
	{
		// 没开启衰减 → 直接满值
		if (!bEnableFalloff)
		{
			return 1.f;
		}

		// 超出最大范围 → 无效果
		if (DisToCenter >= Radius)
		{
			return 0.f;
		}

		// 在满伤范围内 → 不衰减
		if (DisToCenter <= FullDamageRadius)
		{
			return 1.f;
		}

		// 防止除0
		float FalloffRange = Radius - FullDamageRadius;
		if (FalloffRange <= KINDA_SMALL_NUMBER)
		{
			return 1.f;
		}

		// 归一化 [0,1]
		float Alpha = (DisToCenter - FullDamageRadius) / FalloffRange;
		Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

		// 反向（越远越小）
		float Weight = 1.f - Alpha;

		// 指数控制曲线
		Weight = FMath::Pow(Weight, FalloffExponent);
		return Weight;
	}


	
};

//特效传递的数据快照
USTRUCT(BlueprintType, Category = "MagicEffect")
struct FEffectContext
{
	 
	GENERATED_BODY()

public:
 
	// 结构体内部建议这样改
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagicEffect")
	TSet<AActor*> IgnoreTargetSet;
	 
	//创建的第几代 用来计算分裂发射次数控制 与伤害衰减等
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	int Generation=0;
	
	// 2. 溯源信息
	// 加上 UPROPERTY 才能在蓝图中显示
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	ACombatCharacter * Instigator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	ACombatCharacter * TargetActor;

	// 3. 战斗快照（防止飞行过程中属性变了）
 
	FSkillBaseVo * SkillBaseVo;
	
	UPROPERTY()
	TObjectPtr<USkillVisualDataAsset> SkillVisualDataAsset = nullptr;
	UPROPERTY()
	TObjectPtr<USkillLogicBase> SkillLogic=nullptr;
	float distanceToEffect;
     
};