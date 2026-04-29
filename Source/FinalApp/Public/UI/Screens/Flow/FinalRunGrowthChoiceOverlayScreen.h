#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunGrowthChoiceOverlayScreen.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UFinalRunFlowOptionButton;

UCLASS()
class FINALAPP_API UFinalRunGrowthChoiceOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool SelectChoiceByIndex(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool ConfirmCurrentChoice();

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	int32 GetSelectedChoiceIndex() const;

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FName GetSelectedChoiceInstanceId() const;

private:
	UFUNCTION()
	void HandlePrimaryActionClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();
	void RebuildChoiceList();
	void ClearChoiceList();
	void ClampSelectionIndex();
	void HandleChoiceOptionClicked(UFinalRunFlowOptionButton* OptionButton);

	const FFinalRunGrowthChoiceInstance* GetSelectedChoice() const;
	const FFinalRunCharacterViewData* GetTargetCharacter() const;
	FText BuildCharacterSummaryText() const;
	FText BuildSelectionSummaryText() const;
	FText BuildPrimaryActionText() const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CharacterSummaryText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectionSummaryText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> GrowthChoiceListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PrimaryActionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PrimaryActionButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CloseButtonText;

	UPROPERTY(Transient)
	int32 SelectedChoiceIndex = INDEX_NONE;
};
