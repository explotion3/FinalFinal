#include "UI/Screens/Flow/FinalRunFlowOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
FText FormatRewardOptionText(const FFinalRunPendingBattleRewardViewData& PendingReward, const int32 RewardIndex)
{
	if (PendingReward.RewardEntryViews.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntryViewData& RewardView = PendingReward.RewardEntryViews[RewardIndex];
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionView", "选择卡牌 {0}: {1}"),
			FText::AsNumber(RewardIndex + 1),
			FormatRewardEntryViewPrimaryText(RewardView));
	}

	if (PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntry& RewardEntry = PendingReward.RewardEntries[RewardIndex];
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionRaw", "选择卡牌 {0}: {1}"),
			FText::AsNumber(RewardIndex + 1),
			FormatRewardEntryName(RewardEntry));
	}

	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionMissing", "卡牌候选 {0}: 无"),
		FText::AsNumber(RewardIndex + 1));
}

FText BuildNextNodeSelectionText(const FFinalRunNodeOptionViewData& Node)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeSelection", "{0} [{1}]\n章节/楼层: {2}/{3}\n状态: {4}"),
		FormatOptionalText(Node.DisplayName, FormatOptionalName(Node.NodeId, NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeUnnamed", "未命名节点"))),
		FormatNodeTypeText(Node.NodeType),
		FText::AsNumber(Node.ChapterIndex),
		FText::AsNumber(Node.FloorIndex),
		FormatOptionalText(Node.AvailabilityMessage, Node.bLocked
			? NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeLocked", "不可前往")
			: NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeAvailable", "可前往")));
}

FText BuildEventOptionSelectionText(const FFinalRunEventOptionViewData& Option)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionSelection", "{0}\n结果: {1}\n状态: {2}\n奖励:\n{3}"),
		FormatOptionalText(Option.DisplayText, FormatOptionalName(Option.OptionId, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionUnnamed", "未命名选项"))),
		FormatOptionalText(Option.OutcomeSummary, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionNoOutcome", "无额外结果说明。")),
		FormatOptionalText(Option.AvailabilityMessage, Option.bSelectable
			? NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionSelectable", "可选择")
			: NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionBlocked", "不可选择")),
		FText::FromString(BuildRewardPresentationSummaryString(Option.RewardEntryViews, Option.RewardEntries)));
}

FText BuildShopOfferSelectionText(const FFinalRunShopOfferViewData& Offer)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferSelection", "{0}\n价格: {1}\n说明: {2}\n状态: {3}\n奖励:\n{4}"),
		FormatOptionalText(Offer.DisplayName, FormatOptionalName(Offer.OfferId, NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferUnnamed", "未命名商品"))),
		FText::AsNumber(Offer.Price),
		FormatOptionalText(Offer.Description, NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferNoDescription", "无额外说明。")),
		FormatOptionalText(Offer.AvailabilityMessage, Offer.bPurchasable
			? NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferPurchasable", "可购买")
			: NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferBlocked", "不可购买")),
		FText::FromString(BuildRewardPresentationSummaryString(Offer.RewardEntryViews, Offer.RewardEntries)));
}

FText BuildCompactCurrentNodeText(const FFinalRunProgressionViewData& Progression)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowCompactCurrentNode", "当前节点: {0} [{1}]\n章节/楼层: {2}/{3}"),
		FormatOptionalText(
			Progression.CurrentNodeDisplayName,
			FormatOptionalName(Progression.CurrentNodeId, NSLOCTEXT("FinalFlowUI", "RunFlowCurrentNodeUnnamed", "未命名节点"))),
		FormatNodeTypeText(Progression.CurrentNodeType),
		FText::AsNumber(Progression.CurrentChapter),
		FText::AsNumber(Progression.CurrentFloor));
}
}

void UFinalRunFlowOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	ClampSelectionIndices();
	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::HandleRewardOption0Clicked()
{
	HandleRewardOptionClicked(0);
}

void UFinalRunFlowOverlayScreen::HandleRewardOption1Clicked()
{
	HandleRewardOptionClicked(1);
}

void UFinalRunFlowOverlayScreen::HandleRewardOption2Clicked()
{
	HandleRewardOptionClicked(2);
}

void UFinalRunFlowOverlayScreen::HandlePreviousChoiceClicked()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (Snapshot.Progression.AvailableNextNodes.Num() > 0)
		{
			SelectedNextNodeIndex = SelectedNextNodeIndex == INDEX_NONE
				? 0
				: (SelectedNextNodeIndex - 1 + Snapshot.Progression.AvailableNextNodes.Num()) % Snapshot.Progression.AvailableNextNodes.Num();
		}
		break;

	case EFinalRunFlowStage::PendingEventNode:
		if (Snapshot.PendingEventNode.Options.Num() > 0)
		{
			SelectedEventOptionIndex = SelectedEventOptionIndex == INDEX_NONE
				? 0
				: (SelectedEventOptionIndex - 1 + Snapshot.PendingEventNode.Options.Num()) % Snapshot.PendingEventNode.Options.Num();
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		if (Snapshot.PendingShopNode.Offers.Num() > 0)
		{
			SelectedShopOfferIndex = SelectedShopOfferIndex == INDEX_NONE
				? 0
				: (SelectedShopOfferIndex - 1 + Snapshot.PendingShopNode.Offers.Num()) % Snapshot.PendingShopNode.Offers.Num();
		}
		break;

	default:
		break;
	}

	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::HandleNextChoiceClicked()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (Snapshot.Progression.AvailableNextNodes.Num() > 0)
		{
			SelectedNextNodeIndex = SelectedNextNodeIndex == INDEX_NONE
				? 0
				: (SelectedNextNodeIndex + 1) % Snapshot.Progression.AvailableNextNodes.Num();
		}
		break;

	case EFinalRunFlowStage::PendingEventNode:
		if (Snapshot.PendingEventNode.Options.Num() > 0)
		{
			SelectedEventOptionIndex = SelectedEventOptionIndex == INDEX_NONE
				? 0
				: (SelectedEventOptionIndex + 1) % Snapshot.PendingEventNode.Options.Num();
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		if (Snapshot.PendingShopNode.Offers.Num() > 0)
		{
			SelectedShopOfferIndex = SelectedShopOfferIndex == INDEX_NONE
				? 0
				: (SelectedShopOfferIndex + 1) % Snapshot.PendingShopNode.Offers.Num();
		}
		break;

	default:
		break;
	}

	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::HandlePrimaryActionClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowMissingSubsystem", "当前无法访问 RunFlowSubsystem。"));
		RebuildVisual();
		return;
	}

	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	bool bAccepted = false;
	FText SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionSucceeded", "操作已提交。");
	FText FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionFailed", "操作提交失败。");

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			bAccepted = RunFlowSubsystem->AdvanceToNode(SelectedNode->NodeId);
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceSucceeded", "已推进到选中节点。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceFailed", "推进节点失败。");
		}
		break;

	case EFinalRunFlowStage::PendingRewardNode:
		bAccepted = RunFlowSubsystem->ResolveRewardNode();
		SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveRewardSucceeded", "已确认奖励节点。");
		FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveRewardFailed", "确认奖励节点失败。");
		break;

	case EFinalRunFlowStage::PendingEventNode:
		if (const FFinalRunEventOptionViewData* SelectedOption = GetSelectedEventOption())
		{
			bAccepted = RunFlowSubsystem->ResolveEventOption(SelectedOption->OptionId);
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveEventSucceeded", "已提交事件选项。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveEventFailed", "提交事件选项失败。");
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		if (const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedShopOffer())
		{
			bAccepted = RunFlowSubsystem->ResolveShopOffer(SelectedOffer->OfferId);
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveShopSucceeded", "已提交商店商品。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveShopFailed", "提交商店商品失败。");
		}
		break;

	default:
		break;
	}

	RefreshAfterFlowAction(bAccepted, SuccessText, FailureText);
}

