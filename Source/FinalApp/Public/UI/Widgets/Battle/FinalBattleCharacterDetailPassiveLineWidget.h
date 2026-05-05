#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterDetailPassiveLineWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCharacterDetailPassiveLineWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterDetail")
	void ApplyPassiveLineView(const FFinalBattleHUDCharacterDetailPassiveEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|CharacterDetail")
	FFinalBattleHUDCharacterDetailPassiveEntry GetPassiveLineViewData() const { return PassiveLineViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterDetail")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|CharacterDetail")
	void OnPassiveLineViewApplied(const FFinalBattleHUDCharacterDetailPassiveEntry& ViewData);

private:
	void EnsureWidgetTree();
	FText BuildStackText() const;
	FText BuildDurationText() const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PassiveNameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StackText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(Transient)
	FFinalBattleHUDCharacterDetailPassiveEntry PassiveLineViewData;
};
