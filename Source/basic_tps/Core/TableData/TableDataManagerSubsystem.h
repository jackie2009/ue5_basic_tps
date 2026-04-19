// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
 

#include "Subsystems/GameInstanceSubsystem.h"
#include "TableDataManagerSubsystem.generated.h"
struct FWeaponTypeBaseVo;
struct FMonsterBaseVo;
struct FBuffBaseVo;
struct FRoleLevelBaseVo;
struct FSkillBaseVo;
struct FRoleBaseVo;
/**
 * 
 */
UCLASS()
class BASIC_TPS_API UTableDataManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 逻辑入口：初始化
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// 逻辑出口：清理静态引用，防止内存悬空
	virtual void Deinitialize() override;
	FORCEINLINE static UTableDataManagerSubsystem* Get()
	{
		// 这里可以加一个 ensure 或 check，确保在 Subsystem 初始化后调用
		return Instance;
	}
	
	// 数据字典缓存
	 
	 TMap<int32, FRoleBaseVo*> RoleBaseMap;
	 TMap<int32, FRoleLevelBaseVo*> RoleLevelBaseMap;
	 TMap<int32, FMonsterBaseVo*> MonsterBaseMap;
	 
	 TMap<int32, FWeaponTypeBaseVo*> WeaponTypeBaseMap;
	 TMap<int32, TArray<FSkillBaseVo*>> SkillBaseMap;
 
	// 泛型 ID 列表查数据工具
	template<typename T>
	TArray<T*> GetListFromIDs(const TMap<int32, T*>& Collection, const TArray<int32>& IDList) const
	{
		TArray<T*> Result;
		for (int32 ID : IDList)
		{
			if (T* const* Ptr = Collection.Find(ID))
			{
				Result.Add(*Ptr);
			}
		}
		return Result;
	}

 

private:
	
	static UTableDataManagerSubsystem* Instance;
	// 保持 DataTable 的强引用，防止被 GC 导致 Map 里的指针悬空
	UPROPERTY()
	TArray<UDataTable*> LoadedTables;

	// 模板加载函数：单行映射
	template<typename TVo>
	void LoadTableToMap(const FString& Path, TMap<int32, TVo*>& OutMap);

	// 模板加载函数：字符串Key映射
	template<typename TVo>
	void LoadTableToStringMap(const FString& Path, TMap<FString, TVo*>& OutMap);

	// 模板加载函数：分组映射 (对应 C# 的 readMapList)
	template<typename TVo>
	void LoadTableToGroupMap(const FString& Path, TMap<int32, TArray<TVo*>>& OutGroupMap);
};