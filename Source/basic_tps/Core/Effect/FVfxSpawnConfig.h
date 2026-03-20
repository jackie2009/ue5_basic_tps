#pragma once

#include "CoreMinimal.h"
 

#include "FVfxSpawnConfig.generated.h"
class AMagicEffect;
struct  FSkillBaseVo;
class ACombatCharacter;
 
inline TObjectPtr<USkillVisualDataAsset> GetSkillVisualDataById(FString DataID)
{
	 
	// 这里的路径必须是绝对路径
	FString AssetPath = FString::Printf(TEXT("/Game/TableDataExtra/Skills/DA_%s.DA_%s"), *DataID, *DataID);
    
	return LoadObject<USkillVisualDataAsset>(nullptr, *AssetPath);
 
}
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
};

//特效传递的数据快照
 
struct FEffectContext
{
	 

 

	// 2. 溯源信息
 
	ACombatCharacter * Instigator;
 
	ACombatCharacter * TargetActor;

	// 3. 战斗快照（防止飞行过程中属性变了）
 
	FSkillBaseVo * SkillBaseVo;
 
	
     
};