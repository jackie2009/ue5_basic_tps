#pragma once

#include "CoreMinimal.h"
 
#include "FVfxSpawnConfig.generated.h"
class AMagicEffect;
struct  FSkillBaseVo;
class ACombatCharacter;
inline TSoftObjectPtr<USkillVfxDataAsset> GetVfxDataById(FString VfxResID)
{
	 
	// 这里的路径必须是绝对路径
	FString AssetPath = FString::Printf(TEXT("/Game/VFX/Skills/DA_%s.DA_%s"), *VfxResID, *VfxResID);
    
	// 2. 返回软引用
	return TSoftObjectPtr<USkillVfxDataAsset>(FSoftObjectPath(AssetPath));
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
};

UENUM(BlueprintType)
enum class EVfxObjectType : uint8
{
	Niagara     UMETA(DisplayName = "Niagara 粒子"),
	Emitter     UMETA(DisplayName = "Cascade 传统粒子"),
	Actor       UMETA(DisplayName = "Actor 蓝图/逻辑物件")
};
UENUM(BlueprintType)
enum class ECreateChildMode : uint8
{
	None     UMETA(DisplayName = "不创建新特效"),
	Hit     UMETA(DisplayName = "碰撞时创建新特效"),
	Notify       UMETA(DisplayName = "SpawnFlyVfxNotify时创建新特效")
};

UCLASS(BlueprintType)
class BASIC_TPS_API USkillVfxDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	// 1. 选择生成类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	EVfxObjectType ObjectType = EVfxObjectType::Niagara;

	// 2. Niagara 系统：仅在选择 Niagara 时显示
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (EditCondition = "ObjectType == EVfxObjectType::Niagara", EditConditionHides))
	class UNiagaraSystem* NiagaraSystem;

	// 3. Cascade 粒子：仅在选择 Emitter 时显示
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (EditCondition = "ObjectType == EVfxObjectType::Emitter", EditConditionHides))
	class UParticleSystem* CascadeParticle;

	// 4. Actor 蓝图：仅在选择 Actor 时显示
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (EditCondition = "ObjectType == EVfxObjectType::Actor", EditConditionHides))
	TSubclassOf<AActor> ActorClass;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSubclassOf<AMagicEffect>  ContainerClass;
	
	//   生成的空间规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	EVfxSpawnSpace SpawnSpace = EVfxSpawnSpace::WorldSpace;

	//  挂载点名字 (如果是 Attach 类型)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	FName SocketName = NAME_None;

	//   相对偏移 (相对于 Socket 或 Actor 中心)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	FVector RelativeLocation = FVector::ZeroVector;

	//  相对旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	//   缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	FVector RelativeScale = FVector::OneVector;

	//   生命周期 (如果为 0 则由蓝图内部逻辑销毁)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	float LifeSpan = 0.0f;

	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Chain")
	ECreateChildMode ChildMode = ECreateChildMode::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Chain" ,meta = (EditCondition = "ChildMode != ECreateChildMode::None", EditConditionHides))
	TSoftObjectPtr<USkillVfxDataAsset> NextEffect;
	 
	
};

//特效传递的数据快照
USTRUCT(BlueprintType)
struct FEffectContext
{
	GENERATED_BODY()

	// 1. 核心资源配置（你的 DataAsset）
 
	USkillVfxDataAsset * VfxConfig;

	// 2. 溯源信息
	 
	ACombatCharacter * Instigator;
	ACombatCharacter * TargetActor;

	// 3. 战斗快照（防止飞行过程中属性变了）
	 
	FSkillBaseVo * SkillBaseVo;
     
};