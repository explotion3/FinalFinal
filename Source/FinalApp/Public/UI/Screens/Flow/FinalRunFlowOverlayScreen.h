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

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StageDetailText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectionText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RewardOption0Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardOption0ButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RewardOption1Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardOption1ButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RewardOption2Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardOption2ButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PreviousChoiceButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PreviousChoiceButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NextChoiceButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NextChoiceButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PrimaryActionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PrimaryActionButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SecondaryActionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SecondaryActionButtonText;

	UPROPERTY(Transient)
	int32 SelectedNextNodeIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 SelectedEventOptionIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 SelectedShopOfferIndex = INDEX_NONE;
};
