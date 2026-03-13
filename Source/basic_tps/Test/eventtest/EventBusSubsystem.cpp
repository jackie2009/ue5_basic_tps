#include "EventBusSubsystem.h"

FDelegateHandle UEventBusSubsystem::Subscribe(
	FName EventName,
	TFunction<void(const FEventBusMessage&)> Callback)
{
	FEventChannel& Channel = Channels.FindOrAdd(EventName);
	return Channel.Delegate.AddLambda(
		[Callback = MoveTemp(Callback)](const FEventBusMessage& Msg) mutable
		{
			Callback(Msg);
		});
}

FDelegateHandle UEventBusSubsystem::SubscribeOnce(
	FName EventName,
	TFunction<void(const FEventBusMessage&)> Callback)
{
	FEventChannel& Channel = Channels.FindOrAdd(EventName);

	TSharedRef<FDelegateHandle> HandleRef = MakeShared<FDelegateHandle>();
	*HandleRef = Channel.Delegate.AddLambda(
		[this, EventName, Callback = MoveTemp(Callback), HandleRef](const FEventBusMessage& Msg) mutable
		{
			Callback(Msg);
			Unsubscribe(EventName, *HandleRef);
		});

	return *HandleRef;
}

void UEventBusSubsystem::Unsubscribe(FName EventName, FDelegateHandle Handle)
{
	if (FEventChannel* Channel = Channels.Find(EventName))
	{
		Channel->Delegate.Remove(Handle);

		// 没监听者就移除频道（保持干净）
		if (!Channel->Delegate.IsBound())
		{
			Channels.Remove(EventName);
		}
	}
}

void UEventBusSubsystem::Publish(const FEventBusMessage& Message)
{
	if (FEventChannel* Channel = Channels.Find(Message.EventName))
	{
		Channel->Delegate.Broadcast(Message);
	}
}

void UEventBusSubsystem::ClearEvent(FName EventName)
{
	Channels.Remove(EventName);
}