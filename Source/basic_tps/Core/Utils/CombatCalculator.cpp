// Fill out your copyright notice in the Description page of Project Settings.
#include "CombatCalculator.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/CombatTypes.h"
 
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "basic_tps/Core/Character/BuffComponent.h"
#include "basic_tps/Core/Character/CombatComponent.h"
#include "basic_tps/Core/Character/SkillComponent.h"
#include "basic_tps/Core/Effect/BuffLogicBase.h"
#include "basic_tps/Core/Effect/SkillLogicBase.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"

FCombatResult UCombatCalculator::DamagePipeline(ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FEffectContext &  EffectContext)
{
    auto  & SkillVo= *EffectContext.SkillBaseVo;
    //修改属性 测试
    float  WeightAfterFadeoff=1.0f;
    
    if (EffectContext.SkillLogic!=nullptr&&EffectContext.SkillLogic->bEnableFalloff)
    {
        WeightAfterFadeoff=EffectContext.SkillLogic->CalWeightAfterFalloff(EffectContext.distanceToEffect/100.0f);
    
    }

    //测试内容结尾
    FCombatResult Result; // 创建结果对象
    if (!Attacker) return Result;
    Result.Attacker=Attacker;
    Result.Victim=Defencer;
    Result.SkillVo=&SkillVo;
    // 阶段 0 技能合法性检查 不是伤害类技能不走伤害流水线
    int32 SkillEffectType = SkillVo.EffectType;
    bool bIsHarmSkill = SkillEffectType > 0;
    bool isPureBuffEffect = !bIsHarmSkill;
    Result.SkillBaseHarm=SkillVo.Power.Num()>0?SkillVo.Power[0]:0;
    Result.WeightAfterFadeoff=WeightAfterFadeoff;
    // 设置 基础伤害 与创建buff 等具体逻辑蓝图执行
    if (Result.Attacker&&EffectContext.SkillLogic!=nullptr)
    {
        EffectContext.SkillLogic->CombatResult= Result;
        EffectContext.SkillLogic->ExecuteOnDealSigleTarget(EffectContext);
        Result=EffectContext.SkillLogic->CombatResult;
    

    }
    // 阶段 I：初始化快照 (Snapshot Acquisition)
    // 就算不产生伤害 也需要初始化创建buff
    CaptureAttributeSnapshot(Result,isPureBuffEffect);
    
 
    if (!bIsHarmSkill) return Result;

 
  
    
    //阶段 II：前置伤害的buff修正 忽视防御 低血量斩杀等 (Source-Side Pre-Processing)
    PreDamageProcess(Result);
    // 设置   具体逻辑蓝图 前置伤害修正
    if (Result.Attacker&&EffectContext.SkillLogic!=nullptr)
    {
        EffectContext.SkillLogic->CombatResult= Result;
        EffectContext.SkillLogic->PreDamageProcess();
        Result=EffectContext.SkillLogic->CombatResult;
    

    }
    
    // 阶段 III：核心核心公式解算 (Mathematical Evaluation)
    // 执行：WoW 除法公式、减法保底、等级压制修正
    // 输出：FinalDamage
    EvaluateCoreFormula(Result);
 
    // 阶段 IV：伤害结果修正 (伤害加深、百分比减伤、护盾抵扣)
    // 此时已经有 FinalDamage 了，Buff 可以基于这个值做乘除法
    AdjustFinalDamage(Result); 
    // 设置   具体逻辑蓝图 伤害结果修正
    if (Result.Attacker&&EffectContext.SkillLogic!=nullptr)
    {
        EffectContext.SkillLogic->CombatResult= Result;
        EffectContext.SkillLogic->AdjustFinalDamage();
        Result=EffectContext.SkillLogic->CombatResult;

    }
    


   Result.FinalDamage=FMath::RoundToInt32(Result.FinalDamage* WeightAfterFadeoff);
    
    
    // 阶段 V：伤害应用与分流 (Damage Application)
    // 执行：实际扣除 HP
    // 输出：ActualDamage (真正扣掉的血量)
    CommitActualDamage(Result);
    
    //阶段 VI：后置反馈与副作用 根据最终伤害 攻击者吸血等 (Post-Process & Feedback)
    PostDamageProcess(Result);
    // 设置   具体逻辑蓝图 后置反馈与副作用
    if (Result.Attacker&&EffectContext.SkillLogic!=nullptr)
    {
        EffectContext.SkillLogic->CombatResult= Result;
        EffectContext.SkillLogic->PostDamageProcess();
        Result=EffectContext.SkillLogic->CombatResult;

    }
    
    return Result;
}

