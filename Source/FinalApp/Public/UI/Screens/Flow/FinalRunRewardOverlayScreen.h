#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunRewardOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunRewardOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

private:
	UFUNCTION()
	void HandleClaimRewardClicked();

	UFUNCTION()
	void HandleClaimRewardOption0Clicked();

	UFUNCTION()
	void HandleClaimRewardOption1Clicked();

	UFUNCTION()
	void HandleClaimRewardOption2Clicked();

	UFUNCTION()
	void HandleSkipRewardClicked();

	UFUNCTION()
	void HandleOpenNodePageClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleOpenModalClicked();

	void HandleClaimRewardOptionClicked(int32 RewardIndex);
	void EnsureWidgetTree();
	void RebuildVisual();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardEntriesText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClaimRewardButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimRewardButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClaimRewardOption0Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimRewardOption0ButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClaimRewardOption1Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimRewardOption1ButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClaimRewardOption2Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimRewardOption2ButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SkipRewardButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SkipRewardButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenNodePageButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenNodePageButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenModalButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenModalButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;
};
