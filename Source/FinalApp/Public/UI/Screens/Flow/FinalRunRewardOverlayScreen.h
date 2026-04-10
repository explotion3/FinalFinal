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
	void HandleOpenNodePageClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleOpenModalClicked();

	void EnsureWidgetTree();
	void RebuildVisual();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardEntriesText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClaimRewardButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimRewardButtonText;

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
