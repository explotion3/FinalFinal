#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunSnapshot.h"
#include "UI/Screens/FinalOverlayScreenBase.h"
#include "FinalRunNodeOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunNodeOverlayScreen : public UFinalOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot);

private:
	UFUNCTION()
	void HandleSelectPreviousNodeClicked();

	UFUNCTION()
	void HandleSelectNextNodeClicked();

	UFUNCTION()
	void HandleAdvanceSelectedNodeClicked();

	UFUNCTION()
	void HandleOpenRewardPageClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleOpenModalClicked();

	void ClampSelectedNodeIndex();
	void EnsureWidgetTree();
	void RebuildVisual();

	UPROPERTY(Transient)
	FFinalRunSnapshot CachedSnapshot;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvailableNodesText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GapText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FeedbackText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PreviousNodeButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PreviousNodeButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NextNodeButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NextNodeButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> AdvanceNodeButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AdvanceNodeButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenRewardPageButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenRewardPageButtonText;

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

	UPROPERTY(Transient)
	int32 SelectedNodeIndex = INDEX_NONE;
};
