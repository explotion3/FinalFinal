#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalRunEventNodeOverlayScreen.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UVerticalBox;
class UWidget;

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalRunEventOptionEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FName OptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	int32 OptionIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FText PreviewRewardText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FText CostText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FText StateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FText DisabledReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	EFinalRunRewardPresentationKind PresentationKind = EFinalRunRewardPresentationKind::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	EFinalRunRewardVisualTier VisualTier = EFinalRunRewardVisualTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunEvent")
	bool bEnabled = false;
};

class UFinalRunEventOptionEntryWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FFinalRunEventOptionClickedNative, UFinalRunEventOptionEntryWidget*);

UCLASS()
class FINALAPP_API UFinalRunEventOptionEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void ApplyOptionView(const FFinalRunEventOptionEntryViewData& InViewData);
	const FFinalRunEventOptionEntryViewData& GetOptionViewData() const { return CachedViewData; }

	FFinalRunEventOptionClickedNative OnOptionClicked;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|RunEvent")
	void OnOptionViewApplied(const FFinalRunEventOptionEntryViewData& ViewData);

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
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PreviewRewardText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CostText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DisabledReasonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TierVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SelectedVisual;

	FFinalRunEventOptionEntryViewData CachedViewData;
};

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

	void HandleResolveOptionById(FName OptionId);
	void HandleOptionClicked(UFinalRunEventOptionEntryWidget* OptionEntry);
	void EnsureWidgetTree();
	void RebuildVisual();
	void RebuildOptionList();
	FFinalRunEventOptionEntryViewData BuildOptionEntryData(int32 OptionIndex) const;
	void NormalizeSelectedOptionIndex();
	void StepSelectedOption(int32 Direction);

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> OptionListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RewardPreviewBox;

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
