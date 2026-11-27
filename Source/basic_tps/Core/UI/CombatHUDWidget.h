#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "basic_tps/Core/Data/HeroStaminaTypes.h"
#include "CombatHUDWidget.generated.h"

class ACombatCharacter;
class AHeroCharacter;
class UBorder;
class UCanvasPanel;
class UHorizontalBox;
class UHorizontalBoxSlot;
class UImage;
class UMaterialInterface;
class UOverlay;
class UProgressBar;
class UTextBlock;
class UTexture2D;

/**
 * 战斗主 HUD 控件基类（对应 WBP_CombatCharater）：
 * - 顶部居中：敌人血条与名称（支持锁定目标与受击目标）
 * - 右上角：操作键位提示图
 * - 底部居中：玩家血条 + 金色分割线 + 怒气火焰 + 4段动态比例体力条
 */
UCLASS(Blueprintable)
class BASIC_TPS_API UCombatHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCombatHUDWidget(const FObjectInitializer& ObjectInitializer);

	// ==================== HUD 贴图与材质资源（默认指向 /Game/UI/HUD/*） ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> EnemyHealthFrameTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UMaterialInterface> EnemyHealthFillMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> ControlHintsBgTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> ControlHintsOrnamentTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> ControlHintsKeycapTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> PlayerHealthFrameTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UMaterialInterface> PlayerHealthFillMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> RageDividerTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> RageOnTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> RageOffTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> RageGlowTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> PlayerStaminaFrameTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> StaminaExhaustedTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> StaminaFatiguedTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> StaminaNormalTexture = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Assets")
	TObjectPtr<UTexture2D> StaminaVigorousTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style")
	FSlateFontInfo EnemyNameFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style")
	FSlateFontInfo HintKeyFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style")
	FSlateFontInfo HintLabelFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Hints")
	FText DodgeHintText = NSLOCTEXT("CombatHUD", "DodgeHint", "闪避");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Hints")
	FText JumpHintText = NSLOCTEXT("CombatHUD", "JumpHint", "跳跃");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Hints")
	FText JumpKeyText = NSLOCTEXT("CombatHUD", "JumpKey", "Space");

	/** 手动触发一次四段体力条比例刷新（通常每帧自动监测，无需手动调用） */
	UFUNCTION(BlueprintCallable, Category = "HUD|Stamina")
	void RefreshStaminaZoneLayout(const TArray<int32>& ZoneCapacities);

	/** 手动刷新整个 HUD 状态（便于测试或即时同步） */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshHUDFromHero(AHeroCharacter* InHero);

	/** 测试辅助：读取某个区域在 HorizontalBox 中的当前 Fill 权重 */
	UFUNCTION(BlueprintPure, Category = "HUD|Stamina")
	float GetAppliedStaminaZoneSlotWeight(EStaminaZone Zone) const;

	/** 测试辅助：读取某个区域当前进度条百分比 */
	UFUNCTION(BlueprintPure, Category = "HUD|Stamina")
	float GetAppliedStaminaZonePercent(EStaminaZone Zone) const;

	/** 读取右上角动态按键提示文本 */
	UFUNCTION(BlueprintPure, Category = "HUD|Hints")
	FString GetHintSkillQText() const;

	UFUNCTION(BlueprintPure, Category = "HUD|Hints")
	FString GetHintSkillEText() const;

	UFUNCTION(BlueprintPure, Category = "HUD|Hints")
	FString GetHintDodgeText() const;

	UFUNCTION(BlueprintPure, Category = "HUD|Hints")
	FString GetHintJumpText() const;

	/** 测试与外部查询：当前 HUD 显示的已激活怒气点数 / 下一格积累进度 / 单格状态 */
	UFUNCTION(BlueprintPure, Category = "HUD|Rage")
	int32 GetDisplayedActivatedRageCount() const { return LastAppliedFullRage; }

	UFUNCTION(BlueprintPure, Category = "HUD|Rage")
	float GetDisplayedChargingRageProgress() const { return LastAppliedChargingProgress; }

	UFUNCTION(BlueprintPure, Category = "HUD|Rage")
	bool IsRagePipActivated(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "HUD|Rage")
	bool IsRagePipCharging(int32 Index) const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void EnsureHudLayoutBuilt();
	void BuildEnemyHealthSection(UCanvasPanel* Root);
	void BuildControlHintsSection(UCanvasPanel* Root);
	void BuildPlayerStatusSection(UCanvasPanel* Root);
	void RebuildRageFlameWidgets(int32 MaxCount);
	void UpdateRageFlameDisplay(float CurrentRageValue, int32 MaxCount);
	void UpdateRageActivationFlash(float DeltaTime);
	void ApplyDesignTimePreview();

	AHeroCharacter* ResolveHeroCharacter() const;
	ACombatCharacter* ResolveEnemyTarget(AHeroCharacter* Hero) const;
	ACombatCharacter* GetBlueprintFallbackEnemyCharacter() const;

	// 动态构建的控件引用（带 UPROPERTY 避免 GC）
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> HudRootCanvas = nullptr;

	// 顶部敌人血条
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> EnemyBarContainer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> EnemyHealthFrameImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> EnemyHealthFillBar = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EnemyNameText = nullptr;

	// 右上动态键位提示（元素图 + 动态文字）
	UPROPERTY(Transient)
	TObjectPtr<UOverlay> ControlHintsContainer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> ControlHintsBgImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> ControlHintsOrnamentImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintSkillQLabelText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintSkillELabelText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintDodgeLabelText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintJumpLabelText = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHorizontalBox>> ControlHintRows;

	// 底部玩家状态容器
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> PlayerStatusContainer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> PlayerHealthFrameImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> PlayerHealthFillBar = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> RageDividerImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> RageFlamesBox = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> RageFlameImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> RageGlowImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UProgressBar>> RageChargingFillBars;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UProgressBar>> RageMiniGaugeBars;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UOverlay>> RageFlameOverlays;

	TArray<float> RageActivationFlashTimers;

	// 底部四段动态比例体力条
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> StaminaBarContainer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UImage> StaminaFrameImage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> StaminaTroughBackdrop = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> StaminaZonesBox = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UOverlay>> StaminaZoneOverlays;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHorizontalBoxSlot>> StaminaZoneSlots;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UProgressBar>> StaminaZoneBars;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> StaminaZoneDividers;

	int32 LastAppliedZoneCapacities[HeroStaminaDefaults::ZoneCount] = { -1, -1, -1, -1 };
	int32 LastAppliedMaxRage = -1;
	int32 LastAppliedFullRage = -1;
	float LastAppliedChargingProgress = -1.0f;
	int32 PreviousActivatedRageForFlash = -1;
};