void UCombatCalculator::CaptureAttributeSnapshot(FCombatResult& Result,bool bPureBuffEffect)
{



    if (bPureBuffEffect)return;
    
    int32 HarmTypeOffset = Result.SkillVo->EffectType - SkillEffectTypeEnum::DC;

    if (Result.Attacker)
    {
         
        int32 DamageAttrIndex =  AttributeEnum::Damage1 + HarmTypeOffset;
        Result.AttackPoint = Result.Attacker->CharacterDataComp->GetAttribute(DamageAttrIndex);

        
        int32 SkillAttack = Result.SkillBaseHarm; //UGameModel::GetDynamicTableValue(AttackerAttr, SkillVo.Power);
        Result.AttackPoint +=SkillAttack;
    }
   
    if (Result.Victim)
    {
        int32 DefenceAttrIndex =  AttributeEnum::Defence1 + HarmTypeOffset;
        Result.DefencePoint = Result.Victim->CharacterDataComp->GetAttribute(DefenceAttrIndex);
    }
   
}

void UCombatCalculator::PreDamageProcess(FCombatResult& Result)
{
    if (Result.Attacker)
    {

    
        
        Result.bIsCritical = (FMath::FRand() * 100.f) <Result.Attacker->CharacterDataComp->GetAttribute(AttributeEnum::Critical);
 
     
         
    }
    if (Result.Victim)
    {
        Result.bIsBlock = (FMath::FRand() * 100.f) < Result.Victim->CharacterDataComp->GetAttribute(AttributeEnum::Block);
    
    }
    
}

void  UCombatCalculator::EvaluateCoreFormula( FCombatResult& Result)
{
    if (Result.bIsDeathBlow)return;//斩杀 结果固定不需要计算
    UCharacterDataComponent* AttackerAttr = Result.Attacker->FindComponentByClass<UCharacterDataComponent>();
  
    // 7. 核心护甲曲线计算: attack * (1 - defence / (defence + 4 + 7 * level))
    // 注意：UE 中 GetLevel() 是 ACharacter 的标准方法
    float ArmorReduction = (float)Result.DefencePoint / (Result.DefencePoint + 4.0f + 7.0f * AttackerAttr->Level);
    Result.Damage=Result.FinalDamage = FMath::TruncToInt(Result.AttackPoint * (1.0f - ArmorReduction));

}

void UCombatCalculator::AdjustFinalDamage(FCombatResult& Result)
{
    if (Result.bIsDeathBlow)  //斩杀 结果固定不需要修正
    {
        Result.Damage=Result.FinalDamage=Result.Victim->CharacterDataComp->GetCurrentHP();
        return;
    }
       
    // 3. 暴击修正 (150%)
    if (Result.bIsCritical) 
    {
        Result.FinalDamage = Result.FinalDamage * 150 / 100;
    }

    // 4. 格挡修正 (25%)
    if (Result.bIsBlock) 
    {
         Result.FinalDamage =  Result.FinalDamage * 25 / 100;
       
    }

     
        // 8. 受击者减伤 Buff (HarmReducePercent)
        //  float HarmReducePercent = Defencer->GetBuffExtraValue(EBuffAttribute::HarmReducePercent);
        //  float HarmReduceMultiplier = FMath::Clamp(1.0f - (HarmReducePercent * 0.01f), 0.0f, 1.0f);
        //  Attack = FMath::TruncToInt(Attack * HarmReduceMultiplier);
   

    // 9. 攻击者最终伤害百分比修正 (FinalAttackHarmPercent)
    // int32 FinalAttackHarmPercent = Attacker->GetBuffExtraValue(EBuffAttribute::FinalAttackHarmPercent);
    //Attack += (Attack * FinalAttackHarmPercent) / 100;
}

void UCombatCalculator::CommitActualDamage( FCombatResult& Result)
{
    if (Result.Victim)
    {
        Result.Victim->CombatComp->HandleHurt(Result);
    }
    
}

void UCombatCalculator::PostDamageProcess( FCombatResult& Result)
{
//基本留空 预留函数 主要处理在各自技能蓝图
 
}


 

 


