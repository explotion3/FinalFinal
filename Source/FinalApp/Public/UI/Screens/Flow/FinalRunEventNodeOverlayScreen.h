#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunEventNodeOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunEventNodeOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

private:
	UFUNCTION()
	void HandlePreviousOptionClicked();

	UFUNCTION()
	void HandleNextOptionClicked();

	UFUNCTION()
	void HandleResolveOptionClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();
	void NormalizeSelectedOptionIndex();
	void StepSelectedOption(int32 Direction);

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OptionsListText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectedOptionText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PreviousOptionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PreviousOptionButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NextOptionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NextOptionButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ResolveOptionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResolveOptionButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;

	int32 SelectedOptionIndex = INDEX_NONE;
};
