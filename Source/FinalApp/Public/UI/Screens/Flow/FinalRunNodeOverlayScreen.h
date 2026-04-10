#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunNodeOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunNodeOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

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
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AvailableNodesText;

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
	int32 SelectedNodeIndex = INDEX_NONE;
};
