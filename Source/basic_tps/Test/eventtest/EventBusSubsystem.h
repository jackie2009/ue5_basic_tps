#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EventBusSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FEventBusMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName EventName = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> Sender = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int32 IntValue = 0;

	UPROPERTY(BlueprintReadWrite)
	FString StringValue;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FEventBusNativeDelegate, const FEventBusMessage&);

UCLASS()
class BASIC_TPS_API UEventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 常规监听
	FDelegateHandle Subscribe(FName EventName, TFunction<void(const FEventBusMessage&)> Callback);

	// 一次性监听
	FDelegateHandle SubscribeOnce(FName EventName, TFunction<void(const FEventBusMessage&)> Callback);

	// 取消监听
	void Unsubscribe(FName EventName, FDelegateHandle Handle);

	// 派发事件
	void Publish(const FEventBusMessage& Message);

	// 清空某个事件频道的所有监听
	void ClearEvent(FName EventName);

private:
	struct FEventChannel
	{
		FEventBusNativeDelegate Delegate;
	};

	TMap<FName, FEventChannel> Channels;
};