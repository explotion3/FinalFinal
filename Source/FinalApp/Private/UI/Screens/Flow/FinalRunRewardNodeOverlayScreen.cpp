#include "UI/Screens/Flow/FinalRunRewardNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

void UFinalRunRewardNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunRewardNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	RebuildVisual();
}

void UFinalRunRewardNodeOverlayScreen::HandleOpenNodeSelectClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
	}
}

void UFinalRunRewardNodeOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "RewardNodeModalTitle", "奖励节点页占位"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeModalBody", "当前已拆出独立的奖励节点页挂点。后续若 FinalRun 提供奖励节点专用查询和 RunCommand，这一页应承接奖励节点自身的选择与确认流程。"));
	}
}

void UFinalRunRewardNodeOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunRewardNodeOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.09f, 0.07f, 0.12f, 0.96f), TEXT("RewardNodeOverlayRoot"), TEXT("RewardNodeOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (CurrentNodeText == nullptr)
	{
		CurrentNodeText = CreateStageLabel(TEXT("RewardNodeOverlayCurrentNode"), 13);
		ContentBox->InsertChildAt(2, CurrentNodeText);
	}

	if (MissingFieldsText == nullptr)
	{
		MissingFieldsText = CreateStageLabel(TEXT("RewardNodeOverlayMissingFields"), 13);
		ContentBox->InsertChildAt(3, MissingFieldsText);
	}

	if (OpenNodeSelectButton == nullptr)
	{
		OpenNodeSelectButton = CreateStageButton(
			TEXT("RewardNodeOverlayNodeSelectButton"),
			TEXT("RewardNodeOverlayNodeSelectButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeOpenNodeSelectButton", "打开节点选择页"),
			OpenNodeSelectButtonText);
		OpenNodeSelectButton->OnClicked.AddDynamic(this, &UFinalRunRewardNodeOverlayScreen::HandleOpenNodeSelectClicked);
		ContentBox->AddChildToVerticalBox(OpenNodeSelectButton);
	}

	if (OpenModalButton == nullptr)
	{
		OpenModalButton = CreateStageButton(
			TEXT("RewardNodeOverlayModalButton"),
			TEXT("RewardNodeOverlayModalButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeOpenModalButton", "打开奖励节点说明模态"),
			OpenModalButtonText);
		OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunRewardNodeOverlayScreen::HandleOpenModalClicked);
		ContentBox->AddChildToVerticalBox(OpenModalButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("RewardNodeOverlayCloseButton"),
			TEXT("RewardNodeOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeCloseButton", "关闭奖励节点页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunRewardNodeOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunRewardNodeOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RewardNodeOverlayTitleText", "奖励节点页"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardNodeOverlaySummaryText", "流程阶段: {0}\n当前金币: {1} | 遗物数: {2} | 牌库数: {3}\n可推进下一节点: {4}"),
			FormatFlowStageText(Progression.FlowStage),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount),
			FormatBool(Progression.bCanAdvanceToNextNode)));
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (MissingFieldsText)
	{
		MissingFieldsText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardNodeOverlayMissingFieldsText",
			"当前奖励节点页已经从“节点选择页”拆出，但仍缺奖励节点专用查询：候选奖励条目、确认领取/放弃/替换命令、以及奖励节点专用展示元数据。"));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardNodeOverlayGapText",
			"这是一张结构化占位页：它已经按流程阶段单独承接 PendingRewardNode，但不会在 FinalApp 内伪造奖励节点规则真相。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "RewardNodeOverlayFeedbackDefault", "等待 Reward Node 专用查询与命令接入。")));
	}

	if (OpenNodeSelectButton)
	{
		OpenNodeSelectButton->SetIsEnabled(Progression.AvailableNextNodes.Num() > 0 || Progression.bCanAdvanceToNextNode);
	}
}
