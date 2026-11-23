// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatCharacter.generated.h"

class ACombatCharacter;
class UBuffComponent;
class UStaticMesh;
class USkillComponent;
class UCombatComponent;
class UCharacterDataComponent;
class UCombatTargetingComponent;
class UCharacterTargetableComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDeath, ACombatCharacter*);

// 死亡掉落武器配置：武器原本蒙皮在角色网格里（某个材质槽），死亡时隐藏该槽，并在骨骼处生成独立物理武器
USTRUCT(BlueprintType)
struct FDeathDropWeapon
{
	GENERATED_BODY()

	// 掉落时生成的静态网格
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	// 生成位置所跟随的骨骼/插槽（例如 ik_hand_gun / ik_hand_shield / hand_r）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FName Bone = NAME_None;

	// 相对骨骼的偏移（用于对齐网格枢轴）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	FTransform Offset = FTransform::Identity;
};

UCLASS()
class BASIC_TPS_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatCharacter();
	FOnCharacterDeath OnCharacterDeath;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void ExecutePhysicsDisabled();
	
	

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Targeting")
	TObjectPtr<UCombatTargetingComponent> TargetingComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Targeting")
	TObjectPtr<UCharacterTargetableComponent> TargetableComp;

		

	
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCharacterDataComponent> CharacterDataComp = nullptr;
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatComponent> CombatComp = nullptr;
	
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkillComponent> SkillComp = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBuffComponent> BuffComp = nullptr;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const;

	// 死亡时要隐藏的角色网格材质槽（武器做在网格里时用，例如骷髅填 1）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Death")
	TArray<int32> DeathHideMaterialSlots;

	// 死亡时生成的独立物理武器
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Death")
	TArray<FDeathDropWeapon> DeathDropWeapons;

	// 死亡掉落武器：隐藏 DeathHideMaterialSlots，生成 DeathDropWeapons；
	// 另外，带 DropTag 标签的已有组件也会被分离并开启物理。
	// 在蓝图里与 Mesh 的 SetSimulatePhysics(true) 同一时刻调用。
	UFUNCTION(BlueprintCallable, Category = "Combat|Death")
	void DropWeaponsOnDeath(float ExtraImpulse = 150.f, FName DropTag = TEXT("DropOnDeath"));

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	virtual void SelfOnHurt(float Damage, FVector HitLocation)  ;
	virtual void SelfOnDead()  ;
	
	
	
	// 定义一个给蓝图用的事件：受伤
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnHurt(float Damage, FVector HitLocation);

	// 定义一个给蓝图用的事件：死亡
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnDead();


	UPROPERTY( BlueprintReadWrite, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> CurrentWeaponMesh = nullptr;
};
