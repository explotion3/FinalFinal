#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunFlowOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunFlowOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

private:
	UFUNCTION()
	void HandleRewardOption0Clicked();

	UFUNCTION()
	void HandleRewardOption1Clicked();

	UFUNCTION()
	void HandleRewardOption2Clicked();

	UFUNCTION()
	void HandlePreviousChoiceClicked();

	UFUNCTION()
	void HandleNextChoiceClicked();

	UFUNCTION()
	void HandlePrimaryActionClicked();

	UFUNCTION()
	void HandleSecondaryActionClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();
	void ClampSelectionIndices();
	void HandleRewardOptionClicked(int32 RewardIndex);
	bool RefreshAfterFlowAction(bool bAccepted, const FText& SuccessText, const FText& FailureText);

	FText BuildStageDetailText() const;
	FText BuildSelectionText() const;
	FText BuildPrimaryActionText() const;
	FText BuildSecondaryActionText() const;
	bool CanUsePreviousNext() const;
	bool CanUsePrimaryAction() const;
	bool CanUseSecondaryAction() const;

	const FFinalRunNodeOptionViewData* GetSelectedNextNode() const;
	const FFinalRunEventOptionViewData* GetSelectedEventOption() const;
	const FFinalRunShopOfferViewData* GetSelectedShopOffer() const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StageDetailText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectionText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardOption0Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardOption0ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardOption1Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardOption1ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardOption2Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardOption2ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PreviousChoiceButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PreviousChoiceButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> NextChoiceButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NextChoiceButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PrimaryActionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PrimaryActionButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SecondaryActionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SecondaryActionButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CloseButtonText;

	UPROPERTY(Transient)
	int32 SelectedNextNodeIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 SelectedEventOptionIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 SelectedShopOfferIndex = INDEX_NONE;
};
