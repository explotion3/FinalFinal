#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterDetailStatusLineWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCharacterDetailStatusLineWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterDetail")
	void ApplyStatusLineView(const FFinalBattleHUDCharacterDetailStatusEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|CharacterDetail")
	FFinalBattleHUDCharacterDetailStatusEntry GetStatusLineViewData() const { return StatusLineViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterDetail")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|CharacterDetail")
	void OnStatusLineViewApplied(const FFinalBattleHUDCharacterDetailStatusEntry& ViewData);

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
	FFinalBattleHUDCharacterDetailStatusEntry StatusLineViewData;
};
