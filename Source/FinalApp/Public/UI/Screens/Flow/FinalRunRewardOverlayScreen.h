#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunSnapshot.h"
#include "UI/Screens/FinalOverlayScreenBase.h"
#include "FinalRunRewardOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunRewardOverlayScreen : public UFinalOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot);

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
	FFinalRunSnapshot CachedSnapshot;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardEntriesText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GapText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FeedbackText;

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

	UPROPERTY(Transient)
	FText LastActionFeedback;
};
