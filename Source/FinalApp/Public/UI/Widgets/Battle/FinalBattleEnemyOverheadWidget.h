#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "World/FinalBattlePresentationTypes.h"
#include "FinalBattleEnemyOverheadWidget.generated.h"

class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;
class UButton;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleEnemyOverheadWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyOverhead")
	void ApplyEnemyOverheadView(const FFinalBattleEnemyOverheadViewData& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|EnemyOverhead")
	FFinalBattleEnemyOverheadViewData GetEnemyOverheadViewData() const { return EnemyOverheadViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyOverhead")
	void RefreshBoundWidgets();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyOverhead")
	bool InspectEnemy();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|EnemyOverhead")
	void OnEnemyOverheadViewApplied(const FFinalBattleEnemyOverheadViewData& ViewData);

private:
	UFUNCTION()
	void HandleInspectButtonClicked();

	bool TryInspectEnemy() const;
	FText BuildHPText() const;
	FText BuildStatusText(const FFinalBattleOverheadStatusViewData& StatusView) const;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyOverhead")
	bool bAllowInspectOnClick = true;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyOverhead", meta = (ClampMin = "1"))
	int32 MaxVisibleStatusEntries = 6;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyOverhead")
	bool bHideShieldFrameWhenEmpty = true;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyOverhead")
	bool bHideDefeatedWidget = false;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyOverhead", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefeatedOpacity = 0.35f;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ShieldFrameBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InitiativeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IntentText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> InspectButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> StatusBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TargetedVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> DefeatedVisual;

	UPROPERTY(Transient)
	FFinalBattleEnemyOverheadViewData EnemyOverheadViewData;
};
