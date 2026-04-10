#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunRewardNodeOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunRewardNodeOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

private:
	UFUNCTION()
	void HandleResolveRewardClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RewardEntriesText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ResolveRewardButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResolveRewardButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;
};