void UFinalRunFlowOverlayScreen::HandleSecondaryActionClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowMissingSubsystemSecondary", "当前无法访问 RunFlowSubsystem。"));
		RebuildVisual();
		return;
	}

	if (GetCachedSnapshot().Progression.FlowStage != EFinalRunFlowStage::PendingBattleReward)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowSecondaryUnavailable", "当前阶段没有可用的次要操作。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->SkipPendingBattleReward();
	RefreshAfterFlowAction(
		bAccepted,
		NSLOCTEXT("FinalFlowUI", "RunFlowSkipRewardSucceeded", "已跳过战后卡牌奖励。"),
		NSLOCTEXT("FinalFlowUI", "RunFlowSkipRewardFailed", "跳过战后卡牌奖励失败。"));
}

void UFinalRunFlowOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunFlowOverlayScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (WidgetTree->RootWidget == nullptr)
	{
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RunFlowOverlayRoot"));
		RootOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = RootOverlay;

		USizeBox* PanelSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RunFlowOverlayPanelSizeBox"));
		PanelSizeBox->SetWidthOverride(520.0f);

		UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(PanelSizeBox);
		if (PanelSlot != nullptr)
		{
			PanelSlot->SetHorizontalAlignment(HAlign_Right);
			PanelSlot->SetVerticalAlignment(VAlign_Fill);
			PanelSlot->SetPadding(FMargin(0.0f, 72.0f, 24.0f, 72.0f));
		}

		UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RunFlowOverlayPanel"));
		PanelBorder->SetBrushColor(FLinearColor(0.035f, 0.04f, 0.035f, 0.92f));
		PanelBorder->SetPadding(FMargin(18.0f));
		PanelSizeBox->SetContent(PanelBorder);

		UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("RunFlowOverlayScrollBox"));
		PanelBorder->SetContent(ScrollBox);

		ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RunFlowOverlayContent"));
		ScrollBox->AddChild(ContentBox);

		TitleText = CreateStageLabel(TEXT("RunFlowOverlayTitle"), 22);
		ContentBox->AddChildToVerticalBox(TitleText);

		SummaryText = CreateStageLabel(TEXT("RunFlowOverlaySummary"), 14);
		ContentBox->AddChildToVerticalBox(SummaryText);

		CurrentNodeText = CreateStageLabel(TEXT("RunFlowCurrentNode"), 13);
		ContentBox->AddChildToVerticalBox(CurrentNodeText);

		StageDetailText = CreateStageLabel(TEXT("RunFlowStageDetail"), 13);
		ContentBox->AddChildToVerticalBox(StageDetailText);

		SelectionText = CreateStageLabel(TEXT("RunFlowSelection"), 13);
		ContentBox->AddChildToVerticalBox(SelectionText);

		FeedbackText = CreateStageLabel(TEXT("RunFlowOverlayFeedback"), 12);
		ContentBox->AddChildToVerticalBox(FeedbackText);
	}

	if (ContentBox == nullptr)
	{
		return;
	}

	if (RewardOption0Button == nullptr)
	{
		RewardOption0Button = CreateStageButton(
			TEXT("RunFlowRewardOption0Button"),
			TEXT("RunFlowRewardOption0ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOption0", "选择卡牌 1"),
			RewardOption0ButtonText);
		ContentBox->AddChildToVerticalBox(RewardOption0Button);
	}

	if (RewardOption1Button == nullptr)
	{
		RewardOption1Button = CreateStageButton(
			TEXT("RunFlowRewardOption1Button"),
			TEXT("RunFlowRewardOption1ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOption1", "选择卡牌 2"),
			RewardOption1ButtonText);
		ContentBox->AddChildToVerticalBox(RewardOption1Button);
	}

	if (RewardOption2Button == nullptr)
	{
		RewardOption2Button = CreateStageButton(
			TEXT("RunFlowRewardOption2Button"),
			TEXT("RunFlowRewardOption2ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOption2", "选择卡牌 3"),
			RewardOption2ButtonText);
		ContentBox->AddChildToVerticalBox(RewardOption2Button);
	}

	if (PreviousChoiceButton == nullptr)
	{
		PreviousChoiceButton = CreateStageButton(
			TEXT("RunFlowPreviousChoiceButton"),
			TEXT("RunFlowPreviousChoiceButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowPreviousChoice", "上一个"),
			PreviousChoiceButtonText);
		ContentBox->AddChildToVerticalBox(PreviousChoiceButton);
	}

	if (NextChoiceButton == nullptr)
	{
		NextChoiceButton = CreateStageButton(
			TEXT("RunFlowNextChoiceButton"),
			TEXT("RunFlowNextChoiceButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowNextChoice", "下一个"),
			NextChoiceButtonText);
		ContentBox->AddChildToVerticalBox(NextChoiceButton);
	}

	if (PrimaryActionButton == nullptr)
	{
		PrimaryActionButton = CreateStageButton(
			TEXT("RunFlowPrimaryActionButton"),
			TEXT("RunFlowPrimaryActionButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowPrimaryAction", "执行当前操作"),
			PrimaryActionButtonText);
		ContentBox->AddChildToVerticalBox(PrimaryActionButton);
	}

	if (SecondaryActionButton == nullptr)
	{
		SecondaryActionButton = CreateStageButton(
			TEXT("RunFlowSecondaryActionButton"),
			TEXT("RunFlowSecondaryActionButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowSecondaryAction", "跳过"),
			SecondaryActionButtonText);
		ContentBox->AddChildToVerticalBox(SecondaryActionButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("RunFlowCloseButton"),
			TEXT("RunFlowCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowCloseAction", "关闭"),
			CloseButtonText);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}

	if (RewardOption0Button)
	{
		RewardOption0Button->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleRewardOption0Clicked);
	}
	if (RewardOption1Button)
	{
		RewardOption1Button->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleRewardOption1Clicked);
	}
	if (RewardOption2Button)
	{
		RewardOption2Button->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleRewardOption2Clicked);
	}
	if (PreviousChoiceButton)
	{
		PreviousChoiceButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandlePreviousChoiceClicked);
	}
	if (NextChoiceButton)
	{
		NextChoiceButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleNextChoiceClicked);
	}
	if (PrimaryActionButton)
	{
		PrimaryActionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandlePrimaryActionClicked);
	}
	if (SecondaryActionButton)
	{
		SecondaryActionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleSecondaryActionClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleCloseClicked);
	}
}

void UFinalRunFlowOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;
	const bool bPendingBattleReward = Snapshot.PendingBattleReward.bHasPendingReward
		|| Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowOverlayTitle", "Run 主流程"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowOverlaySummary", "阶段: {0}\n金币: {1} | 牌库: {2} | 遗物: {3}"),
			FormatFlowStageText(Progression.FlowStage),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.DeckCount),
			FText::AsNumber(Snapshot.RelicCount)));
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCompactCurrentNodeText(Progression));
	}

	if (StageDetailText)
	{
		StageDetailText->SetText(BuildStageDetailText());
	}

	if (SelectionText)
	{
		SelectionText->SetText(BuildSelectionText());
	}

	if (GapText)
	{
		GapText->SetText(FText::GetEmpty());
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "RunFlowOverlayFeedbackDefault", "等待当前流程操作。")));
	}

	const int32 RewardCount = Snapshot.PendingBattleReward.RewardEntries.Num();
	if (RewardOption0Button)
	{
		RewardOption0Button->SetVisibility(bPendingBattleReward ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		RewardOption0Button->SetIsEnabled(bPendingBattleReward && Snapshot.PendingBattleReward.bCanClaim && RewardCount > 0);
	}
	if (RewardOption0ButtonText)
	{
		RewardOption0ButtonText->SetText(FormatRewardOptionText(Snapshot.PendingBattleReward, 0));
	}

	if (RewardOption1Button)
	{
		RewardOption1Button->SetVisibility(bPendingBattleReward ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		RewardOption1Button->SetIsEnabled(bPendingBattleReward && Snapshot.PendingBattleReward.bCanClaim && RewardCount > 1);
	}
	if (RewardOption1ButtonText)
	{
		RewardOption1ButtonText->SetText(FormatRewardOptionText(Snapshot.PendingBattleReward, 1));
	}

	if (RewardOption2Button)
	{
		RewardOption2Button->SetVisibility(bPendingBattleReward ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		RewardOption2Button->SetIsEnabled(bPendingBattleReward && Snapshot.PendingBattleReward.bCanClaim && RewardCount > 2);
	}
	if (RewardOption2ButtonText)
	{
		RewardOption2ButtonText->SetText(FormatRewardOptionText(Snapshot.PendingBattleReward, 2));
	}

	if (PreviousChoiceButton)
	{
		PreviousChoiceButton->SetVisibility(CanUsePreviousNext() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		PreviousChoiceButton->SetIsEnabled(CanUsePreviousNext());
	}
	if (PreviousChoiceButtonText)
	{
		PreviousChoiceButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowPreviousChoiceLabel", "上一个"));
	}

	if (NextChoiceButton)
	{
		NextChoiceButton->SetVisibility(CanUsePreviousNext() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		NextChoiceButton->SetIsEnabled(CanUsePreviousNext());
	}
	if (NextChoiceButtonText)
	{
		NextChoiceButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowNextChoiceLabel", "下一个"));
	}

	if (PrimaryActionButton)
	{
		PrimaryActionButton->SetVisibility(CanUsePrimaryAction() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		PrimaryActionButton->SetIsEnabled(CanUsePrimaryAction());
	}
	if (PrimaryActionButtonText)
	{
		PrimaryActionButtonText->SetText(BuildPrimaryActionText());
	}

	if (SecondaryActionButton)
	{
		SecondaryActionButton->SetVisibility(CanUseSecondaryAction() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		SecondaryActionButton->SetIsEnabled(CanUseSecondaryAction());
	}
	if (SecondaryActionButtonText)
	{
		SecondaryActionButtonText->SetText(BuildSecondaryActionText());
	}

	if (CloseButton)
	{
		CloseButton->SetVisibility(ESlateVisibility::Visible);
		CloseButton->SetIsEnabled(true);
	}
	if (CloseButtonText)
	{
		CloseButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowCloseLabel", "关闭"));
	}
}

void UFinalRunFlowOverlayScreen::ClampSelectionIndices()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();

	if (Snapshot.Progression.AvailableNextNodes.Num() <= 0)
	{
		SelectedNextNodeIndex = INDEX_NONE;
	}
	else if (!Snapshot.Progression.AvailableNextNodes.IsValidIndex(SelectedNextNodeIndex))
	{
		SelectedNextNodeIndex = 0;
	}

	if (Snapshot.PendingEventNode.Options.Num() <= 0)
	{
		SelectedEventOptionIndex = INDEX_NONE;
	}
	else if (!Snapshot.PendingEventNode.Options.IsValidIndex(SelectedEventOptionIndex))
	{
		SelectedEventOptionIndex = 0;
	}

	if (Snapshot.PendingShopNode.Offers.Num() <= 0)
	{
		SelectedShopOfferIndex = INDEX_NONE;
	}
	else if (!Snapshot.PendingShopNode.Offers.IsValidIndex(SelectedShopOfferIndex))
	{
		SelectedShopOfferIndex = 0;
	}
}

void UFinalRunFlowOverlayScreen::HandleRewardOptionClicked(const int32 RewardIndex)
{
	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	if (!PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionMissingFeedback", "当前没有对应的战后卡牌候选。"));
		RebuildVisual();
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowRewardMissingSubsystem", "当前无法访问 RunFlowSubsystem，无法领取奖励。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->ClaimPendingBattleRewardById(PendingReward.RewardEntries[RewardIndex].RewardId);
	RefreshAfterFlowAction(
		bAccepted,
		NSLOCTEXT("FinalFlowUI", "RunFlowClaimRewardSucceeded", "已领取战后卡牌奖励。"),
		NSLOCTEXT("FinalFlowUI", "RunFlowClaimRewardFailed", "领取战后卡牌奖励失败。"));
}

bool UFinalRunFlowOverlayScreen::RefreshAfterFlowAction(const bool bAccepted, const FText& SuccessText, const FText& FailureText)
{
	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
		SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
			? RunFlowSubsystem->GetLastFlowMessage()
			: (bAccepted ? SuccessText : FailureText));
	}
	else
	{
		SetLastActionFeedback(bAccepted ? SuccessText : FailureText);
	}

	ClampSelectionIndices();
	RebuildVisual();
	return bAccepted;
}

FText UFinalRunFlowOverlayScreen::BuildStageDetailText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const EFinalRunFlowStage FlowStage = Snapshot.Progression.FlowStage;
	if (Snapshot.PendingBattleReward.bHasPendingReward || FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowBattleRewardDetail", "战斗结果: {0}\n自动入账金币: {1}\n卡牌候选:\n{2}"),
			FormatBattleOutcomeText(Snapshot.PendingBattleReward.SourceBattleOutcome),
			FText::AsNumber(Snapshot.PendingBattleReward.RewardGold),
			FText::FromString(BuildRewardPresentationSummaryString(Snapshot.PendingBattleReward.RewardEntryViews, Snapshot.PendingBattleReward.RewardEntries)));
	}

	switch (FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceDetail", "当前节点已处理完成。可选下一节点数量: {0}"),
			FText::AsNumber(Snapshot.Progression.AvailableNextNodes.Num()));

	case EFinalRunFlowStage::PendingRewardNode:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardNodeDetail", "{0}\n{1}\n奖励:\n{2}"),
			FormatOptionalText(Snapshot.PendingRewardNode.Title, NSLOCTEXT("FinalFlowUI", "RunFlowRewardNodeNoTitle", "奖励节点")),
			FormatOptionalText(Snapshot.PendingRewardNode.Summary, NSLOCTEXT("FinalFlowUI", "RunFlowRewardNodeNoSummary", "确认当前节点奖励。")),
			FText::FromString(BuildRewardPresentationSummaryString(Snapshot.PendingRewardNode.RewardEntryViews, Snapshot.PendingRewardNode.RewardEntries)));

	case EFinalRunFlowStage::PendingEventNode:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowEventNodeDetail", "{0}\n{1}\n选项数量: {2}"),
			FormatOptionalText(Snapshot.PendingEventNode.Title, NSLOCTEXT("FinalFlowUI", "RunFlowEventNodeNoTitle", "事件节点")),
			FormatOptionalText(Snapshot.PendingEventNode.Summary, NSLOCTEXT("FinalFlowUI", "RunFlowEventNodeNoSummary", "选择一个事件选项。")),
			FText::AsNumber(Snapshot.PendingEventNode.Options.Num()));

	case EFinalRunFlowStage::PendingShopNode:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowShopNodeDetail", "{0}\n{1}\n商品数量: {2}"),
			FormatOptionalText(Snapshot.PendingShopNode.Title, NSLOCTEXT("FinalFlowUI", "RunFlowShopNodeNoTitle", "商店节点")),
			FormatOptionalText(Snapshot.PendingShopNode.Summary, NSLOCTEXT("FinalFlowUI", "RunFlowShopNodeNoSummary", "选择一个商店商品。")),
			FText::AsNumber(Snapshot.PendingShopNode.Offers.Num()));

	case EFinalRunFlowStage::RunEnded:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowEndedDetail", "本局已结束。\n最终金币: {0}\n牌库数量: {1}\n遗物数量: {2}\n最近战斗结果: {3}"),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.DeckCount),
			FText::AsNumber(Snapshot.RelicCount),
			FormatBattleOutcomeText(Snapshot.LastBattleOutcome));

	case EFinalRunFlowStage::PreparingBattle:
	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoActionDetail", "当前阶段没有需要 RunFlowOverlay 处理的操作。");
	}
}

