#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "basic_tps/Core/Data/CombatTypes.h"

#include "FVfxSpawnConfig.h"
 
#include "SkillLogicBase.generated.h"


class UBuffLogicBase;
struct FEffectContext;
 
UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced)
class USkillLogicBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FCombatResult    CombatResult;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACombatCharacter> Owner;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USkillComponent> SkillComp;
    
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="技能被创建出来时 这时还没碰撞到目标"))
	void ExecuteOnStart(const  FEffectContext& EffectContext);
	
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="开始处理每个目标对象"))
 
	void ExecuteOnDealSigleTarget(const  FEffectContext& EffectContext);
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="伤害公式计算前修正参数 比如 忽视目标防御"))
	void PreDamageProcess();
	
	
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="伤害公式计算后修正结果 伤害抵扣"))
	void AdjustFinalDamage();
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="扣除目标血量后做处理 比如吸血"))
	void PostDamageProcess();

	//以下函数 不能在 start里调用 这时候还没 CombatResult
	UFUNCTION(BlueprintCallable)
	void SetSkillBaseHarm(int32 attackPoint);
	void Init(ACombatCharacter* InOwner, USkillComponent* InSkillComp);


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UAnimMontage> SkillMontage;
	
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<AMagicEffect> MagicEffectClass;

	 
	
	

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