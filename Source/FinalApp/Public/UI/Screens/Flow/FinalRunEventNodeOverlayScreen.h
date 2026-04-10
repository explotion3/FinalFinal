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
	void HandleOpenNodeSelectClicked();

	UFUNCTION()
	void HandleOpenModalClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MissingFieldsText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenNodeSelectButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenNodeSelectButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenModalButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenModalButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;
};
