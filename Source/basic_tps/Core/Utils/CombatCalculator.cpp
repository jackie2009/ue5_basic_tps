// Fill out your copyright notice in the Description page of Project Settings.
#include "CombatCalculator.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/CombatTypes.h"

#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "basic_tps/Core/Character/SkillComponent.h"

FCombatResult UCombatCalculator::CalHurtPoint(ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FSkillBaseVo& SkillVo)
{
    FCombatResult Result; // 创建结果对象
    
    if (!Attacker) return Result;

    // 1. 技能合法性检查
    int32 SkillEffectType = (int32)SkillVo.EffectType;
    bool bIsHarmSkill = (SkillEffectType > 0) || (SkillVo.Power.Num() > 0);
    if (!bIsHarmSkill) return Result;

    UCharacterDataComponent* AttackerAttr = Attacker->FindComponentByClass<UCharacterDataComponent>();
    UCharacterDataComponent* DefencerAttr = Defencer ? Defencer->FindComponentByClass<UCharacterDataComponent>() : nullptr;

    // 2. 命中/闪避判定 (假设增加 Miss 逻辑)
    // float HitChance = ... 
    // if (FMath::FRand() > HitChance) { Result.bIsMiss = true; return Result; }

    // 3. 斩杀逻辑
    if (DefencerAttr)
    {
        // int32 DeathBlow = Attacker->GetBuffExtraValue(BuffAttributeEnum::DeathBlow);
        // if (DeathBlow > 0 && (DeathBlow > (DefencerAttr->GetCurrentHP() * 100 / DefencerAttr->GetMaxHP())))
        // {
        //     Result.Damage = DefencerAttr->GetMaxHP() + 1;
        //     Result.bIsDeathBlow = true;
        //     return Result; 
        // }
    }

    // 4. 基础属性判定
    Result.bIsCritical = (FMath::FRand() * 100.f) < AttackerAttr->GetAttribute(AttributeEnum::Critical);
    if (DefencerAttr)
    {
        Result.bIsBlock = (FMath::FRand() * 100.f) < DefencerAttr->GetAttribute(AttributeEnum::Block);
    }

    // 5. 调用 MultiHurt 计算
    int32 SkillAttack =SkillVo.Power.Num()<1?0: SkillVo.Power[0]; //UGameModel::GetDynamicTableValue(AttackerAttr, SkillVo.Power);
    
    if (SkillEffectType == SkillEffectTypeEnum::DC || SkillEffectType == SkillEffectTypeEnum::MC)
    {
        // 传递 Result 引用，让内部函数去修改 Damage 和状态
        InternalCalMultiHurt(Result, Attacker, Defencer, SkillVo, SkillAttack);
    }
    else
    {
        Result.Damage = SkillAttack;
    }

    // 6. 计算吸血 (仅记录数值，不执行加血)
    if (Defencer)
    {
        //int32 LifestealPercent = Attacker->GetBuffExtraValue(EBuffAttribute::AddHpFromAttack);
      //  Result.LifestealHp = (LifestealPercent * Result.Damage) / 100;
    }

    return Result;
}



void UCombatCalculator::InternalCalMultiHurt(FCombatResult& OutResult, ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FSkillBaseVo& SkillVo, int32 SkillAttack)
{
    if (!Attacker) return;

    UCharacterDataComponent* AttackerAttr = Attacker->FindComponentByClass<UCharacterDataComponent>();
    UCharacterDataComponent* DefencerAttr = Defencer ? Defencer->FindComponentByClass<UCharacterDataComponent>() : nullptr;

    // 1. 获取基础攻击力：AttributeEnum.Damage1 + harmTypeOffset
    // harmTypeOffset 在你原版里是 skillEffectType - SkillEffectTypeEnum.DC
    int32 HarmTypeOffset = (int32)SkillVo.EffectType - (int32)SkillEffectTypeEnum::DC;
    int32 BaseDamageAttrIndex = (int32)AttributeEnum::Damage1 + HarmTypeOffset;
    int32 Attack = AttackerAttr->GetAttribute(BaseDamageAttrIndex);
    GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red,   FString::Printf(TEXT("--------------Attack:%d---%d---------------"),Attack,BaseDamageAttrIndex));

    // 2. 累加技能基础伤害
    Attack += SkillAttack;

    // 3. 暴击修正 (150%)
    if (OutResult.bIsCritical) 
    {
        Attack = Attack * 150 / 100;
    }

    // 4. 格挡修正 (25%)
    if (OutResult.bIsBlock) 
    {
        Attack = Attack * 25 / 100;
        UE_LOG(LogTemp, Log, TEXT("isBlock: true"));
    }

    if (Defencer && DefencerAttr)
    {
        // 5. 受击者 Buff 修正：额外伤害百分比与固定值
        int32 ExtraHarmPercent =0;// Defencer->GetBuffExtraValue(EBuffAttribute::ExtraHarmPercent);
        Attack += (Attack * ExtraHarmPercent) / 100;
        //Attack += Defencer->GetBuffExtraValue(EBuffAttribute::ExtraHarm);

        // 6. 防御力计算与忽略护甲
        int32 Defence = 0;
        bool bIgnoreArmor =0;// Attacker->GetBuffExtraValue(EBuffAttribute::IgnoreArmor) > 0;
        
        if (!bIgnoreArmor)
        {
            int32 DefenceAttrIndex = (int32)AttributeEnum::Defence1 + HarmTypeOffset;
            Defence = DefencerAttr->GetAttribute(DefenceAttrIndex);
        }

        // 7. 核心护甲曲线计算: attack * (1 - defence / (defence + 4 + 7 * level))
        // 注意：UE 中 GetLevel() 是 ACharacter 的标准方法
        float ArmorReduction = (float)Defence / (Defence + 4.0f + 7.0f * AttackerAttr->Level);
        Attack = FMath::TruncToInt(Attack * (1.0f - ArmorReduction));

        // 8. 受击者减伤 Buff (HarmReducePercent)
      //  float HarmReducePercent = Defencer->GetBuffExtraValue(EBuffAttribute::HarmReducePercent);
      //  float HarmReduceMultiplier = FMath::Clamp(1.0f - (HarmReducePercent * 0.01f), 0.0f, 1.0f);
      //  Attack = FMath::TruncToInt(Attack * HarmReduceMultiplier);
    }

    // 9. 攻击者最终伤害百分比修正 (FinalAttackHarmPercent)
   // int32 FinalAttackHarmPercent = Attacker->GetBuffExtraValue(EBuffAttribute::FinalAttackHarmPercent);
    //Attack += (Attack * FinalAttackHarmPercent) / 100;

    // 10. 填充结果
    OutResult.Damage = Attack;
}


