// Fill out your copyright notice in the Description page of Project Settings.


#include "TableDataManagerSubsystem.h"

#include "BuffBaseVo.h"
#include "SkillBaseVo.h"
#include "RoleBaseVo.h"
#include "RoleLevelBaseVo.h"
#include "WeaponTypeBaseVo.h"
#include "MonsterBaseVo.h"
 


// 声明固定路径常量
 

static const FString RoleBasePath = TEXT("/Game/TableData/RoleBaseVo.RoleBaseVo");
static const FString RoleLevelBasePath = TEXT("/Game/TableData/RoleLevelBaseVo.RoleLevelBaseVo");
static const FString MonsterBasePath = TEXT("/Game/TableData/MonsterBaseVo.MonsterBaseVo");
static const FString BuffBasePath = TEXT("/Game/TableData/BuffBaseVo.BuffBaseVo");
 static const FString WeaponTypeBasePath = TEXT("/Game/TableData/WeaponTypeBaseVo.WeaponTypeBaseVo");
 
static const FString SkillBasePath = TEXT("/Game/TableData/SkillBaseVo.SkillBaseVo");

void UTableDataManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
 
    GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("UTableDataManagerSubsystem"));
    // 同步加载所有表格

    LoadTableToMap<FRoleBaseVo>(RoleBasePath, RoleBaseMap);
    LoadTableToMap<FRoleLevelBaseVo>(RoleLevelBasePath, RoleLevelBaseMap);
    LoadTableToMap<FMonsterBaseVo>(MonsterBasePath, MonsterBaseMap);
    LoadTableToMap<FBuffBaseVo>(BuffBasePath, BuffBaseMap);

    LoadTableToMap<FWeaponTypeBaseVo>(WeaponTypeBasePath, WeaponTypeBaseMap);


    LoadTableToGroupMap<FSkillBaseVo>(SkillBasePath, SkillBaseMap);
}

 

template<typename TVo>
void UTableDataManagerSubsystem::LoadTableToMap(const FString& Path, TMap<int32, TVo*>& OutMap)
{
    if (UDataTable* Table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path)))
    {
        LoadedTables.Add(Table);
        TArray<TVo*> AllRows;
        Table->GetAllRows<TVo>(TEXT("LoadTable"), AllRows);
        for (TVo* Row : AllRows)
        {
            if (Row) OutMap.Add(Row->ID, Row);
        }
    }
}

template<typename TVo>
void UTableDataManagerSubsystem::LoadTableToStringMap(const FString& Path, TMap<FString, TVo*>& OutMap)
{
    if (UDataTable* Table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path)))
    {
        LoadedTables.Add(Table);
        TArray<TVo*> AllRows;
        Table->GetAllRows<TVo>(TEXT("LoadTable"), AllRows);
        for (TVo* Row : AllRows)
        {
            if (Row) OutMap.Add(Row->ID, Row); // 这里的 ID 字段应为 FString 类型
        }
    }
}

template<typename TVo>
void UTableDataManagerSubsystem::LoadTableToGroupMap(const FString& Path, TMap<int32, TArray<TVo*>>& OutGroupMap)
{
    if (UDataTable* Table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path)))
    {
        LoadedTables.Add(Table);
        TArray<TVo*> AllRows;
        Table->GetAllRows<TVo>(TEXT("LoadTable"), AllRows);
        for (TVo* Row : AllRows)
        {
            if (Row) OutGroupMap.FindOrAdd(Row->ID).Add(Row);
        }
    }
}