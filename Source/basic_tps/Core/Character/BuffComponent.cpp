#include "BuffComponent.h"

#include "CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"


UBuffComponent::UBuffComponent()
{
}

void UBuffComponent::AddBuff(FBuffVo NewBuff) {
    if (NewBuff.Duration <=0)return;
    // 1. 同组覆盖逻辑 (类似 Unity 里的 group 查找)
    for (int32 i = BuffList.Num() - 1; i >= 0; --i) {
        if (BuffList[i].BaseVo->group == NewBuff.BaseVo->group) {
            RemoveBuff(i);
        }
    }

    // 2. 初始化时间
    auto StartTime = GetWorld()->GetTimeSeconds();
    NewBuff.DieTime = (NewBuff.Duration > 0) ? (StartTime + NewBuff.Duration) : MAX_FLT;
     
    BuffList.Add(NewBuff);
    
    // TODO: 这里触发特效 MagicRoot::CreateEffect
}

// ---------------- 核心流水线 ----------------
// 阶段 A：伤害前修正 (斩杀 无视防御 等 buff影响伤害值计算)
void UBuffComponent::PreDamageProcess(FCombatResult& Package) {
    for (auto& Buff : BuffList) {
            if (Buff.BaseVo->attribute ==static_cast<int32>(EBuffAttribute::Execution)) {
                // 修改 Package 里的 FinalDamage 逻辑 
                Package.FinalDamage = 9999999; 
            }
        }
    
}

// 阶段 B：伤害后反馈 (计算吸血等 依赖本次伤害量)
void UBuffComponent::PostDamageProcess(const FCombatResult& Package) {
    for (auto& Buff : BuffList) {
         
            // 示例：如果是吸血 Buff
            if (Buff.BaseVo->attribute ==static_cast<int32>(EBuffAttribute::LifeSteal)) {
                // 根据实际伤害 Package.ActualDamage 回血
                int32 HealAmount = Package.FinalDamage * Buff.Value/100;
             
                if (Package.Attacker!=nullptr)
                {
                    Package.Attacker->CharacterDataComp->AddCurrentHP(HealAmount);
                }
            }
         
    }
}

int32 UBuffComponent::GetBuffValue(int32 BuffAttType) const
{
    int32 TotalValue = 0;

    // 使用 const 自动推导迭代器，保证读取安全
    for (const FBuffVo& Item : BuffList)
    {
        // 假设 FBuffVo 结构体里包含 BaseVo 指针或直接有 Attribute 字段
        // 这里根据你之前的结构体定义来匹配
        if (Item.BaseVo && Item.BaseVo->attribute == BuffAttType)
        {
            TotalValue += Item.Value;
        }
    }
 
    return TotalValue;
}

void UBuffComponent::BeginPlay()
{
    Super::BeginPlay();
}

// ---------------- 周期性 Tick (处理 DoT) ----------------

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (int32 i = BuffList.Num() - 1; i >= 0; --i) {
        // 过期移除
        if (CurrentTime > BuffList[i].DieTime) {
            RemoveBuff(i);
            continue;
        }

        // 处理 DoT 周期 (每 X 秒一次)
        if (BuffList[i].BaseVo->tick >0) {
            if (CurrentTime >= BuffList[i].NextEffectTime) {
                ExecuteDoT(BuffList[i]);
                BuffList[i].NextEffectTime = CurrentTime + BuffList[i].BaseVo->tick;
            }
        }
    }
}

void UBuffComponent::RemoveBuff(const FBuffVo& BuffVo, bool bReplaceMode)
{
    // UE5 的 TArray::Find 可以代替 IndexOf
    int32 Index = BuffList.Find(BuffVo);
    if (Index == -1) 
    {
        return;
    }
    RemoveBuff(Index,bReplaceMode);

   
}

void UBuffComponent::RemoveBuff(int32 Index, bool bReplaceMode)
{
    if (BuffList.IsValidIndex(Index)==false) return;
 

    
    // 打印调试信息 (UE_LOG 代替 print)
    UE_LOG(LogTemp, Log, TEXT("RemoveBuff: %d, %s"), BuffList[Index].BaseID, *GetOwner()->GetName());

    // 从数组中移除
    BuffList.RemoveAt(Index);

    

    // 事件分发 (假设你使用的是 UE 的 Delegate 或自定义事件系统)
    // 这里演示全局事件分发的写法
    //  if (UEventDispatcher* Dispatcher = UEventDispatcher::Get())
    //   {
    //     Dispatcher->DispatchEvent(EGameEvents::OnBuffRemove, GetOwner(), BuffVo, bReplaceMode);
    // }
}
void UBuffComponent::RemoveAllBuffs()
{
    // 遍历当前的 Buff 列表并发送移除事件
    for (const FBuffVo& BuffVo : BuffList)
    {
       // if (UEventDispatcher* Dispatcher = UEventDispatcher::Get())
       // {
            // replaceMode 在全部移除时通常为 false
         //   Dispatcher->DispatchEvent(EGameEvents::OnBuffRemove, GetOwner(), BuffVo, false);
        //}
    }

    // 清空整个 TArray
    BuffList.Empty(); 
}

void UBuffComponent::ExecuteDoT(FBuffVo& Buff)
{
    
}
