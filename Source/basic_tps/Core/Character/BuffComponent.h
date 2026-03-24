// BuffComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Data/CombatTypes.h"
#include "BuffComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class  BASIC_TPS_API UBuffComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UBuffComponent();

	// 添加 Buff
	void AddBuff(FBuffVo NewBuff);

 
	int32 GetBuffValue(int32 BuffAttType) const;

	int32 GetBuffValue(EBuffAttribute BuffType, TSharedPtr<FBuffVo> WorkingBuffVo);
	
	

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CalBuffAttributes();
private:
	UPROPERTY()
	TArray<FBuffVo> BuffList;

	void RemoveBuff(const FBuffVo& BuffVo, bool bReplaceMode=false);
	void RemoveBuff(int32 Index, bool bReplaceMode=false);
	void RemoveAllBuffs();
	void ExecuteDoT(FBuffVo& Buff);
};