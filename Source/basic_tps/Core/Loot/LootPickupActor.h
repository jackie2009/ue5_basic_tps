#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "Loot/LootTypes.h"
#include "LootPickupActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UMaterialInstanceDynamic;
class UInventoryGridComponent;
class UBorder;
class UTextBlock;

/** 轻量内置地面名牌 Widget（非 Abstract，支持中文显示，无需额外建 WBP 即可开箱使用） */
UCLASS()
class BASIC_TPS_API ULootLabelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetLabelData(const FString& InText, const FLinearColor& InColor, int32 InFontSize = 13);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void ApplyLabelStyle();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText = nullptr;

	FString CachedText;
	FLinearColor CachedColor = FLinearColor::White;
	int32 CachedFontSize = 13;
};

/**
 * 通用地面掉落物 Actor（单 Actor 数据驱动，全游戏装备/金币共用）。
 * 支持：抛物线防重叠落地、按品质变色光柱、Screen-Space 中文名牌、金币走近自动拾取、装备按 F 或点击拾取。
 * 如需替换 3D 模型、Niagara 粒子或音效，新建蓝图子类覆盖事件即可。
 */
UCLASS()
class BASIC_TPS_API ALootPickupActor : public AActor
{
	GENERATED_BODY()

public:
	ALootPickupActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UStaticMeshComponent> BeamMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UWidgetComponent> LabelWidgetComp;

	/** 可选自定义地面名牌 WBP（留空时自动用 C++ 创建带底框与品质色的中文名牌） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Loot|UI")
	TSubclassOf<UUserWidget> CustomLabelWidgetClass;

	/** 抛物线落地动画时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Motion")
	float LaunchDuration = 0.48f;

	/** 抛物线最高点高度（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Motion")
	float LaunchArcHeight = 95.0f;

	/** 金币是否在玩家靠近时自动拾取 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Pickup")
	bool bAutoPickupGold = true;

	/** 普通/装备是否在玩家靠近时自动拾取（默认 false：按 F 或点击拾取） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Pickup")
	bool bAutoPickupItems = false;

	/** 金币自动拾取半径（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Pickup")
	float AutoPickupRadius = 150.0f;

	/** 初始化掉落数据并启动抛物线落地动画 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void InitializeLoot(const FLootDropResult& InDropResult, const FVector& InStartLocation, const FVector& InTargetLocation, float InArcHeight = 95.0f);

	/** 尝试由指定 Pawn 拾取（装备放入 PlayerController 的 UInventoryGridComponent，金币入账） */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool TryPickupBy(APawn* Picker);

	UFUNCTION(BlueprintPure, Category = "Loot")
	const FLootDropResult& GetDropResult() const { return DropResult; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool HasLanded() const { return bHasLanded; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool IsPickedUp() const { return bPickedUp; }

	/** 获取展示给玩家的地面名牌文字与颜色 */
	UFUNCTION(BlueprintPure, Category = "Loot")
	FString GetDisplayLabelText() const;

	UFUNCTION(BlueprintPure, Category = "Loot")
	FLinearColor GetDisplayColor() const;

	/** 蓝图扩展事件：掉落数据初始化完成（可挂载自定义 Niagara/音效） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Loot")
	void OnLootInitialized(const FLootDropResult& Result);

	/** 蓝图扩展事件：落地瞬间（可播放叮/金属落地音效） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Loot")
	void OnLootLanded(const FLootDropResult& Result);

	/** 蓝图扩展事件：拾取成功 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Loot")
	void OnLootPickedUp(APawn* Picker, const FLootDropResult& Result);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

private:
	void SetupVisuals();
	void SetupLabelWidget();
	UInventoryGridComponent* FindOrCreatePlayerBag(APawn* Picker) const;

	UPROPERTY(BlueprintReadOnly, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	FLootDropResult DropResult;

	FVector StartLoc = FVector::ZeroVector;
	FVector TargetLoc = FVector::ZeroVector;
	float ElapsedLaunchTime = 0.0f;
	bool bLaunching = false;
	bool bHasLanded = false;
	bool bPickedUp = false;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> VisualMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BeamMID = nullptr;
};
