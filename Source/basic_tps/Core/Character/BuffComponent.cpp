#include "BuffComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "CombatCharacter.h"
#include "CombatComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/ICalBaseAttributes.h"
#include "basic_tps/Core/Effect/BuffLogicBase.h"
 
#include "Perception/AIPerceptionComponent.h"


UBuffComponent::UBuffComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 设置 Tick 间隔为 0.1 秒（单位是秒）
    PrimaryComponentTick.TickInterval = 0.1f;
  
}

void UBuffComponent::AddBuff(UBuffLogicBase *NewBuff) {
    if (bIsOwnerDead)return;
    if (IsValid(NewBuff->EffectRole)==false)return;
    if (NewBuff->Duration <=0)return;
    NewBuff->Owner=NewBuff->EffectRole;
    NewBuff->OwnerBuff=this;
    // 1. 同组覆盖逻辑 (类似 Unity 里的 group 查找)
    for (int32 i = BuffList.Num() - 1; i >= 0; --i) {
        if (BuffList[i]->group == NewBuff->group) {
            RemoveBuff(i);
        }
    }

    // 2. 初始化时间
    auto StartTime = GetWorld()->GetTimeSeconds();
    NewBuff->NextEffectTime = StartTime + NewBuff->tick;
    NewBuff->DieTime = (NewBuff->Duration > 0) ? (StartTime + NewBuff->Duration) : MAX_FLT;
     
   

    //创建特效
    auto Res = NewBuff->buffEffectRes;
    if (Res)
    {
 
        UClass* BuffEffectClass =Res.Get();
        if (IsValid(BuffEffectClass))
        {
            UWorld* World = GetWorld();
            FTransform SpawnTransform = FTransform::Identity;
            auto NewEffectActor = World->SpawnActorDeferred<AActor>(BuffEffectClass, SpawnTransform, GetOwner());
            if (NewEffectActor)
            {
                // 在这里可以设置 Actor 的属性 (因为是 Deferred 延迟生成)
                // NewEffectActor->SetSomeValue(100);
                NewEffectActor->FinishSpawning(SpawnTransform);
                NewBuff->View=NewEffectActor;
            }
            
        }
    
    }
    if (IsValid(NewBuff->FromRole))
    {
        NewBuff->FromRole->BuffComp->RegisterActor(NewBuff->GetClass(), Character);
    }

    //因为 是传值 所以需要最后 Add到数组
    BuffList.Add(NewBuff);
  
    // TODO: 这里触发特效 MagicRoot::CreateEffect
    CalBuffAttributes();
}
 
  

 

void UBuffComponent::BeginPlay()
{
    Super::BeginPlay();
   Character->OnCharacterDeath.AddUObject(this,&UBuffComponent::HandleOwnerDeath);
}

// ---------------- 周期性 Tick (处理 DoT) ----------------

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsOwnerDead)return;
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (int32 i = BuffList.Num() - 1; i >= 0; --i) {
        // 过期移除
        if (CurrentTime > BuffList[i]->DieTime) {
            RemoveBuff(i);
            continue;
        }

        // 处理 DoT 周期 (每 X 秒一次)
        if (BuffList[i]->tick >0) {
            if (CurrentTime >= BuffList[i]->NextEffectTime) {
                BuffList[i]->OnIntervalTick();
                if (bIsOwnerDead)return;  // 死亡跳出 
                BuffList[i]->NextEffectTime = CurrentTime + BuffList[i]->tick;
            }
        }
    }
}



void UBuffComponent::RemoveBuff(UBuffLogicBase *BuffLogic, bool bReplaceMode)
{
    // UE5 的 TArray::Find 可以代替 IndexOf
    int32 Index = BuffList.Find(BuffLogic);
    if (Index == -1) 
    {
        return;
    }
    RemoveBuff(Index,bReplaceMode);

   
}

UBuffLogicBase* UBuffComponent::GetBuff(TSubclassOf<UBuffLogicBase> BuffClass)
{
    for (int32 i = BuffList.Num() - 1; i >= 0; --i) {
        if (BuffList[i]->GetClass()== BuffClass) {
            return BuffList[i];
        }
    }
    return nullptr;
}

