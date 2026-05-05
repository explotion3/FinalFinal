#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterEntryWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCharacterEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void Configure(const FFinalBattleHUDCharacterEntry& InEntry);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterEntry")
	void ApplyCharacterEntryView(const FFinalBattleHUDCharacterEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle|CharacterEntry")
	FFinalBattleHUDCharacterEntry GetCharacterEntryViewData() const { return CharacterEntryViewData; }

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|CharacterEntry")
	void RefreshBoundWidgets();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle|CharacterEntry")
	void OnCharacterEntryViewApplied(const FFinalBattleHUDCharacterEntry& ViewData);

private:
	UFUNCTION()
	void HandleInspectClicked();

	void EnsureWidgetTree();
	bool TryInspectCharacter() const;
	FText BuildFallbackLabel() const;
	FText BuildLevelText() const;
	FText BuildStressText() const;
	FText BuildBreakthroughText() const;
	FText BuildStatusText() const;
	void RefreshStatusBox();

	UPROPERTY(Transient)
	FFinalBattleHUDCharacterEntry CharacterEntryViewData;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|CharacterEntry")
	bool bAllowInspectOnClick = true;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UWidget> ContentRoot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> InspectButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> PortraitImage;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StressText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> StressBar;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BreakthroughText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakthroughBar;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakthroughProgressBar;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> StatusBox;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UWidget> CollapsedVisual;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UWidget> BreakthroughReadyVisual;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContentBox;
};
