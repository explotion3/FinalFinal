#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleEnemyDetailWidget.generated.h"

class UButton;
class UFinalBattleEnemyDetailStatusLineWidget;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleEnemyDetailWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyDetail")
	void ApplyEnemyDetailView(const FFinalBattleHUDEnemyDetailData& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|EnemyDetail")
	FFinalBattleHUDEnemyDetailData GetEnemyDetailViewData() const { return EnemyDetailViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyDetail")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|EnemyDetail")
	void OnEnemyDetailViewApplied(const FFinalBattleHUDEnemyDetailData& ViewData);

private:
	UFUNCTION()
	void HandleCloseClicked();

	FText BuildHPText() const;
	FText BuildShieldText() const;
	FText BuildBreakText() const;
	void RefreshStatusLines();

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyDetail", meta = (ClampMin = "1"))
	int32 MaxVisibleStatusEntries = 12;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|EnemyDetail")
	TSubclassOf<UFinalBattleEnemyDetailStatusLineWidget> StatusLineWidgetClass;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ContentRoot;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EmptyText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShieldText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ShieldFrameBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BreakText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InitiativeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IntentNameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IntentDetailText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IntentText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PhaseText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RankText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TargetStateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> StatusBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EmptyStatusText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	FFinalBattleHUDEnemyDetailData EnemyDetailViewData;
};
