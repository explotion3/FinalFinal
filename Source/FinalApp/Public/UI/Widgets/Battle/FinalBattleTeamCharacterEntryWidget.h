#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleTeamCharacterEntryWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UButton;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleTeamCharacterEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Team")
	void ApplyTeamCharacterEntryView(const FFinalBattleHUDTeamCharacterEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|Team")
	FFinalBattleHUDTeamCharacterEntry GetTeamCharacterEntryViewData() const { return TeamCharacterEntryViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Team")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|Team")
	void OnTeamCharacterEntryViewApplied(const FFinalBattleHUDTeamCharacterEntry& ViewData);

private:
	UFUNCTION()
	void HandleInspectClicked();

	void EnsureWidgetTree();
	bool TryInspectCharacter() const;
	FText BuildStressText() const;
	FText BuildBreakthroughText() const;

	UPROPERTY(Transient)
	FFinalBattleHUDTeamCharacterEntry TeamCharacterEntryViewData;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Team")
	bool bAllowInspectOnClick = true;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> InspectButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> PortraitImage;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StressText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> StressBar;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BreakthroughText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakthroughBar;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UWidget> CollapsedVisual;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UWidget> BreakthroughReadyVisual;
};
