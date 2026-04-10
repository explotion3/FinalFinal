#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "FinalRunShopNodeOverlayScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalRunShopNodeOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

private:
	UFUNCTION()
	void HandlePreviousOfferClicked();

	UFUNCTION()
	void HandleNextOfferClicked();

	UFUNCTION()
	void HandlePurchaseOfferClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();
	void NormalizeSelectedOfferIndex();
	void StepSelectedOffer(int32 Direction);

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OffersListText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectedOfferText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PreviousOfferButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PreviousOfferButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NextOfferButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NextOfferButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PurchaseOfferButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PurchaseOfferButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;

	int32 SelectedOfferIndex = INDEX_NONE;
};
