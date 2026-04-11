#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

void UFinalRunRewardOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystem", "当前无法访问 RunFlowSubsystem，无法领取待领奖励。"));
		RebuildVisual();
		return;
	}

	const bool bClaimed = RunFlowSubsystem->ClaimPendingBattleReward();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bClaimed
			? NSLOCTEXT("FinalFlowUI", "RewardClaimSucceeded", "已转发 ClaimPendingBattleReward。")
			: NSLOCTEXT("FinalFlowUI", "RewardClaimFailed", "ClaimPendingBattleReward 执行失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
	}
}

void UFinalRunRewardOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunRewardOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "RewardModalTitle", "奖励确认占位"),
			NSLOCTEXT("FinalFlowUI", "RewardModalBody", "当前 RootLayout 已支持在 Overlay 之上打开 Modal。后续奖励确认、放弃奖励、二次确认都应落在这一层。"));
	}
}

void UFinalRunRewardOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.06f, 0.08f, 0.11f, 0.96f), TEXT("RewardOverlayRoot"), TEXT("RewardOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (RewardEntriesText == nullptr)
	{
		RewardEntriesText = CreateStageLabel(TEXT("RewardOverlayEntries"), 13);
		ContentBox->InsertChildAt(2, RewardEntriesText);
	}

	if (ClaimRewardButton == nullptr)
	{
		ClaimRewardButton = CreateStageButton(
			TEXT("RewardOverlayClaimButton"),
			TEXT("RewardOverlayClaimButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取当前奖励条目"),
			ClaimRewardButtonText);
		ClaimRewardButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardClicked);
		ContentBox->AddChildToVerticalBox(ClaimRewardButton);
	}

	if (OpenNodePageButton == nullptr)
	{
		OpenNodePageButton = CreateStageButton(
			TEXT("RewardOverlayNodeButton"),
			TEXT("RewardOverlayNodeButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeButton", "查看节点选择页"),
			OpenNodePageButtonText);
		OpenNodePageButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked);
		ContentBox->AddChildToVerticalBox(OpenNodePageButton);
	}

	if (OpenModalButton == nullptr)
	{
		OpenModalButton = CreateStageButton(
			TEXT("RewardOverlayModalButton"),
			TEXT("RewardOverlayModalButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardModalButton", "打开奖励说明模态"),
			OpenModalButtonText);
		OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenModalClicked);
		ContentBox->AddChildToVerticalBox(OpenModalButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("RewardOverlayCloseButton"),
			TEXT("RewardOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardCloseButton", "关闭奖励页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunRewardOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingBattleRewardViewData& PendingReward = Snapshot.PendingBattleReward;
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RewardOverlayTitleText", "战后奖励页"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardOverlaySummaryText", "待领奖励: {0}\n来源节点: {1}\n节点标签: {2}\n节点类型: {3}\n来源节点Id: {4}\n来源遭遇: {5}\n战斗结果: {6}\n奖励条目数: {7}\n金币汇总: {8}\n奖励可领取: {9}\n流程允许领取: {10}\n当前金币: {11} | 遗物数: {12} | 牌库数: {13}"),
			PendingReward.bHasPendingReward
				? NSLOCTEXT("FinalFlowUI", "RewardHasPendingReward", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardHasNoPendingReward", "否"),
			FormatOptionalText(
				PendingReward.SourceNodeDisplayName,
				FormatOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNodeDisplayName", "未公开显示名"))),
			FormatOptionalName(PendingReward.SourceNodeDisplayLabel, NSLOCTEXT("FinalFlowUI", "RewardNoNodeLabel", "无")),
			FormatNodeTypeText(PendingReward.SourceNodeType),
			FormatOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNode", "无")),
			PendingReward.SourceEncounterId.IsValid()
				? FText::FromName(PendingReward.SourceEncounterId.Value)
				: NSLOCTEXT("FinalFlowUI", "RewardNoEncounter", "无"),
			FormatBattleOutcomeText(PendingReward.SourceBattleOutcome),
			FText::AsNumber(PendingReward.RewardEntryViews.Num() > 0 ? PendingReward.RewardEntryViews.Num() : PendingReward.RewardEntries.Num()),
			FText::AsNumber(PendingReward.RewardGold),
			FormatBool(PendingReward.bCanClaim),
			FormatBool(Progression.bCanClaimPendingBattleReward),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (RewardEntriesText)
	{
		RewardEntriesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardEntriesText", "奖励条目:\n{0}"),
			FText::FromString(BuildRewardPresentationSummaryString(PendingReward.RewardEntryViews, PendingReward.RewardEntries))));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardOverlayGapText",
			"当前页已优先消费 PendingBattleReward.RewardEntryViews，并在缺失时回退到 raw RewardEntries。剩余缺口主要是奖励图标/稀有度/描述等 richer 呈现，以及多奖励选择、替换、跳过这类更复杂流程。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "RewardOverlayFeedbackDefault", "当前页面会把领取奖励意图转发给 RunFlowSubsystem，由它统一决定刷新与切页。")));
	}

	if (ClaimRewardButton)
	{
		const bool bCanClaimReward = PendingReward.bHasPendingReward
			&& PendingReward.bCanClaim
			&& Progression.bCanClaimPendingBattleReward;
		ClaimRewardButton->SetIsEnabled(bCanClaimReward);
	}

	if (ClaimRewardButtonText)
	{
		if (!PendingReward.bHasPendingReward)
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButtonDisabled", "当前没有待领奖励"));
		}
		else if (!PendingReward.bCanClaim || !Progression.bCanClaimPendingBattleReward)
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButtonBlocked", "当前待领奖励暂不可领取"));
		}
		else
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取当前奖励条目"));
		}
	}

	if (OpenNodePageButton)
	{
		OpenNodePageButton->SetIsEnabled(Progression.bCanAdvanceToNextNode || Progression.CurrentNodeId != NAME_None);
	}
}
