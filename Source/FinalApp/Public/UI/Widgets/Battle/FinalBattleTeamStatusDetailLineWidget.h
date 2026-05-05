#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleTeamStatusDetailLineWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleTeamStatusDetailLineWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Team")
	void ApplyTeamStatusDetailLineView(const FFinalBattleHUDTeamStatusEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Team")
	FFinalBattleHUDTeamStatusEntry GetTeamStatusDetailLineViewData() const { return TeamStatusDetailLineViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Team")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|Team")
	void OnTeamStatusDetailLineViewApplied(const FFinalBattleHUDTeamStatusEntry& ViewData);

private:
	void EnsureWidgetTree();
	FText BuildStackText() const;
	FText BuildDurationText() const;

	UPROPERTY(Transient)
	FFinalBattleHUDTeamStatusEntry TeamStatusDetailLineViewData;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> OwnerText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusNameText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StackText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SummaryText;
};
