#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleEnemyDetailStatusLineWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleEnemyDetailStatusLineWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyDetail")
	void ApplyStatusLineView(const FFinalBattleHUDEnemyDetailStatusEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|EnemyDetail")
	FFinalBattleHUDEnemyDetailStatusEntry GetStatusLineViewData() const { return StatusLineViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|EnemyDetail")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|EnemyDetail")
	void OnStatusLineViewApplied(const FFinalBattleHUDEnemyDetailStatusEntry& ViewData);

private:
	void EnsureWidgetTree();
	FText BuildStackText() const;
	FText BuildDurationText() const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusNameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StackText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(Transient)
	FFinalBattleHUDEnemyDetailStatusEntry StatusLineViewData;
};
