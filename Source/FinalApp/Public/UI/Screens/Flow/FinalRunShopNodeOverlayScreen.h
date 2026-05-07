#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalRunShopNodeOverlayScreen.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UVerticalBox;
class UWidget;

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalRunShopOfferEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FName OfferId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	int32 OfferIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FText PriceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FText PreviewRewardText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FText StateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FText DisabledReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	EFinalRunRewardPresentationKind PresentationKind = EFinalRunRewardPresentationKind::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	EFinalRunRewardVisualTier VisualTier = EFinalRunRewardVisualTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunShop")
	bool bPurchased = false;
};

class UFinalRunShopOfferEntryWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FFinalRunShopOfferClickedNative, UFinalRunShopOfferEntryWidget*);

UCLASS()
class FINALAPP_API UFinalRunShopOfferEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	void ApplyOfferView(const FFinalRunShopOfferEntryViewData& InViewData);
	const FFinalRunShopOfferEntryViewData& GetOfferViewData() const { return CachedViewData; }
	UWidget* GetFocusTarget() const;

	FFinalRunShopOfferClickedNative OnOfferClicked;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|RunShop")
	void OnOfferViewApplied(const FFinalRunShopOfferEntryViewData& ViewData);

private:
	UFUNCTION()
	void HandleClicked();

	void EnsureWidgetTree();
	void RefreshBoundWidgets();

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> OfferButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PriceText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PreviewRewardText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DisabledReasonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TierVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PurchasedVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SelectedVisual;

	FFinalRunShopOfferEntryViewData CachedViewData;
};

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
	void HandleLeaveShopClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void HandlePurchaseOfferById(FName OfferId);
	void HandleOfferClicked(UFinalRunShopOfferEntryWidget* OfferEntry);
	void EnsureWidgetTree();
	void RebuildVisual();
	void RebuildOfferList();
	FFinalRunShopOfferEntryViewData BuildOfferEntryData(int32 OfferIndex) const;
	void NormalizeSelectedOfferIndex();
	void StepSelectedOffer(int32 Direction);

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> OfferListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RewardPreviewBox;

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

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> LeaveShopButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LeaveShopButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;

	int32 SelectedOfferIndex = INDEX_NONE;
};