FText UFinalRunFlowOverlayScreen::BuildSelectionText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			return BuildNextNodeSelectionText(*SelectedNode);
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoNextNodeSelection", "当前没有可选下一节点。");

	case EFinalRunFlowStage::PendingEventNode:
		if (const FFinalRunEventOptionViewData* SelectedOption = GetSelectedEventOption())
		{
			return BuildEventOptionSelectionText(*SelectedOption);
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoEventSelection", "当前事件没有可选项。");

	case EFinalRunFlowStage::PendingShopNode:
		if (const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedShopOffer())
		{
			return BuildShopOfferSelectionText(*SelectedOffer);
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoShopSelection", "当前商店没有商品。");

	default:
		return FText::GetEmpty();
	}
}

FText UFinalRunFlowOverlayScreen::BuildPrimaryActionText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			return FText::Format(
				NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceAction", "继续到: {0}"),
				FormatOptionalText(SelectedNode->DisplayName, FormatOptionalName(SelectedNode->NodeId, NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceActionUnnamed", "未命名节点"))));
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceActionMissing", "没有可推进节点");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowResolveRewardAction", "确认奖励节点");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowResolveEventAction", "选择当前事件选项");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowResolveShopAction", "购买当前商品");

	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowPrimaryActionUnavailable", "当前无主操作");
	}
}

