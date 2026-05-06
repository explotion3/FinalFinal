#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalRunFlowOverlayScreen.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UWidget;

enum class EFinalRunFlowOptionKind : uint8
{
	Reward,
	NextNode,
	EventOption,
	ShopOffer,
	FlowAction,
	GrowthChoice
};

class UFinalRunFlowOptionButton;
class UFinalRunRouteNodeEntryWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FFinalRunFlowOptionClickedNative, UFinalRunFlowOptionButton*);

struct FINALAPP_API FFinalRunFlowOptionButtonData
{
	EFinalRunFlowOptionKind Kind = EFinalRunFlowOptionKind::Reward;
	EFinalRunCommandType CommandType = EFinalRunCommandType::AdvanceToNode;
	FName PayloadId = NAME_None;
	int32 PayloadIndex = INDEX_NONE;
	FText Title;
	FText Subtitle;
	FText Meta;
	FText State;
	bool bEnabled = false;
};

UCLASS()
class FINALAPP_API UFinalRunFlowOptionButton : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void ConfigureOption(EFinalRunFlowOptionKind InKind, FName InPayloadId, int32 InPayloadIndex, const FText& InLabel, bool bInEnabled);
	void ConfigureOption(const FFinalRunFlowOptionButtonData& InData);

	EFinalRunFlowOptionKind GetOptionKind() const { return OptionKind; }
	EFinalRunCommandType GetCommandType() const { return CachedData.CommandType; }
	FName GetPayloadId() const { return PayloadId; }
	int32 GetPayloadIndex() const { return PayloadIndex; }

	FFinalRunFlowOptionClickedNative OnOptionClicked;

private:
	UFUNCTION()
	void HandleClicked();

	void EnsureWidgetTree();

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OptionLabel;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SubtitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MetaText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	FFinalRunFlowOptionButtonData CachedData;

	EFinalRunFlowOptionKind OptionKind = EFinalRunFlowOptionKind::Reward;

	UPROPERTY(Transient)
	FName PayloadId = NAME_None;

	UPROPERTY(Transient)
	int32 PayloadIndex = INDEX_NONE;
};

UCLASS()
class FINALAPP_API UFinalRunRouteNodeEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void ApplyRouteNodeView(const FFinalRunRouteNodeViewData& InViewData);
	const FFinalRunRouteNodeViewData& GetRouteNodeViewData() const { return CachedViewData; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|RunFlow")
	void OnRouteNodeViewApplied(const FFinalRunRouteNodeViewData& ViewData);

private:
	void EnsureWidgetTree();
	void RefreshBoundWidgets();

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeLabelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeTypeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AvailabilityText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CurrentVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> VisitedVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> LockedVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ResolvedVisual;

	FFinalRunRouteNodeViewData CachedViewData;
};

UCLASS()
class FINALAPP_API UFinalRunFlowOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

private:
	UFUNCTION()
	void HandleRewardOption0Clicked();

	UFUNCTION()
	void HandleRewardOption1Clicked();

	UFUNCTION()
	void HandleRewardOption2Clicked();

	UFUNCTION()
	void HandlePreviousChoiceClicked();

	UFUNCTION()
	void HandleNextChoiceClicked();

	UFUNCTION()
	void HandlePrimaryActionClicked();

	UFUNCTION()
	void HandleSecondaryActionClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();
	void RebuildRouteNodeList();
	void RebuildOptionLists();
	void ClearOptionLists();
	void ClampSelectionIndices();
	void HandleRewardOptionClicked(int32 RewardIndex);
	void HandleListOptionClicked(UFinalRunFlowOptionButton* OptionButton);
	bool RefreshAfterFlowAction(bool bAccepted, const FText& SuccessText, const FText& FailureText);

	FText BuildStageDetailText() const;
	FText BuildSelectionText() const;
	FText BuildPrimaryActionText() const;
	FText BuildSecondaryActionText() const;
	bool CanUsePreviousNext() const;
	bool CanUsePrimaryAction() const;
	bool CanUseSecondaryAction() const;
	FText BuildRouteSummaryText() const;
	FText BuildCurrentStageText() const;

	const FFinalRunNodeOptionViewData* GetSelectedNextNode() const;
	const FFinalRunEventOptionViewData* GetSelectedEventOption() const;
	const FFinalRunShopOfferViewData* GetSelectedShopOffer() const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentNodeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentStageText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RouteSummaryText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StageDetailText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectionText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RewardOptionListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> NextNodeListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> EventOptionListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ShopOfferListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RouteNodeListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ActionListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardOption0Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardOption0ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardOption1Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardOption1ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardOption2Button;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardOption2ButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PreviousChoiceButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PreviousChoiceButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> NextChoiceButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NextChoiceButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PrimaryActionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PrimaryActionButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SecondaryActionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SecondaryActionButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CloseButtonText;

	UPROPERTY(Transient)
	int32 SelectedNextNodeIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 SelectedEventOptionIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 SelectedShopOfferIndex = INDEX_NONE;
};
