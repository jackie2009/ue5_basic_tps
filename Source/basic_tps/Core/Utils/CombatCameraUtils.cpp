#include "CombatCameraUtils.h"
#include "Kismet/GameplayStatics.h"

bool UCombatCameraUtils::GetCameraViewHitLocation(const UObject* WorldContextObject, float MaxRange, FVector& OutHitLocation)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    if (!World || !PC) return false;

    FVector Start;
    FRotator Rot;
    PC->GetPlayerViewPoint(Start, Rot);
    FVector End = Start + (Rot.Vector() * MaxRange);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(PC->GetPawn());

    if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        OutHitLocation = Hit.Location;
        return true;
    }
    
    OutHitLocation = End; // 没撞到也返回最远点，方便后续逻辑
    return false;
}

bool UCombatCameraUtils::ProjectLocationToGround(const UObject* WorldContextObject,float MaxRange,FVector& OutHitLocation)
{
    FVector CameraHitPoint;
   
    // 1. 利用第一个封装：找准星落点
    // 如果没撞到东西，GetCameraViewHitLocation 内部应该返回 Start + Forward * MaxRange
   if ( GetCameraViewHitLocation(WorldContextObject, MaxRange, CameraHitPoint)==false) return false;
    auto InLocation=CameraHitPoint;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    FVector End = InLocation - FVector(0, 0, 10000);
    FHitResult Hit;
    
    // 注意：这里建议只检测 WorldStatic 频道，避开人物模型
    if (World->LineTraceSingleByChannel(Hit, InLocation + FVector(0,0,1), End, ECC_WorldStatic))
    {
        OutHitLocation=Hit.ImpactPoint;
        return true;
    }

    return false; 
}