// BuffComponent.h
#pragma once
#include "CoreMinimal.h"
#include "CharacterComponent.h"
#include "Components/ActorComponent.h"
#include "../Data/CombatTypes.h"
#include "BuffComponent.generated.h"
USTRUCT()
struct FBuffCharacterGroup
{
	GENERATED_BODY()

	// 使用 TSet 保证成员唯一性，使用 TObjectPtr 配合 UPROPERTY 确保 GC 安全
	UPROPERTY()
	TSet<TObjectPtr<ACombatCharacter>> Members;
};

UENUM(BlueprintType)
enum class EBuffAddMode : uint8
{
	Replace,AddValue,AddTime,CustomAddFunc
};
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))

class  BASIC_TPS_API UBuffComponent : public UCharacterComponent {
	GENERATED_BODY()

public:
	UBuffComponent();
	UPROPERTY(BlueprintReadOnly)
	FCombatResult    TempCombatResult;

	UFUNCTION(BlueprintCallable)
	void AddBuff(UBuffLogicBase *NewBuff);

	 
	UFUNCTION(BlueprintCallable)
	void RemoveBuff(UBuffLogicBase *BuffLogic, bool bReplaceMode=false);
	UFUNCTION(BlueprintCallable)
	UBuffLogicBase* GetBuff(TSubclassOf<UBuffLogicBase> BuffClass);
	UFUNCTION(BlueprintCallable)
	int32  GetBuffValue(TSubclassOf<UBuffLogicBase> BuffClass);
	void RemoveBuff(int32 Index, bool bReplaceMode=false);
	
	UFUNCTION(BlueprintCallable)
	void RemoveAllBuffs();
	
	void BroadcastOnTakeDamage(FCombatResult& Result);
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsMovementAllowed=true;
	UPROPERTY(BlueprintReadWrite)
	bool bIsAttackAllowed=true;
	

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CalBuffAttributes();
	void HandleOwnerDeath(ACombatCharacter* Victim);
	
	bool bIsOwnerDead=false;

	bool bAIStoppedByBuff;
private:
	UPROPERTY()
	TArray<TObjectPtr<UBuffLogicBase>> BuffList;

	

 
	void TryDestroyBuffView(UBuffLogicBase *BuffLogic);

	//对象管理
	// 注册：Buff 启动时调用
	void RegisterActor(TSubclassOf<UBuffLogicBase> BuffClass, ACombatCharacter* Target);

	void UnregisterActor(TSubclassOf<UBuffLogicBase> BuffClass, ACombatCharacter* Target);
	// 获取：技能伤害传导时调用
	UFUNCTION(BlueprintCallable)
	TSet< ACombatCharacter*> GetActorsByBuff(TSubclassOf<UBuffLogicBase> BuffClass);
	 
private:
	UPROPERTY()
	TMap<TSubclassOf<UBuffLogicBase>, FBuffCharacterGroup> RegisteredActors;
 
};