FText UFinalRunFlowOverlayScreen::BuildSecondaryActionText() const
{
	if (GetCachedSnapshot().Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward
		|| GetCachedSnapshot().PendingBattleReward.bHasPendingReward)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowSkipRewardAction", "跳过卡牌奖励");
	}

	return NSLOCTEXT("FinalFlowUI", "RunFlowSecondaryActionUnavailable", "当前无次要操作");
}

bool UFinalRunFlowOverlayScreen::CanUsePreviousNext() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return Snapshot.Progression.AvailableNextNodes.Num() > 1;

	case EFinalRunFlowStage::PendingEventNode:
		return Snapshot.PendingEventNode.Options.Num() > 1;

	case EFinalRunFlowStage::PendingShopNode:
		return Snapshot.PendingShopNode.Offers.Num() > 1;

	default:
		return false;
	}
}

bool UFinalRunFlowOverlayScreen::CanUsePrimaryAction() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			return Snapshot.Progression.bCanAdvanceToNextNode && !SelectedNode->bLocked;
		}
		return false;

	case EFinalRunFlowStage::PendingRewardNode:
		return Snapshot.PendingRewardNode.bHasPendingContent
			&& Snapshot.PendingRewardNode.bCanResolve
			&& !Snapshot.PendingRewardNode.bResolved;

	case EFinalRunFlowStage::PendingEventNode:
		if (const FFinalRunEventOptionViewData* SelectedOption = GetSelectedEventOption())
		{
			return Snapshot.PendingEventNode.bHasPendingContent
				&& Snapshot.PendingEventNode.bCanResolve
				&& !Snapshot.PendingEventNode.bResolved
				&& SelectedOption->bSelectable;
		}
		return false;

	case EFinalRunFlowStage::PendingShopNode:
		if (const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedShopOffer())
		{
			return Snapshot.PendingShopNode.bHasPendingContent
				&& Snapshot.PendingShopNode.bCanResolve
				&& !Snapshot.PendingShopNode.bResolved
				&& SelectedOffer->bPurchasable
				&& !SelectedOffer->bPurchased;
		}
		return false;

	default:
		return false;
	}
}

