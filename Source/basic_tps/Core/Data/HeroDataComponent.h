#pragma once

#include "CoreMinimal.h"
 
#include "CombatTypes.h"
#include "ICalBaseAttributes.h"
#include "basic_tps/Core/TableData/RoleLevelBaseVo.h"
#include "HeroDataComponent.generated.h"

UCLASS()
class BASIC_TPS_API UHeroDataComponent : public UActorComponent,public IICalBaseAttributes
{
    GENERATED_BODY()
public:
    UHeroDataComponent();
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override; 
private:
    int32 Sex = 1;
    int32 Job = 1;
    int32 Exp = 0;
    
public:
    // 外部 C++ 调这个，FORCEINLINE 保证性能几乎等同于直接访问变量
    FORCEINLINE int32 GetExp() const { return Exp; }
   
     
    FRoleLevelBaseVo *levelBaseVo;
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    void UpdateLevel(int32 level);
     
    virtual void CalBaseAttributes() override;
};


