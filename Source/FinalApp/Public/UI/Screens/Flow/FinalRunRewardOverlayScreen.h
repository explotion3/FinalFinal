#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalRunRewardOverlayScreen.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UVerticalBox;
class UWidget;

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalRunRewardCandidateEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FName RewardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	int32 RewardIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FText Subtitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FText Detail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FText Meta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FText State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	EFinalRunRewardPresentationKind PresentationKind = EFinalRunRewardPresentationKind::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	EFinalRunRewardVisualTier VisualTier = EFinalRunRewardVisualTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunReward")
	bool bEnabled = false;
};

class UFinalRunRewardCandidateEntryWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FFinalRunRewardCandidateClickedNative, UFinalRunRewardCandidateEntryWidget*);

UCLASS()
class FINALAPP_API UFinalRunRewardCandidateEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void ApplyCandidateView(const FFinalRunRewardCandidateEntryViewData& InViewData);
	const FFinalRunRewardCandidateEntryViewData& GetCandidateViewData() const { return CachedViewData; }

	FFinalRunRewardCandidateClickedNative OnCandidateClicked;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|RunReward")
	void OnCandidateViewApplied(const FFinalRunRewardCandidateEntryViewData& ViewData);

private:
	UFUNCTION()
	void HandleClicked();

	void EnsureWidgetTree();
	void RefreshBoundWidgets();

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SubtitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MetaText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TierVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SelectedVisual;

	FFinalRunRewardCandidateEntryViewData CachedViewData;
};

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
	void HandleClaimRewardById(FName RewardId);
	void HandleCandidateClicked(UFinalRunRewardCandidateEntryWidget* CandidateEntry);
	void EnsureWidgetTree();
	void RebuildVisual();
	void RebuildCandidateList();
	FFinalRunRewardCandidateEntryViewData BuildCandidateEntryData(int32 RewardIndex) const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SourceText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> CandidateListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardEntriesText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ClaimRewardButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClaimRewardButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ClaimRewardOption0Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClaimRewardOption0ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ClaimRewardOption1Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClaimRewardOption1ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ClaimRewardOption2Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClaimRewardOption2ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SkipRewardButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkipRewardButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> OpenNodePageButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OpenNodePageButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> OpenModalButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OpenModalButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CloseButtonText;
};
