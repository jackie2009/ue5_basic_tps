#include "BuffComponent.h"

#include "CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/ICalBaseAttributes.h"


UBuffComponent::UBuffComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 设置 Tick 间隔为 0.1 秒（单位是秒）
    PrimaryComponentTick.TickInterval = 0.1f;
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

 

      CalBuffAttributes();
   
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

int32 UBuffComponent::GetBuffValue(EBuffAttribute BuffType, TSharedPtr<FBuffVo> WorkingBuffVo)
{
    int32 TotalValue = 0;
    if (WorkingBuffVo!=nullptr&&WorkingBuffVo->BaseVo->attribute == static_cast<int32> (BuffType))
    {
        TotalValue+= WorkingBuffVo->Value;
    }
    TotalValue+=GetBuffValue( static_cast<int32> (BuffType));
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

    CalBuffAttributes();
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
    CalBuffAttributes();
    
}

void UBuffComponent::ExecuteDoT(FBuffVo& Buff)
{
    
}


void UBuffComponent::CalBuffAttributes()
{
    auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;
    if (BaseDataComp==nullptr) return;
    // 假设接口定义类名为 UICalBaseAttributes
    IICalBaseAttributes* CalcInterface = GetOwner()->FindComponentByInterface<IICalBaseAttributes>();

    if (CalcInterface)
    {
        CalcInterface->CalBaseAttributes();
    }
    for (int i = 1; i < AttributeEnum::MAX; i++)
    {
                
        auto addValue= GetBuffValue(i);
        BaseDataComp->Attributes[i] += addValue;
       
                
    }

           
  

    BaseDataComp->SetCurrentHP( BaseDataComp->GetCurrentHP());
 

    
}