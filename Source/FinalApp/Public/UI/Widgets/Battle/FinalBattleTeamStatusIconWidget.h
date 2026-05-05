#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleTeamStatusIconWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleTeamStatusIconWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Team")
	void ApplyTeamStatusIconView(const FFinalBattleHUDTeamStatusEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Team")
	FFinalBattleHUDTeamStatusEntry GetTeamStatusIconViewData() const { return TeamStatusIconViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Team")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|Team")
	void OnTeamStatusIconViewApplied(const FFinalBattleHUDTeamStatusEntry& ViewData);

private:
	UFUNCTION()
	void HandleClicked();

	void EnsureWidgetTree();
	FText BuildStackText() const;

	UPROPERTY(Transient)
	FFinalBattleHUDTeamStatusEntry TeamStatusIconViewData;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> StatusButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusNameText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StackText;
};
