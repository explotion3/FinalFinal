#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterDetailWidget.generated.h"

class UButton;
class UFinalBattleCharacterDetailPassiveLineWidget;
class UFinalBattleCharacterDetailStatusLineWidget;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCharacterDetailWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterDetail")
	void ApplyCharacterDetailView(const FFinalBattleHUDCharacterDetailData& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|CharacterDetail")
	FFinalBattleHUDCharacterDetailData GetCharacterDetailViewData() const { return CharacterDetailViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterDetail")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|CharacterDetail")
	void OnCharacterDetailViewApplied(const FFinalBattleHUDCharacterDetailData& ViewData);

private:
	UFUNCTION()
	void HandleCloseClicked();

	FText BuildStressText() const;
	FText BuildBreakthroughText() const;
	FText BuildVitalText() const;
	FText BuildAwakenText() const;
	FText BuildCollapseText() const;
	FText BuildGrowthText() const;
	FText BuildRuntimeStatsText() const;
	FText BuildUltimateCostText() const;
	FText BuildUltimateStateText() const;
	void RefreshStatusLines();
	void RefreshPassiveLines();

	UPROPERTY(EditAnywhere, Category = "Final|Battle|CharacterDetail", meta = (ClampMin = "1"))
	int32 MaxVisibleStatusEntries = 12;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|CharacterDetail", meta = (ClampMin = "1"))
	int32 MaxVisiblePassiveEntries = 12;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|CharacterDetail")
	TSubclassOf<UFinalBattleCharacterDetailStatusLineWidget> StatusLineWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|CharacterDetail")
	TSubclassOf<UFinalBattleCharacterDetailPassiveLineWidget> PassiveLineWidgetClass;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ContentRoot;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EmptyText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StressText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> StressBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BreakthroughText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakthroughBar;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> VitalText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AwakenText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CollapseText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GrowthText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RuntimeStatsText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> StatusBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EmptyStatusText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> PassiveBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EmptyPassiveText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> UltimateNameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> UltimateCostText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> UltimateStateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> UltimateRulesText;

	UPROPERTY(Transient)
	FFinalBattleHUDCharacterDetailData CharacterDetailViewData;
};