void UBuffComponent::RemoveBuff(int32 Index, bool bReplaceMode)
{
    if (BuffList.IsValidIndex(Index)==false) return;

    auto Buff = BuffList[Index];
    TryDestroyBuffView(Buff);
    
   if ( IsValid(Buff->FromRole))
    Buff->FromRole->BuffComp->UnregisterActor(Buff->GetClass(),  Character);
   

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
    for (UBuffLogicBase *BuffLogic : BuffList)
    {
        TryDestroyBuffView(BuffLogic);
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

 

void UBuffComponent::TryDestroyBuffView(UBuffLogicBase *BuffLogic)
{
    if (BuffLogic->View)
    {
        if (BuffLogic->View && !BuffLogic->View->IsPendingKillPending()) 
        {
           BuffLogic->View->Destroy();
          BuffLogic->View=nullptr;
        }
    }
}


void UBuffComponent::CalBuffAttributes()
{
    auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;
    if (BaseDataComp==nullptr) return;
    bIsAttackAllowed=bIsMovementAllowed=true;
   
    // 假设接口定义类名为 UICalBaseAttributes
    IICalBaseAttributes* CalcInterface = GetOwner()->FindComponentByInterface<IICalBaseAttributes>();

    if (CalcInterface)
    {
        CalcInterface->CalBaseAttributes();
    }
    
   
 
    for (auto Item : BuffList)
    {
        Item->ApplyAttributesModify();
        
    }
    
   
    AAIController* AI = Cast<AAIController>( Character ->GetController() );
    if (!(bIsMovementAllowed&&bIsAttackAllowed))
    {
       
        if (AI&&!bAIStoppedByBuff)
        {
            AI->StopMovement(); 
            AI->GetBrainComponent()->StopLogic("buff");
            Character->GetMesh()->GlobalAnimRateScale= 0.f;
        }
        bAIStoppedByBuff = true;
    }else
    {
        if (AI&&bAIStoppedByBuff)
        {
           
            AI->GetBrainComponent()->RestartLogic();
            Character->GetMesh()->GlobalAnimRateScale= 1.f;
            
        }
        bAIStoppedByBuff=false;
    }
           
  

     
   // GEngine->AddOnScreenDebugMessage(-1,5,FColor::Green,FString::Printf( TEXT("buffcount=%d"),	BuffList.Num()));

    
}
void UBuffComponent::HandleOwnerDeath(ACombatCharacter* Victim)
{
    // 逻辑：清除所有 Buff，停止 Tick
 
    bIsOwnerDead=true;
    RemoveAllBuffs();
    SetComponentTickEnabled(false);
}

void UBuffComponent::BroadcastOnTakeDamage( FCombatResult& Result)
{
     TempCombatResult=Result;
    TArray<TObjectPtr<UBuffLogicBase>> Snapshot = BuffList;
    // 内部统一管理遍历，可以方便地添加性能监控（Profiling）
    for (auto& Buff : Snapshot)
    {
        if (Buff)
        {
            
            Buff->OnTakeDamage();
        }
    }
   
    // 执行清空：直接赋值一个默认构造的结构体
    TempCombatResult = FCombatResult();
}

 void UBuffComponent::RegisterActor(TSubclassOf<UBuffLogicBase> BuffClass, ACombatCharacter* Target)
{
    RegisteredActors.FindOrAdd(BuffClass).Members.Add(Target);
}

 void UBuffComponent::UnregisterActor(TSubclassOf<UBuffLogicBase> BuffClass, ACombatCharacter* Target)
{
    if (FBuffCharacterGroup* Group = RegisteredActors.Find(BuffClass))
    {
        Group->Members.Remove(Target);
        // 如果该组空了，可以考虑清理掉 Key 节省内存
        if (Group->Members.Num() == 0) { RegisteredActors.Remove(BuffClass); }
    }
}

 TSet<ACombatCharacter*> UBuffComponent::GetActorsByBuff(TSubclassOf<UBuffLogicBase> BuffClass)
{
     auto rst= TSet< ACombatCharacter*>();
    if (auto* Group = RegisteredActors.Find(BuffClass))
    {
        for (auto Actor : Group->Members)
        {
            if (IsValid(Actor))
            {
                rst.Add(Actor);
            }
            
        }
        
    }
    return rst;
 
}