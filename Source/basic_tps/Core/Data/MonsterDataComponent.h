#pragma once

#include "CoreMinimal.h"
 
#include "CombatTypes.h"
#include "basic_tps/Core/TableData/RoleLevelBaseVo.h"
#include "MonsterDataComponent.generated.h"

struct FMonsterBaseVo;

UCLASS()
class BASIC_TPS_API UMonsterDataComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMonsterDataComponent();
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override; 
private:
 
    
public:
   
     
    FMonsterBaseVo *baseVo;
    void InitWithBaseID(int32 BaseID);
   

};