bool UFinalRunFlowOverlayScreen::CanUseSecondaryAction() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	return (Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward)
		&& Snapshot.PendingBattleReward.bCanClaim;
}

const FFinalRunNodeOptionViewData* UFinalRunFlowOverlayScreen::GetSelectedNextNode() const
{
	const TArray<FFinalRunNodeOptionViewData>& Nodes = GetCachedSnapshot().Progression.AvailableNextNodes;
	return Nodes.IsValidIndex(SelectedNextNodeIndex) ? &Nodes[SelectedNextNodeIndex] : nullptr;
}

const FFinalRunEventOptionViewData* UFinalRunFlowOverlayScreen::GetSelectedEventOption() const
{
	const TArray<FFinalRunEventOptionViewData>& Options = GetCachedSnapshot().PendingEventNode.Options;
	return Options.IsValidIndex(SelectedEventOptionIndex) ? &Options[SelectedEventOptionIndex] : nullptr;
}

const FFinalRunShopOfferViewData* UFinalRunFlowOverlayScreen::GetSelectedShopOffer() const
{
	const TArray<FFinalRunShopOfferViewData>& Offers = GetCachedSnapshot().PendingShopNode.Offers;
	return Offers.IsValidIndex(SelectedShopOfferIndex) ? &Offers[SelectedShopOfferIndex] : nullptr;
}
