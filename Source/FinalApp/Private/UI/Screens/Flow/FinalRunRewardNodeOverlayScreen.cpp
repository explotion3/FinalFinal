#include "UI/Screens/Flow/FinalRunRewardNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
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

void UFinalRunRewardNodeOverlayScreen::HandleResolveRewardClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNodeMissingRunFlow", "当前无法访问 RunFlowSubsystem，无法确认奖励节点。"));
		RebuildVisual();
		return;
	}

	const bool bResolved = RunFlowSubsystem->ResolveRewardNode();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bResolved
			? NSLOCTEXT("FinalFlowUI", "RewardNodeResolveSucceeded", "已转发 ResolveReward。")
			: NSLOCTEXT("FinalFlowUI", "RewardNodeResolveFailed", "ResolveReward 执行失败。")));
	RebuildVisual();
}

void UFinalRunRewardNodeOverlayScreen::HandleCloseClicked()
{
	RequestCloseOverlay();
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

	if (RewardEntriesText == nullptr)
	{
		RewardEntriesText = CreateStageLabel(TEXT("RewardNodeOverlayRewardEntries"), 13);
		ContentBox->InsertChildAt(3, RewardEntriesText);
	}

	if (ResolveRewardButton == nullptr)
	{
		ResolveRewardButton = CreateStageButton(
			TEXT("RewardNodeOverlayResolveButton"),
			TEXT("RewardNodeOverlayResolveButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeResolveButton", "确认奖励节点"),
			ResolveRewardButtonText);
		ResolveRewardButton->OnClicked.AddDynamic(this, &UFinalRunRewardNodeOverlayScreen::HandleResolveRewardClicked);
		ContentBox->AddChildToVerticalBox(ResolveRewardButton);
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
	const FFinalRunPendingRewardNodeViewData& PendingRewardNode = Snapshot.PendingRewardNode;
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(FormatOptionalText(
			PendingRewardNode.Title,
			NSLOCTEXT("FinalFlowUI", "RewardNodeOverlayTitleText", "奖励节点页")));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardNodeOverlaySummaryText", "流程阶段: {0}\n节点标题: {1}\n节点摘要: {2}\n节点内容存在: {3}\n可解析: {4}\n已解析: {5}\n奖励条目数: {6}\n当前金币: {7} | 遗物数: {8} | 牌库数: {9}"),
			FormatFlowStageText(Progression.FlowStage),
			FormatOptionalText(PendingRewardNode.Title, NSLOCTEXT("FinalFlowUI", "RewardNodeNoTitle", "未公开标题")),
			FormatOptionalText(PendingRewardNode.Summary, NSLOCTEXT("FinalFlowUI", "RewardNodeNoSummary", "当前没有额外摘要说明。")),
			FormatBool(PendingRewardNode.bHasPendingContent),
			FormatBool(PendingRewardNode.bCanResolve),
			FormatBool(PendingRewardNode.bResolved),
			FText::AsNumber(PendingRewardNode.RewardEntryViews.Num() > 0 ? PendingRewardNode.RewardEntryViews.Num() : PendingRewardNode.RewardEntries.Num()),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (RewardEntriesText)
	{
		RewardEntriesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardNodeOverlayRewardEntriesText", "奖励节点条目:\n{0}"),
			FText::FromString(BuildRewardPresentationSummaryString(PendingRewardNode.RewardEntryViews, PendingRewardNode.RewardEntries))));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardNodeOverlayGapText",
			"当前页已优先消费 PendingRewardNode.RewardEntryViews 的 PresentationKind / VisualTier / DetailText / IconId，并在缺失时回退到 raw RewardEntries。剩余缺口主要是真实图标资源、二次确认和多步奖励交互等 richer 呈现。"));
	}

	if (FeedbackText)
	{
		RefreshFeedbackText(NSLOCTEXT("FinalFlowUI", "RewardNodeOverlayFeedbackDefault", "当前页面会把 ResolveReward 意图转发给 RunFlowSubsystem，由它统一刷新或切页。"));
	}

	if (ResolveRewardButton)
	{
		ResolveRewardButton->SetIsEnabled(PendingRewardNode.bHasPendingContent && PendingRewardNode.bCanResolve && !PendingRewardNode.bResolved);
	}

	if (ResolveRewardButtonText)
	{
		if (!PendingRewardNode.bHasPendingContent)
		{
			ResolveRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardNodeResolveButtonMissing", "当前没有待处理奖励节点内容"));
		}
		else if (PendingRewardNode.bResolved)
		{
			ResolveRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardNodeResolveButtonResolved", "当前奖励节点已解析"));
		}
		else if (!PendingRewardNode.bCanResolve)
		{
			ResolveRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardNodeResolveButtonBlocked", "当前奖励节点暂不可确认"));
		}
		else
		{
			ResolveRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardNodeResolveButton", "确认奖励节点"));
		}
	}
}
