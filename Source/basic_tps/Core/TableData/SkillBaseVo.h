#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillBaseVo.generated.h"

// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FSkillBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 ID;                  // 技能序号
	UPROPERTY(EditAnywhere)
	FString MagName;           // 技能名字
	UPROPERTY(EditAnywhere)
	int32 aiPriority;          // ai选择优先级
	UPROPERTY(EditAnywhere)
	TArray<int32> needWeapon;  // 武器需求
	UPROPERTY(EditAnywhere)
	int32 needWeaponEffect;    // 武器需求影响
	UPROPERTY(EditAnywhere)
	int32 level;               // 技能等级
	UPROPERTY(EditAnywhere)
	int32 effectLevel;         // buff有效等级
	UPROPERTY(EditAnywhere)
	int32 TargetType;          // 0敌方 1自己 2友方
	UPROPERTY(EditAnywhere)
	int32 GroupTargets;        // 群体目标
	UPROPERTY(EditAnywhere)
	int32 isTargetMousePos;    // 释放目标用鼠标位置
		UPROPERTY(EditAnywhere)
	int32 isFirstHitOnly;      // 第一个碰撞为伤害对象
	UPROPERTY(EditAnywhere)
	int32 allowMiss;           // 允许miss
	UPROPERTY(EditAnywhere)
	int32 EffectType;          // 效果类型
	UPROPERTY(EditAnywhere)
	int32 animation;           // 技能动画
	UPROPERTY(EditAnywhere)
	int32 Spell;               // 每次使用技能使用的魔法值
	UPROPERTY(EditAnywhere)
	TArray<int32> Power;       // 技能的伤害值参数
	UPROPERTY(EditAnywhere)
	int32 needLevel;           // 技能升级需要玩家等级
	UPROPERTY(EditAnywhere)
	int32 needExp;             // 技能升级需要技能经验
	UPROPERTY(EditAnywhere)
	int32 distanceTarget;      // 释放距离
	UPROPERTY(EditAnywhere)
	int32 CD;                  // 冷却延迟
	UPROPERTY(EditAnywhere)
	int32 isBuffForSelf;       // buff是否给自己
	UPROPERTY(EditAnywhere)
	int32 buffID;              // 产生的buff ID
	UPROPERTY(EditAnywhere)
	int32 buffRandom;          // buff作用成功率
	UPROPERTY(EditAnywhere)
	int32 buffLife;    // buff持续时间
	UPROPERTY(EditAnywhere)
	TArray<int32> buffValue;   // buff属性值
	UPROPERTY(EditAnywhere)
	FString icon;              // 图标
	UPROPERTY(EditAnywhere)
	FString desc;              // 描述
	UPROPERTY(EditAnywhere)
	TArray<int32> extraValues; // 特殊自定义参数

};